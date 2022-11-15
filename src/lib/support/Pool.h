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

#include <lib/support/CHIPMem.h>
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

protected:
    void * Allocate();
    void Deallocate(void * element);
    void * At(size_t index) { return static_cast<uint8_t *>(mElements) + mElementSize * index; }
    size_t IndexOf(void * element);

    using Lambda = Loop (*)(void * context, void * object);
    Loop ForEachActiveObjectInner(void * context, Lambda lambda);
    Loop ForEachActiveObjectInner(void * context, Loop lambda(void * context, const void * object)) const
    {
        return const_cast<StaticAllocatorBitmap *>(this)->ForEachActiveObjectInner(context, reinterpret_cast<Lambda>(lambda));
    }

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
    static Loop ConstCall(void * context, const void * target)
    {
        return static_cast<LambdaProxy *>(context)->mFunction(static_cast<const T *>(target));
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

    using Lambda = Loop (*)(void *, void *);
    Loop ForEachNode(void * context, Lambda lambda);
    Loop ForEachNode(void * context, Loop lambda(void * context, const void * object)) const
    {
        return const_cast<HeapObjectList *>(this)->ForEachNode(context, reinterpret_cast<Lambda>(lambda));
    }

    size_t mIterationDepth         = 0;
    bool mHaveDeferredNodeRemovals = false;
};

#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace internal

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
template <class T, size_t N>
class BitMapObjectPool : public internal::StaticAllocatorBitmap, public internal::PoolCommon<T>
{
public:
    BitMapObjectPool() : StaticAllocatorBitmap(mData.mMemory, mUsage, N, sizeof(T)) {}
    ~BitMapObjectPool() { VerifyOrDie(Allocated() == 0); }

    template <typename... Args>
    T * CreateObject(Args &&... args)
    {
        T * element = static_cast<T *>(Allocate());
        if (element != nullptr)
            return new (element) T(std::forward<Args>(args)...);
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
     *  @param     function A functor of type `Loop (*)(T*)`.
     *                      Return Loop::Break to break the iteration.
     *                      The only modification the functor is allowed to make
     *                      to the pool before returning is releasing the
     *                      object that was passed to the functor.  Any other
     *                      desired changes need to be made after iteration
     *                      completes.
     *  @return    Loop     Returns Break if some call to the functor returned
     *                      Break.  Otherwise returns Finish.
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
    template <typename Function>
    Loop ForEachActiveObject(Function && function) const
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<const T *>()))>::value,
                      "The function must take const T* and return Loop");
        internal::LambdaProxy<T, Function> proxy(std::forward<Function>(function));
        return ForEachActiveObjectInner(&proxy, &internal::LambdaProxy<T, Function>::ConstCall);
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

class HeapObjectPoolExitHandling
{
public:
    // If IgnoreLeaksOnExit is called, some time after all static initializers have
    // run, HeapObjectPool will not assert that everything in it has been
    // released if its destructor runs under exit() (i.e. when the application
    // is quitting anyway).
    static void IgnoreLeaksOnExit();

protected:
    static bool sIgnoringLeaksOnExit;

private:
    static void ExitHandler();
    static bool sExitHandlerRegistered;
};

/**
 * A class template used for allocating objects from the heap.
 *
 *  @tparam     T   type to be allocated.
 */
template <class T>
class HeapObjectPool : public internal::Statistics, public internal::PoolCommon<T>, public HeapObjectPoolExitHandling
{
public:
    HeapObjectPool() {}
    ~HeapObjectPool()
    {
#ifndef __SANITIZE_ADDRESS__
#ifdef __clang__
#if __has_feature(address_sanitizer)
#define __SANITIZE_ADDRESS__ 1
#endif
#endif
#endif
#if __SANITIZE_ADDRESS__
        // Free all remaining objects so that ASAN can catch specific use-after-free cases.
        ReleaseAll();
#else  // __SANITIZE_ADDRESS__
        if (!sIgnoringLeaksOnExit)
        {
            // Verify that no live objects remain, to prevent potential use-after-free.
            VerifyOrDie(Allocated() == 0);
        }
#endif // __SANITIZE_ADDRESS__
    }

