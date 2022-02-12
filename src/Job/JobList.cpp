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
        c_Job.Add(p_Job);
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
    std::shared_ptr<Job> p_Result(NULL);
    
    while (b_Locked == false)
    {
        // Get a job
        std::shared_ptr<Job> p_Job = c_Job.GetElement(true);
        
        // No job available, wait for one
        if (p_Job == NULL)
        {
            std::unique_lock<std::mutex> c_Lock(c_Mutex);
            c_Condition.wait(c_Lock);
        }
    }
    
    throw Exception("Job list locked!");
}
