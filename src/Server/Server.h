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
