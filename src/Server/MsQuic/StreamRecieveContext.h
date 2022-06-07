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

#ifndef StreamRecieveContext_h
#define StreamRecieveContext_h

// C / C++
#include <atomic>
#include <vector>
#include <memory>

// External
#include <msquic.h>

// Project
#include "./StreamData.h"
#include "../../Job/JobList.h"
#include "../ClientPool.h"


struct StreamRecieveContext
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
     *  \param c_ClientPool The client pool containing all clients.
     *  \param us_ClientID The id of the client which recieves.
     */
    
    StreamRecieveContext(const QUIC_API_TABLE* p_APITable,
                         HQUIC p_Connection,
                         ClientPool& c_ClientPool,
                         size_t us_ClientID) noexcept : p_APITable(p_APITable),
                                                        p_Connection(p_Connection),
                                                        c_ClientPool(c_ClientPool),
                                                        us_ClientID(us_ClientID)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Connection;
    
    ClientPool& c_ClientPool;
    size_t us_ClientID;
    
    StreamData c_Data;
};

#endif /* StreamRecieveContext_h */
