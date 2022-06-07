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

#ifndef Configuration_h
#define Configuration_h

// C / C++
#include <cstdint>
#include <string>

// External

// Project
#include "./Exception.h"


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
    int i_Port;
    std::string s_CertFilePath;
    std::string s_KeyFilePath;
    int i_MaxClientCount;
    int i_ConnectionTimeoutS;
    
    // MySQL
    std::string s_MySQLAddress;
    int i_MySQLPort;
    std::string s_MySQLUser;
    std::string s_MySQLPassword;
    std::string s_MySQLDatabase;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:

};

#endif /* Configuration_h */
