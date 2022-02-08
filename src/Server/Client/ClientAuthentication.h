/**
 *  ClientAuthentication.h
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
