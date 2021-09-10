/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013 Nest Labs, Inc.
 *    All rights reserved.
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

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

#include <new>

#include <lib/support/TypeTraits.h>
#include <system/SystemPoolNonHeapBitmapAllocator.h>

namespace chip {
namespace System {

/**
 *  @brief
 *   A class template used for allocating Objects.
 *
 *  @tparam     T   a subclass of element to be allocated.
 *  @tparam     N   a positive integer max number of elements the pool provides.
 */
template <class T, size_t N>
class ObjectPoolNonHeap : public StaticAllocatorBitmap
{
public:
    ObjectPoolNonHeap() : StaticAllocatorBitmap(mData.mMemory, mUsage, N, sizeof(T)) {}

    template <typename... Args>
    T * CreateObject(Args &&... args)
    {
        T * element = static_cast<T *>(Allocate());
        if (element != nullptr)
            return new (element) T(std::forward<Args>(args)...);
        else
            return nullptr;
    }

    void ReleaseObject(T * element)
    {
        if (element == nullptr)
            return;

        element->~T();
        Deallocate(element);
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
     *
     * caution
     *   this function is not thread-safe, make sure all usage of the
     *   pool is protected by a lock, or else avoid using this function
     */
    template <typename Function>
    bool ForEachActiveObject(Function && function)
    {
        LambdaProxy<Function> proxy(std::forward<Function>(function));
        return ForEachActiveObjectInner(&proxy, &LambdaProxy<Function>::Call);
    }

private:
    template <typename Function>
    class LambdaProxy
    {
    public:
        LambdaProxy(Function && function) : mFunction(std::move(function)) {}
        static bool Call(void * context, void * target)
        {
            return static_cast<LambdaProxy *>(context)->mFunction(static_cast<T *>(target));
        }

    private:
        Function mFunction;
    };

    std::atomic<tBitChunkType> mUsage[(N + kBitChunkSize - 1) / kBitChunkSize];
    union Data
    {
        Data() {}
        ~Data() {}
        alignas(alignof(T)) uint8_t mMemory[N * sizeof(T)];
        T mMemoryViewForDebug[N]; // Just for debugger
    } mData;
};

} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
