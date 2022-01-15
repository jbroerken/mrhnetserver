/**
 *  ConnectionTask.cpp
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
#include <sodium.h>

// Project
#include "./ConnectionTask.h"
#include "../Database/Database.h"
#include "../ServerAuth.h"
#include "../Logger.h"

// Pre-defined
#ifndef CONNECTION_TASK_MAX_AUTH_RETRY
    #define CONNECTION_TASK_MAX_AUTH_RETRY 3
#endif
#ifndef CONNECTION_TASK_MAX_MESSAGE_PER_LOOP
    #define CONNECTION_TASK_MAX_MESSAGE_PER_LOOP 10 // Only process up to 10 messages in a run
#endif
#ifndef CONNECTION_TASK_MAX_UPDATE_DIFF_S
    #define CONNECTION_TASK_MAX_UPDATE_DIFF_S 300
#endif
#ifndef CONNECTION_TASK_EXTENDED_LOGGING
    #define CONNECTION_TASK_EXTENDED_LOGGING 1//0
#endif

using namespace NetMessageV1;
using namespace DatabaseTable;
using namespace mysqlx;

namespace
{
    Table inline GetTable(std::unique_ptr<WorkerShared>& p_Shared, const char* p_Table) noexcept
    {
        
        Database& c_Database = dynamic_cast<Database&>(*(p_Shared.get()));
        return c_Database.c_Session
                         .getSchema(c_Database.s_Database)
                         .getTable(p_Table);
    }
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

ConnectionTask::ConnectionTask(std::unique_ptr<NetConnection>& p_Connection) : WorkerTask(),
                                                                               p_Connection(p_Connection.release()),
                                                                               b_Authenticated(false),
                                                                               s32_AuthAttempts(CONNECTION_TASK_MAX_AUTH_RETRY),
                                                                               u8_ClientType(ACTOR_TYPE_COUNT),
                                                                               s_Password(""),
                                                                               s_DeviceKey("")
{
    if (this->p_Connection == NULL || this->p_Connection->GetConnected() == false)
    {
        throw ServerException("Invalid conncetion!", SERVER_CONNECTION);
    }
}

ConnectionTask::~ConnectionTask() noexcept
{}

//*************************************************************************************
// Perform
//*************************************************************************************

bool ConnectionTask::Perform(std::unique_ptr<WorkerShared>& p_Shared) noexcept
{
    // Connection went away?
    if (p_Connection->GetConnected() == false)
    {
        LogClientEvent("Disconnected (Connection lost).", __LINE__);
        return false;
    }
    
    NetMessage c_Message(NetMessage::CS_MSG_UNK);
    int i_Recieved = 0;
    
    while (i_Recieved < CONNECTION_TASK_MAX_MESSAGE_PER_LOOP && p_Connection->Recieve(c_Message) == true)
    {
        // Encrypted?
        if (c_Message.GetEncrypted() == true)
        {
            // Connection server does not accept e2e encrypted messages!
            LogClientEvent("Disconnected (Sent encrypted message to connection server).", __LINE__);
            return false;
        }
        
        // Server-readable
        switch (c_Message.GetID())
        {
            /**
             *  Message Version 1
             */
                
            // Server Auth
            case NetMessage::C_MSG_AUTH_REQUEST:
                if (AuthRequest(p_Shared, ToData<C_MSG_AUTH_REQUEST_DATA>(c_Message.v_Data)) == false)
                {
                    // Auth fail, kick
                    LogClientEvent("Disconnected (Failed to authenticate).", __LINE__);
                    return false;
                }
                break;
            case NetMessage::C_MSG_AUTH_PROOF:
                if (AuthProof(ToData<C_MSG_AUTH_PROOF_DATA>(c_Message.v_Data)) == false)
                {
                    LogClientEvent("Disconnected (Failed to authenticate).", __LINE__);
                    return false;
                }
                break;
                
            // Channel
            case NetMessage::C_MSG_CHANNEL_REQUEST:
                if (ChannelRequest(p_Shared, ToData<C_MSG_CHANNEL_REQUEST_DATA>(c_Message.v_Data)) == false)
                {
                    LogClientEvent("Disconnected (Sent invalid message while not authenticated).", __LINE__);
                    return false;
                }
                break;
                
            /**
             *  Not Handled (Any version)
             */
                
            default:
                if (b_Authenticated == false)
                {
                    LogClientEvent("Disconnected (Sent invalid message while not authenticated).", __LINE__);
                    return false;
                }
                break;
        }
        
        ++i_Recieved;
    }
    
    return true;
}

