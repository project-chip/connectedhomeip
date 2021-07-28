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

/**
 * @file
 *   Defines a memory pool class BitMapObjectPool.
 */

#pragma once

#include <array>
#include <assert.h>
#include <atomic>
#include <limits>
#include <new>
#include <stddef.h>

namespace chip {

class StaticAllocatorBase
{
public:
    StaticAllocatorBase(size_t capacity) : mAllocated(0), mCapacity(capacity) {}

    size_t Capacity() const { return mCapacity; }
    size_t Allocated() const { return mAllocated; }
    bool Exhausted() const { return mAllocated == mCapacity; }

protected:
    size_t mAllocated;
    const size_t mCapacity;
};

class StaticAllocatorBitmap : public StaticAllocatorBase
{
protected:
    /**
     * Use the largest data type supported by `std::atomic`. Putting multiple atomic inside a single cache line won't improve
     * concurrency, while the use of larger data type can improve the performance by reducing the number of outer loop iterations.
     */
    using tBitChunkType                         = unsigned long;
    static constexpr const tBitChunkType kBit1  = 1; // make sure bitshifts produce the right type
    static constexpr const size_t kBitChunkSize = std::numeric_limits<tBitChunkType>::digits;
    static_assert(ATOMIC_LONG_LOCK_FREE, "StaticAllocatorBitmap is not lock free");

public:
    StaticAllocatorBitmap(void * storage, std::atomic<tBitChunkType> * usage, size_t capacity, size_t elementSize);
    void * Allocate();
    void Deallocate(void * element);

protected:
    void * At(size_t index) { return static_cast<uint8_t *>(mElements) + mElementSize * index; }
    size_t IndexOf(void * element);

    using Lambda = bool (*)(void *, void *);
    bool ForEachActiveObjectInner(void * context, Lambda lambda);

private:
    void * mElements;
    const size_t mElementSize;
    std::atomic<tBitChunkType> * mUsage;
};

/**
 *  @brief
 *   A class template used for allocating Objects.
 *
 *  @tparam     T   a subclass of element to be allocated.
 *  @tparam     N   a positive integer max number of elements the pool provides.
 */
template <class T, size_t N>
class BitMapObjectPool : public StaticAllocatorBitmap
{
public:
    BitMapObjectPool() : StaticAllocatorBitmap(mMemory, mUsage, N, sizeof(T)) {}

    static size_t Size() { return N; }

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
    alignas(alignof(T)) uint8_t mMemory[N * sizeof(T)];
};

} // namespace chip
