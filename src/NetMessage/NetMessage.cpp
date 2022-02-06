/**
 *  NetMessage.cpp
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
#include "./NetMessage.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

NetMessage::NetMessage(uint8_t u8_ID)
{
    if (u8_ID > NET_MESSAGE_LIST_MAX)
    {
        throw Exception("Unknown message ID!");
    }
    
    v_Data[us_IDPos] = u8_ID;
}

NetMessage::NetMessage(std::vector<uint8_t>& v_Data)
{
    if (v_Data.size() < us_DataPos)
    {
        throw Exception("Invalid message data size!");
    }
    
    this->v_Data.swap(v_Data);
}

NetMessage::NetMessage(std::vector<uint8_t> const& v_Data)
{
    if (v_Data.size() < us_DataPos)
    {
        throw Exception("Invalid message data size!");
    }
    
    this->v_Data = v_Data;
}

NetMessage::~NetMessage() noexcept
{}

//*************************************************************************************
// Getters
//*************************************************************************************

NetMessage::NetMessageList NetMessage::GetID() const noexcept
{
    if (v_Data.size() < us_DataPos)
    {
        return MSG_UNK;
    }
    
    return static_cast<NetMessageList>(v_Data[us_IDPos]);
}
