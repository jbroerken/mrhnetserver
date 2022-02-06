/**
 *  Job.h
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

#ifndef Job_h
#define Job_h

// C / C++

// External

// Project
#include "./ThreadShared.h"


class Job
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_Job Job class source.
     */
    
    Job(Job const& c_Job) = delete;
    
    /**
     *  Default destructor.
     */
    
    virtual ~Job() noexcept
    {}
    
    //*************************************************************************************
    // Perform
    //*************************************************************************************
    
    /**
     *  Perform the job.
     *
     *  \param p_Shared Thread shared data.
     *
     *  \return true on success, false on failure.
     */
    
    virtual bool Perform(std::shared_ptr<ThreadShared>& p_Shared) noexcept
    {
        return true;
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
    
    Job() noexcept
    {}
};

#endif /* Job_h */
