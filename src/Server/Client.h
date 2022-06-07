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

#ifndef Client_h
#define Client_h

// C / C++
#include <mutex>
#include <deque>
#include <list>
#include <utility>

// External

// Project
#include "./MsQuic/StreamSendContext.h"
#include "./Client/UserInfo.h"
#include "../NetMessage/NetMessage.h"
#include "../Job/Job.h"
#include "../SharedList.h"


class Client : public Job
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The api table to use for sending.
     *  \param p_Connection The connection for the client.
     *  \param us_ClientID The id for the client.
     */
    
    Client(const QUIC_API_TABLE* p_APITable,
           HQUIC p_Connection,
           size_t us_ClientID) noexcept;
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_NetConnection NetConnection class source.
     */
    
    Client(Client const& c_Client) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~Client() noexcept;
    
    //*************************************************************************************
    // Disconnect
    //*************************************************************************************
    
    /**
     *  Set the client as disconnected.
     */
    
    void Disconnected() noexcept;
    
    //*************************************************************************************
    // Perform
    //*************************************************************************************
    
    /**
     *  Perform the job.
     *
     *  \param p_Shared Thread shared data.
     *
     *  \return true on success, false on failure.
     */
    
    bool Perform(std::shared_ptr<ThreadShared>& p_Shared) noexcept override;
    
    //*************************************************************************************
    // Recieve
    //*************************************************************************************
    
    /**
     *  Recieve a net message with stream data.
     *
     *  \param c_Data The recieved stream data.
     */
    
    void RecieveNetMessage(StreamData& c_Data) noexcept;
    
    /**
     *  Recieve a data available notification.
     */
    
    void RecieveDataAvailable() noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the id for the client.
     *
     *  \return The client id.
     */
    
    size_t GetClientID() const noexcept;
    
private:
    
    //*************************************************************************************
    // Disconnect
    //*************************************************************************************
    
    /**
     *  Disconnect the client.
     */
    
    void Disconnect() noexcept;
    
    //*************************************************************************************
    // Send
    //*************************************************************************************
    
    /**
     *  Send current messages as stream data.
     */
    
    void Send();
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // State
    size_t us_ClientID;
    std::mutex c_PerformMutex; // Stop multiple job threads
    
    // Net Message
    SharedList<NetMessage> c_Recieved;
    SharedList<NetMessage> c_Send;
    
    // MsQuic
    const QUIC_API_TABLE* p_APITable;
    std::atomic<HQUIC> p_Connection; // Connection is accessed by msquic threads and job
    std::list<StreamSendContext> l_StreamContext;
    
    // User
    UserInfo c_UserInfo;
    
protected:

};

#endif /* Client_h */
