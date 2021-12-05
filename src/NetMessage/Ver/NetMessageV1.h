/**
 *  NetMessageV1.h
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

#ifndef NetMessageV1_h
#define NetMessageV1_h

// C / C++

// External
#include <sodium.h>

// Project
#include "../NetMessage.h"


namespace NetMessageV1
{
    //*************************************************************************************
    // NetMessage Size
    //*************************************************************************************
    
    // Server
    constexpr size_t us_SizeServerAddress = 256;
    constexpr size_t us_SizeServerChannel = 64;
    
    // Account
    constexpr size_t us_SizeAccountMail = 256;
    constexpr size_t us_SizeAccountPassword = crypto_secretbox_KEYBYTES;
    constexpr size_t us_SizeAccountPasswordSalt = crypto_pwhash_SALTBYTES;
    
    constexpr size_t us_SizeNonceHash = crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES + sizeof(uint32_t);
    
    // Device
    constexpr size_t us_SizeDeviceKey = 64;
    constexpr size_t us_SizeDevicePassword = 256;
    
    //*************************************************************************************
    // NetMessage Data
    //*************************************************************************************

    // @NOTE: Client to client messages are not listed!
    
    //
    //  Server Auth
    //
    
    struct C_MSG_AUTH_REQUEST_DATA
    {
        char p_Mail[us_SizeAccountMail]; // The account mail
        uint8_t u8_Actor;  // Which type of client (platform or app)
        uint8_t u8_Version; // NetMessage version in use
    };
    
    struct S_MSG_AUTH_CHALLENGE_DATA
    {
        char p_Salt[us_SizeAccountPasswordSalt]; // Salt to use for pw hash
        uint32_t u32_Nonce; // Nonce to hash
        uint8_t u8_HashType;
    };
    
    struct C_MSG_AUTH_PROOF_DATA
    {
        char p_NonceHash[us_SizeNonceHash]; // Created hash
        char p_DeviceKey[us_SizeDeviceKey]; // Device valid for server
    };
    
    struct S_MSG_AUTH_RESULT_DATA
    {
        uint8_t u8_Result; // The result of the auth
    };
    
    //
    //  Channel
    //
    
    struct C_MSG_CHANNEL_REQUEST_DATA
    {
        char p_Channel[us_SizeServerChannel]; // Requested channel
    };
    
    struct S_MSG_CHANNEL_RESPONSE_DATA
    {
        char p_Channel[us_SizeServerChannel]; // Requested channel
        char p_Address[us_SizeServerAddress];
        uint32_t u32_Port;
        uint8_t u8_Result;
    };
    
    //*************************************************************************************
    // Data Creation
    //*************************************************************************************
    
    /**
     *  Convert a given client net message buffer to net message data.
     *
     *  \param v_Buffer The net message buffer.
     *
     *  \return The converted net message data.
     */

    template <typename T> T ToData(std::vector<uint8_t> const& v_Buffer);
    
    //*************************************************************************************
    // Buffer Creation
    //*************************************************************************************
    
    /**
     *  Convert given server net message data to a net message buffer.
     *
     *  \param Data The data to convert.
     *
     *  \return The net message buffer.
     */
    
    template <typename T> std::vector<uint8_t> ToBuffer(T const& Data);
};

#endif /* NetMessageV1_h */
