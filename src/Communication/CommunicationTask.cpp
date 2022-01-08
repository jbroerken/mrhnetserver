/**
 *  CommunicationTask.cpp
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
#include "./CommunicationTask.h"
#include "../Database/Database.h"
#include "../ServerAuth.h"
#include "../Logger.h"

// Pre-defined
#ifndef COMMUNICATION_TASK_MAX_AUTH_RETRY
    #define COMMUNICATION_TASK_MAX_AUTH_RETRY 3
#endif
#ifndef COMMUNICATION_TASK_MAX_MESSAGE_PER_LOOP
    #define COMMUNICATION_TASK_MAX_MESSAGE_PER_LOOP 10 // Only process up to 10 messages in a run
#endif
#ifndef COMMUNICATION_TASK_MAX_UPDATE_DIFF_S
    #define COMMUNICATION_TASK_MAX_UPDATE_DIFF_S 300
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

CommunicationTask::CommunicationTask(std::unique_ptr<NetConnection>& p_Connection,
                                     ExchangeContainer& c_ExchangeContainer,
                                     uint32_t u32_ChannelID) : WorkerTask(),
                                                               p_Connection(p_Connection.release()),
                                                               c_ExchangeContainer(c_ExchangeContainer),
                                                               p_MessageExchange(NULL),
                                                               u32_ChannelID(u32_ChannelID),
                                                               b_Authenticated(false),
                                                               s32_AuthAttempts(COMMUNICATION_TASK_MAX_AUTH_RETRY),
                                                               u8_ClientType(ACTOR_TYPE_COUNT),
                                                               s_Password(""),
                                                               s_DeviceKey("")
{
    if (this->p_Connection == NULL || this->p_Connection->GetConnected() == false)
    {
        throw ServerException("Invalid conncetion!", SERVER_COMMUNICATION);
    }
}

CommunicationTask::~CommunicationTask() noexcept
{
    // Remove from exchange container if platform client
    if (p_MessageExchange != NULL && u8_ClientType == CLIENT_PLATFORM)
    {
        c_ExchangeContainer.RemoveExchange(p_MessageExchange->s_DeviceKey);
    }
}

//*************************************************************************************
// Perform
//*************************************************************************************

bool CommunicationTask::Perform(std::unique_ptr<WorkerShared>& p_Shared) noexcept
{
    // Connection went away?
    if (p_Connection->GetConnected() == false)
    {
        NetMessage c_Message(NetMessage::S_MSG_PARTNER_CLOSED);
        GiveMessage(c_Message);
        
        // Remove platform connection from table
        if (u8_ClientType == CLIENT_PLATFORM && b_Authenticated == true)
        {
            try
            {
                GetTable(p_Shared, p_CDCTableName)
                    .remove()
                    .where(std::string(p_CDCFieldName[CDC_CHANNEL_ID]) +
                           " == :valueA AND" +
                           std::string(p_CDCFieldName[CDC_DEVICE_KEY]) +
                           " == :valueB")
                    .bind("valueA",
                          u32_ChannelID)
                    .bind("valueB",
                          s_DeviceKey)
                    .execute();
            }
            catch (std::exception& e)
            {
                Logger::Singleton().Log(Logger::ERROR, std::string(e.what()) + " (Communication)",
                                        "CommunicationTask.cpp", __LINE__);
            }
        }
        
        return false;
    }
    
    NetMessage c_Message(NetMessage::CS_MSG_UNK);
    int i_Count = 0;
    
    while (i_Count < COMMUNICATION_TASK_MAX_MESSAGE_PER_LOOP)
    {
        bool b_Processed = false;
        
        // Get message to send to other client or process by server
        if (p_Connection->Recieve(c_Message) == true)
        {
            switch (c_Message.GetID())
            {
                /**
                 *  Message Version 1
                 */
                    
                // Availability
                case NetMessage::C_MSG_HELLO:
                    if (b_Authenticated == false)
                    {
                        // Kick connections which spam wrong stuff
                        return false;
                    }
                    break;
                    
                // Server Auth
                case NetMessage::C_MSG_AUTH_REQUEST:
                    if (AuthRequest(p_Shared, ToData<C_MSG_AUTH_REQUEST_DATA>(c_Message.v_Data)) == false)
                    {
                        // Auth fail, kick
                        return false;
                    }
                    break;
                case NetMessage::C_MSG_AUTH_PROOF:
                    if (AuthProof(p_Shared, ToData<C_MSG_AUTH_PROOF_DATA>(c_Message.v_Data)) == false)
                    {
                        // Auth fail, kick
                        return false;
                    }
                    break;
                    
                // Not handled
                case NetMessage::S_MSG_PARTNER_CLOSED:
                case NetMessage::S_MSG_AUTH_CHALLENGE:
                case NetMessage::S_MSG_AUTH_RESULT:
                case NetMessage::C_MSG_CHANNEL_REQUEST:
                case NetMessage::S_MSG_CHANNEL_RESPONSE:
                case NetMessage::CS_MSG_CUSTOM: // Not handled by default server
                    if (b_Authenticated == false)
                    {
                        // Kick connections which spam wrong stuff
                        return false;
                    }
                    break;
                    
                /**
                 *  Client (Any Version)
                 */
                    
                default:
                    if (b_Authenticated == false)
                    {
                        // Kick if going around auth
                        return false;
                    }
                    
                    GiveMessage(c_Message);
                    break;
            }
            
            b_Processed = true;
        }
        
        // Got something from the other client and we're authenticated for recieving
        // @NOTE: if order is important, this way messages are retrieved and
        //        discarded!
        if (RetrieveMessage(c_Message) == true && b_Authenticated == true)
        {
            switch (c_Message.GetID())
            {
                case NetMessage::S_MSG_PARTNER_CLOSED:
                {
                    p_Connection->Send(c_Message);
                    
                    // Kick clients other than platform which waits for partners
                    if (u8_ClientType == CLIENT_PLATFORM)
                    {
                        // Clear and return exchange for later app client
                        ClearExchange();
                        c_ExchangeContainer.AddExchange(p_MessageExchange);
                        
                        b_Processed = true;
                        break;
                    }
                    else
                    {
                        return false;
                    }
                }
                    
                default:
                {
                    p_Connection->Send(c_Message);
                    b_Processed = true;
                    break;
                }
            }
        }
        
        // Cancel early? nothing to do
        if (b_Processed == false)
        {
            return true;
        }
        else
        {
            ++i_Count;
        }
    }
    
    return true;
}

