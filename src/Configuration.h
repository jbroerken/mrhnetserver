/**
 *  Configuration.h
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

#ifndef Configuration_h
#define Configuration_h

// C / C++
#include <cstdint>
#include <string>

// External

// Project
#include "./ServerException.h"


class Configuration
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param s_FilePath The full path to the configuration file.
     */
    
    Configuration(std::string const& s_FilePath);
    
    /**
     *  Default destructor.
     */
    
    ~Configuration() noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Shared
    ActorType e_Type;
    
    int i_Port;
    
    std::string s_CertFilePath;
    std::string s_KeyFilePath;
    
    int i_MaxClientCount;
    
    int i_ConnectionTimeoutS;
    int i_ConnectionPulseMS;
    int i_MessagePulseMS;
    
    // MySQL
    std::string s_MySQLAddress;
    int i_MySQLPort;
    std::string s_MySQLUser;
    std::string s_MySQLPassword;
    std::string s_MySQLDatabase;
    
    // Connection Server
    
    // Communication Server
    int i_ChannelID;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:

};

#endif /* Configuration_h */
