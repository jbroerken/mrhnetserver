/**
 *  WorkerShared.h
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

#ifndef WorkerShared_h
#define WorkerShared_h

// C / C++

// External

// Project
#include "../ServerException.h"


class WorkerShared
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_WorkerShared WorkerShared class source.
     */
    
    WorkerShared(WorkerShared const& c_WorkerShared) = delete;
    
    /**
     *  Default destructor.
     */
    
    virtual ~WorkerShared() noexcept
    {}
    
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
    
    WorkerShared() noexcept
    {}
};

#endif /* WorkerShared_h */
