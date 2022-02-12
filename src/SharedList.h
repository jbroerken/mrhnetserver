/**
 *  SharedList.h
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

#ifndef SharedList_h
#define SharedList_h

// C / C++
#include <mutex>
#include <atomic>
#include <memory>
#include <deque>
#include <list>

// External

// Project
#include "./Exception.h"


template<typename T> class SharedList
{
public:
    
    //*************************************************************************************
    // Constructor / Destructor
    //*************************************************************************************
    
    /**
     *  Default constructor.
     */
    
    SharedList() noexcept : us_TotalCount(0),
                            us_AvailableCount(0)
    {}
    
    /**
     *  Default destructor.
     */
    
    ~SharedList() noexcept
    {}
    
    //*************************************************************************************
    // Add
    //*************************************************************************************
    
    /**
     *  Add a element to the shared list.
     *
     *  \param p_Element The element to add.
     */
    
    void Add(std::shared_ptr<T> p_Element)
    {
        if (p_Element == NULL || p_Element == nullptr)
        {
            throw Exception("Invalid job added!");
        }
        
        // First try to reuse a entry
        // @NOTE: Element deque never shrinks!
        for (size_t i = 0; i < us_TotalCount; ++i)
        {
            // Element free to be checked?
            if (dq_Element[i].c_Mutex.try_lock() == false)
            {
                continue;
            }
            
            // Task can be added here?
            if (dq_Element[i].p_Element == NULL)
            {
                dq_Element[i].p_Element.swap(p_Element);
                us_AvailableCount += 1;
                
                dq_Element[i].c_Mutex.unlock();
                return;
            }
            else
            {
                dq_Element[i].c_Mutex.unlock();
            }
        }
        
        // No free entry, add a new one
        try
        {
            c_Mutex.lock(); // Lock for outside multithreading
            
            dq_Element.emplace_back(p_Element);
            us_TotalCount += 1;
            us_AvailableCount += 1;
            
            c_Mutex.unlock();
        }
        catch (std::exception& e)
        {
            throw Exception("Failed to add entry: " + std::string(e.what()));
        }
    }
    
    //*************************************************************************************
    // Getters
    //*************************************************************************************
    
    /**
     *  Get the first available list element.
     *
     *  \param b_Remove If the element should be removed.
     *
     *  \return The requested element on success, NULL on failure.
     */
    
    std::shared_ptr<T> GetElement(bool b_Remove) noexcept
    {
        std::shared_ptr<T> p_Result(NULL);
        
        for (size_t i = 0; i < us_TotalCount; ++i)
        {
            if (dq_Element[i].c_Mutex.try_lock() == true && dq_Element[i].p_Element != NULL)
            {
                if (b_Remove == true)
                {
                    p_Result.swap(dq_Element[i].p_Element);
                }
                else
                {
                    p_Result = dq_Element[i].p_Element;
                }
                
                us_AvailableCount -= 1;
                dq_Element[i].c_Mutex.unlock();
                
                return p_Result;
            }
        }
        
        return p_Result;
    }
    
    /**
     *  Get a copy of the list elements.
     *
     *  \param b_Remove If the elements should be removed.
     *
     *  \return The list elements.
     */
    
    std::list<std::shared_ptr<T>> GetElements(bool b_Remove) noexcept
    {
        std::list<std::shared_ptr<T>> l_Result;
        std::shared_ptr<T> p_Element;
        
        while ((p_Element = GetElement(b_Remove)) != NULL)
        {
            l_Result.emplace_back(p_Element);
        }
        
        return l_Result;
    }
    
    /**
     *  Get the amount of stored elements.
     *
     *  \return The amount of stored elements.
     */
    
    size_t GetElementsCount() const noexcept
    {
        return us_AvailableCount;
    }
    
private:
    
    //*************************************************************************************
    // Types
    //*************************************************************************************
    
    struct Entry
    {
    public:
        
        //*************************************************************************************
        // Constructor / Destructor
        //*************************************************************************************
        
        /**
         *  Default constructor.
         */
            
        Entry() noexcept : p_Element((T*)nullptr)
        {}
        
        /**
         *  Entry constructor.
         *
         *  \param p_Element The element to add.
         */
            
        Entry(std::shared_ptr<T>& p_Element) noexcept : p_Element(p_Element)
        {}
        
        /**
         *  Copy constructor. Disabled for this class.
         *
         *  \param c_Entry Entry class source.
         */
        
        Entry(Entry const& c_Entry) = delete;
        
        /**
         *  Default destructor.
         */
        
        ~Entry() noexcept
        {}
        
        //*************************************************************************************
        // Data
        //*************************************************************************************
        
        std::mutex c_Mutex;
        std::shared_ptr<T> p_Element;
    };
    
    //*************************************************************************************
    // Data
    //*************************************************************************************
    
    std::mutex c_Mutex;
    
    std::deque<Entry> dq_Element;
    std::atomic<size_t> us_TotalCount;
    std::atomic<size_t> us_AvailableCount;
    
protected:
    
};

#endif /* SharedList_h */
