/**
 *  ConnectionMain.h
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

#ifndef ConnectionMain_h
#define ConnectionMain_h

// C / C++

// External

// Project
#include "../Configuration.h"


namespace ConnectionMain
{
    //*************************************************************************************
    // Run
    //*************************************************************************************
    
    /**
     *  Run as connection server.
     *
     *  \param c_Config The server configuration to use.
     *  \param b_Run The server run flag.
     */
    
    void Run(Configuration& c_Config, bool& b_Run);
};

#endif /* ConnectionMain_h */
