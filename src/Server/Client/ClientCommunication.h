/**
 *  ClientCommunication.h
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

#ifndef ClientCommunication_h
#define ClientCommunication_h

// C / C++
#include <list>

// External

// Project
#include "./UserInfo.h"
#include "../../NetMessage/Ver/NetMessageV1.h"
#include "../../Database/Database.h"
#include "../../Exception.h"


namespace ClientCommunication
{
    //*************************************************************************************
    // Retrieve
    //*************************************************************************************
    
    /**
     *  Retrieve all sendable communication messages.
     *
     *  \param u8_Type The message type to recieve.
     *  \param c_Database The database to use.
     *  \param c_UserInfo The user info to use.
     *
     *  \return The sendable communication net messages.
     */
    
    std::list<NetMessage> RetrieveMessages(uint8_t u8_Type, Database& c_Database, UserInfo const& c_UserInfo) noexcept;
    
    //*************************************************************************************
    // Store
    //*************************************************************************************
    
    /**
     *  Store a communication message.
     *
     *  \param c_NetMessage The communication message to store.
     *  \param c_Database The database to use.
     *  \param c_UserInfo The user info to write.
     */
    
    void StoreMessage(NetMessage const& c_NetMessage, Database& c_Database, UserInfo const& c_UserInfo) noexcept;
}

#endif /* ClientCommunication_h */
