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

#ifndef StreamData_h
#define StreamData_h

// C / C++
#include <atomic>
#include <vector>

// External

// Project


struct StreamData
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum State
    {
        FREE = 0,
        IN_USE = 1,
        COMPLETED = 2
    };
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    StreamData() noexcept : e_State(FREE)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    std::atomic<State> e_State; // Currently reading
    std::vector<uint8_t> v_Bytes;
};

#endif /* StreamData_h */
