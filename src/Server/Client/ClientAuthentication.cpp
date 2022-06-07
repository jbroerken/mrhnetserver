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
#include <sodium.h>

// Project
#include "./ClientAuthentication.h"
#include "./Base64.h"
#include "../../Logger.h"

// Pre-defined
#ifndef CLIENT_EXTENDED_LOGGING
    #define CLIENT_EXTENDED_LOGGING 0
#endif

using namespace DatabaseTable;
using namespace mysqlx;


//*************************************************************************************
// Auth Result
//*************************************************************************************

static inline NetMessage CreateAuthResult(uint8_t u8_Result) noexcept
{
    MSG_AUTH_RESULT_DATA c_Result;
    c_Result.u8_Result = u8_Result;
    
    return NetMessage(ToBuffer<MSG_AUTH_RESULT_DATA>(c_Result));
}

//*************************************************************************************
// Auth Request
//*************************************************************************************

NetMessage ClientAuthentication::HandleAuthRequest(MSG_AUTH_REQUEST_DATA c_Request, Database& c_Database, UserInfo& c_UserInfo) noexcept
{
    // Already authenticated? Skip db access etc to reduce load
    if (c_UserInfo.b_Authenticated == true)
    {
        return CreateAuthResult(NetMessage::ERR_NONE);
    }
    
    // Remember client type
    switch ((c_UserInfo.u8_ClientType = c_Request.u8_ClientType))
    {
        case CLIENT_APP:
        case CLIENT_PLATFORM:
            break;
            
        default:
            return CreateAuthResult(NetMessage::ERR_SA_UNK_ACTOR);
    }
    
    // Define user login info
    std::string s_Mail = std::string(c_Request.p_Mail,
                                     c_Request.p_Mail + strnlen(c_Request.p_Mail, NetMessageV1::us_SizeAccountMail));
    std::string s_Base64Password("");
    
    // Get data from table user_account first
    try
    {
        RowResult c_Result = c_Database.c_Session
                                     .getSchema(c_Database.s_Database)
                                     .getTable(p_UATableName)
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
            
            c_UserInfo.u32_UserID = c_Row[0].get<uint32_t>();
            s_Base64Password = c_Row[2].get<std::string>();
        }
        else
        {
            return CreateAuthResult(NetMessage::ERR_SA_ACCOUNT);
        }
    }
    catch (std::exception& e)
    {
        return CreateAuthResult(NetMessage::ERR_SG_ERROR);
    }
    
    // Check what we recieved and grab important auth info
    std::string s_Salt = Base64::ToBytesPart(s_Base64Password,
                                             0,
                                             crypto_pwhash_SALTBYTES);
    c_UserInfo.s_Password = Base64::ToBytesPart(s_Base64Password,
                                                crypto_pwhash_SALTBYTES,
                                                crypto_box_SEEDBYTES);
    
    if (s_Salt.size() == 0 || c_UserInfo.s_Password.size() == 0)
    {
        // @NOTE: We need to reset the password from this point onwards,
        //        so that a following auth proof message fails!
        c_UserInfo.s_Password = "";
        return CreateAuthResult(NetMessage::ERR_SA_ACCOUNT);
    }
    
    // Now grab user device list from table user_device_list and search device key
    c_UserInfo.s_DeviceKey = std::string(c_Request.p_DeviceKey,
                                         c_Request.p_DeviceKey + strnlen(c_Request.p_DeviceKey, NetMessageV1::us_SizeDeviceKey));
    
    try
    {
        RowResult c_Result = c_Database.c_Session
                                     .getSchema(c_Database.s_Database)
                                     .getTable(p_UDLTableName)
                                     .select(p_UDLFieldName[UDL_USER_ID],       /* 0 */
                                             p_UDLFieldName[UDL_DEVICE_KEY])    /* 1 */
                                     .where(std::string(p_UDLFieldName[UDL_USER_ID]) +
                                            " == :value")
                                     .bind("value",
                                           c_UserInfo.u32_UserID)
                                     .execute();
        
        size_t us_Count = c_Result.count();
        for (; us_Count > 0; --us_Count)
        {
            if (c_Result.fetchOne()[1].get<std::string>().compare(c_UserInfo.s_DeviceKey) == 0)
            {
                break;
            }
        }
        
        // Not found, send error
        if (us_Count == 0)
        {
            c_UserInfo.s_Password = "";
            return CreateAuthResult(NetMessage::ERR_SA_NO_DEVICE);
        }
    }
    catch (std::exception& e)
    {
        c_UserInfo.s_Password = "";
        return CreateAuthResult(NetMessage::ERR_SG_ERROR);
    }
    
    // Got everything, build challenge
    MSG_AUTH_CHALLENGE_DATA c_Result;
    
    memset(c_Result.p_Salt, '\0', us_SizeAccountPasswordSalt);
    memcpy(c_Result.p_Salt, s_Salt.data(), s_Salt.size());
    
    randombytes_buf(&(c_Result.u32_Nonce), sizeof(c_Result.u32_Nonce));
    c_UserInfo.u32_Nonce = c_Result.u32_Nonce;
    
    c_Result.u8_HashType = 0; // @NOTE: Currently only 1 type, read from db if needed
    
    // Send challenge and wait for proof
    return NetMessage(ToBuffer<MSG_AUTH_CHALLENGE_DATA>(c_Result));
}

//*************************************************************************************
// Auth Proof
//*************************************************************************************

NetMessage ClientAuthentication::HandleAuthProof(MSG_AUTH_PROOF_DATA c_Proof, Database& c_Database, UserInfo& c_UserInfo) noexcept
{
    NetMessage c_Result = CreateAuthResult(NetMessage::ERR_NONE);
    
    // Already authenticated? Skip account checking to reduce load
    if (c_UserInfo.b_Authenticated == true)
    {
        return c_Result;
    }
    
    // We have a valid password?
    // @NOTE: Directly giving auth proof is okay and will fail because the password is not set at this
    //        point! The size call always returns 0.
    if (c_UserInfo.s_Password.size() != crypto_box_SEEDBYTES)
    {
        c_Result.v_Data[0] = NetMessage::ERR_SA_ACCOUNT;
        return c_Result;
    }
    
    // Now get the nonce
    uint32_t u32_Nonce;
    
    if (crypto_secretbox_open_easy((unsigned char*)&u32_Nonce,
                                   (const unsigned char*)&(c_Proof.p_NonceHash[crypto_secretbox_NONCEBYTES]),
                                   crypto_secretbox_MACBYTES + sizeof(uint32_t),
                                   (const unsigned char*)(c_Proof.p_NonceHash),
                                   (const unsigned char*)(c_UserInfo.s_Password.data())) != 0)
    {
        c_Result.v_Data[0] = NetMessage::ERR_SA_ACCOUNT;
        return c_Result;
    }
    
    // We are now authenticated
#if CLIENT_EXTENDED_LOGGING > 0
    Logger::Singleton().Log(Logger::INFO, "Client (User ID " +
                                          std::to_string(c_UserInfo.u32_UserID) +
                                          ", Device Key: " +
                                          c_UserInfo.s_DeviceKey +
                                          ", Client Type: " +
                                          std::to_string(c_UserInfo.u8_ClientType) +
                                          "): Authenticated.",
                            "ClientAuthentication.cpp", __LINE__);
#endif
    
    c_UserInfo.b_Authenticated = true;
    return c_Result;
}
