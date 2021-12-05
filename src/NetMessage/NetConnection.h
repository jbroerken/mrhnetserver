/**
 *  NetConnection.h
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

#ifndef NetConnection_h
#define NetConnection_h

// C / C++

// External

// Project
#include "./Ver/NetMessageV1.h"


struct MsQuicConnectionContext;


class NetConnection
{
    friend class NetServer; // Allow construction by net server
    
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_NetConnection NetConnection class source.
     */
    
    NetConnection(NetConnection const& c_NetConnection) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~NetConnection() noexcept;
    
    //*************************************************************************************
    // Recieve
    //*************************************************************************************
    
    /**
     *  Recieve a message from the peer.
     *
     *  \param c_Message The message recieved. The message data will be overwritten.
     *
     *  \return true if a message was recieved, false if not.
     */
    
    bool Recieve(NetMessage& c_Message);
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Send a message to the peer.
     *
     *  \param c_Message The message to send. The message data will be consumed.
     */
    
    void Send(NetMessage& c_Message);
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Check if the connection is valid.
     *
     *  \return true if valid, false if not.
     */
    
    bool GetConnected() noexcept;
    
private:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_Context The managed connection context.
     */
    
    NetConnection(MsQuicConnectionContext* p_Context);
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    MsQuicConnectionContext* p_Context;
    
protected:

};

#endif /* NetConnection_h */
