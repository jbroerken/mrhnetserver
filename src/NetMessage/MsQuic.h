/**
 *  MsQuic.h
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

#ifndef MsQuic_h
#define MsQuic_h

// C / C++
#include <atomic>
#include <mutex>
#include <list>
#include <vector>

// External
#include <msquic.h>

// Project


//*************************************************************************************
// Context
//*************************************************************************************

struct MsQuicMessageContext
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    enum State
    {
        FREE = 0,
        IN_USE = 1,
        COMPLETED = 2
    };
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     *  \param e_State The initial message state.
     */
    
    MsQuicMessageContext(const QUIC_API_TABLE* p_APITable,
                         State e_State) noexcept : p_APITable(p_APITable),
                                                   e_State(e_State)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    
    std::atomic<State> e_State; // Currently reading / writing
    
    std::vector<uint8_t> v_Bytes;
};

struct MsQuicConnectionContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     *  \param p_Connection The managed connection.
     */
    
    MsQuicConnectionContext(const QUIC_API_TABLE* p_APITable,
                            HQUIC p_Connection,
                            std::atomic<int>& i_CurClientCount) noexcept : b_Shared(true),
                                                                           p_APITable(p_APITable),
                                                                           p_Connection(p_Connection),
                                                                           i_CurClientCount(i_CurClientCount)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    std::atomic<bool> b_Shared; // Needed, decides who destroys the context
    
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Connection;
    
    std::list<MsQuicMessageContext> l_Recieved;
    std::list<MsQuicMessageContext> l_Send;
    
    std::atomic<int>& i_CurClientCount;
};

struct MsQuicListenerContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     *  \param p_Configuration The library configuration.
     *  \param i_MaxClientCount The max number of clients which can connect.
     */
    
    MsQuicListenerContext(const QUIC_API_TABLE* p_APITable,
                          HQUIC p_Configuration,
                          int i_MaxClientCount) noexcept : p_APITable(p_APITable),
                                                           p_Configuration(p_Configuration),
                                                           i_MaxClientCount(i_MaxClientCount),
                                                           i_CurClientCount(0)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    std::mutex c_Mutex; // Connection list
    
    const QUIC_API_TABLE* p_APITable;
    HQUIC p_Configuration;
    
    std::list<MsQuicConnectionContext*> l_Connection;
    
    int i_MaxClientCount;
    std::atomic<int> i_CurClientCount;
};

//*************************************************************************************
// Listener
//*************************************************************************************

/**
 *  MsQuic server listener callback.
 *
 *  \param Listener The used listener.
 *  \param Context The provided listener context.
 *  \param Event The recieved listener event.
 *
 *  \return The callback result.
 */

_IRQL_requires_max_(PASSIVE_LEVEL)
_Function_class_(QUIC_LISTENER_CALLBACK)
QUIC_STATUS QUIC_API MsQuicListenerCallback(_In_ HQUIC Listener, _In_opt_ void* Context, _Inout_ QUIC_LISTENER_EVENT* Event);

//*************************************************************************************
// Connection
//*************************************************************************************

/**
 *  MsQuic server connection callback.
 *
 *  \param Connection The connection for the callback.
 *  \param Context The provided connection context.
 *  \param Event The recieved connection event.
 *
 *  \return The callback result.
 */

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_CONNECTION_CALLBACK)
QUIC_STATUS QUIC_API MsQuicConnectionCallback(_In_ HQUIC Connection, _In_opt_ void* Context, _Inout_ QUIC_CONNECTION_EVENT* Event);

//*************************************************************************************
// Stream
//*************************************************************************************

/**
 *  MsQuic server stream callback.
 *
 *  \param Stream The stream for the callback.
 *  \param Context The provided stream context.
 *  \param Event The recieved stream event.
 *
 *  \return The callback result.
 */

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
QUIC_STATUS QUIC_API MsQuicStreamCallback(_In_ HQUIC Stream, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event);


#endif /* MsQuic_h */
