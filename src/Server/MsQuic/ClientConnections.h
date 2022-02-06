/**
 *  ClientConnections.h
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

#ifndef ClientConnections_h
#define ClientConnections_h

// C / C++
#include <atomic>

// External

// Project


struct ClientConnections
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param i_ClientConnectionsMax The max number of clients which can connect.
     */
    
    ClientConnections(int i_ClientConnectionsMax) noexcept : i_ClientConnectionsMax(i_ClientConnectionsMax),
                                                             i_ClientConnections(0)
    {}
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const int i_ClientConnectionsMax;
    std::atomic<int> i_ClientConnections;
};

#endif /* ClientConnections_h */
