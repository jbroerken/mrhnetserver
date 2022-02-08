/**
 *  DatabaseTable.h
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

#ifndef DatabaseTable_h
#define DatabaseTable_h

// C / C++
#include <cstdint>
#include <string>

// External

// Project


namespace DatabaseTable
{
    //*************************************************************************************
    // User Account Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_UATableName = "user_account";
    
    /**
     *  Field Names
     */
    
    enum UAFields
    {
        UA_USER_ID = 0,
        UA_MAIL_ADDRESS = 1,
        UA_PASSWORD = 2,
        
        UA_FIELDS_MAX = UA_PASSWORD,
        UA_FIELDS_COUNT = UA_FIELDS_MAX + 1
    };
    
    constexpr const char* p_UAFieldName[UA_FIELDS_COUNT] =
    {
        "user_id",
        "mail_address",
        "password"
    };
    
    /**
     *  Row Data
     */
    
    struct UARow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_UserID;
        std::string s_MailAddress;
        std::string s_Password;
        uint32_t u32_DeviceListID;
    };
    
    constexpr size_t us_UAMailAddresSize = 128;
    constexpr size_t us_UAPasswordSize = 256;
    
    //*************************************************************************************
    // User Device List Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_UDLTableName = "user_device_list";
    
    /**
     *  Field Names
     */
    
    enum UDLFields
    {
        UDL_USER_ID = 0,
        UDL_DEVICE_KEY = 1,
        
        UDL_FIELDS_MAX = UDL_DEVICE_KEY,
        UDL_FIELDS_COUNT = UDL_FIELDS_MAX + 1
    };
    
    constexpr const char* p_UDLFieldName[UDL_FIELDS_COUNT] =
    {
        "user_id",
        "device_key"
    };
    
    /**
     *  Row Data
     */
    
    struct UDLRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_UserID;
        std::string s_DeviceKey;
    };
    
    constexpr size_t us_UDLDeviceKeySize = 25;
    
    //*************************************************************************************
    // Message Data Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_MDTextTableName = "message_text";
    constexpr const char* p_MDLocationTableName = "message_location";
    
    /**
     *  Field Names
     */
    
    enum MDFields
    {
        MD_MESSAGE_ID = 0,
        MD_USER_ID = 1,
        MD_DEVICE_KEY = 2,
        MD_ACTOR_TYPE = 3,
        MD_MESSAGE_DATA = 4,
        
        MD_FIELDS_MAX = MD_MESSAGE_DATA,
        MD_FIELDS_COUNT = MD_FIELDS_MAX + 1
    };
    
    constexpr const char* p_MDFieldName[MD_FIELDS_COUNT] =
    {
        "message_id",
        "user_id",
        "device_key",
        "actor_type",
        "message_data"
    };
    
    /**
     *  Row Data
     */
    
    struct MDRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_MessageID;
        uint32_t u32_UserID;
        std::string s_DeviceKey;
        uint8_t u8_ActorType;
        std::string s_MessageData;
    };
    
    constexpr size_t us_MDDeviceKeySize = 25;
    constexpr size_t us_MDMessageDataSize = 2048;
}

#endif /* DatabaseTable_h */
