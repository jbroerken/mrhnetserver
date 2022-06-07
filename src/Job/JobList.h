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
#include "../SharedList.h"


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
    // Data
    //*************************************************************************************
    
    std::condition_variable c_Condition;
    std::mutex c_Mutex;
    
    SharedList<Job> c_JobList;
    std::atomic<bool> b_Locked;
    
protected:
    
};

#endif /* JobList_h */
