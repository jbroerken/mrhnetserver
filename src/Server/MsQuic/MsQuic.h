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
