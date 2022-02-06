/**
 *  StreamContext.h
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

#ifndef StreamContext_h
#define StreamContext_h

// C / C++
#include <atomic>
#include <vector>
#include <memory>

// External
#include <msquic.h>

// Project
#include "./StreamData.h"
#include "../../Job/JobList.h"
#include "../Client.h"


struct StreamContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     *  \param p_Connection The connection for this stream.
     *  \param p_Client The client to recieve and send messages.
     *  \param c_JobList The job list to hand to the connection.
     */
    
    StreamContext(const QUIC_API_TABLE* p_APITable,
                  HQUIC p_Connection,
                  std::shared_ptr<Client>& p_Client,
                  JobList& c_JobList) noexcept : p_APITable(p_APITable),
                                                 p_Connection(p_Connection),
                                                 p_Client(p_Client),
                                                 c_JobList(c_JobList)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Connection;
    
    std::shared_ptr<Client> p_Client;
    JobList& c_JobList;
    
    StreamData c_Data;
};

#endif /* StreamContext_h */
