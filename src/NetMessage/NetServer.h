/**
 *  NetServer.h
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

#ifndef NetServer_h
#define NetServer_h

// C / C++
#include <list>
#include <memory>
#include <string>

// External

// Project
#include "./NetConnection.h"


struct MsQuicListenerContext;


class NetServer
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    NetServer();
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_NetServer NetServer class source.
     */
    
    NetServer(NetServer const& c_NetServer) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~NetServer() noexcept;
    
    //*************************************************************************************
    // Run
    //*************************************************************************************
    
    /**
     *  Start accepting connections.
     *
     *  \param i_Port The port to accept connections on.
     *  \param s_CertFilePath The full path to the server certificate file.
     *  \param s_KeyFilePath The full path to the server key file.
     *  \param i_TimeoutS The connection timeout in seconds.
     */
    
    void Start(int i_Port, std::string const& s_CertFilePath, std::string const& s_KeyFilePath, int i_TimeoutS);
    
    /**
     *  Stop accepting connections.
     */
    
    void Stop() noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get all newly available connections.
     *
     *  \return All newly available connections.
     */
    
    std::list<std::unique_ptr<NetConnection>> GetConnections() noexcept;
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    const void* p_APITable;
    void* p_Registration;
    void* p_Listener;
    
    MsQuicListenerContext* p_Context;
    
    bool b_Started;
    std::list<std::unique_ptr<NetConnection>> l_Connection;
    
protected:

};

#endif /* NetServer_h */
