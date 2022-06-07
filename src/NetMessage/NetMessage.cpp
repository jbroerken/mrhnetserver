/**
 *  Copyright (C) 2021 - 2022 The MRH Project Authors.
 * 
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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
    
    v_Data.emplace_back(u8_ID);
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
