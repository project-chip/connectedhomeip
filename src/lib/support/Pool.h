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
 * Defines memory pool classes.
 */

#pragma once

#include <lib/support/CodeUtils.h>

#include <assert.h>
#include <atomic>
#include <limits>
#include <new>
#include <stddef.h>
#include <utility>

namespace chip {

namespace internal {

class Statistics
{
public:
    Statistics() : mAllocated(0), mHighWaterMark(0) {}

    size_t Allocated() const { return mAllocated; }
    size_t HighWaterMark() const { return mHighWaterMark; }
    void IncreaseUsage()
    {
        if (++mAllocated > mHighWaterMark)
        {
            mHighWaterMark = mAllocated;
        }
    }
    void DecreaseUsage() { --mAllocated; }

protected:
    size_t mAllocated;
    size_t mHighWaterMark;
};

class StaticAllocatorBase : public Statistics
{
public:
    StaticAllocatorBase(size_t capacity) : mCapacity(capacity) {}
    size_t Capacity() const { return mCapacity; }
    bool Exhausted() const { return mAllocated == mCapacity; }

protected:
    const size_t mCapacity;
};

class StaticAllocatorBitmap : public internal::StaticAllocatorBase
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

    bool ForEachActiveObjectInner(void * context, bool lambda(void * context, void * object));

private:
    void * mElements;
    const size_t mElementSize;
    std::atomic<tBitChunkType> * mUsage;
};

template <class T>
class PoolCommon
{
public:
    template <typename... Args>
    void ResetObject(T * element, Args &&... args)
    {
        element->~T();
        new (element) T(std::forward<Args>(args)...);
    }
};

template <typename T, typename Function>
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

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

struct HeapObjectListNode
{
    void Remove()
    {
        mNext->mPrev = mPrev;
        mPrev->mNext = mNext;
    }

    void * mObject;
    HeapObjectListNode * mNext;
    HeapObjectListNode * mPrev;
};

struct HeapObjectList : HeapObjectListNode
{
    HeapObjectList() { mNext = mPrev = this; }

    void Append(HeapObjectListNode * node)
    {
        node->mNext  = this;
        node->mPrev  = mPrev;
        mPrev->mNext = node;
        mPrev        = node;
    }

    HeapObjectListNode * FindNode(void * object) const;

    bool ForEachNode(void * context, bool lambda(void * context, void * object));
};

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace internal

/**
 * A class template used for allocating objects from a fixed-size static pool.
 *
 *  @tparam     T   a subclass of element to be allocated.
 *  @tparam     N   a positive integer max number of elements the pool provides.
 */
template <class T, size_t N>
class BitMapObjectPool : public internal::StaticAllocatorBitmap, public internal::PoolCommon<T>
{
public:
    BitMapObjectPool() : StaticAllocatorBitmap(mData.mMemory, mUsage, N, sizeof(T)) {}
    ~BitMapObjectPool() { ReleaseAll(); }

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

    void ReleaseAll() { ForEachActiveObjectInner(this, ReleaseObject); }

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
        internal::LambdaProxy<T, Function> proxy(std::forward<Function>(function));
        return ForEachActiveObjectInner(&proxy, &internal::LambdaProxy<T, Function>::Call);
    }

private:
    static bool ReleaseObject(void * context, void * object)
    {
        static_cast<BitMapObjectPool *>(context)->ReleaseObject(static_cast<T *>(object));
        return true;
    }

    std::atomic<tBitChunkType> mUsage[(N + kBitChunkSize - 1) / kBitChunkSize];
    union Data
    {
        Data() {}
        ~Data() {}
        alignas(alignof(T)) uint8_t mMemory[N * sizeof(T)];
        T mMemoryViewForDebug[N]; // Just for debugger
    } mData;
};

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

/**
 * A class template used for allocating objects from the heap.
 *
 *  @tparam     T   a class to be allocated.
 */
template <class T>
class HeapObjectPool : public internal::Statistics, public internal::PoolCommon<T>
{
public:
    HeapObjectPool() {}
    ~HeapObjectPool() { ReleaseAll(); }

    template <typename... Args>
    T * CreateObject(Args &&... args)
    {
        T * object = new T(std::forward<Args>(args)...);
        if (object != nullptr)
        {
            auto node = new internal::HeapObjectListNode();
            if (node != nullptr)
            {
                node->mObject = object;
                mObjects.Append(node);
                IncreaseUsage();
                return object;
            }
        }
        return nullptr;
    }

    void ReleaseObject(T * object)
    {
        if (object != nullptr)
        {
            internal::HeapObjectListNode * node = mObjects.FindNode(object);
            if (node != nullptr)
            {
                // Note that the node is not removed here; that is deferred until the end of the next pool iteration.
                node->mObject = nullptr;
                delete object;
                DecreaseUsage();
            }
        }
    }

    void ReleaseAll() { mObjects.ForEachNode(this, ReleaseObject); }

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
        // return ForEachNode([function](void *object) { return function(static_cast<T*>(object)); });
        internal::LambdaProxy<T, Function> proxy(std::forward<Function>(function));
        return mObjects.ForEachNode(&proxy, &internal::LambdaProxy<T, Function>::Call);
    }

private:
    static bool ReleaseObject(void * context, void * object)
    {
        static_cast<HeapObjectPool *>(context)->ReleaseObject(static_cast<T *>(object));
        return true;
    }

    internal::HeapObjectList mObjects;
};

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, unsigned int N>
using ObjectPool = HeapObjectPool<T>;
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, unsigned int N>
using ObjectPool = BitMapObjectPool<T, N>;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

enum class ObjectPoolMem
{
    kStatic,
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    kDynamic
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
};

template <typename T, size_t N, ObjectPoolMem P>
class MemTypeObjectPool;

template <typename T, size_t N>
class MemTypeObjectPool<T, N, ObjectPoolMem::kStatic> : public BitMapObjectPool<T, N>
{
};

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, size_t N>
class MemTypeObjectPool<T, N, ObjectPoolMem::kDynamic> : public HeapObjectPool<T>
{
};
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace chip