void ConnectionTask::LogClientEvent(std::string const& s_Event, int i_Line) noexcept
{
    Logger::Singleton().Log(Logger::INFO, "[ Device Key " +
                                          s_DeviceKey +
                                          ", User ID " +
                                          std::to_string(u32_UserID) +
                                          ", Client Type: " +
                                          (u8_ClientType == CLIENT_PLATFORM ? "Platform" : "App") +
                                          " ]: " +
                                          s_Event,
                            "ConnectionTask.cpp", i_Line);
}

//*************************************************************************************
// Auth
//*************************************************************************************

void ConnectionTask::SendAuthResult(uint8_t u8_Result) noexcept
{
    // Build result first
    S_MSG_AUTH_RESULT_DATA c_Result;
    
    c_Result.u8_Result = u8_Result;
    
    // Send
    NetMessage c_Message(NetMessage::CS_MSG_UNK);
    
    c_Message.v_Data = ToBuffer<S_MSG_AUTH_RESULT_DATA>(c_Result);
    
    p_Connection->Send(c_Message);
}

bool ConnectionTask::DecrementAuthAttempt() noexcept
{
    if ((s32_AuthAttempts -= 1) <= 0)
    {
        return false;
    }
    
    return true;
}

bool ConnectionTask::AuthRequest(std::unique_ptr<WorkerShared>& p_Shared, NetMessageV1::C_MSG_AUTH_REQUEST_DATA c_Request) noexcept
{
    // Already authenticated? Skip db access etc to reduce load
    if (b_Authenticated == true)
    {
        SendAuthResult(NetMessage::ERR_NONE);
        return true;
    }
    
    // Remember client type
    u8_ClientType = c_Request.u8_Actor;
    
    // Version and type valid?
    if (u8_ClientType != CLIENT_APP && u8_ClientType != CLIENT_PLATFORM)
    {
        if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
        {
            LogClientEvent("Authentication failed: Invalid client type.", __LINE__);
        }
        
        SendAuthResult(NetMessage::ERR_SA_UNK_ACTOR);
        return DecrementAuthAttempt();
    }
    else if (c_Request.u8_Version != NetMessage::u8_NetMessageVersion)
    {
        if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
        {
            LogClientEvent("Authentication failed: Invalid net message version.", __LINE__);
        }
        
        SendAuthResult(NetMessage::ERR_SA_VERSION);
        return DecrementAuthAttempt();
    }
    
    std::string s_Mail = std::string(c_Request.p_Mail,
                                     c_Request.p_Mail + strnlen(c_Request.p_Mail, NetMessageV1::us_SizeAccountMail));
    std::string s_Base64Password("");
    
    // Get data from table user_account first
    try
    {
        RowResult c_Result = GetTable(p_Shared, p_UATableName)
                                     .select(p_UAFieldName[UA_USER_ID],         /* 0 */
                                             p_UAFieldName[UA_MAIL_ADDRESS],    /* 1 */
                                             p_UAFieldName[UA_PASSWORD])        /* 2 */
                                     .where(std::string(p_UAFieldName[UA_MAIL_ADDRESS]) +
                                            " == :value")
                                     .bind("value",
                                           s_Mail)
                                     .execute();
        
        if (c_Result.count() == 1) // Unique account rows
        {
            Row c_Row = c_Result.fetchOne();
            
            u32_UserID = c_Row[0].get<uint32_t>();
            s_Base64Password = c_Row[2].get<std::string>();
        }
        else
        {
            if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
            {
                LogClientEvent("Authentication failed: Invalid results for given account mail address.", __LINE__);
            }
            
            SendAuthResult(NetMessage::ERR_SG_ERROR);
            return DecrementAuthAttempt();
        }
    }
    catch (std::exception& e)
    {
        if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
        {
            LogClientEvent("SQL querry failed: " + std::string(e.what()), __LINE__);
        }
        SendAuthResult(NetMessage::ERR_SA_ACCOUNT);
        return DecrementAuthAttempt();
    }
    
    // Check what we recieved and grab important auth info
    std::string s_Salt("");
    
    if (s_Base64Password.size() == 0 ||
        ServerAuth::ExtractSalt(s_Base64Password, s_Salt) == false ||
        ServerAuth::ExtractPassword(s_Base64Password, s_Password) == false ||
        s_Salt.size() > us_SizeAccountPasswordSalt ||
        s_Password.size() > us_SizeAccountPassword)
    {
        if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
        {
            LogClientEvent("Authentication failed: Failed to get account password hash and salt.", __LINE__);
        }
        
        s_Password = std::string(""); // Reset for auth proof check
        
        SendAuthResult(NetMessage::ERR_SA_ACCOUNT);
        return DecrementAuthAttempt();
    }
    
    // Now grab user device list from table user_device_list and search device key
    s_DeviceKey = std::string(c_Request.p_DeviceKey,
                              c_Request.p_DeviceKey + strnlen(c_Request.p_DeviceKey, NetMessageV1::us_SizeDeviceKey));
    
    try
    {
        RowResult c_Result = GetTable(p_Shared, p_UDLTableName)
                                     .select(p_UDLFieldName[UDL_USER_ID],       /* 0 */
                                             p_UDLFieldName[UDL_DEVICE_KEY])    /* 1 */
                                     .where(std::string(p_UDLFieldName[UDL_USER_ID]) +
                                            " == :value")
                                     .bind("value",
                                           std::to_string(u32_UserID))
                                     .execute();
        
        size_t i = 0;
        size_t us_Count = c_Result.count();
        
        for (; i < us_Count; ++i)
        {
            if (c_Result.fetchOne()[1].get<std::string>().compare(s_DeviceKey) == 0)
            {
                break;
            }
        }
        
        // Not found, send error
        if (i == us_Count)
        {
            SendAuthResult(NetMessage::ERR_SA_NO_DEVICE);
            return DecrementAuthAttempt();
        }
    }
    catch (std::exception& e)
    {
        s_Password = std::string("");
        
        if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
        {
            LogClientEvent("SQL querry failed: " + std::string(e.what()), __LINE__);
        }
        SendAuthResult(NetMessage::ERR_SA_ACCOUNT);
        return DecrementAuthAttempt();
    }
    
    // Got everything, build challenge
    S_MSG_AUTH_CHALLENGE_DATA c_Result;
    
    memset(c_Result.p_Salt, '\0', us_SizeAccountPasswordSalt);
    memcpy(c_Result.p_Salt, s_Salt.data(), s_Salt.size());
    
    randombytes_buf(&(c_Result.u32_Nonce), sizeof(c_Result.u32_Nonce));
    u32_Nonce = c_Result.u32_Nonce;
    
    c_Result.u8_HashType = 0; // @NOTE: Currently only 1 type, read from db if needed
    
    // Challenge built, send it
    NetMessage c_Message(NetMessage::CS_MSG_UNK);
    
    c_Message.v_Data = ToBuffer<S_MSG_AUTH_CHALLENGE_DATA>(c_Result);
    
    p_Connection->Send(c_Message);
    
    // Now wait for proof
    return true;
}

