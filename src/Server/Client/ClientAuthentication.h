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

#ifndef ClientAuthentication_h
#define ClientAuthentication_h

// C / C++

// External

// Project
#include "./UserInfo.h"
#include "../../NetMessage/Ver/NetMessageV1.h"
#include "../../Database/Database.h"
#include "../../Exception.h"

using namespace NetMessageV1;


namespace ClientAuthentication
{
    //*************************************************************************************
    // Auth Request
    //*************************************************************************************
    
    /**
     *  Handle a client authentication request.
     *
     *  \param c_Request The recieved request.
     *  \param c_Database The database to use.
     *  \param c_UserInfo The user info to write.
     *
     *  \return The result net message to send.
     */
    
    NetMessage HandleAuthRequest(MSG_AUTH_REQUEST_DATA c_Request, Database& c_Database, UserInfo& c_UserInfo) noexcept;
    
    //*************************************************************************************
    // Auth Proof
    //*************************************************************************************
    
    /**
     *  Handle a client authentication proof.
     *
     *  \param c_Proof The recieved proof.
     *  \param c_Database The database to use.
     *  \param c_UserInfo The user info to write.
     *
     *  \return The result net message to send.
     */
    
    NetMessage HandleAuthProof(MSG_AUTH_PROOF_DATA c_Proof, Database& c_Database, UserInfo& c_UserInfo) noexcept;
}

#endif /* ClientAuthentication_h */
