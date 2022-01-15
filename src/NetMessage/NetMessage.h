/**
 *  NetMessage.h
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

#ifndef NetMessage_h
#define NetMessage_h

// C / C++
#include <cstdint>
#include <vector>

// External

// Project
#include "./NetException.h"


class NetMessage
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    // Full unencrypted size
    static constexpr size_t us_BufferSize = 1024;
    
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
        CS_MSG_UNK = 0,                     // All - Unknown / Error
        
        // Availability
        S_MSG_PARTNER_CLOSED = 1,           // Server - A client closed connection (for other clients, etc.)
        
        // Server Auth
        C_MSG_AUTH_REQUEST = 2,             // Client - Request authentication
        S_MSG_AUTH_CHALLENGE = 3,           // Server - Challenge client to provide auth data
        C_MSG_AUTH_PROOF = 4,               // Client - Provide proof of valid auth data
        S_MSG_AUTH_RESULT = 5,              // Server - Proof check result
        
        // Device Pairing
        C_MSG_PAIR_REQUEST = 6,             // Client - Request pairing with platform client
        C_MSG_PAIR_CHALLENGE = 7,           // Client - Give nonce to app client to hash
        C_MSG_PAIR_PROOF,                   // Client - Return the hashed nonce to platform client
        C_MSG_PAIR_RESULT,                  // Client - Result of pairing with platform client
        
        // Channel
        C_MSG_CHANNEL_REQUEST,              // Client - Request channel info
        S_MSG_CHANNEL_RESPONSE,             // Server - Provide channel info
        
        // Text
        C_MSG_TEXT,                         // Client - Send text string
        
        // Location
        C_MSG_LOCATION,                     // Client - Send location data
        
        // Custom
        C_MSG_CUSTOM,                       // Client - Custom message
        CS_MSG_CUSTOM,                      // All - Custom message
        
        /**
         *  Bounds
         */
        
        NET_MESSAGE_LIST_MAX = CS_MSG_CUSTOM,
        
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
        ERR_SA_NO_DEVICE = 3,               // No device found for device key
        ERR_SA_VERSION = 4,                 // Wrong OpCode Version
        ERR_SA_UNK_ACTOR = 5,               // Unknown actor id
        ERR_SA_ACCOUNT = 6,                 // Account data given was wrong
        ERR_SA_ALREADY_CONNECTED = 7,       // Connection already exists
        ERR_SA_MAINTENANCE,                 // Temporary downtime
        
        // Channel
        ERR_CR_NO_CHANNEL,                  // No channel was found for identifier
        ERR_CR_FULL,                        // All channels are full
        ERR_CR_NO_PLATFORM,                 // No platform client found for app client
        
        // Device Auth
        ERR_DA_PAIR,                        // Device pairing failed
        
        // Bounds
        NET_MESSAGE_ERROR_MAX = ERR_DA_PAIR,
        
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
    
    /**
     *  Check if the net message is encrypted.
     *
     *  \return true if encrypted, false if not.
     */
    
    bool GetEncrypted() const noexcept;
    
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
