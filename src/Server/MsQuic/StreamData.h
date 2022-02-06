/**
 *  StreamData.h
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

#ifndef StreamData_h
#define StreamData_h

// C / C++
#include <atomic>
#include <vector>

// External

// Project


struct StreamData
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum State
    {
        FREE = 0,
        IN_USE = 1,
        COMPLETED = 2
    };
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    StreamData() noexcept : e_State(FREE)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    std::atomic<State> e_State; // Currently reading
    std::vector<uint8_t> v_Bytes;
};

#endif /* StreamData_h */