void CommunicationTask::ClearExchange() noexcept
{
    std::lock_guard<std::mutex> c_APGuard(p_MessageExchange->c_APMutex);
    p_MessageExchange->l_APMessage.clear();
    
    std::lock_guard<std::mutex> c_PAGuard(p_MessageExchange->c_PAMutex);
    p_MessageExchange->l_PAMessage.clear();
}

//*************************************************************************************
// Message Exchange
//*************************************************************************************

void CommunicationTask::GiveMessage(NetMessage& c_Message) noexcept
{
    switch (u8_ClientType)
    {
        case CLIENT_PLATFORM:
        {
            std::lock_guard<std::mutex> c_Guard(p_MessageExchange->c_PAMutex);
            p_MessageExchange->l_PAMessage.emplace_back(c_Message.v_Data);
            break;
        }
            
        case CLIENT_APP:
        {
            std::lock_guard<std::mutex> c_Guard(p_MessageExchange->c_APMutex);
            p_MessageExchange->l_APMessage.emplace_back(c_Message.v_Data);
            break;
        }
            
        default: { break; }
    }
}

bool CommunicationTask::RetrieveMessage(NetMessage& c_Message) noexcept
{
    switch (u8_ClientType)
    {
        case CLIENT_PLATFORM:
        {
            std::lock_guard<std::mutex> c_Guard(p_MessageExchange->c_APMutex);
            
            if (p_MessageExchange->l_APMessage.size() > 0)
            {
                c_Message.v_Data.swap(p_MessageExchange->l_APMessage.front().v_Data);
                p_MessageExchange->l_APMessage.pop_front();
                
                return true;
            }
            
            return false;
        }
            
        case CLIENT_APP:
        {
            std::lock_guard<std::mutex> c_Guard(p_MessageExchange->c_PAMutex);
            
            if (p_MessageExchange->l_PAMessage.size() > 0)
            {
                c_Message.v_Data.swap(p_MessageExchange->l_PAMessage.front().v_Data);
                p_MessageExchange->l_PAMessage.pop_front();
                
                return true;
            }
            
            return false;
        }
            
        default: { return false; }
    }
}

//*************************************************************************************
// Auth
//*************************************************************************************

void CommunicationTask::SendAuthResult(uint8_t u8_Result) noexcept
{
    // Build result first
    S_MSG_AUTH_RESULT_DATA c_Result;
    
    c_Result.u8_Result = u8_Result;
    
    // Send
    NetMessage c_Message(NetMessage::CS_MSG_UNK);
    
    c_Message.v_Data = ToBuffer<S_MSG_AUTH_RESULT_DATA>(c_Result);
    
    p_Connection->Send(c_Message);
}

bool CommunicationTask::DecrementAuthAttempt() noexcept
{
    if ((s32_AuthAttempts -= 1) <= 0)
    {
        return false;
    }
    
    return true;
}

