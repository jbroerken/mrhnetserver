/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
               HQUIC p_Connection,
               size_t us_ClientID) noexcept : us_ClientID(us_ClientID),
                                              p_APITable(p_APITable),
                                              p_Connection(p_Connection)
{}

Client::~Client() noexcept
{
#if CLIENT_EXTENDED_LOGGING > 0
    Logger::Singleton().Log(Logger::INFO, "(Client ID: " +
                                          std::to_string(us_ClientID) +
                                          ", User ID " +
                                          std::to_string(c_UserInfo.u32_UserID) +
                                          ", Device Key: " +
                                          c_UserInfo.s_DeviceKey +
                                          ", Client Type: " +
                                          std::to_string(c_UserInfo.u8_ClientType) +
                                          "): Destroyed.",
                            "Client.cpp", __LINE__);
#endif
}

//*************************************************************************************
// Disconnect
//*************************************************************************************

void Client::Disconnected() noexcept
{
#if CLIENT_EXTENDED_LOGGING > 0
    Logger::Singleton().Log(Logger::INFO, "(Client ID: " +
                                          std::to_string(us_ClientID) +
                                          ", User ID " +
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
    std::shared_ptr<NetMessage> p_Recieved;
    
    while ((p_Recieved = c_Recieved.GetElement()) != NULL)
    {
        try
        {
            auto& Recieved = *p_Recieved;
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
                    
                    c_Send.Add(std::make_shared<NetMessage>(c_Result));
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
                    
                    c_Send.Add(std::make_shared<NetMessage>(c_Result));
                    break;
                }
                    
                // Communication
                case NetMessage::MSG_GET_DATA:
                {
                    if (c_UserInfo.b_Authenticated == false)
                    {
                        Disconnect();
                        break;
                    }
                    
                    c_Send.Add(std::make_shared<NetMessage>(ClientCommunication::RetrieveMessage(c_Database,
                                                                                                 c_UserInfo)));
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
            Logger::Singleton().Log(Logger::ERROR, "(Client ID: " +
                                                   std::to_string(us_ClientID) +
                                                   ", User ID " +
                                                   std::to_string(c_UserInfo.u32_UserID) +
                                                   ", Device Key: " +
                                                   c_UserInfo.s_DeviceKey +
                                                   ", Client Type: " +
                                                   std::to_string(c_UserInfo.u8_ClientType) +
                                                   "): Failed to process recieved net message: " +
                                                   e.what(),
                                    "Client.cpp", __LINE__);
        }
    }
    
    // Processed recieved messages, now send
    bool b_Result = true;
    
    try
    {
        Send();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "(Client ID: " +
                                               std::to_string(us_ClientID) +
                                               ", User ID " +
                                               std::to_string(c_UserInfo.u32_UserID) +
                                               ", Device Key: " +
                                               c_UserInfo.s_DeviceKey +
                                               ", Client Type: " +
                                               std::to_string(c_UserInfo.u8_ClientType) +
                                               " ): Failed to send net messages: " +
                                               e.what(),
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

void Client::RecieveNetMessage(StreamData& c_Data) noexcept
{
#if CLIENT_EXTENDED_LOGGING > 0
        Logger::Singleton().Log(Logger::INFO, "(Client ID: " +
                                              std::to_string(us_ClientID) +
                                              ", User ID " +
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
    
    // No free space, add new
    try
    {
        std::shared_ptr<NetMessage> p_Message = std::make_shared<NetMessage>(c_Data.v_Bytes);
        c_Recieved.Add(p_Message);
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "(Client ID: " +
                                               std::to_string(us_ClientID) +
                                               ", User ID " +
                                               std::to_string(c_UserInfo.u32_UserID) +
                                               ", Device Key: " +
                                               c_UserInfo.s_DeviceKey +
                                               ", Client Type: " +
                                               std::to_string(c_UserInfo.u8_ClientType) +
                                               " ): Failed to add recieved net message: " +
                                               e.what(),
                                "Client.cpp", __LINE__);
    }
}

void Client::RecieveDataAvailable() noexcept
{
    try
    {
        std::shared_ptr<NetMessage> p_Message = std::make_shared<NetMessage>(NetMessage::MSG_DATA_AVAILABLE);
        c_Recieved.Add(p_Message);
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "(Client ID: " +
                                               std::to_string(us_ClientID) +
                                               ", User ID " +
                                               std::to_string(c_UserInfo.u32_UserID) +
                                               ", Device Key: " +
                                               c_UserInfo.s_DeviceKey +
                                               ", Client Type: " +
                                               std::to_string(c_UserInfo.u8_ClientType) +
                                               " ): Failed to add recieved data available notification: " +
                                               e.what(),
                                "Client.cpp", __LINE__);
    }
}

//*************************************************************************************
// Send
//*************************************************************************************

void Client::Send()
{
    while (true)
    {
        // Grab send message
        std::shared_ptr<NetMessage> p_Send = c_Send.GetElement();
        
        if (p_Send == NULL)
        {
            // Nothing left to send for this update
            return;
        }
        
#if CLIENT_EXTENDED_LOGGING > 0
        Logger::Singleton().Log(Logger::INFO, "(Client ID: " +
                                              std::to_string(us_ClientID) +
                                              ", Client (User ID " +
                                              std::to_string(c_UserInfo.u32_UserID) +
                                              ", Device Key: " +
                                              c_UserInfo.s_DeviceKey +
                                              ", Client Type: " +
                                              std::to_string(c_UserInfo.u8_ClientType) +
                                              "): Sending NetMessage " +
                                              std::to_string(p_Send->GetID()) +
                                              " (Size: " +
                                              std::to_string(p_Send->v_Data.size()) +
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
        p_Context->c_Data.v_Bytes.swap(p_Send->v_Data);
        
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
            c_Send.Add(p_Send); // Return to send
            p_Context->c_Data.e_State = StreamData::FREE;
            
            throw Exception("Failed to open stream!");
        }
        else if (QUIC_FAILED(p_APITable->StreamStart(p_Stream,
                                                     QUIC_STREAM_START_FLAG_SHUTDOWN_ON_FAIL)))
        {
            c_Send.Add(p_Send);
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
            c_Send.Add(p_Send);
            p_APITable->StreamClose(p_Stream);
            p_Context->c_Data.e_State = StreamData::FREE;
            
            throw Exception("Failed to send on stream!");
        }
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

size_t Client::GetClientID() const noexcept
{
    return us_ClientID;
}
