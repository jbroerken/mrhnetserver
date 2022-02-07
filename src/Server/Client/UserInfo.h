/**
 *  UserInfo.h
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

#ifndef UserInfo_h
#define UserInfo_h

// C / C++
#include <cstdint>
#include <string>

// External

// Project


//*************************************************************************************
// Client Type
//*************************************************************************************

typedef enum
{
    CLIENT_APP = 0,
    CLIENT_PLATFORM = 1,
        
    CLIENT_TYPE_MAX = CLIENT_PLATFORM,
        
    CLIENT_TYPE_COUNT = CLIENT_TYPE_MAX + 1
    
}ClientType;

//*************************************************************************************
// User Info
//*************************************************************************************

struct UserInfo
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    UserInfo() noexcept : u32_UserID(0),
                          s_DeviceKey(""),
                          u8_ClientType(0),
                          b_Authenticated(false),
                          s_Password(""),
                          u32_Nonce(0)
    {}
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    uint32_t u32_UserID;
    std::string s_DeviceKey;
    uint8_t u8_ClientType;
    
    bool b_Authenticated;
    std::string s_Password;
    uint32_t u32_Nonce;
};


#endif /* UserInfo_h */