bool ConnectionTask::AuthProof(NetMessageV1::C_MSG_AUTH_PROOF_DATA c_Proof) noexcept
{
    // Build result data
    S_MSG_AUTH_RESULT_DATA c_Result;
    c_Result.u8_Result = NetMessage::ERR_NONE;
    
    // Already authenticated? Skip account checking to reduce load
    if (b_Authenticated == true)
    {
        SendAuthResult(NetMessage::ERR_NONE);
        return true;
    }
    
    // @NOTE: Directly giving auth proof is okay and will fail because the password is not set at this
    //        point! The size call always returns 0.
    if (s_Password.size() == 0 || ServerAuth::CompareNonce(u32_Nonce, c_Proof.p_NonceHash, s_Password.data()) == false)
    {
        if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
        {
            LogClientEvent("Authentication failed: Account encrypted nonce mismatch.", __LINE__);
        }
        
        SendAuthResult(NetMessage::ERR_SA_ACCOUNT);
        return DecrementAuthAttempt();
    }
    
    // We are authenticated
    LogClientEvent("Connected (Authentication successfull).", __LINE__);
    b_Authenticated = true;
    
    // Now create the message to send
    SendAuthResult(NetMessage::ERR_NONE);
    return true;
}

//*************************************************************************************
// Channel
//*************************************************************************************

