/**
 *  Base64.cpp
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
#include "./Base64.h"


//*************************************************************************************
// To Bytes
//*************************************************************************************

std::string Base64::ToBytes(std::string const& s_Base64) noexcept
{
    // Base64 is 1.25 bigger
    size_t us_FullLength = (s_Base64.size() / 4) * 3;
    
    if (us_FullLength == 0)
    {
        return "";
    }
    
    unsigned char p_Bin[us_FullLength];
    size_t us_Length = 0;
    
    if (sodium_base642bin(p_Bin, us_FullLength,
                          s_Base64.data(), s_Base64.size(),
                          NULL, &us_Length,
                          NULL,
                          sodium_base64_VARIANT_ORIGINAL) != 0)
    {
        return "";
    }
    else if (us_Length == 0)
    {
        return "";
    }
    
    return std::string(p_Bin,
                       p_Bin + us_Length);
}

std::string Base64::ToBytesPart(std::string const& s_Base64, size_t us_Pos, size_t us_Length) noexcept
{
    std::string s_Result = ToBytes(s_Base64);
    
    if (s_Result.size() < (us_Pos + us_Length))
    {
        return "";
    }
    
    return s_Result.substr(us_Pos, us_Length);
}

//*************************************************************************************
// To String
//*************************************************************************************

std::string Base64::ToString(std::vector<uint8_t> const& v_Bytes) noexcept
{
    return ToString(v_Bytes.data(), v_Bytes.size());
}

std::string Base64::ToString(const uint8_t* p_Bytes, size_t us_Length) noexcept
{
    size_t us_Base64Len = sodium_base64_encoded_len(us_Length, sodium_base64_VARIANT_ORIGINAL);
    char p_Base64[us_Base64Len];
    
    p_Base64[us_Base64Len - 1] = '\0';
    
    char* p_Result = sodium_bin2base64(p_Base64, us_Base64Len,
                                       p_Bytes, us_Length,
                                       sodium_base64_VARIANT_ORIGINAL);
    
    if (p_Result == NULL)
    {
        return "";
    }
    
    // @NOTE: Terminator included
    return std::string(p_Result);
}
