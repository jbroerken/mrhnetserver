/**
 *  Timer.cpp
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
#include "./Timer.h"

// Pre-defined
using namespace std::chrono;


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Timer::Timer() noexcept
{}

Timer::~Timer() noexcept
{}

//*************************************************************************************
// Set
//*************************************************************************************

void Timer::Set(uint32_t u32_MS) noexcept
{
    c_EndTime = system_clock::now() + milliseconds(u32_MS);
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool Timer::GetFinished() noexcept
{
    if (system_clock::now() < c_EndTime)
    {
        return false;
    }
    
    return true;
}

std::chrono::milliseconds Timer::GetTimeRemaining() noexcept
{
    if (system_clock::now() < c_EndTime)
    {
        return duration_cast<milliseconds>(c_EndTime - system_clock::now());
    }
    
    return milliseconds(0);
}
