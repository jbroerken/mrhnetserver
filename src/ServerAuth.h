/**
 *  ServerAuth.h
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

#ifndef ServerAuth_h
#define ServerAuth_h

// C / C++
#include <string>

// External

// Project


namespace ServerAuth
{
    //*************************************************************************************
    // Base64
    //*************************************************************************************
    
    /**
     *  Extract the password hash from a base64 password string.
     *
     *  \param s_Base64 The base 64 string to use.
     *  \param s_Password The password hash to set.
     *
     *  \return true on success, false on failure.
     */
    
    bool ExtractPassword(std::string const& s_Base64, std::string& s_Password) noexcept;
    
    /**
     *  Extract the salt from a base64 password string.
     *
     *  \param s_Base64 The base 64 string to use.
     *  \param s_Salt The salt to set.
     *
     *  \return true on success, false on failure.
     */
    
    bool ExtractSalt(std::string const& s_Base64, std::string& s_Salt) noexcept;
    
    //*************************************************************************************
    // Nonce
    //*************************************************************************************
    
    /**
     *  Compare a nonce with the content of a encrypted nonce.
     *
     *  \param u32_Nonce The nonce to compare with.
     *  \param p_EncryptedNonce The encrypted none. Needs to be of size
     *                          MRH_NetMessageV1::us_SizeNonceHash.
     *  \param p_Key The key to decrypt the nonce with.
     */
    
    bool CompareNonce(uint32_t u32_Nonce, const char* p_EncryptedNonce, const char* p_Key) noexcept;
};

#endif /* ServerAuth_h */
