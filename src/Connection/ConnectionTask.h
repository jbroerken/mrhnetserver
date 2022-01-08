/**
 *  ConnectionTask.h
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

#ifndef ConnectionTask_h
#define ConnectionTask_h

// C / C++
#include <list>

// External

// Project
#include "../WorkerPool/WorkerTask.h"
#include "../NetMessage/NetConnection.h"


class ConnectionTask : public WorkerTask
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_Connection The connection for the task.
     */
    
    ConnectionTask(std::unique_ptr<NetConnection>& p_Connection);
    
    /**
     *  Default destructor.
     */
    
    ~ConnectionTask() noexcept;
    
    //*************************************************************************************
    // Perform
    //*************************************************************************************
    
    /**
     *  Perform the task.
     *
     *  \param p_Shared The thread shared worker data.
     *
     *  \return true if the task should continue, false if not.
     */
    
    bool Perform(std::unique_ptr<WorkerShared>& p_Shared) noexcept override;
    
private:
    
    //*************************************************************************************
    // Auth
    //*************************************************************************************
    
    /**
     *  Send a auth result message.
     *
     *  \param u8_Result The request result.
     */
    
    void SendAuthResult(uint8_t u8_Result) noexcept;
    
    /**
     *  Decrement the auth attempts and get the connection state.
     *
     *  \return true if the connect should be kept active, false if not.
     */
    
    inline bool DecrementAuthAttempt() noexcept;
    
    /**
     *  Handle a recieved client authenticate request.
     *
     *  \param p_Shared The thread shared worker data.
     *  \param c_Request The client request message.
     *
     *  \return true if the connection should be kept, false if not.
     */
    
    bool AuthRequest(std::unique_ptr<WorkerShared>& p_Shared, NetMessageV1::C_MSG_AUTH_REQUEST_DATA c_Request) noexcept;
    
    /**
     *  Handle a recieved client authenticate proof.
     *
     *  \param c_Proof The client proof message.
     *
     *  \return true if the connection should be kept, false if not.
     */
    
    bool AuthProof(NetMessageV1::C_MSG_AUTH_PROOF_DATA c_Proof) noexcept;
    
    //*************************************************************************************
    // Channel
    //*************************************************************************************
    
    /**
     *  Send a channel result message.
     *
     *  \param s_Channel The channel name.
     *  \param s_Address The channel address.
     *  \param u32_Port The channel port.
     *  \param u8_Result The request result.
     */
    
    void SendChannelResponse(std::string const& s_Channel, std::string const& s_Address, uint32_t u32_Port, uint8_t u8_Result) noexcept;
    
    /**
     *  Handle a recieved client channel request.
     *
     *  \param p_Shared The thread shared worker data.
     *  \param c_Request The client request message.
     *
     *  \return true if the connection should be kept, false if not.
     */
    
    bool ChannelRequest(std::unique_ptr<WorkerShared>& p_Shared, NetMessageV1::C_MSG_CHANNEL_REQUEST_DATA c_Request) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    // Connection
    std::unique_ptr<NetConnection> p_Connection;
    
    // Authentication
    bool b_Authenticated;
    int32_t s32_AuthAttempts;
    uint32_t u32_Nonce;
    uint8_t u8_ClientType;
    
    // User Account Data
    std::string s_Password;
    std::string s_DeviceKey;
    
protected:
    
};

#endif /* ConnectionTask_h */
