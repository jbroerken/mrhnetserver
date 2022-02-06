/**
 *  NetMessageV1.cpp
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
#include <cstring>
#ifdef __APPLE__
    #include <libkern/OSByteOrder.h>
#else
    #include <byteswap.h>
#endif

// External

// Project
#include "./NetMessageV1.h"

// Pre-defined
#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)
#ifdef __APPLE__
    #define bswap_16(x) OSSwapInt16(x)
    #define bswap_32(x) OSSwapInt32(x)
    #define bswap_64(x) OSSwapInt64(x)
#endif

using namespace NetMessageV1;

namespace
{
    // Data Creation
    constexpr size_t us_MsgAuthRequestSize = NetMessage::us_DataPos +
                                             us_SizeAccountMail +
                                             us_SizeDeviceKey +
                                             sizeof(uint8_t) +      /* Client Type */
                                             sizeof(uint8_t);       /* Net Message Version */
    constexpr size_t us_MsgAuthProofSize = NetMessage::us_DataPos +
                                           us_SizeNonceHash;
    constexpr size_t us_MsgDataAvailSize = NetMessage::us_DataPos +
                                           sizeof(uint8_t);         /* Data Type */
    constexpr size_t us_MsgNotificationSize = NetMessage::us_DataPos +
                                              us_SizeNotificationString;
    
    // Buffer Creation
    constexpr size_t us_MsgAuthChallengeSize = NetMessage::us_DataPos +
                                               us_SizeAccountPasswordSalt +
                                               sizeof(uint32_t) +   /* Nonce */
                                               sizeof(uint8_t);     /* Hash Type */
    constexpr size_t us_MsgAuthSateSize = NetMessage::us_DataPos +
                                          sizeof(uint8_t);          /* Auth State Result */
    constexpr size_t us_MsgNoDataSize = NetMessage::us_DataPos +
                                        sizeof(uint8_t);            /* Data Type */
    
}


//*************************************************************************************
// Data Creation
//*************************************************************************************

template<> MSG_AUTH_REQUEST_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != us_MsgAuthRequestSize)
    {
        throw Exception("Invalid data buffer!");
    }
    
    MSG_AUTH_REQUEST_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    memcpy(&(c_Data.p_Mail[0]),
           &(v_Buffer[us_Pos]),
           us_SizeAccountMail);
    us_Pos += us_SizeAccountMail;
    
    memcpy(&(c_Data.p_DeviceKey[0]),
           &(v_Buffer[us_Pos]),
           us_SizeDeviceKey);
    us_Pos += us_SizeDeviceKey;
    
    c_Data.u8_ClientType = v_Buffer[us_Pos];
    us_Pos += 1;
    
    c_Data.u8_Version = v_Buffer[us_Pos];
    
    return c_Data;
}

template<> MSG_AUTH_PROOF_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != us_MsgAuthProofSize)
    {
        throw Exception("Invalid data buffer!");
    }
    
    MSG_AUTH_PROOF_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    memcpy(&(c_Data.p_NonceHash[0]),
           &(v_Buffer[us_Pos]),
           us_SizeNonceHash);
    
    return c_Data;
}

template<> MSG_DATA_AVAIL_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != us_MsgDataAvailSize)
    {
        throw Exception("Invalid data buffer!");
    }
    
    MSG_DATA_AVAIL_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    c_Data.u8_Data = v_Buffer[us_Pos];
    
    return c_Data;
}

template<> MSG_NOTIFICATION_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != us_MsgNotificationSize)
    {
        throw Exception("Invalid data buffer!");
    }
    
    MSG_NOTIFICATION_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    memcpy(&(c_Data.p_String[0]),
           &(v_Buffer[us_Pos]),
           us_SizeNotificationString);
    
    return c_Data;
}

//*************************************************************************************
// Buffer Creation
//*************************************************************************************

template<> std::vector<uint8_t> NetMessageV1::ToBuffer(MSG_AUTH_CHALLENGE_DATA const& Data)
{
    std::vector<uint8_t> v_Buffer(us_MsgAuthChallengeSize, '\0');
    size_t us_Pos = NetMessage::us_IDPos;
    
    v_Buffer[NetMessage::us_IDPos] = NetMessage::MSG_AUTH_CHALLENGE;
    us_Pos += NetMessage::us_IDSize;
    
    memcpy(&(v_Buffer[us_Pos]),
           &(Data.p_Salt[0]),
           us_SizeAccountPasswordSalt);
    us_Pos += us_SizeAccountPasswordSalt;
    
    if (IS_BIG_ENDIAN)
    {
        uint32_t u32_Nonce = bswap_32(Data.u32_Nonce);
    
        memcpy(&(v_Buffer[us_Pos]),
               &u32_Nonce,
               sizeof(u32_Nonce));
    }
    else
    {
        memcpy(&(v_Buffer[us_Pos]),
               &(Data.u32_Nonce),
               sizeof(Data.u32_Nonce));
    }
    
    us_Pos += sizeof(Data.u32_Nonce);
    
    v_Buffer[us_Pos] = Data.u8_HashType;
    
    return v_Buffer;
}

template<> std::vector<uint8_t> NetMessageV1::ToBuffer(MSG_AUTH_RESULT_DATA const& Data)
{
    std::vector<uint8_t> v_Buffer(us_MsgAuthSateSize, '\0');
    size_t us_Pos = NetMessage::us_IDPos;
    
    v_Buffer[NetMessage::us_IDPos] = NetMessage::MSG_AUTH_RESULT;
    us_Pos += NetMessage::us_IDSize;
    
    v_Buffer[us_Pos] = Data.u8_Result;
    
    return v_Buffer;
}

template<> std::vector<uint8_t> NetMessageV1::ToBuffer(MSG_NO_DATA_DATA const& Data)
{
    std::vector<uint8_t> v_Buffer(us_MsgNoDataSize, '\0');
    size_t us_Pos = NetMessage::us_IDPos;
    
    v_Buffer[NetMessage::us_IDPos] = NetMessage::MSG_NO_DATA;
    us_Pos += NetMessage::us_IDSize;
    
    v_Buffer[us_Pos] = Data.u8_Data;
    
    return v_Buffer;
}