bool CommunicationTask::AuthRequest(std::unique_ptr<WorkerShared>& p_Shared, NetMessageV1::C_MSG_AUTH_REQUEST_DATA c_Request) noexcept
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
        SendAuthResult(NetMessage::ERR_SA_UNK_ACTOR);
        return DecrementAuthAttempt();
    }
    else if (c_Request.u8_Version != NetMessage::u8_NetMessageVersion)
    {
        SendAuthResult(NetMessage::ERR_SA_VERSION);
        return DecrementAuthAttempt();
    }
    
    std::string s_Mail = std::string(c_Request.p_Mail,
                                     c_Request.p_Mail + strnlen(c_Request.p_Mail, NetMessageV1::us_SizeAccountMail));
    std::string s_Base64Password("");
    uint32_t u32_UserID;
    
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
            Logger::Singleton().Log(Logger::ERROR, "Invalid results for account mail address (Communication)",
                                    "CommunicationTask.cpp", __LINE__);
            
            SendAuthResult(NetMessage::ERR_SG_ERROR);
            return DecrementAuthAttempt();
        }
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, std::string(e.what()) + " (Communication)",
                                "CommunicationTask.cpp", __LINE__);
        
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
        Logger::Singleton().Log(Logger::ERROR, "Failed to get account password hash (Communication)",
                                "CommunicationTask.cpp", __LINE__);
        
        s_Password = std::string(""); // Reset for auth proof check
        
        SendAuthResult(NetMessage::ERR_SA_ACCOUNT);
        return DecrementAuthAttempt();
    }
    
    // Now grab user device list from table user_device_list
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
        
        for (i = 0; i < us_Count; ++i)
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
        Logger::Singleton().Log(Logger::ERROR, std::string(e.what()) + " (Communication)",
                                "CommunicationTask.cpp", __LINE__);
        
        s_Password = std::string("");
        
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

bool CommunicationTask::AuthProof(std::unique_ptr<WorkerShared>& p_Shared, NetMessageV1::C_MSG_AUTH_PROOF_DATA c_Proof) noexcept
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
        Logger::Singleton().Log(Logger::ERROR, "Account encrypted nonce mismatch (Communication)",
                                "CommunicationTask.cpp", __LINE__);
        
        SendAuthResult(NetMessage::ERR_SA_ACCOUNT);
        return DecrementAuthAttempt();
    }
    
    // Now create message exchange for platform or grab for app client
    if (u8_ClientType == CLIENT_PLATFORM)
    {
        try
        {
            // Check if a platform connection for this device key already exists
            Table c_CDCTable = GetTable(p_Shared, p_CDCTableName);
            RowResult c_CDCResult = c_CDCTable
                                        .select(p_CDCFieldName[CDC_CHANNEL_ID],     /* 0 */
                                                p_CDCFieldName[CDC_DEVICE_KEY])     /* 1 */
                                        .where(std::string(p_CDCFieldName[CDC_CHANNEL_ID]) +
                                               " == :valueA AND " +
                                               std::string(p_CDCFieldName[CDC_DEVICE_KEY]) +
                                               " == :valueB")
                                        .bind("valueA",
                                              u32_ChannelID)
                                        .bind("valueB",
                                              s_DeviceKey)
                                        .execute();
            
            if (c_CDCResult.count() != 0)
            {
                SendAuthResult(NetMessage::ERR_SA_ALREADY_CONNECTED);
                return DecrementAuthAttempt();
            }
            
            // First, create a exchange for app client access
            p_MessageExchange = c_ExchangeContainer.CreateExchange(s_DeviceKey);
            
            // Created, now commit to db that connection exists
            c_CDCTable
                .insert(p_CDCFieldName[CDC_CHANNEL_ID],
                        p_CDCFieldName[CDC_DEVICE_KEY])
                .values(u32_ChannelID,
                        s_DeviceKey)
                .execute();
            
            // @TODO: update assistant_connections field
        }
        catch (ServerException& e)
        {
            Logger::Singleton().Log(Logger::ERROR, e.what2() + " (Communication)",
                                    "CommunicationTask.cpp", __LINE__);
            
            c_ExchangeContainer.RemoveExchange(s_DeviceKey); // Remove again for safety
            
            SendAuthResult(NetMessage::ERR_SG_ERROR);
            return DecrementAuthAttempt();
        }
    }
    else if (u8_ClientType == CLIENT_APP)
    {
        try
        {
            // Get platform client created exchange
            // @NOTE: GetExchange removes exchange from container,
            //        Removing the chance for a second app client to connect
            //        with the first one. The second one errors out.
            p_MessageExchange = c_ExchangeContainer.GetExchange(s_DeviceKey);
            
            // Got exchange, clean for new communication
            ClearExchange();
        }
        catch (ServerException& e)
        {
            Logger::Singleton().Log(Logger::ERROR, e.what2() + " (Communication)",
                                    "CommunicationTask.cpp", __LINE__);
            
            SendAuthResult(NetMessage::ERR_SA_NO_DEVICE);
            return DecrementAuthAttempt();
        }
    }
    else
    {
        Logger::Singleton().Log(Logger::ERROR, "Unknown actor type (Communication)",
                                "CommunicationTask.cpp", __LINE__);
        
        SendAuthResult(NetMessage::ERR_SG_ERROR);
        return DecrementAuthAttempt();
    }
    
    // We are authenticated
    b_Authenticated = true;
    
    // Send result
    SendAuthResult(NetMessage::ERR_NONE);
    return true;
}
