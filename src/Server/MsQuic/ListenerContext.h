/**
 *  ListenerContext.h
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

#ifndef ListenerContext_h
#define ListenerContext_h

// C / C++

// External
#include <msquic.h>

// Project
#include "../ClientPool.h"
#include "./ClientConnections.h"


struct ListenerContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     *  \param p_Configuration The library configuration.
     *  \param c_ClientPool The client pool to hand to connections.
     *  \param i_ClientConnectionsMax The max number of clients which can connect.
     */
    
    ListenerContext(const QUIC_API_TABLE* p_APITable,
                    HQUIC p_Configuration,
                    ClientPool& c_ClientPool,
                    int i_ClientConnectionsMax) noexcept : p_APITable(p_APITable),
                                                           p_Configuration(p_Configuration),
                                                           c_ClientPool(c_ClientPool),
                                                           c_Connections(i_ClientConnectionsMax)
    {}
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Configuration;
    
    ClientPool& c_ClientPool;
    ClientConnections c_Connections;
};

#endif /* ListenerContext_h */
