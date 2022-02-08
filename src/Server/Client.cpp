/**
 *  Client.cpp
 *
 *  This file is part of the MRH project.
 *  See the AUTHORS file for Copyright information.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// C / C++

// External

// Project
#include "./Client.h"
#include "./Client/ClientAuthentication.h"
#include "./Client/ClientCommunication.h"
#include "./MsQuic/MsQuic.h"
#include "../Logger.h"

// Pre-defined
#ifndef CLIENT_EXTENDED_LOGGING
    #define CLIENT_EXTENDED_LOGGING 0
#endif

using namespace ClientAuthentication;
using namespace ClientCommunication;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Client::Client(const QUIC_API_TABLE* p_APITable,
               HQUIC p_Connection) noexcept : p_APITable(p_APITable),
                                              p_Connection(p_Connection),
                                              us_RecievedCount(0)
{}

Client::~Client() noexcept
{}

Client::Recieved::Recieved(StreamData& c_Data) : c_Message(c_Data.v_Bytes)
{}

//*************************************************************************************
// Disconnect
//*************************************************************************************

void Client::Disconnected() noexcept
{
#if CLIENT_EXTENDED_LOGGING > 0
    Logger::Singleton().Log(Logger::INFO, "Client (User ID " +
                                          std::to_string(c_UserInfo.u32_UserID) +
                                          ", Device Key: " +
                                          c_UserInfo.s_DeviceKey +
                                          ", Client Type: " +
                                          std::to_string(c_UserInfo.u8_ClientType) +
                                          "): Disconnected.",
                            "Client.cpp", __LINE__);
#endif
    
    p_Connection = NULL;
}

void Client::Disconnect() noexcept
{
    if (p_Connection == NULL)
    {
        return;
    }
    
    /*
    // Are we still required to send something?
    for (auto It = l_StreamData.begin(); It != l_StreamData.end(); ++It)
    {
        if (It->e_State == StreamData::IN_USE)
        {
            return;
        }
    }
    */
    
    p_APITable->ConnectionShutdown(p_Connection,
                                   QUIC_CONNECTION_SHUTDOWN_FLAG_NONE,
                                   0);
}

//*************************************************************************************
// Perform
//*************************************************************************************

