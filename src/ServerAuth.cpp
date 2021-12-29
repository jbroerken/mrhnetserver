/**
 *  ServerAuth.cpp
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

// External

// Project
#include "./ServerAuth.h"
#include "./NetMessage/Ver/NetMessageV1.h"


//*************************************************************************************
// Base64
//*************************************************************************************

static inline bool ExtractValue(std::string const& s_Base64, std::string& s_Value, size_t us_Pos, size_t us_Length) noexcept
{
    size_t us_FullLength = crypto_pwhash_SALTBYTES + crypto_box_SEEDBYTES;
    
    if (s_Base64.size() < us_FullLength * 1.25f) // Base64 is 1.25 bigger
    {
        return false;
    }
    
    unsigned char p_Binary[us_FullLength];
    
    if (sodium_base642bin(p_Binary, us_FullLength,
                          s_Base64.data(), s_Base64.size(),
                          NULL, NULL,
                          NULL,
                          sodium_base64_VARIANT_ORIGINAL) != 0)
    {
        return false;
    }
    
    s_Value = std::string(us_Length, '\0');
    memcpy(s_Value.data(), &(p_Binary[us_Pos]), us_Length);
    
    return true;
}

bool ServerAuth::ExtractPassword(std::string const& s_Base64, std::string& s_Password) noexcept
{
    return ExtractValue(s_Base64, s_Password, crypto_pwhash_SALTBYTES, crypto_box_SEEDBYTES);
}

bool ServerAuth::ExtractSalt(std::string const& s_Base64, std::string& s_Salt) noexcept
{
    return ExtractValue(s_Base64, s_Salt, 0, crypto_pwhash_SALTBYTES);
}

//*************************************************************************************
// Nonce
//*************************************************************************************

bool ServerAuth::CompareNonce(uint32_t u32_Nonce, const uint8_t* p_EncryptedNonce, const char* p_Key) noexcept
{
    if (p_EncryptedNonce == NULL || p_Key == NULL)
    {
        return false;
    }
    
    uint32_t u32_RecievedNonce;
    
    // Grab nonce from nonce encryption first first
    unsigned char p_FullNonce[crypto_secretbox_NONCEBYTES] = { '\0' };
    memcpy(p_FullNonce, p_EncryptedNonce, crypto_secretbox_NONCEBYTES);
    
    if (crypto_secretbox_open_easy((unsigned char*)&u32_RecievedNonce,
                                   (const unsigned char*)&(p_EncryptedNonce[crypto_secretbox_NONCEBYTES]),
                                   crypto_secretbox_MACBYTES + sizeof(uint32_t),
                                   (const unsigned char*)p_FullNonce,
                                   (const unsigned char*)p_Key) != 0)
    {
        return false;
    }
    
    return u32_RecievedNonce == u32_Nonce;
}
