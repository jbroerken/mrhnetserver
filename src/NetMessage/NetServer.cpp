/**
 *  NetServer.cpp
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
#include "./NetServer.h"
#include "./MsQuic.h"

// Pre-defined
#ifndef MRH_SRV_REGISTRATION_NAME
    #define MRH_SRV_REGISTRATION_NAME "mrh_srv_reg"
#endif
#ifndef MRH_SRV_ALPN_NAME
    #define MRH_SRV_ALPN_NAME "mrh_srv_alpn"
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

NetServer::NetServer() : b_Started(false)
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
        throw NetException("Failed to get msquic api table!");
    }
    else if (QUIC_FAILED(ui_Status = ((const QUIC_API_TABLE*)p_APITable)->RegistrationOpen(&c_RegistrationConfig,
                                                                                           &p_Registration)))
    {
        MsQuicClose(p_APITable);
        throw NetException("Failed to get msquic registration!");
    }
    else
    {
        this->p_Registration = p_Registration;
    }
}

NetServer::~NetServer() noexcept
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
// Run
//*************************************************************************************

void NetServer::Start(int i_Port, std::string const& s_CertFilePath, std::string const& s_KeyFilePath, int i_TimeoutS)
{
    if (b_Started == true)
    {
        return;
    }
    else if (p_APITable == NULL)
    {
        throw NetException("No API table to use!");
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
    
    const QUIC_API_TABLE* p_MsQuic = (const QUIC_API_TABLE*)p_APITable;
    
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
    c_Settings.PeerUnidiStreamCount = 1024;
    c_Settings.IsSet.PeerUnidiStreamCount = TRUE;
    //c_Settings.PeerBidiStreamCount = 1024;
    //c_Settings.IsSet.PeerBidiStreamCount = TRUE;

    c_Config.CertFile.CertificateFile = (char*)s_CertFilePath.c_str();
    c_Config.CertFile.PrivateKeyFile = (char*)s_KeyFilePath.c_str();
    c_Config.CredConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
    c_Config.CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    c_Config.CredConfig.CertificateFile = &c_Config.CertFile;
    
    if (QUIC_FAILED(ui_Status = p_MsQuic->ConfigurationOpen((HQUIC)p_Registration,
                                                            &p_Alpn,
                                                            1,
                                                            &c_Settings,
                                                            sizeof(c_Settings),
                                                            NULL,
                                                            &p_Configuration)))
    {
        throw NetException("Failed to open configuration!");
    }
    else if (QUIC_FAILED(ui_Status = p_MsQuic->ConfigurationLoadCredential(p_Configuration,
                                                                           &c_Config.CredConfig)))
    {
        p_MsQuic->ConfigurationClose(p_Configuration);
        
        throw NetException("Failed to load configuration credentials!");
    }
    
    //
    //  Listener
    //
    
    try
    {
        p_Context = new MsQuicListenerContext(p_MsQuic,
                                              p_Configuration);
    }
    catch (...)
    {
        p_MsQuic->ConfigurationClose(p_Configuration);
        
        throw NetException("Failed to create listener context!");
    }
    
    // We need to open a listener for connections, which then starts listening
    // @NOTE: Destructing this class class the ListenerClose() function,
    //        which in turn calls the blocking call ListenerStop().
    //        MsQuic guarantees no callbacks after ListenerClose(), which means
    //        that the context object will never be used in callbacks after stopping!
    if (QUIC_FAILED(ui_Status = p_MsQuic->ListenerOpen((HQUIC)p_Registration,
                                                       MsQuicListenerCallback,
                                                       p_Context,
                                                       &p_Listener)))
    {
        delete p_Context;
        p_Context = NULL;
        
        p_MsQuic->ConfigurationClose(p_Configuration);
        
        throw NetException("Failed to open server listener!");
    }
    
    // Setup completed, start listening
    if (QUIC_FAILED(ui_Status = p_MsQuic->ListenerStart(p_Listener,
                                                        &p_Alpn,
                                                        1,
                                                        &c_Address)))
    {
        delete p_Context;
        p_Context = NULL;
        
        p_MsQuic->ListenerClose(p_Listener);
        p_MsQuic->ConfigurationClose(p_Configuration);
        
        throw NetException("Failed to start server listener!");
    }
    
    this->p_Listener = p_Listener;
    p_Context->p_Configuration = p_Configuration;
    
    b_Started = true;
}

void NetServer::Stop() noexcept
{
    if (b_Started == false)
    {
        return;
    }
    
    // @NOTE: At this point all pointers are valid!
    const QUIC_API_TABLE* p_MsQuic = (const QUIC_API_TABLE*)p_APITable;
    
    p_MsQuic->ListenerClose((HQUIC)p_Listener);
    p_MsQuic->ConfigurationClose(p_Context->p_Configuration);
    
    p_Listener = NULL;
    
    delete p_Context;
    p_Context = NULL;
    
    b_Started = false;
}

//*************************************************************************************
// Getters
//*************************************************************************************

std::list<std::unique_ptr<NetConnection>> NetServer::GetConnections() noexcept
{
    std::list<std::unique_ptr<NetConnection>> l_Result;
    
    std::lock_guard<std::mutex> c_Guard(p_Context->c_Mutex);
    
    for (auto& Connection : p_Context->l_Connection)
    {
        try
        {
            // @NOTE: No owner modification, owner switches from net server
            //        to pointer!
            NetConnection* p_Connection = new NetConnection(Connection);
            l_Result.emplace_back(p_Connection);
        }
        catch (...)
        {
            if (Connection->b_Shared == false)
            {
                printf("\n(NetServer) Delete Context [ %p ]\n", Connection->p_Connection);
                
                // @NOTE: Already closed at this point
                delete Connection;
            }
            else
            {
                printf("\n(NetServer) Shutdown Connection [ %p ]\n", Connection->p_Connection);
                
                Connection->b_Shared = false;
                p_Context->p_APITable->ConnectionShutdown(Connection->p_Connection,
                                                          QUIC_CONNECTION_SHUTDOWN_FLAG_NONE,
                                                          0);
            }
        }
    }
    
    p_Context->l_Connection.clear();
    
    return l_Result; // Force move
}
