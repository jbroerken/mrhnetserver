/**
 *  StreamSendContext.h
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

#ifndef StreamSendContext_h
#define StreamSendContext_h

// C / C++

// External
#include <msquic.h>

// Project
#include "./StreamData.h"


struct StreamSendContext
{
public:
    
    //*************************************************************************************
    // Constructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     *
     *  \param p_APITable The library api table.
     */
    
    StreamSendContext(const QUIC_API_TABLE* p_APITable) noexcept : p_APITable(p_APITable)
    {}
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    const QUIC_API_TABLE* p_APITable;
    
    StreamData c_Data;
};

#endif /* StreamSendContext_h */
