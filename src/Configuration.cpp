/**
 *  Configuration.cpp
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