bool Client::Perform(std::shared_ptr<ThreadShared>& p_Shared) noexcept
{
    if (p_Connection == NULL)
    {
        // @NOTE: Return success, connection dead and
        //        nothing left to do.
        return true;
    }
    else if (c_PerformMutex.try_lock() == false)
    {
        // @NOTE: Same client might be in different threads due to
        //        multiple added recieved messages!
        //        We also can't simply return a finished result because
        //        the current working thread might already past some
        //        change which needs work!
        return false;
    }
    
    // Grab and process recieved messages
    for (size_t i = 0; i < us_RecievedCount; ++i)
    {
        // Can we work on this message?
        if (dq_Recieved[i].c_Mutex.try_lock() == false)
        {
            // Data is currently added by recieve()
            continue;
        }
        else if (dq_Recieved[i].c_Message.v_Data.size() == 0)
        {
            // This message is empty
            dq_Recieved[i].c_Mutex.unlock();
            continue;
        }
        
        // This message is usable, work on it
        auto& Recieved = dq_Recieved[i].c_Message;
        
        try
        {
            Database& c_Database = dynamic_cast<Database&>(*(p_Shared.get()));
            
            switch (Recieved.GetID())
            {
                /**
                 *  Net Message Version 1
                 */
                
                // Server Auth
                case NetMessage::MSG_AUTH_REQUEST:
                {
                    NetMessage c_Result = HandleAuthRequest(ToData<MSG_AUTH_REQUEST_DATA>(Recieved.v_Data),
                                                            c_Database,
                                                            c_UserInfo);
                    
                    // We should recieve MSG_AUTH_CHALLENGE on success
                    if (c_Result.GetID() == NetMessage::MSG_AUTH_RESULT)
                    {
                        Disconnect();
                    }
                    
                    l_Send.emplace_back(c_Result);
                    break;
                }
                case NetMessage::MSG_AUTH_PROOF:
                {
                    NetMessage c_Result = HandleAuthProof(ToData<MSG_AUTH_PROOF_DATA>(Recieved.v_Data),
                                                          c_Database,
                                                          c_UserInfo);
                    
                    // Our proof result is an error? (Pos 1, uint8_t)
                    if (c_Result.v_Data[NetMessage::us_DataPos] != NetMessage::ERR_NONE)
                    {
                        Disconnect();
                    }
                    
                    l_Send.emplace_back(c_Result);
                    break;
                }
                    
                // Communication
                case NetMessage::MSG_DATA_AVAIL:
                {
                    if (c_UserInfo.b_Authenticated == false)
                    {
                        Disconnect();
                        break;
                    }
                    
                    // @NOTE: Pos 1 of MSG_DATA_AVAIL is the message type requested (uint8_t)!
                    std::list<NetMessage> l_Message = ClientCommunication::RetrieveMessages(Recieved.v_Data[NetMessage::us_DataPos],
                                                                                            c_Database,
                                                                                            c_UserInfo);
                    
                    for (auto& Message : l_Message)
                    {
                        l_Send.emplace_back(Message);
                    }
                    break;
                }
                case NetMessage::MSG_TEXT:
                case NetMessage::MSG_LOCATION:
                {
                    if (c_UserInfo.b_Authenticated == false)
                    {
                        Disconnect();
                        break;
                    }
                    
                    ClientCommunication::StoreMessage(Recieved,
                                                      c_Database,
                                                      c_UserInfo);
                    break;
                }
                case NetMessage::MSG_NOTIFICATION: { break; } // NYI
                case NetMessage::MSG_CUSTOM: { break; } // NYI
                    
                /**
                 *  Unk
                 */
                    
                default:
                {
                    Disconnect();
                    break;
                }
            }
        }
        catch (std::exception& e)
        {
            Logger::Singleton().Log(Logger::ERROR, "Failed to process recieved net message: " +
                                                   std::string(e.what()),
                                    "Client.cpp", __LINE__);
        }
        
        // Recieved message processed, set free
        dq_Recieved[i].c_Message.v_Data.clear();
        dq_Recieved[i].c_Mutex.unlock();
    }
    
    // Processed recieved messages, now send
    bool b_Result = true;
    
    try
    {
        Send();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Failed to send net messages: " +
                                               std::string(e.what()),
                                "Client.cpp", __LINE__);
        b_Result = false;
    }
    
    // Unlock for other threads
    c_PerformMutex.unlock();
    
    // Return finished or retry
    // @NOTE: On disconnected we send true, can't work on a
    //        disconnected client!
    return (p_Connection == NULL ? true : b_Result);
}

//*************************************************************************************
// Recieve
//*************************************************************************************

void Client::Recieve(StreamData& c_Data) noexcept
{
#if CLIENT_EXTENDED_LOGGING > 0
        Logger::Singleton().Log(Logger::INFO, "Client (User ID " +
                                              std::to_string(c_UserInfo.u32_UserID) +
                                              ", Device Key: " +
                                              c_UserInfo.s_DeviceKey +
                                              ", Client Type: " +
                                              std::to_string(c_UserInfo.u8_ClientType) +
                                              "): Recieved NetMessage " +
                                              std::to_string(c_Data.v_Bytes[0]) +
                                              " (Size: " +
                                              std::to_string(c_Data.v_Bytes.size()) +
                                              ").",
                                "Client.cpp", __LINE__);
#endif
    
    // Try to replace a unused message
    for (size_t i = 0; i < us_RecievedCount; ++i)
    {
        // Can we work on this message?
        auto& Entry = dq_Recieved[i];
        
        if (Entry.c_Mutex.try_lock() == false)
        {
            // Message is currently processed by perform()
            continue;
        }
        else if (Entry.c_Message.v_Data.size() != 0)
        {
            // Message is not empty
            Entry.c_Mutex.unlock();
            continue;
        }
        
        // Message is replaceable
        // @NOTE: Swap is ok, bytes of stream data are recreated!
        Entry.c_Message.v_Data.swap(c_Data.v_Bytes);
        Entry.c_Mutex.unlock();
        
        return;
    }
    
    // No free space, add new
    try
    {
        c_RecievedMutex.lock();
        dq_Recieved.emplace_back(c_Data);
        c_RecievedMutex.unlock();
        
        us_RecievedCount += 1;
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Failed to add recieved net message: " +
                                               std::string(e.what()),
                                "Client.cpp", __LINE__);
    }
}

