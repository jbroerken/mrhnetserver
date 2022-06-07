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

#ifndef StreamSendContext_h
#define StreamSendContext_h

// C / C++

// External
#include <msquic.h>

// Project
#include "./StreamData.h"


struct StreamSendContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     */
    
    StreamSendContext(const QUIC_API_TABLE* p_APITable) noexcept : p_APITable(p_APITable)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    
    StreamData c_Data;
};

#endif /* StreamSendContext_h */
