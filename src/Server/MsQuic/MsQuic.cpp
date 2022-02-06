/**
 *  MsQuic.cpp
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
#include "./MsQuic.h"
#include "./ListenerContext.h"
#include "./ConnectionContext.h"
#include "./StreamContext.h"


//*************************************************************************************
// Listener
//*************************************************************************************

_IRQL_requires_max_(PASSIVE_LEVEL)
_Function_class_(QUIC_LISTENER_CALLBACK)
QUIC_STATUS QUIC_API ListenerCallback(_In_ HQUIC Listener, _In_opt_ void* Context, _Inout_ QUIC_LISTENER_EVENT* Event)
{
    ListenerContext* p_Listener = (ListenerContext*)Context;
    QUIC_STATUS ui_Status;
    
    switch (Event->Type)
    {
        case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        {
            // Connections allowed?
            if (p_Listener->c_Connections.i_ClientConnectionsMax <= p_Listener->c_Connections.i_ClientConnections)
            {
                ui_Status = QUIC_STATUS_CONNECTION_REFUSED;
                break;
            }
            
            try
            {
                // First, create the connection context to use
                ConnectionContext* p_Connection = new ConnectionContext(p_Listener->p_APITable,
                                                                        Event->NEW_CONNECTION.Connection,
                                                                        p_Listener->c_JobList,
                                                                        p_Listener->c_Connections);
                
                // Next, perform API setup
                p_Listener->p_APITable->SetCallbackHandler(Event->NEW_CONNECTION.Connection,
                                                           (void*)ConnectionCallback,
                                                           p_Connection);
                ui_Status = p_Listener->p_APITable->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection,
                                                                               p_Listener->p_Configuration);
                
                // Add connection as active
                p_Listener->c_Connections.i_ClientConnections += 1;
            }
            catch (...)
            {
                // @NOTE: Do NOT call ConnectionClose() here!
                //        Doing so causes a double free.
                //        return a error code instead.
                ui_Status = QUIC_STATUS_CONNECTION_REFUSED;
            }
            
            break;
        }
        
        default:
        {
            ui_Status = QUIC_STATUS_NOT_SUPPORTED;
            break;
        }
    }
    
    return ui_Status;
}

//*************************************************************************************
// Connection
//*************************************************************************************

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_CONNECTION_CALLBACK)
QUIC_STATUS QUIC_API ConnectionCallback(_In_ HQUIC Connection, _In_opt_ void* Context, _Inout_ QUIC_CONNECTION_EVENT* Event)
{
    ConnectionContext* p_Context = (ConnectionContext*)Context;
    
    switch (Event->Type)
    {
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        {
            if (p_Context != NULL)
            {
                p_Context->p_APITable->ConnectionShutdown(Connection,
                                                          QUIC_CONNECTION_SHUTDOWN_FLAG_NONE,
                                                          0);
            }
            break;
        }
            
        case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        {
            // Remove connection as active
            p_Context->c_Connections.i_ClientConnections -= 1;
            
            // Remove context
            if (p_Context != NULL)
            {
                p_Context->p_APITable->ConnectionClose(p_Context->p_Connection);
                delete p_Context;
            }
            break;
        }
            
        case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        {
            StreamContext* p_Stream = NULL;
            
            for (auto It = p_Context->l_Recieved.begin(); It != p_Context->l_Recieved.end(); ++It)
            {
                // Select unused stream
                // @NOTE: Callback happens inline, so we can use
                //        The atomic state for checks.
                if (It->c_Data.e_State == StreamData::FREE)
                {
                    It->c_Data.e_State = StreamData::IN_USE;
                    It->c_Data.v_Bytes.clear();
                    
                    p_Stream = &(*(It));
                    break;
                }
            }
            
            
            // No context, add new
            // @NOTE: No lock, insertion at end does not change existing
            //        iterators
            if (p_Stream == NULL)
            {
                p_Context->l_Recieved.emplace_back(p_Context->p_APITable,
                                                   Connection,
                                                   p_Context->p_Client,
                                                   p_Context->c_JobList);
                
                p_Stream = &(*(--(p_Context->l_Recieved.end())));
                p_Stream->c_Data.e_State = StreamData::IN_USE;
            }
            
            // Got context, start callback
            p_Context->p_APITable->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream,
                                                      (void*)StreamCallback,
                                                      p_Stream);
            break;
        }
            
        case QUIC_CONNECTION_EVENT_CONNECTED: { break; }
        case QUIC_CONNECTION_EVENT_RESUMED: { break; }
        default: { break; }
    }
    
    return QUIC_STATUS_SUCCESS;
}

//*************************************************************************************
// Stream
//*************************************************************************************

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
QUIC_STATUS QUIC_API StreamCallback(_In_ HQUIC Stream, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event)
{
    StreamContext* p_Context = (StreamContext*)Context;
    
    switch (Event->Type)
    {
        case QUIC_STREAM_EVENT_SEND_COMPLETE:
        {
            ((StreamData*)(Event->SEND_COMPLETE.ClientContext))->e_State = StreamData::COMPLETED; // Can be used for sending again
            break;
        }
            
        case QUIC_STREAM_EVENT_RECEIVE:
        {
            for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; ++i)
            {
                uint8_t* p_Start = Event->RECEIVE.Buffers[i].Buffer;// + sizeof(QUIC_BUFFER);
                uint8_t* p_End = p_Start + Event->RECEIVE.Buffers[i].Length;
                
                p_Context->c_Data.v_Bytes.insert(p_Context->c_Data.v_Bytes.end(),
                                                 p_Start,
                                                 p_End);
            }
            break;
        }
            
        case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        {
            p_Context->c_Data.e_State = StreamData::FREE;
            p_Context->p_APITable->StreamShutdown(Stream,
                                                  QUIC_STREAM_SHUTDOWN_FLAG_ABORT,
                                                  0);
            break;
        }
        
        case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        {
            p_Context->c_Data.e_State = StreamData::COMPLETED; // Stream shutdown, so full message
            p_Context->p_APITable->StreamShutdown(Stream,
                                                  QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL,
                                                  0);
            
            // Add message to client
            p_Context->p_Client->Recieve(p_Context->c_Data);
            p_Context->c_Data.e_State = StreamData::FREE;
            
            // Now update client
            p_Context->c_JobList.AddJob(p_Context->p_Client);
            break;
        }
            
        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        {
            if (p_Context != NULL)
            {
                p_Context->p_APITable->StreamClose(Stream);
            }
            break;
        }
            
        default: { break; }
    }
    
    return QUIC_STATUS_SUCCESS;
}
