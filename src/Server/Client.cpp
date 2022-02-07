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
using namespace ClientAuthentication;
using namespace ClientCommunication;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Client::Client(const QUIC_API_TABLE* p_APITable,
               HQUIC p_Connection) noexcept : p_APITable(p_APITable),
                                              p_Connection(p_Connection)
{}

Client::~Client() noexcept
{}

//*************************************************************************************
// Disconnect
//*************************************************************************************

void Client::Disconnected() noexcept
{
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
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
    // @NOTE: Same client might be in different threads due to
    //        multiple added recieved messages!
    //        We also can't simply return a finished result because
    //        the current working thread might already past some
    //        change which needs work!
    if (c_Mutex.try_lock() == false)
    {
        return false;
    }
    
    // Grab and process recieved messages
    if (l_Recieved.size() > 0)
    {
        Database& c_Database = dynamic_cast<Database&>(*(p_Shared.get()));
        bool b_Disconnect = false;
        
        for (auto& Recieved : l_Recieved)
        {
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
                    
                    if (c_Result.GetID() == NetMessage::MSG_AUTH_RESULT)
                    {
                        b_Disconnect = true;
                    }
                    
                    l_Send.emplace_back(c_Result);
                    break;
                }
                case NetMessage::MSG_AUTH_PROOF:
                {
                    NetMessage c_Result = HandleAuthProof(ToData<MSG_AUTH_PROOF_DATA>(Recieved.v_Data),
                                                          c_Database,
                                                          c_UserInfo);
                    
                    // Our proof result is an error?
                    if (c_Result.v_Data[NetMessage::us_DataPos] != NetMessage::ERR_NONE)
                    {
                        b_Disconnect = true;
                    }
                    
                    l_Send.emplace_back(c_Result);
                    break;
                }
                    
                // Communication
                case NetMessage::MSG_DATA_AVAIL:
                {
                    break;
                }
                case NetMessage::MSG_TEXT:
                case NetMessage::MSG_LOCATION:
                {
                    break;
                }
                case NetMessage::MSG_NOTIFICATION: { break; } // NYI
                case NetMessage::MSG_CUSTOM: { break; } // NYI
                    
                /**
                 *  Unk
                 */
                    
                default:
                {
                    b_Disconnect = true;
                    break;
                }
            }
            
            if (b_Disconnect == true)
            {
                Disconnect();
                break;
            }
        }
        
        l_Recieved.clear();
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
    
    c_Mutex.unlock();
    
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
    try
    {
        l_Recieved.emplace_back(c_Data.v_Bytes);
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
        // Find free stream data first
        StreamData* p_Data = NULL;
        
        for (auto It = l_StreamData.begin(); It != l_StreamData.end(); ++It)
        {
            // Select empty message available
            if (It->e_State == StreamData::FREE || It->e_State == StreamData::COMPLETED)
            {
                It->e_State = StreamData::IN_USE;
                p_Data = &(*(It));
                
                break;
            }
        }
        
        // No data, add new
        if (p_Data == NULL)
        {
            l_StreamData.emplace_back();
            p_Data = &(*(--(l_StreamData.end())));
        }
        
        // Add the send data
        p_Data->v_Bytes.swap(It->v_Data);
        
        // Now we perform the quic buffer setup
        p_Data->v_Bytes.insert(p_Data->v_Bytes.begin(),
                               sizeof(QUIC_BUFFER),
                               0);
        
        QUIC_BUFFER* p_QuicBuffer;
        
        p_QuicBuffer = (QUIC_BUFFER*)&(p_Data->v_Bytes[0]);
        p_QuicBuffer->Buffer = &(p_Data->v_Bytes[sizeof(QUIC_BUFFER)]);
        p_QuicBuffer->Length = p_Data->v_Bytes.size() - sizeof(QUIC_BUFFER);
        
        // Buffer is setup, send data
        HQUIC p_Stream;
        
        if (QUIC_FAILED(p_APITable->StreamOpen(p_Connection,
                                               QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL, /* QUIC_STREAM_OPEN_FLAG_NONE, */
                                               StreamCallback,
                                               NULL, /* No context for shutdown, etc */
                                               &p_Stream)))
        {
            p_Data->e_State = StreamData::FREE;
            
            throw Exception("Failed to open stream!");
        }
        else if (QUIC_FAILED(p_APITable->StreamStart(p_Stream,
                                                     QUIC_STREAM_START_FLAG_SHUTDOWN_ON_FAIL)))
        {
            p_APITable->StreamClose(p_Stream);
            p_Data->e_State = StreamData::FREE;
            
            throw Exception("Failed to start stream!");
        }
        else if (QUIC_FAILED(p_APITable->StreamSend(p_Stream,
                                                    p_QuicBuffer,
                                                    1,
                                                    QUIC_SEND_FLAG_FIN,
                                                    p_Data))) /* Send context, allows reset on send complete */
        {
            p_APITable->StreamClose(p_Stream);
            p_Data->e_State = StreamData::FREE;
            
            throw Exception("Failed to send on stream!");
        }
        
        // Now sending, remove net message
        It = l_Send.erase(It);
    }
}
