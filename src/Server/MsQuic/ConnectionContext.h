/**
 *  ConnectionContext.h
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

#ifndef ConnectionContext_h
#define ConnectionContext_h

// C / C++
#include <memory>

// External
#include <msquic.h>

// Project
#include "./StreamRecieveContext.h"
#include "./ClientConnections.h"
#include "../Client.h"


struct ConnectionContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     *  \param p_Connection The connection handle to manage.
     *  \param c_ClientPool The client pool to store the client in.
     *  \param c_Connections The client connections information.
     */
    
    ConnectionContext(const QUIC_API_TABLE* p_APITable,
                      HQUIC p_Connection,
                      ClientPool& c_ClientPool,
                      ClientConnections& c_Connections) : p_APITable(p_APITable),
                                                          p_Connection(p_Connection),
                                                          c_ClientPool(c_ClientPool),
                                                          c_Connections(c_Connections)
    {
        try
        {
            us_ClientID = c_ClientPool.AddClient(p_APITable,
                                                 p_Connection);
        }
        catch (...)
        {
            throw;
        }
    }
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Connection;
    
    ClientPool& c_ClientPool;
    ClientConnections& c_Connections;
    
    size_t us_ClientID;
    std::list<StreamRecieveContext> l_Recieved;
};

#endif /* ConnectionContext_h */
