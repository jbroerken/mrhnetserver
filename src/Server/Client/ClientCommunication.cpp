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
#include "./ClientCommunication.h"
#include "./Base64.h"
#include "../../Logger.h"

// Pre-defined
using namespace DatabaseTable;
using namespace mysqlx;



//*************************************************************************************
// Retrieve
//*************************************************************************************

NetMessage ClientCommunication::RetrieveMessage( Database& c_Database, UserInfo const& c_UserInfo) noexcept
{
    Logger& c_Logger = Logger::Singleton();
    
    // Get the sender based on the reciever
    uint8_t u8_SenderType;
    
    switch (c_UserInfo.u8_ClientType)
    {
        case CLIENT_APP:
            u8_SenderType = CLIENT_PLATFORM;
            break;
        case CLIENT_PLATFORM:
            u8_SenderType = CLIENT_APP;
            break;
            
        default:
            c_Logger.Log(Logger::ERROR, "Unknown client type to retrieve for!",
                         "ClientCommunication.cpp", __LINE__);
            
            return NetMessage(NetMessage::MSG_NO_DATA);
    }
    
    // Got all required, read
    try
    {
        Table c_Table = c_Database.c_Session
                            .getSchema(c_Database.s_Database)
                            .getTable(p_MDTableName);
        
        RowResult c_Result = c_Table
                                .select(p_MDFieldName[MD_MESSAGE_ID],      /* 0 */
                                        p_MDFieldName[MD_USER_ID],         /* 1 */
                                        p_MDFieldName[MD_DEVICE_KEY],      /* 2 */
                                        p_MDFieldName[MD_ACTOR_TYPE],      /* 3 */
                                        p_MDFieldName[MD_MESSAGE_TYPE],    /* 4 */
                                        p_MDFieldName[MD_MESSAGE_DATA])    /* 5 */
                                .where(std::string(p_MDFieldName[MD_USER_ID]) +
                                       " == :valueA AND " +
                                       p_MDFieldName[MD_DEVICE_KEY] +
                                       " == :valueB AND " +
                                       p_MDFieldName[MD_ACTOR_TYPE] +
                                       " == :valueC")
                                .limit(1)
                                .bind("valueA",
                                      c_UserInfo.u32_UserID)
                                .bind("valueB",
                                      c_UserInfo.s_DeviceKey)
                                .bind("valueC",
                                      u8_SenderType)
                                .execute();
        
        if (c_Result.count() > 0)
        {
            // Decode base64
            Row c_Row = c_Result.fetchOne();
            
            std::string s_Bin = Base64::ToBytes(c_Row[5].get<std::string>());
            
            if (s_Bin.size() == 0)
            {
                c_Logger.Log(Logger::WARNING, "Failed to decode message base64!",
                             "ClientCommunication.cpp", __LINE__);
                
                return NetMessage(NetMessage::MSG_NO_DATA);
            }
            
            // Create
            NetMessage c_NetMessage(c_Row[4].get<uint32_t>());
            std::move(s_Bin.begin(),
                      s_Bin.end(),
                      std::back_inserter(c_NetMessage.v_Data));
            
            // Remove message
            c_Table.remove()
                .where(std::string(p_MDFieldName[MD_MESSAGE_ID]) +
                       " == :value")
                .bind("value",
                      c_Row[0].get<uint64_t>())
                .execute();
            
            // Now send
            return c_NetMessage;
        }
        else
        {
            return NetMessage(NetMessage::MSG_NO_DATA);
        }
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Message retrieval from database failed: " +
                                               std::string(e.what()),
                                "ClientCommunication.cpp", __LINE__);
        
        return NetMessage(NetMessage::MSG_NO_DATA);
    }
}

//*************************************************************************************
// Store
//*************************************************************************************

void ClientCommunication::StoreMessage(NetMessage const& c_NetMessage, Database& c_Database, UserInfo const& c_UserInfo) noexcept
{
    // Can insert?
    if (c_NetMessage.v_Data.size() <= NetMessage::us_DataPos)
    {
        Logger::Singleton().Log(Logger::WARNING, "Tried to store message without data!",
                                "ClientCommunication.cpp", __LINE__);
        return;
    }
    
    // Convert message data to base64
    std::string s_Base64 = Base64::ToString(&(c_NetMessage.v_Data[NetMessage::us_DataPos]),
                                            c_NetMessage.v_Data.size() - NetMessage::us_DataPos);
    
    if (s_Base64.size() == 0 || s_Base64.size() > us_MDMessageDataSize)
    {
        Logger::Singleton().Log(Logger::ERROR, "Failed to encode message as base64!",
                                "ClientCommunication.cpp", __LINE__);
        return;
    }
    
    // Created data string, now store for user
    try
    {
        c_Database.c_Session
            .getSchema(c_Database.s_Database)
            .getTable(p_MDTableName)
            .insert(DatabaseTable::p_MDFieldName[DatabaseTable::MD_USER_ID],
                    DatabaseTable::p_MDFieldName[DatabaseTable::MD_DEVICE_KEY],
                    DatabaseTable::p_MDFieldName[DatabaseTable::MD_ACTOR_TYPE],
                    DatabaseTable::p_MDFieldName[DatabaseTable::MD_MESSAGE_TYPE],
                    DatabaseTable::p_MDFieldName[DatabaseTable::MD_MESSAGE_DATA])
            .values(c_UserInfo.u32_UserID,
                    c_UserInfo.s_DeviceKey,
                    c_UserInfo.u8_ClientType,
                    c_NetMessage.v_Data[0],
                    s_Base64.c_str())
            .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Message insertion in database failed: " +
                                               std::string(e.what()),
                                "ClientCommunication.cpp", __LINE__);
    }
}
