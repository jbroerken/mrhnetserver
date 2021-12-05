/**
 *  WorkerTask.h
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

#ifndef WorkerTask_h
#define WorkerTask_h

// C / C++
#include <memory>

// External

// Project
#include "./WorkerShared.h"


class WorkerTask
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_WorkerTask WorkerTask class source.
     */
    
    WorkerTask(WorkerTask const& c_WorkerTask) = delete;
    
    /**
     *  Default destructor.
     */
    
    virtual ~WorkerTask() noexcept
    {}
    
    //*************************************************************************************
    // Perform
    //*************************************************************************************
    
    /**
     *  Perform the task.
     *
     *  \param p_Shared The shared worker data.
     *
     *  \return true if the task should continue, false if not.
     */
    
    virtual bool Perform(std::unique_ptr<WorkerShared>& p_Shared) noexcept
    {
        return false;
    }
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
protected:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    WorkerTask() noexcept
    {}
};

#endif /* WorkerTask_h */
