/**
 *  MRH_NetMessageServer.cpp
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


//*************************************************************************************
// Listener
//*************************************************************************************

_IRQL_requires_max_(PASSIVE_LEVEL)
_Function_class_(QUIC_LISTENER_CALLBACK)
QUIC_STATUS QUIC_API MsQuicListenerCallback(_In_ HQUIC Listener, _In_opt_ void* Context, _Inout_ QUIC_LISTENER_EVENT* Event)
{
    MsQuicListenerContext* p_Listener = (MsQuicListenerContext*)Context;
    QUIC_STATUS ui_Status;
    
    switch (Event->Type)
    {
        case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        {
            if (p_Listener->i_MaxClientCount <= p_Listener->i_CurClientCount)
            {
                ui_Status = QUIC_STATUS_CONNECTION_REFUSED;
                break;
            }
            
            try
            {
                // First, create the connection context to use.
                MsQuicConnectionContext* p_Connection = new MsQuicConnectionContext(p_Listener->p_APITable,
                                                                                    Event->NEW_CONNECTION.Connection,
                                                                                    p_Listener->i_CurClientCount);
                
                // Next, perform API setup
                p_Listener->p_APITable->SetCallbackHandler(Event->NEW_CONNECTION.Connection,
                                                           (void*)MsQuicConnectionCallback,
                                                           p_Connection);
                ui_Status = p_Listener->p_APITable->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection,
                                                                               p_Listener->p_Configuration);
                
                // Now add connection to new connections
                // @NOTE: Lock here, list order might be modified
                std::lock_guard<std::mutex> c_Guard(p_Listener->c_Mutex);
                p_Listener->l_Connection.emplace_back(p_Connection);
                
                // Add connection as active
                p_Listener->i_CurClientCount += 1;
                
                int i_NewClientCount = p_Listener->i_CurClientCount;
                //printf("\n(MsQuicListenerCallback) i_CurClientCount [ %d ]\n", i_NewClientCount);
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
QUIC_STATUS QUIC_API MsQuicConnectionCallback(_In_ HQUIC Connection, _In_opt_ void* Context, _Inout_ QUIC_CONNECTION_EVENT* Event)
{
    MsQuicConnectionContext* p_Context = (MsQuicConnectionContext*)Context;
    
    switch (Event->Type)
    {
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        {
            if (p_Context->p_Connection != NULL)
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
            p_Context->i_CurClientCount -= 1;
            
            int i_NewClientCount = p_Context->i_CurClientCount;
            //printf("\n(MsQuicConnectionCallback) i_CurClientCount [ %d ]\n", i_NewClientCount);
            
            // Close if not already closed
            if (p_Context->p_Connection != NULL)
            {
                p_Context->p_APITable->ConnectionClose(p_Context->p_Connection);
                p_Context->p_Connection = NULL;
            }
            
            // No longer owned by anyone?
            if (p_Context->b_Shared == false)
            {
                //printf("\n(MsQuicConnectionCallback) Delete Context [ %p ]\n", Connection);
                delete p_Context;
            }
            else
            {
                //printf("\n(MsQuicConnectionCallback) Shutdown Connection [ %p ]\n", Connection);
                p_Context->b_Shared = false; // Signal deletion OK
            }
            break;
        }
            
        case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        {
            MsQuicMessageContext* p_Message = NULL;
            
            for (auto It = p_Context->l_Recieved.begin(); It != p_Context->l_Recieved.end(); ++It)
            {
                // Select empty message available
                // @NOTE: Callback happens inline, so we can use
                //        The atomic state for checks.
                if (It->e_State == MsQuicMessageContext::FREE)
                {
                    It->e_State = MsQuicMessageContext::IN_USE;
                    It->v_Bytes.clear();
                    
                    p_Message = &(*(It));
                }
            }
            
            // No message, add new
            // @NOTE: No lock, insertion at end does not change existing
            //        iterators
            if (p_Message == NULL)
            {
                p_Context->l_Recieved.emplace_back(p_Context->p_APITable,
                                                   MsQuicMessageContext::IN_USE);
                
                p_Message = &(*(--(p_Context->l_Recieved.end())));
            }
            
            p_Context->p_APITable->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream,
                                                      (void*)MsQuicStreamCallback,
                                                      p_Message);
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
QUIC_STATUS QUIC_API MsQuicStreamCallback(_In_ HQUIC Stream, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event)
{
    MsQuicMessageContext* p_Message = (MsQuicMessageContext*)Context;
    
    switch (Event->Type)
    {
        case QUIC_STREAM_EVENT_SEND_COMPLETE:
        {
            ((MsQuicMessageContext*)(Event->SEND_COMPLETE.ClientContext))->e_State = MsQuicMessageContext::COMPLETED; // Can be used for sending again
            break;
        }
            
        case QUIC_STREAM_EVENT_RECEIVE:
        {
            for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; ++i)
            {
                uint8_t* p_Start = Event->RECEIVE.Buffers[i].Buffer;// + sizeof(QUIC_BUFFER);
                uint8_t* p_End = p_Start + Event->RECEIVE.Buffers[i].Length;
                
                p_Message->v_Bytes.insert(p_Message->v_Bytes.end(),
                                          p_Start,
                                          p_End);
            }
            break;
        }
            
        case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        {
            p_Message->e_State = MsQuicMessageContext::FREE;
            p_Message->p_APITable->StreamShutdown(Stream,
                                                  QUIC_STREAM_SHUTDOWN_FLAG_ABORT,
                                                  0);
            break;
        }
        
        case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        {
            p_Message->e_State = MsQuicMessageContext::COMPLETED; // Stream shutdown, so full message
            p_Message->p_APITable->StreamShutdown(Stream,
                                                  QUIC_STREAM_SHUTDOWN_FLAG_GRACEFUL,
                                                  0);
            break;
        }
            
        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        {
            if (p_Message != NULL)
            {
                p_Message->p_APITable->StreamClose(Stream);
            }
            break;
        }
            
        default: { break; }
    }
    
    return QUIC_STATUS_SUCCESS;
}
