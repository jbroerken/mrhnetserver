/**
 *  Base64.h
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

#ifndef Base64_h
#define Base64_h

// C / C++
#include <cstdint>
#include <string>

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
}

#endif /* Base64_h */
