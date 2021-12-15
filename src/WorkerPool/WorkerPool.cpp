/**
 *  WorkerPool.cpp
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
#include "./WorkerPool.h"
#include "../Timer.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

WorkerPool::WorkerPool(std::list<std::unique_ptr<WorkerShared>>& l_ThreadInfo,
                       ActorType e_Type,
                       int i_PulseMS) : e_Type(e_Type),
                                        b_Run(true),
                                        us_TaskCount(0)
{
    if (e_Type != SERVER_CONNECTION && e_Type != SERVER_COMMUNICATION)
    {
        throw ServerException("Invalid server type!", e_Type);
    }
    else if (l_ThreadInfo.size() == 0)
    {
        throw ServerException("No thread info!", e_Type);
    }
    
    if (i_PulseMS < 0)
    {
        i_PulseMS = 0;
    }
    
    try
    {
        for (auto It = l_ThreadInfo.begin(); It != l_ThreadInfo.end(); ++It)
        {
            l_Thread.emplace_back(Update,
                                  this,
                                  It->release(),
                                  i_PulseMS);
        }
    }
    catch (std::exception& e)
    {
        throw ServerException(e.what(), e_Type);
    }
}

WorkerPool::~WorkerPool() noexcept
{
    b_Run = false;
    c_Condition.notify_all();
    
    for (auto& Thread : l_Thread)
    {
        Thread.join();
    }
}

WorkerPool::Task::Task(std::unique_ptr<WorkerTask>& p_Task) noexcept : p_Task(p_Task.release())
{}

WorkerPool::Task::~Task() noexcept
{}

//*************************************************************************************
// Update
//*************************************************************************************

void WorkerPool::Update(WorkerPool* p_Instance, WorkerShared* p_Shared, int i_PulseMS) noexcept
{
    // @NOTE: This is a bit anyoing, but it allows easy passing
    //        to the thread and still manages
    std::unique_ptr<WorkerShared> p_ManagedShared(p_Shared);
    Timer c_Timer;
    
    std::deque<Task>& dq_Task = p_Instance->dq_Task;
    std::atomic<size_t>& us_TaskCount = p_Instance->us_TaskCount;
    
    while (p_Instance->b_Run == true)
    {
        // No active tasks? Wait until one is added
        if (us_TaskCount == 0)
        {
            std::unique_lock<std::mutex> c_Lock(p_Instance->c_Mutex);
            p_Instance->c_Condition.wait(c_Lock);
            
            // Return to start, was notify due to quit?
            continue;
        }
        
        // Set timer for pulse diff
        c_Timer.Set(i_PulseMS);
        
        // Run connections
        // @NOTE: Task deque never shrinks!
        for (size_t i = 0; i < us_TaskCount; ++i)
        {
            // Can we work on this task?
            if (dq_Task[i].c_Mutex.try_lock() == false)
            {
                continue;
            }
            
            // Call virtual function from thread static
            if (dq_Task[i].p_Task != nullptr && dq_Task[i].p_Task->Perform(p_ManagedShared) == false)
            {
                // Valid task is meant to be removed
                dq_Task[i].p_Task.reset();
            }
            
            // Unlock, processed
            dq_Task[i].c_Mutex.unlock();
        }
        
        // Pulse timer not met, wait for next pulse
        if (c_Timer.GetFinished() == false)
        {
            std::this_thread::sleep_for(c_Timer.GetTimeRemaining());
        }
    }
}

//*************************************************************************************
// Add
//*************************************************************************************

void WorkerPool::AddTask(std::unique_ptr<WorkerTask>& p_Task)
{
    if (p_Task == NULL || p_Task == nullptr)
    {
        throw ServerException("Invalid task added!", e_Type);
    }
    
    // First try to reuse a
    // @NOTE: Task deque never shrinks!
    for (size_t i = 0; i < us_TaskCount; ++i)
    {
        // Task free to be checked?
        // @NOTE: Locking does not need a retry, a task locked
        //        which is removed by the worker thread is simply
        //        bad timing. But at this point it's still a valid
        //        usable task
        if (dq_Task[i].c_Mutex.try_lock() == false)
        {
            continue;
        }
        
        // Task can be added here?
        if (dq_Task[i].p_Task == nullptr)
        {
            dq_Task[i].p_Task.swap(p_Task);
            
            dq_Task[i].c_Mutex.unlock();
            c_Condition.notify_one();
            
            return;
        }
        else
        {
            dq_Task[i].c_Mutex.unlock();
        }
    }
    
    // No free entry, add a new one
    try
    {
        c_Mutex.lock();
        
        dq_Task.emplace_back(p_Task);
        us_TaskCount += 1;
        
        c_Mutex.unlock();
        c_Condition.notify_one();
    }
    catch (std::exception& e)
    {
        throw ServerException("Failed to add task: " + std::string(e.what()), e_Type);
    }
}
