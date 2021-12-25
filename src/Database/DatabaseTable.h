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
    
    //*************************************************************************************
    // Channel List Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_CLTableName = "channel_list";
    
    /**
     *  Field Names
     */
    
    enum CLFields
    {
        CL_CHANNEL_ID = 0,
        CL_NAME = 1,
        
        CL_FIELDS_MAX = CL_NAME,
        CL_FIELDS_COUNT = CL_FIELDS_MAX + 1
    };
    
    constexpr const char* p_CLFieldName[CL_FIELDS_COUNT] =
    {
        "channel_id",
        "name"
    };
    
    /**
     *  Row Data
     */
    
    struct CLRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_ChannelID;
        std::string s_Identifier;
    };
    
    //*************************************************************************************
    // Active Channels Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_ACTableName = "active_channels";
    
    /**
     *  Field Names
     */
    
    enum ACFields
    {
        AC_CHANNEL_ID = 0,
        AC_ADDRESS = 1,
        AC_PORT = 2,
        AC_CONNECTIONS = 3,
        AC_CONNECTION_LIST_ID = 4,
        AC_LAST_UPDATE = 5,
        
        AC_FIELDS_MAX = AC_LAST_UPDATE,
        AC_FIELDS_COUNT = AC_FIELDS_MAX + 1
    };
    
    constexpr const char* p_ACFieldName[AC_FIELDS_COUNT] =
    {
        "channel_id",
        "address",
        "port",
        "connections",
        "connection_list_id",
        "last_update_s"
    };
    
    /**
     *  Row Data
     */
    
    struct ACRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_ChannelID;
        std::string s_Address;
        int32_t s32_Port;
        uint32_t u32_Connections;
        uint32_t u32_ConnectionListID;
        uint64_t u64_LastUpdate;
    };
    
    //*************************************************************************************
    // Channel Device Connections Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_CDCTableName = "channel_device_connections";
    
    /**
     *  Field Names
     */
    
    enum CDCFields
    {
        CDC_CONNECTION_LIST_ID = 0,
        CDC_CHANNEL_ID = 1,
        CDC_USER_UD = 2,
        CDC_DEVICE_KEY = 3,
        
        CDC_FIELDS_MAX = CDC_DEVICE_KEY,
        CDC_FIELDS_COUNT = CDC_FIELDS_MAX + 1
    };
    
    constexpr const char* p_CDCFieldName[CDC_FIELDS_COUNT] =
    {
        "connection_list_id",
        "channel_id",
        "user_id",
        "device_key"
    };
    
    /**
     *  Row Data
     */
    
    struct CDCRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_ConnectionListID;
        uint32_t u32_ChannelID;
        uint32_t u32_UserID;
        std::string s_DeviceKey;
    };
}

#endif /* DatabaseTable_h */
