/**
 *  WorkerPool.h
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

#ifndef WorkerPool_h
#define WorkerPool_h

// C / C++
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <list>
#include <utility>

// External

// Project
#include "./WorkerTask.h"


class WorkerPool
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Task
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
            
        Task(std::unique_ptr<WorkerTask>& p_Task) noexcept;
        
        /**
         *  Copy constructor. Disabled for this class.
         *
         *  \param c_Task Task class source.
         */
        
        Task(Task const& c_Task) = delete;
        
        /**
         *  Default destructor.
         */
        
        ~Task() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::mutex c_Mutex;
        std::unique_ptr<WorkerTask> p_Task;
    };
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param l_ThreadInfo The list defining thread count with thread shared data.
     *  \param e_Type The server type this worker pool runs for.
     *  \param i_PulseMS The pulse time in milliseconds.
     */
    
    WorkerPool(std::list<std::unique_ptr<WorkerShared>>& l_ThreadInfo,
               ActorType e_Type,
               int i_PulseMS);
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_WorkerPool WorkerPool class source.
     */
    
    WorkerPool(WorkerPool const& c_WorkerPool) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~WorkerPool() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a task to perform.
     *
     *  \param p_Task The task to perform.
     */
    
    void AddTask(std::unique_ptr<WorkerTask>& p_Task);
    
private:
    
    //*************************************************************************************
    // Update
    //*************************************************************************************
    
    /**
     *  Run the worker update.
     *
     *  \param p_Instance The worker pool instance to update with.
     *  \param p_Shared The thread shared worker data.
     *  \param i_PulseMS The pulse time in milliseconds.  
     */
    
    static void Update(WorkerPool* p_Instance, WorkerShared* p_Shared, int i_PulseMS) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    ActorType e_Type;
    
    std::list<std::thread> l_Thread;
    std::atomic<bool> b_Run;
    
    std::condition_variable c_Condition;
    std::mutex c_Mutex;
    std::deque<Task> dq_Task;
    std::atomic<size_t> us_TaskCount; // Number of active tasks
    
protected:

};

#endif /* WorkerPool_h */
