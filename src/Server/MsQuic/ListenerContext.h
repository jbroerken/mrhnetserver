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