void ConnectionTask::SendChannelResponse(std::string const& s_Channel, std::string const& s_Address, uint32_t u32_Port, uint8_t u8_Result) noexcept
{
    // Prepare result message
    S_MSG_CHANNEL_RESPONSE_DATA c_Result;
    
    memset(c_Result.p_Channel, '\0', us_SizeServerChannel);
    memset(c_Result.p_Address, '\0', us_SizeServerAddress);
    
    strncpy(c_Result.p_Channel, s_Channel.c_str(), s_Channel.size());
    strncpy(c_Result.p_Address, s_Address.c_str(), s_Address.size());
    
    c_Result.u32_Port = u32_Port;
    
    c_Result.u8_Result = u8_Result;
    
    // Built, now send
    NetMessage c_Message(NetMessage::CS_MSG_UNK);
    
    c_Message.v_Data = ToBuffer<S_MSG_CHANNEL_RESPONSE_DATA>(c_Result);
    
    p_Connection->Send(c_Message);
}

bool ConnectionTask::ChannelRequest(std::unique_ptr<WorkerShared>& p_Shared, NetMessageV1::C_MSG_CHANNEL_REQUEST_DATA c_Request) noexcept
{
    // Not authenticated? Kill connection
    if (b_Authenticated == false)
    {
        return false;
    }
    
    // Get the channel list first
    std::string s_ChannelName(c_Request.p_Channel,
                              c_Request.p_Channel + strnlen(c_Request.p_Channel, NetMessageV1::us_SizeServerChannel));
    
    // Last update limit
    uint64_t u64_MaxTimeDiff = time(NULL) - CONNECTION_TASK_MAX_UPDATE_DIFF_S;
    
    // App clients want a already existing connection, platform clients empty servers
    // Platform clients can therefore simply check by channel name which server is the least occupied.
    // App clients check first which connections for the device key exist, and those connections are then
    // used to find the servers they're connected to. If one is connected to the request channel we return
    // said channel
    if (u8_ClientType == CLIENT_APP)
    {
        try
        {
            // First we need to check for an actual platform connection
            // This also shows us where the connection is held
            RowResult c_CDCResult = GetTable(p_Shared, p_CDCTableName)
                                            .select(p_CDCFieldName[CDC_CHANNEL_ID],     /* 0 */
                                                    p_CDCFieldName[CDC_USER_ID],        /* 1 */
                                                    p_CDCFieldName[CDC_DEVICE_KEY])     /* 2 */
                                            .where(std::string(p_CDCFieldName[CDC_USER_ID]) +
                                                   " == :valueA AND " +
                                                   std::string(p_CDCFieldName[CDC_DEVICE_KEY]) +
                                                   " == :valueB")
                                            .bind("valueA",
                                                  u32_UserID)
                                            .bind("valueB",
                                                  s_DeviceKey)
                                            .execute();
            
            // Now get the channel address
            std::string s_Address("");
            uint32_t u32_Port(0);
            uint8_t u8_Result = NetMessage::ERR_CR_NO_CHANNEL;
            
            // @NOTE: There might be dead channels etc inside, so we pick the
            //        newest one by time stamp!
            size_t us_Count = c_CDCResult.count();
            uint64_t u64_LastUpdate = 0;
            
            for (size_t i = 0; i < us_Count; ++i)
            {
                RowResult c_CLResult = GetTable(p_Shared, p_CLTableName)
                                               .select(p_CLFieldName[CL_CHANNEL_ID],   /* 0 */
                                                       p_CLFieldName[CL_NAME],         /* 1 */
                                                       p_CLFieldName[CL_ADDRESS],      /* 2 */
                                                       p_CLFieldName[CL_PORT],         /* 3 */
                                                       p_CLFieldName[CL_LAST_UPDATE])  /* 4 */
                                               .where(std::string(p_CLFieldName[CL_CHANNEL_ID]) +
                                                      " == :valueA AND " +
                                                      std::string(p_CLFieldName[CL_NAME]) +
                                                      " == :valueB AND " +
                                                      std::string(p_CLFieldName[CL_LAST_UPDATE]) +
                                                      " >= :valueC")
                                               .bind("valueA",
                                                     c_CDCResult.fetchOne()[0].get<uint32_t>())
                                               .bind("valueB",
                                                     s_ChannelName)
                                               .bind("valueC",
                                                     u64_MaxTimeDiff)
                                               .execute();
                
                // Check row result, there should be only one!
                if (c_CLResult.count() != 1)
                {
                    continue;
                }
                
                Row c_Row = c_CLResult.fetchOne();
                
                // Now check the last update
                uint64_t u64_RowUpdate = c_Row[4].get<uint64_t>();
                
                if (u64_RowUpdate < u64_LastUpdate)
                {
                    continue;
                }
                
                u64_LastUpdate = u64_RowUpdate;
                
                // Now set the channel info
                s_Address = c_Row[2].get<std::string>();
                u32_Port = c_Row[3].get<int32_t>();
                u8_Result = NetMessage::ERR_NONE;
            }
            
            // Return the result
            SendChannelResponse(s_ChannelName, s_Address, u32_Port, u8_Result);
            return true;
        }
        catch (std::exception& e)
        {
            if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
            {
                LogClientEvent("SQL querry failed: " + std::string(e.what()), __LINE__);
            }
            SendChannelResponse(s_ChannelName, "", 0, NetMessage::ERR_CR_NO_CHANNEL);
            return true;
        }
    }
    else if (u8_ClientType == CLIENT_PLATFORM)
    {
        try
        {
            // First we need to grab all active channels based based on our
            // channel identifier
            RowResult c_Result = GetTable(p_Shared, p_CLTableName)
                                         .select(p_CLFieldName[CL_CHANNEL_ID],              /* 0 */
                                                 p_CLFieldName[CL_NAME],                    /* 1 */
                                                 p_CLFieldName[CL_ADDRESS],                 /* 2 */
                                                 p_CLFieldName[CL_PORT],                    /* 3 */
                                                 p_CLFieldName[CL_ASSISTANT_CONNECTIONS],   /* 4 */
                                                 p_CLFieldName[CL_LAST_UPDATE])             /* 5 */
                                         .where(std::string(p_CLFieldName[CL_NAME]) +
                                                " == :valueA AND " +
                                                std::string(p_CLFieldName[CL_LAST_UPDATE]) +
                                                " >= :valueB")
                                         .bind("valueA",
                                               s_ChannelName)
                                         .bind("valueB",
                                               u64_MaxTimeDiff)
                                         .execute();
            
            // Now get the channel address
            std::string s_Address("");
            uint32_t u32_Port(0);
            uint8_t u8_Result = NetMessage::ERR_CR_NO_CHANNEL;
            
            // We now search for the channel with the least connections
            // @NOTE: There might be dead channels etc inside, so we pick the
            //        newest one by time stamp!
            size_t us_Count = c_Result.count();
            uint32_t u32_Connections = ((uint32_t) - 1);
            
            for (size_t i = 0; i < us_Count; ++i)
            {
                Row c_Row = c_Result.fetchOne();
                
                // We need to check assistant connections
                uint32_t u32_RowConnections = c_Row[4].get<uint32_t>();
                
                if (u32_RowConnections >= u32_Connections)
                {
                    continue;
                }
                
                // Now set the channel info
                s_Address = c_Row[2].get<std::string>();
                u32_Port = c_Row[3].get<int32_t>();
                u8_Result = NetMessage::ERR_NONE;
            }
            
            // Return the result
            SendChannelResponse(s_ChannelName, s_Address, u32_Port, u8_Result);
            return true;
        }
        catch (std::exception& e)
        {
            if (CONNECTION_TASK_EXTENDED_LOGGING > 0)
            {
                LogClientEvent("SQL querry failed: " + std::string(e.what()), __LINE__);
            }
            SendChannelResponse(s_ChannelName, "", 0, NetMessage::ERR_CR_NO_CHANNEL);
            return true;
        }
    }
    else
    {
        // This shouldn't be reached because auth blocks on wrong actor type.
        // Keep anyway if for any reason a wrong actor type is stored
        SendChannelResponse(s_ChannelName, "", 0, NetMessage::ERR_SG_ERROR);
        return true;
    }
}
