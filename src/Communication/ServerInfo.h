/**
 *  ServerInfo.h
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

#ifndef ServerInfo_h
#define ServerInfo_h

// C / C++
#include <cstdint>
#include <mutex>

// External

// Project


class ServerInfo
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param u32_ServerID The id for the server.
     *  \param u32_MaxConnections The max connections combined for the server.
     */
    
    ServerInfo(uint32_t u32_ServerID,
               uint32_t u32_MaxConnections) noexcept : u32_ServerID(u32_ServerID),
                                                       u32_MaxConnections(u32_MaxConnections)
    {}
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_MessageExchange MessageExchange class source.
     */
    
    ServerInfo(MessageExchange const& c_MessageExchange) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~ServerInfo() noexcept
    {}
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Identification
    const uint32_t u32_ServerID;
    const uint32_t u32_MaxConnections;
    
    // Platform Connections
    uint32_t u32_PlatformConnections;
    std::mutex c_PCMutex;
    
    // App Connections
    uint32_t u32_AppConnections;
    std::mutex c_ACMutex;
    
private:
    
protected:
    
};

#endif /* ServerInfo_h */
