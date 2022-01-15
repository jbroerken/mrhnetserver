/**
 *  CLI.cpp
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
#include <string.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

// External
#include <mysqlx/xdevapi.h>
#include <sodium.h>

// Project
#include "./CLI.h"
#include "./Logger.h"
#include "./Database/DatabaseTable.h"

// Pre-defined
namespace
{
    std::thread c_Thread;
    std::atomic<bool> b_Run(false);
    
    enum CLICommand
    {
        CREATE_ACCOUNT = 0,
        REMOVE_ACCOUNT = 1,
        ADD_DEVICE = 2,
        REMOVE_DEVICE = 3,
        
        CLI_COMMAND_MAX = REMOVE_DEVICE,
        
        CLI_COMMAND_COUNT = CLI_COMMAND_MAX + 1
    };
    
    const char* p_CLICommand[CLI_COMMAND_COUNT] =
    {
        "createaccount",
        "removeaccount",
        "adddevice",
        "removedevice"
    };
    
    std::string s_MySQLAddress("");
    int i_MySQLPort(-1);
    std::string s_MySQLUser("");
    std::string s_MySQLPassword("");
    std::string s_MySQLDatabase("");
}

//*************************************************************************************
// Account
//*************************************************************************************

static void CreateAccount(std::string const& s_Mail, std::string const& s_Password) noexcept
{
    if (s_Mail.size() == 0 || s_Password.size() == 0)
    {
        Logger::Singleton().Log(Logger::ERROR, "Invalid create account params",
                                "CLI.cpp", __LINE__);
        return;
    }
    else if (s_Password.size() > crypto_secretbox_KEYBYTES)
    {
        Logger::Singleton().Log(Logger::ERROR, "Invalid passwort length, maximum length is " +
                                               std::to_string(crypto_secretbox_KEYBYTES),
                                "CLI.cpp", __LINE__);
        return;
    }
    
    // Create a password hash with a random salt
    char p_Password[crypto_secretbox_KEYBYTES] = { '\0' };
    unsigned char p_Key[crypto_box_SEEDBYTES] = { '\0' };
    unsigned char p_Salt[crypto_pwhash_SALTBYTES] = { '\0' };
    
    strncpy(p_Password, s_Password.c_str(), s_Password.size());
    randombytes_buf(p_Salt, crypto_pwhash_SALTBYTES);
    
    if (crypto_pwhash(p_Key,
                      crypto_box_SEEDBYTES,
                      p_Password,
                      crypto_secretbox_KEYBYTES,
                      p_Salt,
                      crypto_pwhash_OPSLIMIT_INTERACTIVE,
                      128 * 1024 * 1024,//crypto_pwhash_argon2id_MEMLIMIT_SENSITIVE,
                      crypto_pwhash_ALG_ARGON2ID13) != 0)
    {
        Logger::Singleton().Log(Logger::ERROR, "Failed to hash password",
                                "CLI.cpp", __LINE__);
        return;
    }
    
    // Now we create a base64 string for storing byte data
    size_t us_FullLen = crypto_pwhash_SALTBYTES + crypto_box_SEEDBYTES;
    unsigned char p_Full[us_FullLen];
    
    memcpy(p_Full, p_Salt, crypto_pwhash_SALTBYTES);
    memcpy(&(p_Full[crypto_pwhash_SALTBYTES]), p_Key, crypto_box_SEEDBYTES);
    
    size_t us_B64Len = sodium_base64_ENCODED_LEN(us_FullLen, sodium_base64_VARIANT_ORIGINAL);
    char p_B64[us_B64Len];
    
    memset(p_B64, '\0', us_B64Len);
    
    char* p_B64Start = sodium_bin2base64(p_B64, us_B64Len,
                                         p_Full, us_FullLen,
                                         sodium_base64_VARIANT_ORIGINAL);
    
    if (p_B64Start == NULL)
    {
        Logger::Singleton().Log(Logger::ERROR, "Failed to create base64 string",
                                "CLI.cpp", __LINE__);
        return;
    }
    
    // Got base 64 password, now insert
    try
    {
        mysqlx::Session c_Session(s_MySQLAddress,
                                  i_MySQLPort,
                                  s_MySQLUser,
                                  s_MySQLPassword);
        
        mysqlx::Table c_Table = c_Session.getSchema(s_MySQLDatabase)
                                         .getTable(DatabaseTable::p_UATableName);
        
        c_Table.insert(DatabaseTable::p_UAFieldName[DatabaseTable::UA_MAIL_ADDRESS],
                       DatabaseTable::p_UAFieldName[DatabaseTable::UA_PASSWORD])
               .values(s_Mail,
                       p_B64)
               .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Account insertion to database failed: " +
                                               std::string(e.what()),
                                "CLI.cpp", __LINE__);
    }
}

static void RemoveAccount(std::string const& s_UserID) noexcept
{
    try
    {
        uint32_t u32_UserID = std::stoull(s_UserID);
        
        mysqlx::Session c_Session(s_MySQLAddress,
                                  i_MySQLPort,
                                  s_MySQLUser,
                                  s_MySQLPassword);
        
        mysqlx::Table c_Table = c_Session.getSchema(s_MySQLDatabase)
                                         .getTable(DatabaseTable::p_UATableName);
        
        
        c_Table.remove()
               .where(std::string(DatabaseTable::p_UAFieldName[DatabaseTable::UA_USER_ID]) +
                      " == :value")
               .bind("value",
                     u32_UserID)
               .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Account removal from database failed: " +
                                               std::string(e.what()),
                                "CLI.cpp", __LINE__);
    }
}

//*************************************************************************************
// Device
//*************************************************************************************

static void AddDevice(std::string const& s_UserID, std::string const& s_DeviceKey) noexcept
{
    try
    {
        uint32_t u32_UserID = std::stoull(s_UserID);
        
        mysqlx::Session c_Session(s_MySQLAddress,
                                  i_MySQLPort,
                                  s_MySQLUser,
                                  s_MySQLPassword);
        
        mysqlx::Table c_Table = c_Session.getSchema(s_MySQLDatabase)
                                         .getTable(DatabaseTable::p_UDLTableName);
        
        c_Table.insert(DatabaseTable::p_UDLFieldName[DatabaseTable::UDL_USER_ID],
                       DatabaseTable::p_UDLFieldName[DatabaseTable::UDL_DEVICE_KEY])
               .values(u32_UserID,
                       s_DeviceKey)
               .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Device insertion to database failed: " +
                                               std::string(e.what()),
                                "CLI.cpp", __LINE__);
    }
}

static void RemoveDevice(std::string const& s_UserID, std::string const& s_DeviceKey) noexcept
{
    try
    {
        uint32_t u32_UserID = std::stoull(s_UserID);
        
        mysqlx::Session c_Session(s_MySQLAddress,
                                  i_MySQLPort,
                                  s_MySQLUser,
                                  s_MySQLPassword);
        
        mysqlx::Table c_Table = c_Session.getSchema(s_MySQLDatabase)
                                         .getTable(DatabaseTable::p_UDLTableName);
        
        
        c_Table.remove()
               .where(std::string(DatabaseTable::p_UDLFieldName[DatabaseTable::UDL_USER_ID]) +
                      " == :valueA AND " +
                      std::string(DatabaseTable::p_UDLFieldName[DatabaseTable::UDL_DEVICE_KEY]) +
                      " == :valueB")
               .bind("valueA",
                     u32_UserID)
               .bind("valueB",
                     s_DeviceKey)
               .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Device removal from database failed: " +
                                               std::string(e.what()),
                                "CLI.cpp", __LINE__);
    }
}

//*************************************************************************************
// Run
//*************************************************************************************

static std::vector<std::string> GetCLICommand(std::string s_Input)
{
    std::vector<std::string> v_Result;
    size_t us_Pos;
    
    while ((us_Pos = s_Input.find(' ')) != std::string::npos)
    {
        v_Result.emplace_back(s_Input.substr(0, us_Pos));
        s_Input.erase(0, us_Pos + 1);
    }
    
    // Add last part
    if (s_Input.size() > 0)
    {
        v_Result.emplace_back(s_Input);
    }
    
    return v_Result;
}

static void UpdateCLI() noexcept
{
    Logger& c_Logger = Logger::Singleton();
    
    std::string s_Input;
    std::vector<std::string> v_Command;
    
    while (b_Run == true)
    {
        std::getline(std::cin, s_Input);
    
        if (s_Input.size() == 0)
        {
            continue;
        }
        
        std::vector<std::string> v_Command = GetCLICommand(s_Input);
        
        if (v_Command.size() == 0)
        {
            continue;
        }
        else if (v_Command[0].compare(p_CLICommand[CREATE_ACCOUNT]) == 0 && v_Command.size() == 3)
        {
            CreateAccount(v_Command[1], v_Command[2]);
        }
        else if (v_Command[0].compare(p_CLICommand[REMOVE_ACCOUNT]) == 0 && v_Command.size() == 2)
        {
            RemoveAccount(v_Command[1]);
        }
        else if (v_Command[0].compare(p_CLICommand[ADD_DEVICE]) == 0 && v_Command.size() == 3)
        {
            AddDevice(v_Command[1], v_Command[2]);
        }
        else if (v_Command[0].compare(p_CLICommand[REMOVE_DEVICE]) == 0 && v_Command.size() == 3)
        {
            RemoveDevice(v_Command[1], v_Command[2]);
        }
        else
        {
            c_Logger.Log(Logger::WARNING, "Unknown command",
                         "CLI.cpp", __LINE__);
        }
        
        s_Input = "";
    }
}

void CLI::Start(Configuration& c_Configuration) noexcept
{
    if (b_Run == true)
    {
        return;
    }
    
    s_MySQLAddress = c_Configuration.s_MySQLAddress;
    i_MySQLPort = c_Configuration.i_MySQLPort;
    s_MySQLUser = c_Configuration.s_MySQLUser;
    s_MySQLPassword = c_Configuration.s_MySQLPassword;
    s_MySQLDatabase = c_Configuration.s_MySQLDatabase;
    
    try
    {
        b_Run = true;
        c_Thread = std::thread(UpdateCLI);
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, "Failed to start CLI thread: " +
                                               std::string(e.what()),
                                "CLI.cpp", __LINE__);
    }
}

void CLI::Stop() noexcept
{
    if (b_Run == false)
    {
        return;
    }
    
    b_Run = false;
    c_Thread.join();
}
