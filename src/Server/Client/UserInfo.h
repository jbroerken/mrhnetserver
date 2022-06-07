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
