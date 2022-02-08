/**
 *  ClientCommunication.cpp
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
#include "./ClientCommunication.h"
#include "./Base64.h"
#include "../../Logger.h"

// Pre-defined
#ifndef CLIENT_COMMUNICATION_RETRIEVE_LIMIT
    #define CLIENT_COMMUNICATION_RETRIEVE_LIMIT 32
#endif

using namespace DatabaseTable;
using namespace mysqlx;


//*************************************************************************************
// Retrieve
//*************************************************************************************

static NetMessage CreateNoData(uint8_t u8_Type) noexcept
{
    NetMessage c_Result(NetMessage::MSG_NO_DATA);
    c_Result.v_Data.emplace_back(u8_Type);
    
    return c_Result;
}

std::list<NetMessage> ClientCommunication::RetrieveMessages(uint8_t u8_Type, Database& c_Database, UserInfo const& c_UserInfo) noexcept
{
    std::list<NetMessage> l_Result;
    Logger& c_Logger = Logger::Singleton();
    
    uint8_t u8_ActorType;
    
    switch (c_UserInfo.u8_ClientType)
    {
        case CLIENT_APP:
            u8_ActorType = CLIENT_PLATFORM;
            break;
        case CLIENT_PLATFORM:
            u8_ActorType = CLIENT_APP;
            break;
            
        default:
            c_Logger.Log(Logger::ERROR, "Unknown client type to retrieve for!",
                         "ClientCommunication.cpp", __LINE__);
            
            l_Result.emplace_back(CreateNoData(u8_Type));
            return l_Result;
    }
    
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
                                       " == :valueC AND " +
                                       p_MDFieldName[MD_MESSAGE_TYPE] +
                                       " == :valueD")
                                .limit(CLIENT_COMMUNICATION_RETRIEVE_LIMIT)
                                .bind("valueA",
                                      c_UserInfo.u32_UserID)
                                .bind("valueB",
                                      c_UserInfo.s_DeviceKey)
                                .bind("valueC",
                                      u8_ActorType)
                                .bind("valueD",
                                      u8_Type)
                                .execute();
        
        std::string s_Bin;
        
        size_t us_Count = c_Result.count();
        for (; us_Count > 0; --us_Count)
        {
            // Decode base64
            Row c_Row = c_Result.fetchOne();
            
            s_Bin = Base64::ToBytes(c_Row[5].get<std::string>());
            
            if (s_Bin.size() > 0)
            {
                // Add message to list
                l_Result.emplace_back(u8_Type);
                
                std::move(s_Bin.begin(),
                          s_Bin.end(),
                          std::back_inserter(l_Result.back().v_Data));
            }
            else
            {
                c_Logger.Log(Logger::WARNING, "Failed to decode message base64!",
                             "ClientCommunication.cpp", __LINE__);
            }
            
            // Remove message
            c_Table.remove()
                .where(std::string(p_MDFieldName[MD_MESSAGE_ID]) +
                       " == :value")
                .bind("value",
                      c_Row[0].get<uint64_t>())
                .execute();
        }
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Message retrieval from database failed: " +
                                               std::string(e.what()),
                                "ClientCommunication.cpp", __LINE__);
    }
    
    // Got nothing?
    if (l_Result.size() == 0)
    {
        l_Result.emplace_back(CreateNoData(u8_Type));
    }
    
    return l_Result;
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
    
    if (s_Base64.size() == 0)
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
