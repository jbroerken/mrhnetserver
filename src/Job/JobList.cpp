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
#include "./JobList.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

JobList::JobList() noexcept : b_Locked(false)
{}

JobList::~JobList() noexcept
{
    Lock();
}

//*************************************************************************************
// Lock
//*************************************************************************************

void JobList::Lock() noexcept
{
    b_Locked = true;
    c_Condition.notify_all();
}

void JobList::Unlock() noexcept
{
    b_Locked = false;
}

//*************************************************************************************
// Add
//*************************************************************************************

void JobList::AddJob(std::shared_ptr<Job> p_Job)
{
    try
    {
        c_JobList.Add(p_Job);
        c_Condition.notify_one();
    }
    catch (...)
    {
        throw;
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::shared_ptr<Job> JobList::GetJob()
{
    while (b_Locked == false)
    {
        // Get a job
        std::shared_ptr<Job> p_Job = c_JobList.GetElement();
        
        // No job available, wait for one
        if (p_Job == NULL)
        {
            std::unique_lock<std::mutex> c_Lock(c_Mutex);
            c_Condition.wait(c_Lock);
        }
        else
        {
            return p_Job;
        }
    }
    
    throw Exception("Job list locked!");
}
