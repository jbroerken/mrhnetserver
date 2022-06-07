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
     *  \param c_Database The database to use.
     *  \param c_UserInfo The user info to use.
     *
     *  \return The sendable communication net message.
     */
    
    NetMessage RetrieveMessage(Database& c_Database, UserInfo const& c_UserInfo) noexcept;
    
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
