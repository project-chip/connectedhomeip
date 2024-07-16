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
 *  This file defines the chip::System::Timer class and related types that can be used for representing
 *  an in-progress one-shot timer. Implementations of System::Layer may (but are not required to) use
 *  these for their versions of timer events.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <lib/support/DLLUtil.h>
#include <lib/support/Pool.h>

#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemLayer.h>
#include <system/SystemStats.h>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif

namespace chip {
namespace System {

class Layer;

/**
 * Basic Timer information: time and callback.
 */
class DLL_EXPORT TimerData
{
public:
    class Callback
    {
    public:
        Callback(Layer & systemLayer, TimerCompleteCallback onComplete, void * appState) :
            mSystemLayer(&systemLayer), mOnComplete(onComplete), mAppState(appState)
        {}
        void Invoke() const { mOnComplete(mSystemLayer, mAppState); }
        const TimerCompleteCallback & GetOnComplete() const { return mOnComplete; }
        void * GetAppState() const { return mAppState; }
        Layer * GetSystemLayer() const { return mSystemLayer; }

    private:
#if CHIP_SYSTEM_CONFIG_USE_LIBEV
        friend class LayerImplSelect;
#endif
        Layer * mSystemLayer;
        TimerCompleteCallback mOnComplete;
        void * mAppState;
    };

    TimerData(Layer & systemLayer, System::Clock::Timestamp awakenTime, TimerCompleteCallback onComplete, void * appState) :
        mAwakenTime(awakenTime), mCallback(systemLayer, onComplete, appState)
    {}
    ~TimerData() = default;

    /**
     * Return the expiration time.
     */
    Clock::Timestamp AwakenTime() const { return mAwakenTime; }

    /**
     * Return callback information.
     */
    const Callback & GetCallback() const { return mCallback; }

private:
    Clock::Timestamp mAwakenTime;
    Callback mCallback;

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    friend class LayerImplSelect;
    dispatch_source_t mTimerSource = nullptr;
#elif CHIP_SYSTEM_CONFIG_USE_LIBEV
    friend class LayerImplSelect;
    struct ev_timer mLibEvTimer;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    // Not defined
    TimerData(const TimerData &)             = delete;
    TimerData & operator=(const TimerData &) = delete;
};

/**
 * List of `Timer`s ordered by expiration time.
 */
class TimerList
{
public:
    class Node : public TimerData
    {
    public:
        Node(Layer & systemLayer, System::Clock::Timestamp awakenTime, TimerCompleteCallback onComplete, void * appState) :
            TimerData(systemLayer, awakenTime, onComplete, appState), mNextTimer(nullptr)
        {}
        Node * mNextTimer;
    };

    TimerList() : mEarliestTimer(nullptr) {}

    /**
     * Add a timer to the list
     *
     * @return  The new earliest timer in the list. If this is the newly added timer, that implies it is earlier
     *          than any existing timer.
     */
    Node * Add(Node * timer);

    /**
     * Remove the given timer from the list, if present. It is not an error for the timer not to be present.
     *
     * @return  The new earliest timer in the list, or nullptr if the list is empty.
     */
    Node * Remove(Node * remove);

    /**
     * Remove the first timer with the given properties, if present. It is not an error for no such timer to be present.
     *
     * @return  The removed timer, or nullptr if the list contains no matching timer.
     */
    Node * Remove(TimerCompleteCallback onComplete, void * appState);

    /**
     * Remove and return the earliest timer in the list.
     *
     * @return  The earliest timer, or nullptr if the list is empty.
     */
    Node * PopEarliest();

    /**
     * Remove and return the earliest timer in the list, provided it expires earlier than the given time @a t.
     *
     * @return  The earliest timer expiring before @a t, or nullptr if there is no such timer.
     */
    Node * PopIfEarlier(Clock::Timestamp t);

    /**
     * Get the earliest timer in the list.
     *
     * @return  The earliest timer, or nullptr if there are no timers.
     */
    Node * Earliest() const { return mEarliestTimer; }

    /**
     * Test whether there are any timers.
     */
    bool Empty() const { return mEarliestTimer == nullptr; }

    /**
     * Remove and return all timers that expire before the given time @a t.
     */
    TimerList ExtractEarlier(Clock::Timestamp t);

    /**
     * Remove all timers.
     */
    void Clear() { mEarliestTimer = nullptr; }

    /**
     * Find the timer with the given properties, if present, and return its remaining time
     *
     * @return The remaining time on this partifcular timer or 0 if not found.
     */
    Clock::Timeout GetRemainingTime(TimerCompleteCallback aOnComplete, void * aAppState);

private:
    Node * mEarliestTimer;
};

/**
 * ObjectPool wrapper that keeps System Timer statistics.
 */
template <typename T = TimerList::Node>
class TimerPool
{
public:
    using Timer = T;

    /**
     * Create a new timer from the pool.
     */
    Timer * Create(Layer & systemLayer, System::Clock::Timestamp awakenTime, TimerCompleteCallback onComplete, void * appState)
    {
        Timer * timer = mTimerPool.CreateObject(systemLayer, awakenTime, onComplete, appState);
        SYSTEM_STATS_INCREMENT(Stats::kSystemLayer_NumTimers);
        return timer;
    }

    /**
     * Release a timer to the pool.
     */
    void Release(Timer * timer)
    {
        SYSTEM_STATS_DECREMENT(Stats::kSystemLayer_NumTimers);
        mTimerPool.ReleaseObject(timer);
    }

    /**
     * Release all timers.
     */
    void ReleaseAll()
    {
        SYSTEM_STATS_RESET(Stats::kSystemLayer_NumTimers);
        mTimerPool.ReleaseAll();
    }

    /**
     * Release a timer to the pool and invoke its callback.
     */
    void Invoke(Timer * timer)
    {
        typename Timer::Callback callback = timer->GetCallback();
        Release(timer);
        callback.Invoke();
    }

private:
    friend class TestSystemTimer_CheckTimerPool_Test;
    ObjectPool<Timer, CHIP_SYSTEM_CONFIG_NUM_TIMERS> mTimerPool;
};

} // namespace System
} // namespace chip
