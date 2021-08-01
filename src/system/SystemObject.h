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

#include <support/DLLUtil.h>

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
    Object() : mSystemLayer(nullptr)
    {
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        mNext = nullptr;
        mPrev = nullptr;
#endif
    }

    virtual ~Object() {}

    /** Test whether this object is retained by \c aLayer. Concurrency safe. */
    bool IsRetained(const Layer & aLayer) const;

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

    void DeferredRelease(ReleaseDeferralErrorTactic aTactic);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

private:
    Object(const Object &) = delete;
    Object & operator=(const Object &) = delete;

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    Object * mNext;
    Object * mPrev;
    std::mutex * mMutexRef;
#endif

    Layer * volatile mSystemLayer; /**< Pointer to the layer object that owns this object. */
    unsigned int mRefCount;        /**< Count of remaining calls to Release before object is dead. */

    /** If not already retained, attempt initial retention of this object for \c aLayer and zero up to \c aOctets. */
    bool TryCreate(Layer & aLayer, size_t aOctets);

public:
    void * AppState; /**< Generic pointer to app-specific data associated with the object. */
};

/**
 *  @brief
 *      Tests whether this object is retained by \c aLayer.
 *
 *  @note
 *      No memory barrier is applied. If this returns \c false in one thread context, then it does not imply that another thread
 *      cannot have previously retained the object for \c aLayer. If it returns \c true, then the logic using \c aLayer is
 *      responsible for ensuring concurrency safety for this object.
 */
inline bool Object::IsRetained(const Layer & aLayer) const
{
    return this->mSystemLayer == &aLayer;
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
 *      Returns a reference to the CHIP System Layer object provided when the object was initially retained from its corresponding
 *      object pool instance. The object is assumed to be live.
 */
inline Layer & Object::SystemLayer() const
{
    return *this->mSystemLayer;
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
    size_t Size();

    T * Get(const Layer & aLayer, size_t aIndex);
    T * TryCreate(Layer & aLayer);
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
        LambdaProxy<Function> proxy(std::forward<Function>(function));
        return ForEachActiveObjectInner(&proxy, &LambdaProxy<Function>::Call);
    }

private:
    friend class TestObject;

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

    using Lambda = bool (*)(void *, void *);
    bool ForEachActiveObjectInner(void * context, Lambda lambda);

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
 *      Returns the number of objects that can be simultaneously retained from a pool.
 */
template <class T, unsigned int N>
inline size_t ObjectPool<T, N>::Size()
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    size_t count = 0;
    std::lock_guard<std::mutex> lock(mMutex);
    Object * p = mDummyHead.mNext;

    while (p)
    {
        count++;
        p = p->mNext;
    }

    return count;
#else
    return N;
#endif
}

/**
 *  @brief
 *      Returns a pointer the object at \c aIndex or \c NULL if the object is not retained by \c aLayer.
 */
template <class T, unsigned int N>
inline T * ObjectPool<T, N>::Get(const Layer & aLayer, size_t aIndex)
{
    T * lReturn = nullptr;

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    {
        std::lock_guard<std::mutex> lock(mMutex);
        Object * p = mDummyHead.mNext;

        while (aIndex > 0)
        {
            if (p == nullptr)
                break;
            p = p->mNext;
            aIndex--;
        }

        lReturn = static_cast<T *>(p);
    }
#else
    if (aIndex < N)
        lReturn = &reinterpret_cast<T *>(mArena.uMemory)[aIndex];
#endif

    (void) static_cast<Object *>(lReturn); /* In C++-11, this would be a static_assert that T inherits Object. */

    return (lReturn != nullptr) && lReturn->IsRetained(aLayer) ? lReturn : nullptr;
}

/**
 *  @brief
 *      Tries to initially retain the first object in the pool that is not retained by any layer.
 */
template <class T, unsigned int N>
inline T * ObjectPool<T, N>::TryCreate(Layer & aLayer)
{
    T * lReturn = nullptr;

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    T * newNode = new T();

    if (newNode->TryCreate(aLayer, sizeof(T)))
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

        if (lObject.TryCreate(aLayer, sizeof(T)))
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

template <class T, unsigned int N>
inline bool ObjectPool<T, N>::ForEachActiveObjectInner(void * context, Lambda lambda)
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    std::lock_guard<std::mutex> lock(mMutex);
    Object * p = mDummyHead.mNext;
    while (p)
    {
        if (!lambda(context, static_cast<void *>(p)))
        {
            return false;
        }
        p = p->mNext;
    }
#else
    for (unsigned int i = 0; i < N; ++i)
    {
        T & lObject = reinterpret_cast<T *>(mArena.uMemory)[i];

        if (lObject.mSystemLayer != nullptr)
        {
            if (!lambda(context, static_cast<void *>(&lObject)))
                return false;
        }
    }
#endif
    return true;
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

        if (lObject.mSystemLayer != nullptr)
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
