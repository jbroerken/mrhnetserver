/**
 *  MessageExchange.cpp
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

// C / C++

// External

// Project
#include "./ExchangeContainer.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

ExchangeContainer::ExchangeContainer(ActorType e_Type) noexcept : e_Type(e_Type)
{}

ExchangeContainer::~ExchangeContainer() noexcept
{}

//*************************************************************************************
// Add
//*************************************************************************************

std::shared_ptr<MessageExchange> ExchangeContainer::CreateExchange(std::string const& s_DeviceKey)
{
    uint32_t u32_Hash = HashDeviceKey(s_DeviceKey);
    
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    
    // Already a hash for this?
    auto It = m_Exchange.find(u32_Hash);
    
    if (It == m_Exchange.end())
    {
        // Not in use, add map spot
        auto Insert = m_Exchange.insert(std::make_pair(u32_Hash, std::deque<std::shared_ptr<MessageExchange>>()));
        
        if (Insert.second == false)
        {
            throw ServerException("Failed to add exchange list entry for device key " + s_DeviceKey,
                                  e_Type);
        }
        
        It = Insert.first;
    }
    
    // Already added?
    if (It->second.size() > 0)
    {
        for (auto& Exchange : It->second)
        {
            if (Exchange->s_DeviceKey.compare(s_DeviceKey) == 0)
            {
                throw ServerException("Exchange already exists for device key " + s_DeviceKey,
                                      e_Type);
            }
        }
    }
    
    // Add new and return copy
    try
    {
        std::shared_ptr<MessageExchange> p_Exchange = std::make_shared<MessageExchange>(s_DeviceKey);
        It->second.emplace_back(p_Exchange);
        
        return p_Exchange;
    }
    catch (std::exception& e)
    {
        throw ServerException("Failed to add exchange copy for device key " + s_DeviceKey + ": " + std::string(e.what()),
                              e_Type);
    }
}

//*************************************************************************************
// Remove
//*************************************************************************************

void ExchangeContainer::RemoveExchange(std::string const& s_DeviceKey) noexcept
{
    // Get exchange and simply have shared ptr result run out of scope
    try
    {
        GetExchange(s_DeviceKey);
    }
    catch (...)
    {}
}

//*************************************************************************************
// Getters
//*************************************************************************************

uint32_t ExchangeContainer::HashDeviceKey(std::string const& s_DeviceKey) noexcept
{
    uint32_t u32_Result = 0;
    size_t us_Length = s_DeviceKey.size();
    
    for (size_t i = 0; i < us_Length;)
    {
        u32_Result += s_DeviceKey[i++];
        u32_Result += u32_Result << 10;
        u32_Result ^= u32_Result >> 6;
    }
    
    u32_Result += u32_Result << 3;
    u32_Result ^= u32_Result >> 11;
    u32_Result += u32_Result << 15;
    
    return u32_Result;
}

std::shared_ptr<MessageExchange> ExchangeContainer::GetExchange(std::string const& s_DeviceKey)
{
    uint32_t u32_Hash = HashDeviceKey(s_DeviceKey);
    
    std::lock_guard<std::mutex> c_Guard(c_Mutex);
    
    // Find spot first
    auto It = m_Exchange.find(u32_Hash);
    
    if (It == m_Exchange.end())
    {
        throw ServerException("No exchange list for device key " + s_DeviceKey,
                              e_Type);
    }
    
    // Check collision in keys
    if (It->second.size() == 1)
    {
        std::shared_ptr<MessageExchange> p_Result(It->second.front());
        It->second.pop_front();
        
        return p_Result;
    }
    else if (It->second.size() > 1)
    {
        // We have a collision, check each entry for
        for (size_t i = 0; i < It->second.size(); ++i)
        {
            if (It->second[i]->s_DeviceKey.compare(s_DeviceKey) == 0)
            {
                std::shared_ptr<MessageExchange> p_Result(It->second[i]);
                It->second.erase(It->second.begin() + i);
                
                return p_Result;
            }
        }
    }
    
    throw ServerException("No exchange for device key " + s_DeviceKey,
                          e_Type);
}
