/**
 *  ThreadPool.h
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
