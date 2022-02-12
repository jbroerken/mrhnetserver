/**
 *  Client.h
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
