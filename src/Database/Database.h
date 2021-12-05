/**
 *  Database.h
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

#ifndef Database_h
#define Database_h

// C / C++

// External
#include <mysqlx/xdevapi.h>

// Project
#include "../WorkerPool/WorkerShared.h"
#include "./DatabaseTable.h"


class Database : public WorkerShared
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
             std::string const& s_Database);
    
    /**
     *  Default destructor.
     */
    
    ~Database() noexcept;
    
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
