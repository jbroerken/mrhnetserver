/**
 *  NetException.h
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

#ifndef NetException_h
#define NetException_h

// C / C++
#include <string>
#include <exception>

// External

// Project


class NetException : public std::exception
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  std::string constructor.
     *
     *  \param s_Message The error message.
     */
    
    NetException(std::string s_Message) : s_Message(s_Message)
    {}
    
    /**
     *  const char* constructor.
     *
     *  \param p_Message The error message.
     */
    
    NetException(const char* p_Message) : s_Message(p_Message != NULL ? p_Message : "")
    {}
    
    /**
     *  Default destructor.
     */
    
    ~NetException()
    {}
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the const char* exception string.
     *
     *  \return A const char* string with the error message.
     */
    
    const char* what() const throw()
    {
        return s_Message.c_str();
    }
    
    /**
     *  Get the std::string exception string.
     *
     *  \return A std::string string with the error message.
     */
    
    std::string what2() const throw()
    {
        return s_Message;
    }
    
private:
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::string s_Message;
    
protected:
    
};

#endif /* NetException_h */
