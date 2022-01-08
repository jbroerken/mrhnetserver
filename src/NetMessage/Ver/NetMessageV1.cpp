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


//*************************************************************************************
// Data Creation
//*************************************************************************************

template<> C_MSG_AUTH_REQUEST_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != NetMessage::us_BufferSize)
    {
        throw NetException("Invalid data buffer!");
    }
    
    C_MSG_AUTH_REQUEST_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    memcpy(&(c_Data.p_Mail[0]),
           &(v_Buffer[us_Pos]),
           us_SizeAccountMail);
    us_Pos += us_SizeAccountMail;
    
    memcpy(&(c_Data.p_DeviceKey[0]),
           &(v_Buffer[us_Pos]),
           us_SizeDeviceKey);
    us_Pos += us_SizeDeviceKey;
    
    c_Data.u8_Actor = v_Buffer[us_Pos];
    us_Pos += 1;
    
    c_Data.u8_Version = v_Buffer[us_Pos];
    
    return c_Data;
}

template<> C_MSG_AUTH_PROOF_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != NetMessage::us_BufferSize)
    {
        throw NetException("Invalid data buffer!");
    }
    
    C_MSG_AUTH_PROOF_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    memcpy(&(c_Data.p_NonceHash[0]),
           &(v_Buffer[us_Pos]),
           us_SizeNonceHash);
    us_Pos += us_SizeNonceHash;
    
    return c_Data;
}

template<> C_MSG_CHANNEL_REQUEST_DATA NetMessageV1::ToData(std::vector<uint8_t> const& v_Buffer)
{
    if (v_Buffer.size() != NetMessage::us_BufferSize)
    {
        throw NetException("Invalid data buffer!");
    }
    
    C_MSG_CHANNEL_REQUEST_DATA c_Data;
    size_t us_Pos = NetMessage::us_IDPos + NetMessage::us_IDSize;
    
    memcpy(&(c_Data.p_Channel[0]),
           &(v_Buffer[us_Pos]),
           us_SizeServerChannel);
    
    return c_Data;
}

//*************************************************************************************
// Buffer Creation
//*************************************************************************************

template<> std::vector<uint8_t> NetMessageV1::ToBuffer(S_MSG_AUTH_CHALLENGE_DATA const& Data)
{
    std::vector<uint8_t> v_Buffer(NetMessage::us_BufferSize, '\0');
    size_t us_Pos = NetMessage::us_IDPos;
    
    v_Buffer[NetMessage::us_IDPos] = NetMessage::S_MSG_AUTH_CHALLENGE;
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

template<> std::vector<uint8_t> NetMessageV1::ToBuffer(S_MSG_AUTH_RESULT_DATA const& Data)
{
    std::vector<uint8_t> v_Buffer(NetMessage::us_BufferSize, '\0');
    size_t us_Pos = NetMessage::us_IDPos;
    
    v_Buffer[NetMessage::us_IDPos] = NetMessage::S_MSG_AUTH_RESULT;
    us_Pos += NetMessage::us_IDSize;
    
    v_Buffer[us_Pos] = Data.u8_Result;
    
    return v_Buffer;
}

template<> std::vector<uint8_t> NetMessageV1::ToBuffer(S_MSG_CHANNEL_RESPONSE_DATA const& Data)
{
    std::vector<uint8_t> v_Buffer(NetMessage::us_BufferSize, '\0');
    size_t us_Pos = NetMessage::us_IDPos;
    
    v_Buffer[NetMessage::us_IDPos] = NetMessage::S_MSG_CHANNEL_RESPONSE;
    us_Pos += NetMessage::us_IDSize;
    
    memcpy(&(v_Buffer[us_Pos]),
           &(Data.p_Channel[0]),
           us_SizeServerChannel);
    us_Pos += us_SizeServerChannel;
    
    memcpy(&(v_Buffer[us_Pos]),
           &(Data.p_Address[0]),
           us_SizeServerAddress);
    us_Pos += us_SizeServerAddress;
    
    if (IS_BIG_ENDIAN)
    {
        uint32_t u32_Port = bswap_32(Data.u32_Port);
    
        memcpy(&(v_Buffer[us_Pos]),
               &u32_Port,
               sizeof(u32_Port));
    }
    else
    {
        memcpy(&(v_Buffer[us_Pos]),
               &(Data.u32_Port),
               sizeof(Data.u32_Port));
    }
    
    us_Pos += sizeof(Data.u32_Port);
    
    v_Buffer[us_Pos] = Data.u8_Result;
    
    return v_Buffer;
}
