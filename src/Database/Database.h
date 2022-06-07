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

#ifndef Database_h
#define Database_h

// C / C++

// External
#include <mysqlx/xdevapi.h>

// Project
#include "../Job/ThreadShared.h"
#include "./DatabaseTable.h"


class Database : public ThreadShared
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param s_Address The mysql server address.
     *  \param i_Port The mysql server port.
     *  \param s_User The name of the mysql user.
     *  \param s_Password The password of the mysql user.
     *  \param s_Database The database to use.
     */
    
    Database(std::string const& s_Address,
             int i_Port,
             std::string const& s_User,
             std::string const& s_Password,
             std::string const& s_Database) : ThreadShared(),
                                              c_Session(s_Address,
                                                        i_Port,
                                                        s_User,
                                                        s_Password),
                                              s_Database(s_Database)
    {}
    
    /**
     *  Default destructor.
     */
    
    ~Database() noexcept
    {}
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    mysqlx::Session c_Session;
    
    std::string s_Database;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:
    
};

#endif /* Database_h */
