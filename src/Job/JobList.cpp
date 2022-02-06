/**
 *  JobList.cpp
 *
 *  This file is part of the MRH project.
 *  See the AUTHORS file for Copyright information.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// C / C++

// External

// Project
#include "./JobList.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

JobList::JobList() noexcept : us_EntryCount(0),
                              b_Locked(false)
{}

JobList::~JobList() noexcept
{
    Lock();
}

JobList::Entry::Entry() noexcept : p_Job(NULL)
{}

JobList::Entry::Entry(std::shared_ptr<Job>& p_Job) noexcept : p_Job(p_Job)
{}

JobList::Entry::~Entry() noexcept
{}

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
    if (p_Job == NULL || p_Job == nullptr)
    {
        throw Exception("Invalid job added!");
    }
    
    // First try to reuse a entry
    // @NOTE: Task deque never shrinks!
    for (size_t i = 0; i < us_EntryCount; ++i)
    {
        // Task free to be checked?
        // @NOTE: Locking does not need a retry, a task locked
        //        which is removed by the worker thread is simply
        //        bad timing. But at this point it's still a valid
        //        usable task
        if (dq_Job[i].c_Mutex.try_lock() == false)
        {
            continue;
        }
        
        // Task can be added here?
        if (dq_Job[i].p_Job == NULL)
        {
            dq_Job[i].p_Job.swap(p_Job);
            
            dq_Job[i].c_Mutex.unlock();
            c_Condition.notify_one();
            
            return;
        }
        else
        {
            dq_Job[i].c_Mutex.unlock();
        }
    }
    
    // No free entry, add a new one
    try
    {
        c_Mutex.lock(); // Lock for outside multithreading
        
        dq_Job.emplace_back(p_Job);
        us_EntryCount += 1;
        
        c_Mutex.unlock();
        c_Condition.notify_one();
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to add job: " + std::string(e.what()));
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::shared_ptr<Job> JobList::GetJob()
{
    std::shared_ptr<Job> p_Result(NULL);
    
    while (b_Locked == false)
    {
        // Run connections
        // @NOTE: Job deque never shrinks!
        for (size_t i = 0; i < us_EntryCount; ++i)
        {
            // Can we grab this job?
            if (dq_Job[i].c_Mutex.try_lock() == false)
            {
                continue;
            }
            else if (dq_Job[i].p_Job == NULL)
            {
                dq_Job[i].c_Mutex.unlock();
                continue;
            }
            
            // Grab the job
            p_Result.swap(dq_Job[i].p_Job);
            
            // Unlock and return
            dq_Job[i].c_Mutex.unlock();
            return p_Result;
        }
        
        // No job found, wait for one
        std::unique_lock<std::mutex> c_Lock(c_Mutex);
        c_Condition.wait(c_Lock);
    }
    
    throw Exception("Job list locked!");
}
