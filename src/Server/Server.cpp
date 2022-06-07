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
#include "./Server.h"
#include "./MsQuic/MsQuic.h"

// Pre-defined
#ifndef MRH_SRV_REGISTRATION_NAME
    #define MRH_SRV_REGISTRATION_NAME "mrh_srv_reg"
#endif
#ifndef MRH_SRV_ALPN_NAME
    #define MRH_SRV_ALPN_NAME "mrh_srv_alpn"
#endif
#ifndef CLIENT_STREAMS_MULTIPLIER
    #define CLIENT_STREAMS_MULTIPLIER 128 // Client Limit * Streams per Client (expected)
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Server::Server(ClientPool& c_ClientPool) : c_ClientPool(c_ClientPool),
                                           b_Started(false)
{
    QUIC_STATUS ui_Status;
    HQUIC p_Registration;
    QUIC_REGISTRATION_CONFIG c_RegistrationConfig =
    {
        MRH_SRV_REGISTRATION_NAME,
        QUIC_EXECUTION_PROFILE_LOW_LATENCY
    };
    
    if (QUIC_FAILED(ui_Status = MsQuicOpen(&p_APITable)))
    {
        throw Exception("Failed to get msquic api table!");
    }
    else if (QUIC_FAILED(ui_Status = ((const QUIC_API_TABLE*)p_APITable)->RegistrationOpen(&c_RegistrationConfig,
                                                                                           &p_Registration)))
    {
        MsQuicClose(p_APITable);
        throw Exception("Failed to get msquic registration!");
    }
    else
    {
        this->p_Registration = p_Registration;
    }
}

Server::~Server() noexcept
{
    if (p_APITable == NULL)
    {
        return;
    }
    
    if (b_Started == true)
    {
        Stop();
    }
    
    if (p_Registration != NULL)
    {
        ((const QUIC_API_TABLE*)p_APITable)->RegistrationClose((HQUIC)p_Registration);
    }
    
    MsQuicClose(p_APITable);
}

//*************************************************************************************
// Start
//*************************************************************************************

void Server::Start(int i_Port, std::string const& s_CertFilePath, std::string const& s_KeyFilePath, int i_TimeoutS, int i_MaxClientCount)
{
    if (b_Started == true)
    {
        return;
    }
    else if (p_APITable == NULL)
    {
        throw Exception("No API table to use!");
    }
    else if (i_MaxClientCount < 1)
    {
        throw Exception("Server has invalid client connection count!");
    }
    
    //
    //  Values
    //

    // @NOTE: Taken from microsofts sample
    //        https://github.com/microsoft/msquic/blob/main/src/tools/sample/sample.c
    struct QUIC_CREDENTIAL_CONFIG_HELPER {
        QUIC_CREDENTIAL_CONFIG CredConfig;
        union {
            QUIC_CERTIFICATE_FILE CertFile;
            QUIC_CERTIFICATE_FILE_PROTECTED CertFileProtected;
        };
    };
    
    HQUIC p_Configuration;
    HQUIC p_Listener;
    
    QUIC_STATUS ui_Status;
    QUIC_ADDR c_Address = { 0 };
    QUIC_SETTINGS c_Settings = { 0 };
    QUIC_CREDENTIAL_CONFIG_HELPER c_Config;
    QUIC_BUFFER p_Alpn =
    {
        sizeof(MRH_SRV_ALPN_NAME) - 1,
        (uint8_t*)MRH_SRV_ALPN_NAME
    };
    
    //
    //  Configuration
    //
    
    QuicAddrSetFamily(&c_Address, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&c_Address, i_Port);
    
    memset(&c_Config, 0, sizeof(c_Config));
    
    if (i_TimeoutS <= 0)
    {
        i_TimeoutS = 60; // Default recommended
    }
    
    c_Settings.IdleTimeoutMs = i_TimeoutS * 1000;
    c_Settings.IsSet.IdleTimeoutMs = TRUE;
    c_Settings.ServerResumptionLevel = QUIC_SERVER_NO_RESUME;//QUIC_SERVER_RESUME_AND_ZERORTT;
    c_Settings.IsSet.ServerResumptionLevel = TRUE;
    c_Settings.PeerUnidiStreamCount = i_MaxClientCount * CLIENT_STREAMS_MULTIPLIER;
    c_Settings.IsSet.PeerUnidiStreamCount = TRUE;
    c_Settings.KeepAliveIntervalMs = c_Settings.IdleTimeoutMs / 2;
    c_Settings.IsSet.KeepAliveIntervalMs = TRUE;

    c_Config.CertFile.CertificateFile = (char*)s_CertFilePath.c_str();
    c_Config.CertFile.PrivateKeyFile = (char*)s_KeyFilePath.c_str();
    c_Config.CredConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
    c_Config.CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    c_Config.CredConfig.CertificateFile = &c_Config.CertFile;
    
    if (QUIC_FAILED(ui_Status = p_APITable->ConfigurationOpen(p_Registration,
                                                              &p_Alpn,
                                                              1,
                                                              &c_Settings,
                                                              sizeof(c_Settings),
                                                              NULL,
                                                              &p_Configuration)))
    {
        throw Exception("Failed to open configuration!");
    }
    else if (QUIC_FAILED(ui_Status = p_APITable->ConfigurationLoadCredential(p_Configuration,
                                                                             &c_Config.CredConfig)))
    {
        p_APITable->ConfigurationClose(p_Configuration);
        
        throw Exception("Failed to load configuration credentials!");
    }
    
    //
    //  Listener
    //
    
    try
    {
        p_Context = new ListenerContext(p_APITable,
                                        p_Configuration,
                                        c_ClientPool,
                                        i_MaxClientCount);
    }
    catch (...)
    {
        p_APITable->ConfigurationClose(p_Configuration);
        
        throw Exception("Failed to create listener context!");
    }
    
    // We need to open a listener for connections, which then starts listening
    // @NOTE: Destructing this class class the ListenerClose() function,
    //        which in turn calls the blocking call ListenerStop().
    //        MsQuic guarantees no callbacks after ListenerClose(), which means
    //        that the context object will never be used in callbacks after stopping!
    if (QUIC_FAILED(ui_Status = p_APITable->ListenerOpen(p_Registration,
                                                         ListenerCallback,
                                                         p_Context,
                                                         &p_Listener)))
    {
        delete p_Context;
        p_Context = NULL;
        
        p_APITable->ConfigurationClose(p_Configuration);
        
        throw Exception("Failed to open server listener!");
    }
    
    // Setup completed, start listening
    if (QUIC_FAILED(ui_Status = p_APITable->ListenerStart(p_Listener,
                                                          &p_Alpn,
                                                          1,
                                                          &c_Address)))
    {
        delete p_Context;
        p_Context = NULL;
        
        p_APITable->ListenerClose(p_Listener);
        p_APITable->ConfigurationClose(p_Configuration);
        
        throw Exception("Failed to start server listener!");
    }
    
    this->p_Listener = p_Listener;
    p_Context->p_Configuration = p_Configuration;
    
    b_Started = true;
}

//*************************************************************************************
// Stop
//*************************************************************************************

void Server::Stop() noexcept
{
    if (b_Started == false)
    {
        return;
    }
    
    p_APITable->ListenerClose(p_Listener);
    p_APITable->ConfigurationClose(p_Context->p_Configuration);
    
    delete p_Context;
    p_Context = NULL;
    
    b_Started = false;
}
