/**
 *  ExchangeContainer.h
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

#ifndef ExchangeContainer_h
#define ExchangeContainer_h

// C / C++
#include <memory>
#include <unordered_map>
#include <deque>

// External

// Project
#include "./MessageExchange.h"
#include "../ServerException.h"


class ExchangeContainer
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param e_Type The type of server actor using the exchange container.
     */
    
    ExchangeContainer(ActorType e_Type) noexcept;
    
    /**
     *  Copy constructor. Disabled for this class.
     *
     *  \param c_ExchangeContainer ExchangeContainer class source.
     */
    
    ExchangeContainer(ExchangeContainer const& c_ExchangeContainer) = delete;
    
    /**
     *  Default destructor.
     */
    
    ~ExchangeContainer() noexcept;
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Create a message exchange object for two clients. A copy of the exchange object
     *  is stored in the container for the other client to retrieve.
     *
     *  \param s_DeviceKey The device key for the client pair.
     *
     *  \return The created message exchange.
     */
    
    std::shared_ptr<MessageExchange> CreateExchange(std::string const& s_DeviceKey);
    
    /**
     *  Add a message exchange object owned by a client to be retrieved by another client.
     *
     *  \param p_Exchange The message exchange object.
     */
    
    void AddExchange(std::shared_ptr<MessageExchange> p_Exchange);
    
    //*************************************************************************************
    // Remove
    //*************************************************************************************
    
    /**
     *  Remove a stored message exchange object.
     *
     *  \param s_DeviceKey The device key for the message exchange.
     */
    
    void RemoveExchange(std::string const& s_DeviceKey) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get a stored message exchange object.
     *
     *  \param s_DeviceKey The device key for the message exchange.
     *
     *  \return The matching message exchange object.
     */
    
    std::shared_ptr<MessageExchange> GetExchange(std::string const& s_DeviceKey);
    
private:
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Create a hash for a device key.
     *
     *  \param s_DeviceKey The device key to hash.
     *
     *  \return The created hash for the device key.
     */
    
    uint32_t HashDeviceKey(std::string const& s_DeviceKey) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    ActorType e_Type;
    
    std::unordered_map<uint32_t, std::deque<std::shared_ptr<MessageExchange>>> m_Exchange;
    std::mutex c_Mutex;
    
protected:
    
};

#endif /* ExchangeContainer_h */
