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

// C / C++

// External

// Project
#include "./ThreadPool.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

ThreadPool::ThreadPool(JobList& c_JobList,
                       std::list<std::unique_ptr<ThreadShared>>& l_ThreadInfo) : b_Run(true),
                                                                                 c_JobList(c_JobList)
{
    if (l_ThreadInfo.size() == 0)
    {
        throw Exception("No thread info!");
    }
    
    try
    {
        for (auto It = l_ThreadInfo.begin(); It != l_ThreadInfo.end(); ++It)
        {
            l_Thread.emplace_back(Update,
                                  this,
                                  It->release());
        }
    }
    catch (std::exception& e)
    {
        throw Exception(e.what());
    }
}

ThreadPool::~ThreadPool() noexcept
{
    b_Run = false;
    
    for (auto& Thread : l_Thread)
    {
        Thread.join();
    }
}

//*************************************************************************************
// Update
//*************************************************************************************

void ThreadPool::Update(ThreadPool* p_Instance, ThreadShared* p_ThreadShared) noexcept
{
    // Create thread shared
    std::shared_ptr<ThreadShared> p_Shared(p_ThreadShared);
    
    // Simple loop update, wait for job until update can be run
    while (p_Instance->b_Run == true)
    {
        try
        {
            // Get next job
            std::shared_ptr<Job> p_Job = p_Instance->c_JobList.GetJob();
            
            if (p_Job->Perform(p_Shared) == false)
            {
                p_Instance->c_JobList.AddJob(p_Job);
            }
            
            // Reset job to no longer be owner
            p_Job.reset();
        }
        catch (...)
        {}
    }
}
