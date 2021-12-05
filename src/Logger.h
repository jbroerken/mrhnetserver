/**
 *  Logger.h
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

#ifndef Logger_h
#define Logger_h

// C / C++
#include <mutex>
#include <fstream>
#include <string>

// External

// Project


class Logger
{
public:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    typedef enum
    {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
        
        LOG_LEVEL_MAX = 2,
        
        LOG_LEVEL_COUNT = 3
        
    }LogLevel;

    //*************************************************************************************
    // Singleton
    //*************************************************************************************

    /**
     *  Get the class instance. This function is thread safe.
     *
     *  \return The class instance.
     */

    static Logger& Singleton() noexcept;
    
    //*************************************************************************************
    // Log
    //*************************************************************************************
    
    /**
     *  Log a message. This function is thread safe.
     *
     *  \param s_Message The message to log.
     *  \param s_File The source file this log was created from.
     *  \param us_Line The source file line this log was created from.
     */
    
    void Log(LogLevel e_Level, std::string s_Message, std::string s_File, size_t us_Line) noexcept;
    
    //*************************************************************************************
    // Backtrace
    //*************************************************************************************
    
    /**
     *  Write the program backtrace.
     *
     *  \param us_TraceSize The size of the backtrace.
     *  \param s_Message The message describing the backtrace cause.
     */
    
    void Backtrace(size_t us_TraceSize, std::string s_Message) noexcept;
    
private:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    Logger() noexcept;
    
    /**
     *  Default destructor.
     */
    
    ~Logger() noexcept;
    
    //*************************************************************************************
    // Backtrace
    //*************************************************************************************
    
    /**
     *  Write a line from the backtrace.
     *
     *  \param f_File The file to write to.
     *  \param s_Message The message to write.
     */
    
    void WriteBacktrace(std::ofstream& f_File, std::string s_Message) noexcept;
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the log level string.
     *
     *  \return The log level string.
     */
    
    const char* GetLevelString(LogLevel e_Level) noexcept;
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::mutex c_Mutex;
    
    std::ofstream f_LogFile;
    std::ofstream f_BacktraceFile;
    
protected:
    
};

#endif /* Logger_h */
