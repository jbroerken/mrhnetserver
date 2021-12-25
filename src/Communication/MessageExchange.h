/**
 *  MessageExchange.h
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

#ifndef MessageExchange_h
#define MessageExchange_h

// C / C++
#include <list>
#include <mutex>

// External

// Project
#include "../NetMessage/NetMessage.h"


class MessageExchange
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param s_DeviceKey The device key for the message exchange.
     */
    
    MessageExchange(std::string const& s_DeviceKey) noexcept : s_DeviceKey(s_DeviceKey)
    {}
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_MessageExchange MessageExchange class source.
     */
    
    MessageExchange(MessageExchange const& c_MessageExchange) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~MessageExchange() noexcept
    {}
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Identification
    const std::string s_DeviceKey;
    
    // Platform -> App
    std::list<NetMessage> l_PAMessage;
    std::mutex c_PAMutex;
    
    // App -> Platform
    std::list<NetMessage> l_APMessage;
    std::mutex c_APMutex;
    
private:
    
protected:
    
};

#endif /* MessageExchange_h */
