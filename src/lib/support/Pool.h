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
#include <system/SystemConfig.h>

#include <lib/support/Iterators.h>

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

    Loop ForEachActiveObjectInner(void * context, Loop lambda(void * context, void * object));

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
    static Loop Call(void * context, void * target)
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
    HeapObjectList() : mIterationDepth(0) { mNext = mPrev = this; }

    void Append(HeapObjectListNode * node)
    {
        node->mNext  = this;
        node->mPrev  = mPrev;
        mPrev->mNext = node;
        mPrev        = node;
    }

    HeapObjectListNode * FindNode(void * object) const;

    Loop ForEachNode(void * context, Loop lambda(void * context, void * object));

    size_t mIterationDepth;
};

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace internal

/**
 * Action taken if objects remain allocated when a pool is destroyed.
 */
enum class OnObjectPoolDestruction
{
    AutoRelease,                   ///< Release any objects still allocated.
    Die,                           ///< Abort if any objects remain allocated.
    IgnoreUnsafeDoNotUseInNewCode, ///< Do nothing; keep historical behaviour until leaks are fixed.
};

/**
 * @class ObjectPool
 *
 * Depending on build configuration, ObjectPool is either a fixed-size static pool or a heap-allocated pool.
 *
 * @tparam T    Type of element to be allocated.
 * @tparam N    Number of elements in the pool, in the fixed-size case.
 *
 * @fn CreateObject
 * @memberof ObjectPool
 *
 * Create an object from the pool. Forwards its arguments to construct a T.
 *
 * @fn ReleaseObject
 * @memberof ObjectPool
 * @param object   Pointer to object to release (or return to the pool). Its destructor runs.
 *
 * @fn ForEachActiveObject
 * @memberof ObjectPool
 * @param visitor   A function that takes a T* and returns Loop::Continue to continue iterating or Loop::Break to stop iterating.
 * @returns Loop::Break if a visitor call returned Loop::Break, Loop::Finish otherwise.
 *
 * Iteration may be nested. ReleaseObject() can be called during iteration, on the current object or any other.
 * CreateObject() can be called, but it is undefined whether or not a newly created object will be visited.
 */

/**
 * A class template used for allocating objects from a fixed-size static pool.
 *
 *  @tparam     T   type of element to be allocated.
 *  @tparam     N   a positive integer max number of elements the pool provides.
 */
template <class T, size_t N, OnObjectPoolDestruction Action = OnObjectPoolDestruction::Die>
class BitMapObjectPool : public internal::StaticAllocatorBitmap, public internal::PoolCommon<T>
{
public:
    BitMapObjectPool() : StaticAllocatorBitmap(mData.mMemory, mUsage, N, sizeof(T)) {}
    ~BitMapObjectPool()
    {
        switch (Action)
        {
        case OnObjectPoolDestruction::AutoRelease:
            ReleaseAll();
            break;
        case OnObjectPoolDestruction::Die:
            VerifyOrDie(Allocated() == 0);
            break;
        case OnObjectPoolDestruction::IgnoreUnsafeDoNotUseInNewCode:
            break;
        }
    }

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
     *  @param     function The functor of type `Loop (*)(T*)`, return Loop::Break to break the iteration
     *  @return    Loop     Returns Break or Finish according to the iteration
     *
     * caution
     *   this function is not thread-safe, make sure all usage of the
     *   pool is protected by a lock, or else avoid using this function
     */
    template <typename Function>
    Loop ForEachActiveObject(Function && function)
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<T *>()))>::value,
                      "The function must take T* and return Loop");
        internal::LambdaProxy<T, Function> proxy(std::forward<Function>(function));
        return ForEachActiveObjectInner(&proxy, &internal::LambdaProxy<T, Function>::Call);
    }

private:
    static Loop ReleaseObject(void * context, void * object)
    {
        static_cast<BitMapObjectPool *>(context)->ReleaseObject(static_cast<T *>(object));
        return Loop::Continue;
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
 *  @tparam     T   type to be allocated.
 */
template <class T, OnObjectPoolDestruction Action = OnObjectPoolDestruction::Die>
class HeapObjectPool : public internal::Statistics, public internal::PoolCommon<T>
{
public:
    HeapObjectPool() {}
    ~HeapObjectPool()
    {
        switch (Action)
        {
        case OnObjectPoolDestruction::AutoRelease:
            ReleaseAll();
            break;
        case OnObjectPoolDestruction::Die:
            VerifyOrDie(Allocated() == 0);
            break;
        case OnObjectPoolDestruction::IgnoreUnsafeDoNotUseInNewCode:
            break;
        }
    }

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
     *  @param     function The functor of type `Loop (*)(T*)`, return Loop::Break to break the iteration
     *  @return    Loop     Returns Break or Finish according to the iteration
     */
    template <typename Function>
    Loop ForEachActiveObject(Function && function)
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<T *>()))>::value,
                      "The function must take T* and return Loop");
        internal::LambdaProxy<T, Function> proxy(std::forward<Function>(function));
        return mObjects.ForEachNode(&proxy, &internal::LambdaProxy<T, Function>::Call);
    }

private:
    static Loop ReleaseObject(void * context, void * object)
    {
        static_cast<HeapObjectPool *>(context)->ReleaseObject(static_cast<T *>(object));
        return Loop::Continue;
    }

    internal::HeapObjectList mObjects;
};

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, unsigned int N, OnObjectPoolDestruction Action = OnObjectPoolDestruction::Die>
using ObjectPool = HeapObjectPool<T, Action>;
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, unsigned int N, OnObjectPoolDestruction Action = OnObjectPoolDestruction::Die>
using ObjectPool = BitMapObjectPool<T, N, Action>;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

enum class ObjectPoolMem
{
    kStatic,
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    kDynamic
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
};

template <typename T, size_t N, ObjectPoolMem P, OnObjectPoolDestruction Action = OnObjectPoolDestruction::Die>
class MemTypeObjectPool;

template <typename T, size_t N, OnObjectPoolDestruction Action>
class MemTypeObjectPool<T, N, ObjectPoolMem::kStatic, Action> : public BitMapObjectPool<T, N, Action>
{
};

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, size_t N, OnObjectPoolDestruction Action>
class MemTypeObjectPool<T, N, ObjectPoolMem::kDynamic, Action> : public HeapObjectPool<T, Action>
{
};
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace chip
