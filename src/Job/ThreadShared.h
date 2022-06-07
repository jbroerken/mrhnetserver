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

#ifndef ThreadShared_h
#define ThreadShared_h

// C / C++

// External

// Project
#include "../Exception.h"


class ThreadShared
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_ThreadShared ThreadShared class source.
     */
    
    ThreadShared(ThreadShared const& s_ThreadShared) = delete;
    
    /**
     *  Default destructor.
     */
    
    virtual ~ThreadShared() noexcept
    {}
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    ThreadShared() noexcept
    {}
};

#endif /* ThreadShared_h */
