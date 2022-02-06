/**
 *  JobList.h
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

#ifndef JobList_h
#define JobList_h

// C / C++
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <deque>

// External

// Project
#include "./Job.h"
#include "../Exception.h"


class JobList
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    JobList() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~JobList() noexcept;
    
    //*************************************************************************************
    // Lock
    //*************************************************************************************
    
    /**
     *  Lock the list and notify all waiting users about the change.
     */
    
    void Lock() noexcept;
    
    /**
     *  Unlock the list.
     */
    
    void Unlock() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a job to the job list.
     *
     *  \param p_Job The job to add.
     */
    
    void AddJob(std::shared_ptr<Job> p_Job);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get a job. This function blocks until a job is available.
     *
     *  \return The job to perform.
     */
    
    std::shared_ptr<Job> GetJob();
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Entry
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
            
        Entry() noexcept;
        
        /**
         *  Job constructor.
         *
         *  \param p_Job The job to add.
         */
            
        Entry(std::shared_ptr<Job>& p_Job) noexcept;
        
        /**
         *  Copy constructor. Disabled for this class.
         *
         *  \param c_Entry Entry class source.
         */
        
        Entry(Entry const& c_Entry) = delete;
        
        /**
         *  Default destructor.
         */
        
        ~Entry() noexcept;
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::mutex c_Mutex;
        std::shared_ptr<Job> p_Job;
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::condition_variable c_Condition;
    std::mutex c_Mutex;
    
    std::deque<Entry> dq_Job;
    std::atomic<size_t> us_EntryCount; // Job list count
    
    std::atomic<bool> b_Locked;
    
protected:
    
};

#endif /* JobList_h */
