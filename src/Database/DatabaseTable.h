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
    
    constexpr const char* p_SLTableName = "server_list";
    
    /**
     *  Field Names
     */
    
    enum SLFields
    {
        SL_SERVER_ID = 0,
        SL_CHANNEL = 1,
        SL_ADDRESS = 2,
        SL_PORT = 3,
        SL_ASSISTANT_CONNECTIONS = 4,
        SL_MAX_CONNECTIONS = 5,
        SL_LAST_UPDATE = 6,
        
        SL_FIELDS_MAX = SL_LAST_UPDATE,
        SL_FIELDS_COUNT = SL_FIELDS_MAX + 1
    };
    
    constexpr const char* p_SLFieldName[SL_FIELDS_COUNT] =
    {
        "server_id",
        "channel",
        "address",
        "port",
        "assistant_connections",
        "max_connections",
        "last_update_s"
    };
    
    /**
     *  Row Data
     */
    
    struct SLRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_ServerID;
        std::string s_Name;
        std::string s_Address;
        int32_t s32_Port;
        uint32_t u32_AssistantConnections;
        uint32_t u32_MaxConnections;
        uint64_t u64_LastUpdate;
    };
    
    //*************************************************************************************
    // Server Platform Connections Table
    //*************************************************************************************
    
    /**
     *  Table Name
     */
    
    constexpr const char* p_SPCTableName = "server_platform_connections";
    
    /**
     *  Field Names
     */
    
    enum SPCFields
    {
        SPC_SERVER_ID = 0,
        SPC_USER_ID = 1,
        SPC_DEVICE_KEY = 2,
        
        SPC_FIELDS_MAX = SPC_DEVICE_KEY,
        SPC_FIELDS_COUNT = SPC_FIELDS_MAX + 1
    };
    
    constexpr const char* p_SPCFieldName[SPC_FIELDS_COUNT] =
    {
        "server_id",
        "user_id",
        "device_key"
    };
    
    /**
     *  Row Data
     */
    
    struct SPCRow
    {
    public:
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        uint32_t u32_ServerID;
        uint32_t u32_UserID;
        std::string s_DeviceKey;
    };
}

#endif /* DatabaseTable_h */
