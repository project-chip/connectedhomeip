/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file contains declarations of the following classes and
 *      templates:
 *
 *        - class chip::System::Object
 *        - template<typename ALIGN, size_t SIZE> union chip::System::ObjectArena
 *        - template<class T, unsigned int N> class chip::System::ObjectPool
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <utility>
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
#include <memory>
#include <mutex>
#include <vector>
#endif

#include <lib/support/DLLUtil.h>

#include <system/SystemError.h>
#include <system/SystemStats.h>

#ifndef SYSTEM_OBJECT_HWM_TEST_HOOK
#define SYSTEM_OBJECT_HWM_TEST_HOOK()
#endif

namespace chip {
namespace System {

// Forward class and class template declarations
class Layer;
template <class T, unsigned int N>
class ObjectPool;

/**
 *  @class Object
 *
 *  @brief
 *    This represents a reference-counted object allocated from space contained in an ObjectPool<T, N> object.
 *
 *  @note
 *      Instance of this class may only be constructed using the related ObjectPool class template. The copy constructor and the
 *      assignment operator are deleted. A reference counting system is used to track retentions of instances of this class.
 *      When an object is initially retained, its reference count is one. Additional retentions may increment the reference count.
 *      When the object is released, the reference count is decremented. When the reference count is zero, the object is recycled
 *      back to the pool for reallocation. There is no destructor available. Subclasses must be designed to ensure that all
 *      encapsulated resources are released when the final retention is released and the object is recycled.
 *
 *      While this class is defined as concrete, it should be regarded as abstract.
 */
class DLL_EXPORT Object
{
    template <class T, unsigned int N>
    friend class ObjectPool;

public:
    Object() : mRefCount(0)
    {
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        mNext = nullptr;
        mPrev = nullptr;
#endif
    }

    virtual ~Object() {}

    /** Test whether this object is retained. Concurrency safe. */
    bool IsRetained() const;

    void Retain();
    void Release();
    Layer & SystemLayer() const;

protected:
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /**< What to do when DeferredRelease fails to post a kEvent_ReleaseObj. */
    enum ReleaseDeferralErrorTactic
    {
        kReleaseDeferralErrorTactic_Ignore,  /**< No action. */
        kReleaseDeferralErrorTactic_Release, /**< Release immediately. */
        kReleaseDeferralErrorTactic_Die,     /**< Die with message. */
    };

    void DeferredRelease(Layer * aSystemLayer, ReleaseDeferralErrorTactic aTactic);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

private:
    Object(const Object &) = delete;
    Object & operator=(const Object &) = delete;

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    Object * mNext;
    Object * mPrev;
    std::mutex * mMutexRef;
#endif

    unsigned int mRefCount; /**< Count of remaining calls to Release before object is dead. */

    /**
     *  @brief
     *      Attempts to perform an initial retention of this object, in a THREAD-SAFE manner.
     *
     *  @note
     *      If reference count is non-zero, tryCreate will fail and return false.
     *      If reference count is zero, then:
     *         - reference count will be set to  1
     *         - The size of the created object (assumed to be derived from SystemObject) is aOctects.
     *           the method will memset to 0 the bytes following sizeof(SystemObject).
     *
     *       Typical usage is like:
     *           class Foo: public SystemObject {...}
     *           ....
     *           Foo foo;
     *           foo.TryCreate(sizeof(foo));
     *
     *       IMPORTANT inheritance precondition:
     *           0 memset assumes that SystemObject is the top of the inheritance. This will NOT work properly:
     *           class Bar: public Baz, SystemObject {...}
     *           Bar bar;
     *           bar.TryCreate(sizeof(bar)); /// NOT safe: this will clear sizeof(Baz) extra bytes in unallocated space.
     */
    bool TryCreate(size_t aOctets);

public:
    void * AppState; /**< Generic pointer to app-specific data associated with the object. */
};

/**
 *  @brief
 *      Tests whether this object is retained.
 *
 *  @note
 *      No memory barrier is applied. If this returns \c false in one thread context, then it does not imply that another thread
 *      cannot have previously retained the object for \c aLayer. If it returns \c true, then the logic using \c mRefCount is
 *      responsible for ensuring concurrency safety for this object.
 */
inline bool Object::IsRetained() const
{
    return this->mRefCount > 0;
}

/**
 *  @brief
 *      Increments the reference count for the CHIP System Layer object. The object is assumed to be live.
 */
inline void Object::Retain()
{
    __sync_fetch_and_add(&this->mRefCount, 1);
}

/**
 *  @brief
 *      A union template used for representing a well-aligned block of memory.
 *
 *  @tparam     ALIGN   a typename with the alignment properties for the block.
 *  @tparam     SIZE    a constant size of the block in bytes.
 */
template <typename ALIGN, size_t SIZE>
union ObjectArena
{
    uint8_t uMemory[SIZE];
    ALIGN uAlign;
};

/**
 *  @brief
 *      A class template used for allocating Object subclass objects from an ObjectArena<> template union.
 *
 *  @tparam     T   a subclass of Object to be allocated from the arena.
 *  @tparam     N   a positive integer number of objects of class T to allocate from the arena.
 */
template <class T, unsigned int N>
class ObjectPool
{
public:
    void Reset();

