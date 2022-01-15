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
        throw NetException("Unknown message ID!");
    }
    
    v_Data.insert(v_Data.end(), us_BufferSize, 0);
    v_Data[us_IDPos] = u8_ID;
}

NetMessage::NetMessage(std::vector<uint8_t>& v_Data)
{
    if (v_Data.size() < us_DataPos)
    {
        throw NetException("Invalid message data size!");
    }
    
    this->v_Data.swap(v_Data);
    
    if (this->v_Data.size() < us_BufferSize)
    {
        this->v_Data.insert(this->v_Data.end(),
                            us_BufferSize - this->v_Data.size(),
                            0);
    }
}

NetMessage::NetMessage(std::vector<uint8_t> const& v_Data)
{
    if (v_Data.size() < us_DataPos)
    {
        throw NetException("Invalid message data size!");
    }
    
    this->v_Data = v_Data;
    
    if (this->v_Data.size() < us_BufferSize)
    {
        this->v_Data.insert(this->v_Data.end(),
                            us_BufferSize - this->v_Data.size(),
                            0);
    }
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
        return CS_MSG_UNK;
    }
    
    return static_cast<NetMessageList>(v_Data[us_IDPos]);
}

bool NetMessage::GetEncrypted() const noexcept
{
    // @NOTE: Net messages are always a 1024 byte buffer,
    //        encryption always increases that size
    if (v_Data.size() > NetMessage::us_BufferSize)
    {
        return true;
    }
    
    return false;
}
