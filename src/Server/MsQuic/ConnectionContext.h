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
