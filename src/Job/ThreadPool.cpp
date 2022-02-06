/**
 *  ThreadPool.cpp
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
            std::shared_ptr<Job> p_Job = p_Instance->c_JobList.GetJob();
            
            if (p_Job->Perform(p_Shared) == false)
            {
                p_Instance->c_JobList.AddJob(p_Job);
            }
        }
        catch (...)
        {}
    }
}
