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

#ifndef Job_h
#define Job_h

// C / C++

// External

// Project
#include "./ThreadShared.h"


class Job
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_Job Job class source.
     */
    
    Job(Job const& c_Job) = delete;
    
    /**
     *  Default destructor.
     */
    
    virtual ~Job() noexcept
    {}
    
    //*************************************************************************************
    // Perform
    //*************************************************************************************
    
    /**
     *  Perform the job.
     *
     *  \param p_Shared Thread shared data.
     *
     *  \return true on success, false on failure.
     */
    
    virtual bool Perform(std::shared_ptr<ThreadShared>& p_Shared) noexcept
    {
        return true;
    }
    
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
    
    Job() noexcept
    {}
};

#endif /* Job_h */
