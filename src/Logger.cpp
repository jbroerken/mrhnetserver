/**
 *  Logger.cpp
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
#include <execinfo.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <ctime>

// External

// Project
#include "./Logger.h"

// Pre-defined
#ifndef MRH_NET_SERVER_LOG_FILE_PATH
    #define MRH_NET_SERVER_LOG_FILE_PATH "/var/log/mrhnetserver.log"
#endif
#ifndef MRH_NET_SERVER_BACKTRACE_FILE_PATH
    #define MRH_NET_SERVER_BACKTRACE_FILE_PATH "/var/log/bt_mrhnetserver.log"
#endif
#ifndef MRH_LOGGER_PRINT_CLI
    #define MRH_LOGGER_PRINT_CLI 0
#endif


//*************************************************************************************
// Constructor / Destructor
//*************************************************************************************

Logger::Logger() noexcept
{
    f_LogFile.open(MRH_NET_SERVER_LOG_FILE_PATH, std::ios::out | std::ios::trunc);
    f_BacktraceFile.open(MRH_NET_SERVER_BACKTRACE_FILE_PATH, std::ios::out | std::ios::trunc);
    
    if (f_LogFile.is_open() == false)
    {
        Log(Logger::WARNING, "Failed to open log file: " MRH_NET_SERVER_LOG_FILE_PATH,
            "Logger.cpp", __LINE__);
    }
    
    if (f_BacktraceFile.is_open() == false)
    {
        Log(Logger::WARNING, "Failed to open backtrace file: " MRH_NET_SERVER_BACKTRACE_FILE_PATH,
            "Logger.cpp", __LINE__);
    }
}

Logger::~Logger() noexcept
{
    if (f_LogFile.is_open() == true)
    {
        f_LogFile.close();
    }
    
    if (f_BacktraceFile.is_open() == true)
    {
        f_BacktraceFile.close();
    }
}

//*************************************************************************************
// Singleton
//*************************************************************************************

Logger& Logger::Singleton() noexcept
{
    static Logger s_Logger;
    return s_Logger;
}

//*************************************************************************************
// Log
//*************************************************************************************

void Logger::Log(LogLevel e_Level, std::string s_Message, std::string s_File, size_t us_Line) noexcept
{
    c_Mutex.lock();
    
    if (f_LogFile.is_open() == true)
    {
        f_LogFile << "[" << s_File << "][" << std::to_string(us_Line) << "][" << GetLevelString(e_Level) << "]: " << s_Message << std::endl;
    }
    
    if (MRH_LOGGER_PRINT_CLI > 0)
    {
        std::cout << "[" << s_File << "][" << std::to_string(us_Line) << "][" << GetLevelString(e_Level) << "]: " << s_Message << std::endl;
    }
    
    c_Mutex.unlock();
}

//*************************************************************************************
// Backtrace
//*************************************************************************************

void Logger::Backtrace(size_t us_TraceSize, std::string s_Message) noexcept
{
    if (f_BacktraceFile.is_open() == false)
    {
        return;
    }
    
    // Retrieve current stack addresses
    char** p_Traceback = NULL;
    void* p_AdressList[us_TraceSize];
    
    us_TraceSize = backtrace(p_AdressList, (int)(sizeof(p_AdressList) / sizeof(void*)));
    
    // Get names
    if (us_TraceSize > 0)
    {
        p_Traceback = backtrace_symbols(p_AdressList, (int)us_TraceSize);
    }
    
    // File head
    WriteBacktrace(f_BacktraceFile, "====================================");
    WriteBacktrace(f_BacktraceFile, "= " + s_Message);
    WriteBacktrace(f_BacktraceFile, "====================================");
    
    // Print traceback stack
    if (us_TraceSize == 0)
    {
        WriteBacktrace(f_BacktraceFile, "Failed to get traceback!");
    }
    else
    {
        for (int i = 0; i < us_TraceSize && p_Traceback[i] != NULL; ++i)
        {
            WriteBacktrace(f_BacktraceFile, std::string(p_Traceback[i]));
        }
    }
}

void Logger::WriteBacktrace(std::ofstream& f_File, std::string s_Message) noexcept
{
    if (f_File.is_open() == true)
    {
        f_File << s_Message << std::endl;
    }
    
    if (MRH_LOGGER_PRINT_CLI > 0)
    {
        std::cout << s_Message << std::endl;
    }
}

//*************************************************************************************
// Getters
//*************************************************************************************

const char* Logger::GetLevelString(LogLevel e_Level) noexcept
{
    switch (e_Level)
    {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
            
        default:
            return "UNKNOWN";
    }
}
