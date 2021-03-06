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

#ifndef NetMessage_h
#define NetMessage_h

// C / C++
#include <cstdint>
#include <vector>

// External

// Project
#include "../Exception.h"


class NetMessage
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    // Full unencrypted size
    static constexpr size_t us_BufferSizeMax = 1024;
    
    // @NOTE: We want our ids to be 1 byte long at most (unsigned)
    //        Most ids are simply checked for their range and then handed off
    //        Using 1 unsingned byte allows a lot of endianess checking to be skipped
    static constexpr size_t us_IDPos = 0;
    static constexpr size_t us_IDSize = sizeof(uint8_t);
    
    // Helper
    static constexpr size_t us_DataPos = us_IDPos + us_IDSize;
    
    // Versioning in case auth etc changes and needs other data
    static constexpr uint8_t u8_NetMessageVersion = 1;
    
    enum NetMessageList
    {
        /**
         *  Message Version 1
         */
        
        // Unk
        MSG_UNK = 0,                        // Unknown / Error
        
        // Server Auth
        MSG_AUTH_REQUEST = 1,               // Request authentication
        MSG_AUTH_CHALLENGE = 2,             // Challenge client to provide auth data
        MSG_AUTH_PROOF = 3,                 // Provide proof of valid auth data
        MSG_AUTH_RESULT = 4,                // Auth result
        
        // Communication
        MSG_DATA_AVAILABLE = 5,             // Data is available to be requested
        MSG_GET_DATA = 6,                   // Request data
        MSG_NO_DATA = 7,                    // No data available
        MSG_TEXT,                           // Text data
        MSG_LOCATION,                       // Location data
        MSG_NOTIFICATION,                   // Push notification
        MSG_CUSTOM,                         // Custom data
        
        /**
         *  Bounds
         */
        
        NET_MESSAGE_LIST_MAX = MSG_CUSTOM,
        
        NET_MESSAGE_LIST_COUNT = NET_MESSAGE_LIST_MAX + 1
    };
    
    enum NetMessageError
    {
        /**
         *  Net Message Version 1
         */
        
        // No Error
        ERR_NONE = 0,                       // No error
        
        // Unk
        ERR_UNK = 1,                        // ???
        
        // Server Generall
        ERR_SG_ERROR = 2,                   // Internal server error
        
        // Server Auth
        ERR_SA_VERSION = 3,                 // Wrong OpCode Version
        ERR_SA_ACCOUNT = 4,                 // Account data given was wrong
        ERR_SA_ALREADY_CONNECTED = 5,       // Connection already exists
        ERR_SA_MAINTENANCE = 6,             // Temporary downtime
        ERR_SA_UNK_ACTOR = 7,               // Unknown actor type
        ERR_SA_NO_DEVICE,                   // No device found for connection
        
        // Bounds
        NET_MESSAGE_ERROR_MAX = ERR_SA_NO_DEVICE,
        
        NET_MESSAGE_ERROR_COUNT = NET_MESSAGE_ERROR_MAX + 1
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u8_ID The message id to represent.
     */
    
    NetMessage(uint8_t u8_ID);
    
    /**
     *  Data constructor.
     *
     *  \param v_Data The data for the net message. The vector data will be swapped.
     */
    
    NetMessage(std::vector<uint8_t>& v_Data);
    
    /**
     *  Data constructor.
     *
     *  \param v_Data The data for the net message.
     */
    
    NetMessage(std::vector<uint8_t> const& v_Data);
    
    /**
     *  Default destructor.
     */
    
    ~NetMessage() noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the net message id.
     *
     *  \return The net message id.
     */
    
    NetMessageList GetID() const noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::vector<uint8_t> v_Data;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:

};

#endif /* NetMessage_h */
