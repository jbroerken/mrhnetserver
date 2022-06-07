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

#ifndef Base64_h
#define Base64_h

// C / C++
#include <cstdint>
#include <string>
#include <vector>

// External

// Project


namespace Base64
{
    //*************************************************************************************
    // To Bytes
    //*************************************************************************************
    
    /**
     *  Decode a given base64 string to bytes.
     *
     *  \param s_Base64 The base64 string to decode.
     *
     *  \return The byte result on success, a empty string on failure.
     */
    
    std::string ToBytes(std::string const& s_Base64) noexcept;
    
    /**
     *  Get a part of bytes from a given base64 string.
     *
     *  \param s_Base64 The base64 string to decode.
     *
     *  \return The byte result on success, a empty string on failure.
     */
    
    std::string ToBytesPart(std::string const& s_Base64, size_t us_Pos, size_t us_Length) noexcept;
    
    //*************************************************************************************
    // To String
    //*************************************************************************************
    
    /**
     *  Encode bytes in a base64 string.
     *
     *  \param v_Bytes The bytes to encode.
     *
     *  \return The base64 string on result, false if not.
     */
    
    std::string ToString(std::vector<uint8_t> const& v_Bytes) noexcept;
    
    /**
     *  Encode bytes in a base64 string.
     *
     *  \param p_Bytes The bytes to encode.
     *  \param us_Length The length of the bytes to encode.
     *
     *  \return The base64 string on result, false if not.
     */
    
    std::string ToString(const uint8_t* p_Bytes, size_t us_Length) noexcept;
}

#endif /* Base64_h */
