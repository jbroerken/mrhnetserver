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
#include <cstring>
#include <cerrno>
#include <fstream>

// External

// Project
#include "./Configuration.h"

// Pre-defined
namespace
{
    enum Identifier
    {
        // Server
        PORT = 0,
        CERT_FILE_PATH = 1,
        KEY_FILE_PATH = 2,
        MAX_CLIENT_COUNT = 3,
        CONNECTION_TIMEOUT_S = 4,
        
        // MySQL
        MYSQL_ADDRESS = 5,
        MYSQL_PORT = 6,
        MYSQL_USER = 7,
        MYSQL_PASSWORD,
        MYSQL_DATABASE,
        
        // Bounds
        IDENTIFIER_MAX = MYSQL_DATABASE,
        
        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };
    
    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Server
        "ServerPort=",
        "ServerCertFilePath=",
        "ServerKeyFilePath=",
        "ServerMaxClientCount=",
        "ServerConnectionTimeoutS=",
        
        // MySQL
        "MySQLAddress=",
        "MySQLPort=",
        "MySQLUser=",
        "MySQLPassword=",
        "MySQLDatabase="
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration(std::string const& s_FilePath) : i_Port(-1),
                                                              s_CertFilePath("/usr/share/mrhnetserver/cert.crt"),
                                                              s_KeyFilePath("/usr/share/mrhnetserver/key.key"),
                                                              i_MaxClientCount(1024),
                                                              i_ConnectionTimeoutS(60),
                                                              s_MySQLAddress("localhost"),
                                                              i_MySQLPort(33060),
                                                              s_MySQLUser("user"),
                                                              s_MySQLPassword(""),
                                                              s_MySQLDatabase("mrhnetserver")
{
    std::ifstream f_File(s_FilePath);
    std::string s_Line;
    std::string s_File;
    
    if (f_File.is_open() == false)
    {
        throw Exception("Could not open file " +
                        s_FilePath +
                        " for reading: " +
                        std::string(std::strerror(errno)) +
                        " (" +
                        std::to_string(errno) +
                        ")");
    }
    
    while (getline(f_File, s_Line))
    {
        // Skip invalid
        if (s_Line.size() == 0 || s_Line[0] == '#' || s_Line[0] == '\n' || s_Line[0] == '\r')
        {
            continue;
        }
        
        // Find identifier
        for (size_t i = 0; i < IDENTIFIER_COUNT; ++i)
        {
            size_t us_IdentifierLen = std::strlen(p_Identifier[i]);
            
            if (s_Line.size() <= us_IdentifierLen ||
                s_Line.substr(0, us_IdentifierLen).compare(p_Identifier[i]) != 0)
            {
                continue;
            }
            
            s_Line = s_Line.substr(us_IdentifierLen);
            
            try
            {
                switch (i)
                {
                    // Server
                    case PORT:
                        i_Port = std::stoi(s_Line);
                        break;
                    case CERT_FILE_PATH:
                        s_CertFilePath = s_Line;
                        break;
                    case KEY_FILE_PATH:
                        s_KeyFilePath = s_Line;
                        break;
                    case MAX_CLIENT_COUNT:
                        i_MaxClientCount = std::stoi(s_Line);
                        break;
                    case CONNECTION_TIMEOUT_S:
                        i_ConnectionTimeoutS = std::stoi(s_Line);
                        break;
                        
                    // MySQL
                    case MYSQL_ADDRESS:
                        s_MySQLAddress = s_Line;
                        break;
                    case MYSQL_PORT:
                        i_MySQLPort = std::stoi(s_Line);
                        break;
                    case MYSQL_USER:
                        s_MySQLUser = s_Line;
                        break;
                    case MYSQL_PASSWORD:
                        s_MySQLPassword = s_Line;
                        break;
                    case MYSQL_DATABASE:
                        s_MySQLDatabase = s_Line;
                        break;
                        
                    // Unknown
                    default:
                        break;
                }
            }
            catch (...)
            {}
            
            // Done for this identifier
            break;
        }
    }
    
    f_File.close();
}

Configuration::~Configuration() noexcept
{}
