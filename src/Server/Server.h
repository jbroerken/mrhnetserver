/**
 *  Server.h
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

#ifndef Server_h
#define Server_h

// C / C++

// External

// Project
#include "./ClientPool.h"
#include "./MsQuic/ListenerContext.h"


class Server
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_ClientPool The client pool to add clients to.
     */
    
    Server(ClientPool& c_ClientPool);
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_Server Server class source.
     */
    
    Server(Server const& c_Server) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~Server() noexcept;
    
    //*************************************************************************************
    // Start
    //*************************************************************************************
    
    /**
     *  Start accepting connections.
     *
     *  \param i_Port The port to accept connections on.
     *  \param s_CertFilePath The full path to the server certificate file.
     *  \param s_KeyFilePath The full path to the server key file.
     *  \param i_TimeoutS The connection timeout in seconds.
     *  \param i_MaxClientCount The maximum number of clients which can connect.
     */
    
    void Start(int i_Port, std::string const& s_CertFilePath, std::string const& s_KeyFilePath, int i_TimeoutS, int i_MaxClientCount);
    
    //*************************************************************************************
    // Stop
    //*************************************************************************************
    
    /**
     *  Stop accepting connections.
     */
    
    void Stop() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Clients
    ClientPool& c_ClientPool;
    
    // MsQuic
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Registration;
    HQUIC p_Listener;
    
    ListenerContext* p_Context;
    
    // Running
    bool b_Started;
    
protected:

};

#endif /* Server_h */
