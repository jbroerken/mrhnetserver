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
            // Lock for outside multithreading
            std::lock_guard<std::mutex> c_Guard(c_Mutex);
            
            dq_Element.emplace_back(p_Element);
            us_TotalCount += 1;
            us_AvailableCount += 1;
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
     *  Get the first available list element. The element will be removed from the list.
     *
     *  \return The first available list element on success, NULL on failure.
     */
    
    std::shared_ptr<T> GetElement() noexcept
    {
        std::shared_ptr<T> p_Result(NULL);
        
        for (size_t i = 0; i < us_TotalCount; ++i)
        {
            if (dq_Element[i].c_Mutex.try_lock() == false)
            {
                continue;
            }
            
            if (dq_Element[i].p_Element != NULL)
            {
                p_Result.swap(dq_Element[i].p_Element);
                us_AvailableCount -= 1;
            }
            
            dq_Element[i].c_Mutex.unlock();
            
            if (p_Result != NULL)
            {
                return p_Result;
            }
        }
        
        return p_Result;
    }
    
    /**
     *  Get all available elements. All elements will be removed from the list.
     *
     *  \return The available list elements.
     */
    
    std::list<std::shared_ptr<T>> GetElements() noexcept
    {
        std::list<std::shared_ptr<T>> l_Result;
        
        for (size_t i = 0; i < us_TotalCount; ++i)
        {
            if (dq_Element[i].c_Mutex.try_lock() == false)
            {
                continue;
            }
            
            if (dq_Element[i].p_Element != NULL)
            {
                l_Result.push_back(dq_Element[i].p_Element);
                
                dq_Element[i].p_Element.reset();
                dq_Element[i].p_Element = NULL;
                
                us_AvailableCount -= 1;
            }
            
            dq_Element[i].c_Mutex.unlock();
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
