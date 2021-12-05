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
        // Shared
        TYPE = 0,
        PORT = 1,
        CERT_FILE_PATH = 2,
        KEY_FILE_PATH = 3,
        CONNECTION_TIMEOUT_S = 4,
        CONNECTION_PULSE_MS = 5,
        MESSAGE_PULS_MS = 6,
        
        // MySQL
        MYSQL_ADDRESS = 7,
        MYSQL_PORT,
        MYSQL_USER,
        MYSQL_PASSWORD,
        MYSQL_DATABASE,
        
        // Connection Server
        
        // Communication Server
        CHANNEL_NAME,
        
        // Bounds
        IDENTIFIER_MAX = CHANNEL_NAME,
        
        IDENTIFIER_COUNT = IDENTIFIER_MAX + 1
    };
    
    const char* p_Identifier[IDENTIFIER_COUNT] =
    {
        // Shared
        "ServerType=",
        "ServerPort=",
        "ServerCertFilePath=",
        "ServerKeyFilePath=",
        "ServerConnectionTimeoutS=",
        "ServerConnectionPulseMS=",
        "ServerMessagePulseMS=",
        
        // MySQL
        "MySQLAddress=",
        "MySQLPort=",
        "MySQLUser=",
        "MySQLPassword=",
        "MySQLDatabase=",
        
        // Connection Server
        
        // Communication Server
        "ComChannelName="
    };
}


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Configuration::Configuration(std::string const& s_FilePath) : e_Type(ACTOR_TYPE_COUNT),
                                                              i_Port(-1),
                                                              s_CertFilePath("/usr/share/mrhnetserver/cert.crt"),
                                                              s_KeyFilePath("/usr/share/mrhnetserver/key.key"),
                                                              i_ConnectionTimeoutS(60),
                                                              i_ConnectionPulseMS(100),
                                                              i_MessagePulseMS(100),
                                                              s_MySQLAddress("localhost"),
                                                              i_MySQLPort(33060),
                                                              s_MySQLUser("user"),
                                                              s_MySQLPassword(""),
                                                              s_MySQLDatabase("mrhnetserver"),
                                                              s_ChannelName("de.mrh.unknown")
{
    std::ifstream f_File(s_FilePath);
    std::string s_Line;
    std::string s_File;
    
    if (f_File.is_open() == false)
    {
        throw ServerException("Could not open file " +
                              s_FilePath +
                              " for reading: " +
                              std::string(std::strerror(errno)) +
                              " (" +
                              std::to_string(errno) +
                              ")",
                              ACTOR_TYPE_COUNT);
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
                    // Shared
                    case TYPE:
                        e_Type = static_cast<ActorType>(std::stoi(s_Line));
                        break;
                    case PORT:
                        i_Port = std::stoi(s_Line);
                        break;
                    case CERT_FILE_PATH:
                        s_CertFilePath = s_Line;
                        break;
                    case KEY_FILE_PATH:
                        s_KeyFilePath = s_Line;
                        break;
                    case CONNECTION_TIMEOUT_S:
                        i_ConnectionTimeoutS = std::stoi(s_Line);
                        break;
                    case CONNECTION_PULSE_MS:
                        i_ConnectionPulseMS = std::stoi(s_Line);
                        break;
                    case MESSAGE_PULS_MS:
                        i_MessagePulseMS = std::stoi(s_Line);
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
                        
                    // Connection Server
                        
                    // Communication Server
                    case CHANNEL_NAME:
                        s_ChannelName = s_Line;
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