    T * TryCreate();
    void GetStatistics(chip::System::Stats::count_t & aNumInUse, chip::System::Stats::count_t & aHighWatermark);

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
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        std::lock_guard<std::mutex> lock(mMutex);
        Object * p = mDummyHead.mNext;
        while (p)
        {
            if (!function(static_cast<T *>(p)))
            {
                return false;
            }
            p = p->mNext;
        }
#else
        for (unsigned int i = 0; i < N; ++i)
        {
            T & lObject = reinterpret_cast<T *>(mArena.uMemory)[i];

            if (lObject.IsRetained())
            {
                if (!function(&lObject))
                    return false;
            }
        }
#endif
        return true;
    }

private:
    friend class TestObject;

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    std::mutex mMutex;
    Object mDummyHead;
#else
    ObjectArena<void *, N * sizeof(T)> mArena;

#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
    void GetNumObjectsInUse(unsigned int aStartIndex, unsigned int & aNumInUse);
    void UpdateHighWatermark(const unsigned int & aCandidate);
    volatile unsigned int mHighWatermark;
#endif
#endif
};

template <class T, unsigned int N>
inline void ObjectPool<T, N>::Reset()
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    std::lock_guard<std::mutex> lock(mMutex);
    Object * p = mDummyHead.mNext;

    while (p)
    {
        Object * del = p;
        p            = p->mNext;
        delete del;
    }

    mDummyHead.mNext = nullptr;
#else
    memset(mArena.uMemory, 0, N * sizeof(T));

#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
    mHighWatermark = 0;
#endif
#endif
}

/**
 *  @brief
 *      Tries to initially retain the first object in the pool that is not retained.
 */
template <class T, unsigned int N>
inline T * ObjectPool<T, N>::TryCreate()
{
    T * lReturn = nullptr;

    (void) static_cast<Object *>(lReturn); /* In C++-11, this would be a static_assert that T inherits Object. */

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    T * newNode = new T();

    if (newNode->TryCreate(sizeof(T)))
    {
        std::lock_guard<std::mutex> lock(mMutex);
        Object * p = &mDummyHead;
        if (p->mNext)
        {
            p->mNext->mPrev = newNode;
        }
        newNode->mNext     = p->mNext;
        p->mNext           = newNode;
        newNode->mPrev     = p;
        newNode->mMutexRef = &mMutex;
        lReturn            = newNode;
    }
    else
    {
        delete newNode;
    }
#else // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    unsigned int lIndex = 0;

    for (lIndex = 0; lIndex < N; ++lIndex)
    {
        T & lObject = reinterpret_cast<T *>(mArena.uMemory)[lIndex];

        if (lObject.TryCreate(sizeof(T)))
        {
            lReturn = &lObject;
            break;
        }
    }

#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS
    unsigned int lNumInUse = 0;

    if (lReturn != nullptr)
    {
        lIndex++;
        lNumInUse = lIndex;
        GetNumObjectsInUse(lIndex, lNumInUse);
    }
    else
    {
        lNumInUse = N;
    }

    UpdateHighWatermark(lNumInUse);
#endif
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

    return lReturn;
}

#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS && !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <class T, unsigned int N>
inline void ObjectPool<T, N>::UpdateHighWatermark(const unsigned int & aCandidate)
{
    unsigned int lTmp;

    while (aCandidate > (lTmp = mHighWatermark))
    {
        SYSTEM_OBJECT_HWM_TEST_HOOK();
        (void) __sync_bool_compare_and_swap(&mHighWatermark, lTmp, aCandidate);
    }
}

/**
 * Return the number of objects in use starting at a given index
 *
 * @param[in] aStartIndex      The index to start counting from; pass 0 to count over
 *                             the whole pool.
 * @param[in/out] aNumInUse    The number of objects in use. If aStartIndex is not 0,
 *                             the function adds to the counter without resetting it first.
 */
template <class T, unsigned int N>
inline void ObjectPool<T, N>::GetNumObjectsInUse(unsigned int aStartIndex, unsigned int & aNumInUse)
{
    unsigned int count = 0;

    for (unsigned int lIndex = aStartIndex; lIndex < N; ++lIndex)
    {
        T & lObject = reinterpret_cast<T *>(mArena.uMemory)[lIndex];

        if (lObject.IsRetained())
        {
            count++;
        }
    }

    if (aStartIndex == 0)
    {
        aNumInUse = 0;
    }

    aNumInUse += count;
}
#endif // CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS && !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

template <class T, unsigned int N>
inline void ObjectPool<T, N>::GetStatistics(chip::System::Stats::count_t & aNumInUse, chip::System::Stats::count_t & aHighWatermark)
{
#if CHIP_SYSTEM_CONFIG_PROVIDE_STATISTICS && !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    unsigned int lNumInUse;
    unsigned int lHighWatermark;

    GetNumObjectsInUse(0, lNumInUse);
    lHighWatermark = mHighWatermark;

    if (lNumInUse > CHIP_SYS_STATS_COUNT_MAX)
    {
        lNumInUse = CHIP_SYS_STATS_COUNT_MAX;
    }
    if (lHighWatermark > CHIP_SYS_STATS_COUNT_MAX)
    {
        lHighWatermark = CHIP_SYS_STATS_COUNT_MAX;
    }
    aNumInUse      = static_cast<chip::System::Stats::count_t>(lNumInUse);
    aHighWatermark = static_cast<chip::System::Stats::count_t>(lHighWatermark);
#endif
}

} // namespace System
} // namespace chip
