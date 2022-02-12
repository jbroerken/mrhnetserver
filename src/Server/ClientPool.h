/**
 *  ClientPool.h
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

#ifndef ClientPool_h
#define ClientPool_h

// C / C++

// External

// Project
#include "./Client.h"
#include "../Job/JobList.h"


class ClientPool
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param c_JobList The job list to update clients with.
     */
    
    ClientPool(JobList& c_JobList);
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_ClientPool ClientPool class source.
     */
    
    ClientPool(ClientPool const& c_ClientPool) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~ClientPool() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a new client.
     *
     *  \param p_APITable The api table for the client.
     *  \param p_Connection The connection for the client.
     *
     *  \return The id of the added client.
     */
    
    size_t AddClient(const QUIC_API_TABLE* p_APITable, HQUIC p_Connection);
    
    //*************************************************************************************
    // Notify
    //*************************************************************************************
    
    /**
     *  Notify a client of recieved data.
     *
     *  \param us_ClientID The id of the client.
     *  \param c_Data The recieved data.
     */
    
    void DataRecieved(size_t us_ClientID, StreamData& c_Data) noexcept;
    
    /**
     *  Notify a client of available data to send.
     *
     *  \param us_ClientID The id of the client.
     */
    
    void SendableAvailable(size_t us_ClientID) noexcept;
    
    //*************************************************************************************
    // Remove
    //*************************************************************************************
    
    /**
     *  Remove a client from the list.
     *  
     *  \param us_ClientID The id of the client.
     */
    
    void RemoveClient(size_t us_ClientID) noexcept;
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Member
    {
        //*************************************************************************************
        // Constructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         *
         *  \param p_APITable The api table for the member client
         *  \param p_Connection The connection for the member client.
         *  \param us_ID The id for the member client.
         */
        
        Member(const QUIC_API_TABLE* p_APITable,
               HQUIC p_Connection,
               size_t us_ID);
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::mutex c_Mutex;
        std::shared_ptr<Client> p_Client;
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    JobList& c_JobList;
    
    std::mutex c_Mutex;
    std::deque<Member> dq_Member;
    std::atomic<size_t> us_MemberCount;
    
protected:

};

#endif /* ClientPool_h */
