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
