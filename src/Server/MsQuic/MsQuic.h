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

// External
#include <msquic.h>

// Project


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
QUIC_STATUS QUIC_API ListenerCallback(_In_ HQUIC Listener, _In_opt_ void* Context, _Inout_ QUIC_LISTENER_EVENT* Event);

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
QUIC_STATUS QUIC_API ConnectionCallback(_In_ HQUIC Connection, _In_opt_ void* Context, _Inout_ QUIC_CONNECTION_EVENT* Event);

//*************************************************************************************
// Stream
//*************************************************************************************

/**
 *  MsQuic server stream recieve callback.
 *
 *  \param Stream The stream for the callback.
 *  \param Context The provided stream context.
 *  \param Event The recieved stream event.
 *
 *  \return The callback result.
 */

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
QUIC_STATUS QUIC_API StreamRecieveCallback(_In_ HQUIC Stream, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event);

/**
 *  MsQuic server stream send callback.
 *
 *  \param Stream The stream for the callback.
 *  \param Context The provided stream context.
 *  \param Event The recieved stream event.
 *
 *  \return The callback result.
 */

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
QUIC_STATUS QUIC_API StreamSendCallback(_In_ HQUIC Stream, _In_opt_ void* Context, _Inout_ QUIC_STREAM_EVENT* Event);


#endif /* MsQuic_h */
