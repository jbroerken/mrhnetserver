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

// C / C++

// External

// Project
#include "./ClientPool.h"
#include "../Logger.h"

// Pre-defined
#ifndef CLIENT_EXTENDED_LOGGING
    #define CLIENT_EXTENDED_LOGGING 0
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

ClientPool::ClientPool(JobList& c_JobList) : c_JobList(c_JobList),
                                             us_MemberCount(0)
{}

ClientPool::~ClientPool() noexcept
{}

ClientPool::Member::Member(const QUIC_API_TABLE* p_APITable,
                           HQUIC p_Connection,
                           size_t us_ID)
{
    p_Client = std::make_shared<Client>(p_APITable,
                                        p_Connection,
                                        us_ID);
}

//*************************************************************************************
// Add
//*************************************************************************************

size_t ClientPool::AddClient(const QUIC_API_TABLE* p_APITable, HQUIC p_Connection)
{
    for (size_t i = 0; i < us_MemberCount; ++i)
    {
        if (dq_Member[i].c_Mutex.try_lock() == false)
        {
            continue;
        }
        
        if (dq_Member[i].p_Client == NULL)
        {
            try
            {
                dq_Member[i].p_Client = std::make_shared<Client>(p_APITable,
                                                                 p_Connection,
                                                                 i);
                dq_Member[i].c_Mutex.unlock();
                
#if CLIENT_EXTENDED_LOGGING > 0
                Logger::Singleton().Log(Logger::INFO, "Added client to pool with id " +
                                                      std::to_string(i),
                                        "ClientPool.cpp", __LINE__);
#endif
                return i;
            }
            catch (...)
            {}
        }
        
        dq_Member[i].c_Mutex.unlock();
    }
    
    // No free entry, add a new one
    try
    {
        // Lock for outside multithreading
        std::lock_guard<std::mutex> c_Guard(c_Mutex);
        
        dq_Member.emplace_back(p_APITable,
                               p_Connection,
                               us_MemberCount);
        us_MemberCount += 1;
        
#if CLIENT_EXTENDED_LOGGING > 0
        Logger::Singleton().Log(Logger::INFO, "Added client to pool with id " +
                                              std::to_string(us_MemberCount - 1),
                                "ClientPool.cpp", __LINE__);
#endif
        return (us_MemberCount - 1);
    }
    catch (std::exception& e)
    {
        throw Exception("Failed to add entry: " + std::string(e.what()));
    }
}
 
//*************************************************************************************
// Notify
//*************************************************************************************

void ClientPool::DataRecieved(size_t us_ClientID, StreamData& c_Data) noexcept
{
    if (us_ClientID < us_MemberCount)
    {
        dq_Member[us_ClientID].c_Mutex.lock();
        std::shared_ptr<Client> p_Client = dq_Member[us_ClientID].p_Client;
        dq_Member[us_ClientID].c_Mutex.unlock();
        
        if (p_Client != NULL)
        {
            p_Client->RecieveNetMessage(c_Data);
            c_JobList.AddJob(p_Client);
    
            return;
        }
    }
    
#if CLIENT_EXTENDED_LOGGING > 0
    Logger::Singleton().Log(Logger::WARNING, "Failed to hand recieved net message data to client " +
                                             std::to_string(us_ClientID),
                            "ClientPool.cpp", __LINE__);
#endif
}

void ClientPool::SendableAvailable(size_t us_ClientID) noexcept
{
    if (us_ClientID < us_MemberCount)
    {
        dq_Member[us_ClientID].c_Mutex.lock();
        std::shared_ptr<Client> p_Client = dq_Member[us_ClientID].p_Client;
        dq_Member[us_ClientID].c_Mutex.unlock();
        
        if (p_Client != NULL)
        {
            p_Client->RecieveDataAvailable();
            c_JobList.AddJob(p_Client);
        
            return;
        }
    }
    
#if CLIENT_EXTENDED_LOGGING > 0
    Logger::Singleton().Log(Logger::WARNING, "Failed to hand data available notification to client " +
                                             std::to_string(us_ClientID),
                            "ClientPool.cpp", __LINE__);
#endif
}

//*************************************************************************************
// Remove
//*************************************************************************************

void ClientPool::RemoveClient(size_t us_ClientID) noexcept
{
    if (us_ClientID > us_MemberCount)
    {
        return;
    }
    
    std::lock_guard<std::mutex> c_Guard(dq_Member[us_ClientID].c_Mutex);
    
    dq_Member[us_ClientID].p_Client.reset();
    dq_Member[us_ClientID].p_Client = NULL;
}
