/**
 *  Main.cpp
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
#include <unistd.h>
#include <sys/stat.h>
#include <csignal>
#include <cstring>
#include <iostream>

// External
#include <sodium.h>

// Project
#include "./Server/Server.h"
#include "./Database/Database.h"
#include "./Job/ThreadPool.h"
#include "./CLI.h"
#include "./Logger.h"
#include "./Revision.h"

// Pre-defined
#ifndef MRH_SRV_DEFAULT_CONFIG_FILE_PATH
    #define MRH_SRV_DEFAULT_CONFIG_FILE_PATH "/usr/local/etc/mrhnetserver.conf"
#endif


//*************************************************************************************
// Data
//*************************************************************************************

namespace
{
    // Parameters
    enum Parameter
    {
        PARAM_BIN = 0,
        PARAM_HELP = 1,
        PARAM_DAEMON = 2,
        PARAM_CONFIG = 3,
        
        PARAM_MIN = PARAM_BIN,
        
        PARAM_MAX = PARAM_CONFIG,

        PARAM_COUNT = PARAM_MAX + 1
        
    };
    
    const char* p_Parameter[PARAM_COUNT] =
    {
        "", // Bin, given by OS
        "-help",
        "-daemon",
        "-config"
    };
    
    // Running
    bool b_Run = true;
}

//*************************************************************************************
// Signal Handler
//*************************************************************************************

// Prevent name wrangling
extern "C"
{
    void SignalHandler(int i_Signal)
    {
        switch (i_Signal)
        {
            case SIGILL:
            case SIGTRAP:
            case SIGFPE:
            case SIGABRT:
            case SIGSEGV:
                Logger::Singleton().Backtrace(25, "Caught Signal: " + std::to_string(i_Signal));
                exit(EXIT_FAILURE);
                break;
                
            case SIGTERM:
                b_Run = false;
                break;
                
            default:
                break;
        }
    }
}

//*************************************************************************************
// Daemon
//*************************************************************************************

static void Daemonize() noexcept
{
    pid_t s32_Pid = 0;
    
    // First fork to leave parent
    if ((s32_Pid = fork()) < 0)
    {
        // Fork error
        exit(EXIT_FAILURE);
    }
    else if (s32_Pid > 0)
    {
        // Parent
        exit(EXIT_SUCCESS);
    }
    
    // Set session id
    if (setsid() < 0)
    {
        exit(EXIT_FAILURE);
    }
    
    // Ignore child signal
    signal(SIGCHLD, SIG_IGN);

    // Second fork to attach to proccess 1
    if ((s32_Pid = fork()) < 0)
    {
        // Fork error
        exit(EXIT_FAILURE);
    }
    else if (s32_Pid > 0)
    {
        // Parent
        exit(EXIT_SUCCESS);
    }
    
    // Set new file permissions
    umask(0);

    // Return to root
    chdir("/");

    // in, out and error to NULL
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");
}

//*************************************************************************************
// Help
//*************************************************************************************

static void PrintHelp() noexcept
{
    std::cout << std::endl;
    std::cout << "MRH Net Server Help:" << std::endl;
    std::cout << "--------------------" << std::endl;
    std::cout << std::endl;
    std::cout << "Binary Parameters:" << std::endl;
    std::cout << "-help: Print help" << std::endl;
    std::cout << "-daemon: Run server as a daemon" << std::endl;
    std::cout << "-config <Path>: Load a config by full path" << std::endl;
    std::cout << std::endl;
}

//*************************************************************************************
// Main
//*************************************************************************************

int main(int argc, const char* argv[])
{
    // Log Setup
    Logger& c_Logger = Logger::Singleton();
    
    c_Logger.Log(Logger::INFO, "=============================================", "Main.cpp", __LINE__);
    c_Logger.Log(Logger::INFO, "= Started MRH Net Server (" + std::string(VERSION_NUMBER) + ")", "Main.cpp", __LINE__);
    c_Logger.Log(Logger::INFO, "=============================================", "Main.cpp", __LINE__);
    
    // Check params
    if (argc < PARAM_MIN)
    {
        c_Logger.Log(Logger::ERROR, "Missing server parameters!",
                     "Main.cpp", __LINE__);
        return EXIT_FAILURE;
    }
    
    bool b_Daemon = false;
    std::string s_ConfigPath(MRH_SRV_DEFAULT_CONFIG_FILE_PATH);
    
    // @NOTE: Skip bin arg
    for (int i = 1; i < argc; ++i)
    {
        for (int j = 1; j < PARAM_COUNT; ++j)
        {
            if (strcmp(p_Parameter[j], argv[i]) != 0)
            {
                continue;
            }
            
            switch (j)
            {
                case PARAM_HELP:
                    PrintHelp();
                    break;
                case PARAM_DAEMON:
                    b_Daemon = true;
                    break;
                case PARAM_CONFIG:
                    if (i + 1 < argc)
                    {
                        s_ConfigPath = argv[i + 1];
                        i += 1; // Skip next arg
                    }
                    else
                    {
                        std::cout << "No config given!" << std::endl;
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    // Install signal handlers
    std::signal(SIGTERM, SignalHandler);
    std::signal(SIGILL, SignalHandler);
    std::signal(SIGTRAP, SignalHandler);
    std::signal(SIGFPE, SignalHandler);
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);
    
    // Init sodium
    if (sodium_init() != 0)
    {
        c_Logger.Log(Logger::ERROR, "Failed to initialize sodium!",
                     "Main.cpp", __LINE__);
        return EXIT_FAILURE;
    }
    
    // Start server
    try
    {
        /**
         *  Configuration
         */
        
        Configuration c_Config(s_ConfigPath);
        
        /**
         *  Mode
         */
        
        // Run as daemon or start cli thread
        if (b_Daemon == true)
        {
            Daemonize();
        }
        else
        {
            CLI::Start(c_Config);
        }
        
        /**
         *  Job List
         */
        
        JobList c_JobList;
        
        /**
         *  Server
         */
        
        // We need a client pool for the server
        ClientPool c_ClientPool(c_JobList);
        
        // Create net server and start
        Server c_Server(c_ClientPool);
        
        c_Server.Start(c_Config.i_Port,
                       c_Config.s_CertFilePath,
                       c_Config.s_KeyFilePath,
                       c_Config.i_ConnectionTimeoutS,
                       c_Config.i_MaxClientCount);
        
        /**
         *  Thread Pool
         */
        
        // Now we need the thread pool
        std::list<std::unique_ptr<ThreadShared>> l_ThreadInfo;
        size_t us_ThreadCount = std::thread::hardware_concurrency();
        
        if (us_ThreadCount == 0)
        {
            us_ThreadCount = 1;
        }
        else
        {
            us_ThreadCount /= 2;
            
            if (us_ThreadCount > 1)
            {
                // @NOTE: Main acts as a extra thread for the thread pool
                us_ThreadCount -= 1;
            }
        }
        
        for (size_t i = 0; i < us_ThreadCount; ++i)
        {
            l_ThreadInfo.emplace_back(new Database(c_Config.s_MySQLAddress,
                                                   c_Config.i_MySQLPort,
                                                   c_Config.s_MySQLUser,
                                                   c_Config.s_MySQLPassword,
                                                   c_Config.s_MySQLDatabase));
        }
        
        // Got thread info, create pool
        ThreadPool c_ThreadPool(c_JobList,
                                l_ThreadInfo);
        
        /**
         *  Update
         */
        
        std::shared_ptr<ThreadShared> p_Database = std::make_shared<Database>(c_Config.s_MySQLAddress,
                                                                              c_Config.i_MySQLPort,
                                                                              c_Config.s_MySQLUser,
                                                                              c_Config.s_MySQLPassword,
                                                                              c_Config.s_MySQLDatabase);
        
        while (b_Run == true)
        {
            try
            {
                // Get job
                std::shared_ptr<Job> p_Job = c_JobList.GetJob();
                
                if (p_Job->Perform(p_Database) == false)
                {
                    c_JobList.AddJob(p_Job);
                }
                
                // Reset job to no longer be owner
                p_Job.reset();
            }
            catch (...)
            {}
        }
        
        /**
         *  Shutdown
         */
        
        // Lock list to kick all threads
        c_JobList.Lock();
        
        // Server end, shutdown
        c_Server.Stop();
    }
    catch (Exception& e)
    {
        c_Logger.Log(Logger::WARNING, e.what2(),
                     "Main.cpp", __LINE__);
    }
    catch (std::exception& e)
    {
        c_Logger.Log(Logger::WARNING, e.what(),
                     "Main.cpp", __LINE__);
    }
    
    // Clean up
    if (b_Daemon == false)
    {
        CLI::Stop();
    }
    
    c_Logger.Log(Logger::INFO, "Server shutdown.",
                 "Main.cpp", __LINE__);
    
    return EXIT_SUCCESS;
}
