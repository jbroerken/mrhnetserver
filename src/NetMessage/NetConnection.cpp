/**
 *  NetConnection.cpp
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
#include "./NetConnection.h"
#include "./MsQuic.h"


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

NetConnection::NetConnection(MsQuicConnectionContext* p_Context) : p_Context(p_Context)
{}

NetConnection::~NetConnection() noexcept
{
    if (p_Context->b_Shared == false)
    {
        // @NOTE: Already closed at this point
        //printf("\n(NetConnection) Delete Context [ %p ]\n", p_Context->p_Connection);
        delete p_Context;
    }
    else
    {
        //printf("\n(NetConnection) Shutdown Connection [ %p ]\n", p_Context->p_Connection);
        p_Context->b_Shared = false;
        p_Context->p_APITable->ConnectionShutdown(p_Context->p_Connection,
                                                  QUIC_CONNECTION_SHUTDOWN_FLAG_NONE,
                                                  0);
    }
}

//*************************************************************************************
// Recieve
//*************************************************************************************

bool NetConnection::Recieve(NetMessage& c_Message)
{
    for (auto& Recieved : p_Context->l_Recieved)
    {
        if (Recieved.e_State == MsQuicMessageContext::COMPLETED)
        {
            c_Message.v_Data.swap(Recieved.v_Bytes);
            Recieved.e_State = MsQuicMessageContext::FREE;
            return true;
        }
    }
    
    return false;
}

//*************************************************************************************
// Send
//*************************************************************************************

void NetConnection::Send(NetMessage& c_Message)
{
    if (c_Message.GetID() == NetMessage::CS_MSG_UNK)
    {
        throw NetException("Unknown message id!");
    }
    
    // Find a free message first
    MsQuicMessageContext* p_Message = NULL;
    
    for (auto It = p_Context->l_Send.begin(); It != p_Context->l_Send.end(); ++It)
    {
        // Select empty message available
        if (It->e_State == MsQuicMessageContext::FREE ||
            It->e_State == MsQuicMessageContext::COMPLETED)
        {
            It->e_State = MsQuicMessageContext::IN_USE;
            p_Message = &(*(It));
        }
    }
    
    // No message, add new
    if (p_Message == NULL)
    {
        p_Context->l_Send.emplace_back(p_Context->p_APITable,
                                       MsQuicMessageContext::IN_USE);
        
        p_Message = &(*(--(p_Context->l_Send.end())));
    }
    
    // Add the send data
    p_Message->v_Bytes.swap(c_Message.v_Data);
    
    // Now we perform the quic buffer setup
    p_Message->v_Bytes.insert(p_Message->v_Bytes.begin(),
                              sizeof(QUIC_BUFFER),
                              0);
    
    QUIC_BUFFER* p_QuicBuffer;
    
    p_QuicBuffer = (QUIC_BUFFER*)&(p_Message->v_Bytes[0]);
    p_QuicBuffer->Buffer = &(p_Message->v_Bytes[sizeof(QUIC_BUFFER)]);
    p_QuicBuffer->Length = p_Message->v_Bytes.size() - sizeof(QUIC_BUFFER);
    
    // Buffer is setup, send data
    HQUIC p_Stream;
    
    if (QUIC_FAILED(p_Context->p_APITable->StreamOpen(p_Context->p_Connection,
                                                      QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL, /* QUIC_STREAM_OPEN_FLAG_NONE, */
                                                      MsQuicStreamCallback,
                                                      p_Message, /* Pass message as context */
                                                      &p_Stream)))
    {
        p_Message->e_State = MsQuicMessageContext::COMPLETED;
        throw NetException("Failed to open stream!");
    }
    else if (QUIC_FAILED(p_Context->p_APITable->StreamStart(p_Stream,
                                                            QUIC_STREAM_START_FLAG_SHUTDOWN_ON_FAIL)))
    {
        p_Context->p_APITable->StreamClose(p_Stream);
        p_Message->e_State = MsQuicMessageContext::COMPLETED;
        throw NetException("Failed to start stream!");
    }
    else if (QUIC_FAILED(p_Context->p_APITable->StreamSend(p_Stream,
                                                           p_QuicBuffer,
                                                           1,
                                                           QUIC_SEND_FLAG_FIN,
                                                           p_Message))) /* send context */
    {
        p_Context->p_APITable->StreamClose(p_Stream);
        p_Message->e_State = MsQuicMessageContext::COMPLETED;
        throw NetException("Failed to send on stream!");
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

bool NetConnection::GetConnected() noexcept
{
    return p_Context->p_Connection != NULL ? true : false;
}
