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

#ifndef ThreadPool_h
#define ThreadPool_h

// C / C++
#include <thread>
#include <atomic>
#include <list>

// External

// Project
#include "./ThreadShared.h"
#include "./JobList.h"


class ThreadPool
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_JobList The job list to work on.
     *  \param l_ThreadInfo The list defining thread count with thread shared data.
     */
    
    ThreadPool(JobList& c_JobList,
               std::list<std::unique_ptr<ThreadShared>>& l_ThreadInfo);
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_ThreadPool ThreadPool class source.
     */
    
    ThreadPool(ThreadPool const& c_ThreadPool) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~ThreadPool() noexcept;
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Run a thread update.
     *
     *  \param p_Instance The thread pool instance to update with.
     *  \param p_ThreadShared The thread shared data.
     */
    
    static void Update(ThreadPool* p_Instance, ThreadShared* p_ThreadShared) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::list<std::thread> l_Thread;
    std::atomic<bool> b_Run;
    
    JobList& c_JobList;
    
protected:

};

#endif /* ThreadPool_h */
