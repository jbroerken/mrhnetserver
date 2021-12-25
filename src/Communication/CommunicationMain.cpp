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
#include "./ExchangeContainer.h"//"./CommunicationTask.h"
#include "../WorkerPool/WorkerPool.h"
#include "../NetMessage/NetServer.h"
#include "../Database/Database.h"
#include "../Timer.h"
#include "../Logger.h"


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
                          c_Config.i_ConnectionTimeoutS);
        
        // Create used message exchange
        ExchangeContainer c_ExchangeContainer(SERVER_COMMUNICATION);
        
        // Now we need the worker context and threads
        std::list<std::unique_ptr<WorkerShared>> l_ThreadInfo;
        size_t us_ThreadCount = std::thread::hardware_concurrency();
        
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
        
        // We can now start updating connections
        std::list<std::unique_ptr<NetConnection>> l_Connection;
        Timer c_Timer;
        
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
                    /*
                    std::unique_ptr<WorkerTask> p_Task = std::make_unique<ConnectionTask>(Connection);
                    c_WorkerPool.AddTask(p_Task);
                    */
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
        }
        
        // Server end, shutdown
        c_NetServer.Stop();
    }
    catch (NetException& e)
    {
        c_Logger.Log(Logger::WARNING, e.what2() +
                                      "(Connection Server)",
                     "CommunicationMain.cpp", __LINE__);
    }
    catch (ServerException& e)
    {
        c_Logger.Log(Logger::WARNING, e.what2() +
                                      "(Connection Server)",
                     "CommunicationMain.cpp", __LINE__);
    }
    catch (std::exception& e)
    {
        c_Logger.Log(Logger::WARNING, std::string(e.what()) +
                                      "(Connection Server)",
                     "CommunicationMain.cpp", __LINE__);
    }
}
