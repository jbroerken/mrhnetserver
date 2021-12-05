/**
 *  Timer.h
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

#ifndef Timer_h
#define Timer_h

// C / C++
#include <cstdint>
#include <chrono>

// External

// Project


class Timer
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    Timer() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~Timer() noexcept;
    
    //*************************************************************************************
    // Set
    //*************************************************************************************
    
    /**
     *  Set a timer for X milliseconds.
     *
     *  \param u32_MS The time in milliseconds.
     */
    
    void Set(uint32_t u32_MS) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if the timer was finished.
     *
     *  \return true if finished, false if not.
     */
    
    bool GetFinished() noexcept;
    
    /**
     *  Get the time remaining until the timer is met.
     *
     *  \return The time remaining in milliseconds.
     */
    
    std::chrono::milliseconds GetTimeRemaining() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::chrono::system_clock::time_point c_EndTime;
    
protected:

};

#endif /* Timer_h */
