/**
 *  CommunicationMain.cpp
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
#include "./CommunicationMain.h"
#include "./CommunicationTask.h"
#include "../WorkerPool/WorkerPool.h"
#include "../NetMessage/NetServer.h"
#include "../Database/Database.h"
#include "../Timer.h"
#include "../Logger.h"

// Pre-defined
#ifndef COMMUNICATION_SERVER_SET_LAST_UPDATE_MS
    #define COMMUNICATION_SERVER_SET_LAST_UPDATE_MS 240 * 1000
#endif

//*************************************************************************************
// Channel List
//*************************************************************************************

static void ClearConnections(Database& c_Database, int i_ChannelID)
{
    try
    {
        c_Database
            .c_Session
            .getSchema(c_Database.s_Database)
            .getTable(DatabaseTable::p_CDCTableName)
            .remove()
            .where(std::string(DatabaseTable::p_CDCFieldName[DatabaseTable::CDC_CHANNEL_ID]) +
                   " == :value")
            .bind("value",
                  i_ChannelID)
            .execute();
        
        c_Database
            .c_Session
            .getSchema(c_Database.s_Database)
            .getTable(DatabaseTable::p_CLTableName)
            .update()
            .set(std::string(DatabaseTable::p_CLFieldName[DatabaseTable::CL_ASSISTANT_CONNECTIONS]), 0)
            .where(std::string(DatabaseTable::p_CLFieldName[DatabaseTable::CL_CHANNEL_ID]) +
                   " == :value")
            .bind("value",
                  i_ChannelID)
            .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, e.what(),
                                "CommunicationMain.cpp", __LINE__);
    }
}

static void SetLastUpdate(Database& c_Database, int i_ChannelID) noexcept
{
    try
    {
        c_Database
            .c_Session
            .getSchema(c_Database.s_Database)
            .getTable(DatabaseTable::p_CLTableName)
            .update()
            .set(DatabaseTable::p_CLFieldName[DatabaseTable::CL_LAST_UPDATE],
                 time(NULL))
            .where(std::string(DatabaseTable::p_CLFieldName[DatabaseTable::CL_CHANNEL_ID]) +
                   " == :value")
            .bind("value",
                  i_ChannelID)
            .execute();
    }
    catch (std::exception& e)
    {
        Logger::Singleton().Log(Logger::ERROR, e.what(),
                                "CommunicationMain.cpp", __LINE__);
    }
}

//*************************************************************************************
// Run
//*************************************************************************************

void CommunicationMain::Run(Configuration& c_Config, bool& b_Run)
{
    Logger& c_Logger = Logger::Singleton();
    
    // @NOTE: Everything depends on the top created objects, so
    //        we need everything in the giant try catch block sadly
    try
    {
        // Create net server first and start
        NetServer c_NetServer;
        c_NetServer.Start(c_Config.i_Port,
                          c_Config.s_CertFilePath,
                          c_Config.s_KeyFilePath,
                          c_Config.i_ConnectionTimeoutS,
                          c_Config.i_MaxClientCount);
        
        // Create used message exchange
        ExchangeContainer c_ExchangeContainer(SERVER_COMMUNICATION);
        
        // Now we need the worker context and threads
        std::list<std::unique_ptr<WorkerShared>> l_ThreadInfo;
        size_t us_ThreadCount = std::thread::hardware_concurrency();
        
        if (us_ThreadCount == 0)
        {
            us_ThreadCount = 1;
        }
        
        for (size_t i = 0; i < us_ThreadCount; ++i)
        {
            l_ThreadInfo.emplace_back(new Database(c_Config.s_MySQLAddress,
                                                   c_Config.i_MySQLPort,
                                                   c_Config.s_MySQLUser,
                                                   c_Config.s_MySQLPassword,
                                                   c_Config.s_MySQLDatabase));
        }
        
        WorkerPool c_WorkerPool(l_ThreadInfo,
                                SERVER_CONNECTION,
                                c_Config.i_MessagePulseMS);
        
        // Create session to use for main thread update
        Database c_Database(c_Config.s_MySQLAddress,
                            c_Config.i_MySQLPort,
                            c_Config.s_MySQLUser,
                            c_Config.s_MySQLPassword,
                            c_Config.s_MySQLDatabase);
        
        // Insert active state
        ClearConnections(c_Database, c_Config.i_ChannelID);
        SetLastUpdate(c_Database, c_Config.i_ChannelID);
        
        Logger::Singleton().Log(Logger::INFO, "Performed database setup for server.",
                                "CommunicationMain.cpp", __LINE__);
        
        // We can now start updating connections
        std::list<std::unique_ptr<NetConnection>> l_Connection;
        Timer c_Timer;
        uint64_t u64_SetLastUpdate = 0;
        
        while (b_Run == true)
        {
            // Set timer for pulse diff
            c_Timer.Set(c_Config.i_ConnectionPulseMS);
            
            // Get new connections
            l_Connection = c_NetServer.GetConnections();
            
            for (auto& Connection : l_Connection)
            {
                try
                {
                    std::unique_ptr<WorkerTask> p_Task = std::make_unique<CommunicationTask>(Connection,
                                                                                             c_ExchangeContainer,
                                                                                             c_Config.i_ChannelID);
                    c_WorkerPool.AddTask(p_Task);
                }
                catch (ServerException& e)
                {
                    c_Logger.Log(Logger::WARNING, "Failed to add connection: " +
                                                  e.what2(),
                                 "CommunicationMain.cpp", __LINE__);
                }
                catch (std::exception& e)
                {
                    c_Logger.Log(Logger::WARNING, "Failed to add connection: " +
                                                  std::string(e.what()),
                                 "CommunicationMain.cpp", __LINE__);
                }
            }
            
            // Clear, causes failed additions to be disconnected and destroyed
            l_Connection.clear();
            
            // Wait for next pulse
            if (c_Timer.GetFinished() == false)
            {
                std::this_thread::sleep_for(c_Timer.GetTimeRemaining());
            }
            
            // Set last update field?
            u64_SetLastUpdate += c_Config.i_ConnectionPulseMS;
            
            if (u64_SetLastUpdate > COMMUNICATION_SERVER_SET_LAST_UPDATE_MS)
            {
                SetLastUpdate(c_Database, c_Config.i_ChannelID);
                u64_SetLastUpdate = 0;
            }
        }
        
        // Server end, shutdown
        c_NetServer.Stop();
    }
    catch (NetException& e)
    {
        c_Logger.Log(Logger::WARNING, e.what2(),
                     "CommunicationMain.cpp", __LINE__);
    }
    catch (ServerException& e)
    {
        c_Logger.Log(Logger::WARNING, e.what2(),
                     "CommunicationMain.cpp", __LINE__);
    }
    catch (std::exception& e)
    {
        c_Logger.Log(Logger::WARNING, e.what(),
                     "CommunicationMain.cpp", __LINE__);
    }
}