    template <typename... Args>
    T * CreateObject(Args &&... args)
    {
        T * object = Platform::New<T>(std::forward<Args>(args)...);
        if (object != nullptr)
        {
            auto node = Platform::New<internal::HeapObjectListNode>();
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

    /*
     * This method exists purely to line up with the static allocator version.
     * Consequently, return a nonsensically large number to normalize comparison
     * operations that act on this value.
     */
    size_t Capacity() const { return SIZE_MAX; }

    /*
     * This method exists purely to line up with the static allocator version. Heap based object pool will never be exhausted.
     */
    bool Exhausted() const { return false; }

    void ReleaseObject(T * object)
    {
        if (object != nullptr)
        {
            internal::HeapObjectListNode * node = mObjects.FindNode(object);
            // Releasing an object that is not allocated indicates likely memory
            // corruption; better to safe-crash than proceed at this point.
            VerifyOrDie(node != nullptr);

            node->mObject = nullptr;
            Platform::Delete(object);

            // The node needs to be released immediately if we are not in the middle of iteration.
            // Otherwise cleanup is deferred until all iteration on this pool completes and it's safe to release nodes.
            if (mObjects.mIterationDepth == 0)
            {
                node->Remove();
                Platform::Delete(node);
            }
            else
            {
                mObjects.mHaveDeferredNodeRemovals = true;
            }

            DecreaseUsage();
        }
    }

    void ReleaseAll() { mObjects.ForEachNode(this, ReleaseObject); }

    /**
     * @brief
     *   Run a functor for each active object in the pool
     *
     *  @param     function A functor of type `Loop (*)(T*)`.
     *                      Return Loop::Break to break the iteration.
     *                      The only modification the functor is allowed to make
     *                      to the pool before returning is releasing the
     *                      object that was passed to the functor.  Any other
     *                      desired changes need to be made after iteration
     *                      completes.
     *  @return    Loop     Returns Break if some call to the functor returned
     *                      Break.  Otherwise returns Finish.
     */
    template <typename Function>
    Loop ForEachActiveObject(Function && function)
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<T *>()))>::value,
                      "The function must take T* and return Loop");
        internal::LambdaProxy<T, Function> proxy(std::forward<Function>(function));
        return mObjects.ForEachNode(&proxy, &internal::LambdaProxy<T, Function>::Call);
    }
    template <typename Function>
    Loop ForEachActiveObject(Function && function) const
    {
        static_assert(std::is_same<Loop, decltype(function(std::declval<const T *>()))>::value,
                      "The function must take const T* and return Loop");
        internal::LambdaProxy<const T, Function> proxy(std::forward<Function>(function));
        return mObjects.ForEachNode(&proxy, &internal::LambdaProxy<const T, Function>::ConstCall);
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

/**
 * Specify ObjectPool storage allocation.
 */
enum class ObjectPoolMem
{
    /**
     * Use storage inside the containing scope for both objects and pool management state.
     */
    kInline,
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    /**
     * Allocate objects from the heap, with only pool management state in the containing scope.
     *
     * For this case, the ObjectPool size parameter is ignored.
     */
    kHeap,
    kDefault = kHeap
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    kDefault = kInline
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
};

template <typename T, size_t N, ObjectPoolMem P = ObjectPoolMem::kDefault>
class ObjectPool;

template <typename T, size_t N>
class ObjectPool<T, N, ObjectPoolMem::kInline> : public BitMapObjectPool<T, N>
{
};

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <typename T, size_t N>
class ObjectPool<T, N, ObjectPoolMem::kHeap> : public HeapObjectPool<T>
{
};
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace chip
