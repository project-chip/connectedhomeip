/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <lib/core/CHIPConfig.h>

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

#include <mutex>
#include <set>

#include <lib/support/CodeUtils.h>
#include <system/SystemPoolStatistics.h>

namespace chip {
namespace System {

/**
 *  @brief
 *      A class template used for allocating Object subclass objects from an ObjectArena<> template union.
 *
 *  @tparam     T   a subclass of Object to be allocated from the arena.
 *  @tparam     N   a positive integer number of objects of class T to allocate from the arena.
 */
template <class T, unsigned int N>
class ObjectPoolHeap : public ObjectPoolStatistics
{
public:
    template <typename... Args>
    T * CreateObject(Args &&... args)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        T * object = new T(std::forward<Args>(args)...);
        if (object == nullptr)
        {
            return nullptr;
        }

        mObjects.insert(object);
        IncreaseUsage();
        return object;
    }

    void ReleaseObject(T * object)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        auto iter = mObjects.find(object);
        VerifyOrDie(iter != mObjects.end());
        mObjects.erase(iter);
        delete object;
        DecreaseUsage();
    }

    template <typename... Args>
    void ResetObject(T * element, Args &&... args)
    {
        element->~T();
        new (element) T(std::forward<Args>(args)...);
    }

    /**
     * @brief
     *   Run a functor for each active object in the pool
     *
     *  @param     function The functor of type `bool (*)(T*)`, return false to break the iteration
     *  @return    bool     Returns false if broke during iteration
     */
    template <typename Function>
    bool ForEachActiveObject(Function && function)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        // Create a new copy of original set, allowing add/remove elements while iterating in the same thread.
        for (auto object : std::set<T *>(mObjects))
        {
            if (!function(object))
                return false;
        }
        return true;
    }

private:
    std::recursive_mutex mutex;
    std::set<T *> mObjects;
};

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