//*************************************************************************************
// Send
//*************************************************************************************

void Client::Send()
{
    for (auto It = l_Send.begin(); It != l_Send.end(); ++It)
    {
#if CLIENT_EXTENDED_LOGGING > 0
        Logger::Singleton().Log(Logger::INFO, "Client (User ID " +
                                              std::to_string(c_UserInfo.u32_UserID) +
                                              ", Device Key: " +
                                              c_UserInfo.s_DeviceKey +
                                              ", Client Type: " +
                                              std::to_string(c_UserInfo.u8_ClientType) +
                                              "): Sending NetMessage " +
                                              std::to_string(It->GetID()) +
                                              " (Size: " +
                                              std::to_string(It->v_Data.size()) +
                                              ").",
                                "Client.cpp", __LINE__);
#endif
        
        // Find free stream data first
        StreamSendContext* p_Context = NULL;
        
        for (auto Context = l_StreamContext.begin(); Context != l_StreamContext.end(); ++Context)
        {
            // Select empty message available
            if (Context->c_Data.e_State == StreamData::FREE || Context->c_Data.e_State == StreamData::COMPLETED)
            {
                Context->c_Data.e_State = StreamData::IN_USE;
                p_Context = &(*(Context));
                break;
            }
        }
        
        // No data, add new
        if (p_Context == NULL)
        {
            l_StreamContext.emplace_back(p_APITable);
            p_Context = &(*(--(l_StreamContext.end())));
        }
        
        // Add the send data
        p_Context->c_Data.v_Bytes.swap(It->v_Data);
        
        // Now we perform the quic buffer setup
        p_Context->c_Data.v_Bytes.insert(p_Context->c_Data.v_Bytes.begin(),
                                         sizeof(QUIC_BUFFER),
                                         0);
        
        QUIC_BUFFER* p_QuicBuffer;
        
        p_QuicBuffer = (QUIC_BUFFER*)&(p_Context->c_Data.v_Bytes[0]);
        p_QuicBuffer->Buffer = &(p_Context->c_Data.v_Bytes[sizeof(QUIC_BUFFER)]);
        p_QuicBuffer->Length = p_Context->c_Data.v_Bytes.size() - sizeof(QUIC_BUFFER);
        
        // Buffer is setup, send data
        HQUIC p_Stream;
        
        if (QUIC_FAILED(p_APITable->StreamOpen(p_Connection,
                                               QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL, /* QUIC_STREAM_OPEN_FLAG_NONE, */
                                               StreamSendCallback,
                                               p_Context,
                                               &p_Stream)))
        {
            p_Context->c_Data.e_State = StreamData::FREE;
            
            throw Exception("Failed to open stream!");
        }
        else if (QUIC_FAILED(p_APITable->StreamStart(p_Stream,
                                                     QUIC_STREAM_START_FLAG_SHUTDOWN_ON_FAIL)))
        {
            p_APITable->StreamClose(p_Stream);
            p_Context->c_Data.e_State = StreamData::FREE;
            
            throw Exception("Failed to start stream!");
        }
        else if (QUIC_FAILED(p_APITable->StreamSend(p_Stream,
                                                    p_QuicBuffer,
                                                    1,
                                                    QUIC_SEND_FLAG_FIN,
                                                    NULL)))
        {
            p_APITable->StreamClose(p_Stream);
            p_Context->c_Data.e_State = StreamData::FREE;
            
            throw Exception("Failed to send on stream!");
        }
        
        // Now sending, remove net message
        It = l_Send.erase(It);
    }
}
