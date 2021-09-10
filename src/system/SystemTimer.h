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
 *      This file defines the chip::System::Timer class and its
 *      related types used for representing an in-progress one-shot
 *      timer.
 *
 *      Some platforms use this to implement System::Layer timer events.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <lib/support/DLLUtil.h>

#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemMutex.h>
#include <system/SystemObject.h>
#include <system/SystemStats.h>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif

#if CHIP_SYSTEM_CONFIG_USE_TIMER_POOL
#include <mutex>
#endif // CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

namespace chip {
namespace System {

using TimerCompleteCallback = void (*)(Layer * aLayer, void * appState);

#if CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

/**
 * This is an Object-pool based class that System::Layer implementations can use to assist in providing timer functions.
 */
class DLL_EXPORT Timer : public Object
{
public:
    /**
     * List of timers ordered by completion time.
     *
     * @note
     *  This is an intrusive linked list, using the Timer field `mNextTimer`.
     */
    class List
    {
    public:
        List() : mHead(nullptr) {}
        List(Timer * head) : mHead(head) {}

        /**
         * Add a timer to the list
         *
         * @return  The new earliest timer in the list. If this is the newly added timer, that implies it is earlier
         *          than any existing timer.
         */
        Timer * Add(Timer * add);

        /**
         * Remove the given timer from the list, if present. It is not an error for the timer not to be present.
         *
         * @return  The new earliest timer in the list, or nullptr if the list is empty.
         */
        Timer * Remove(Timer * remove);

        /**
         * Remove the first timer with the given properties, if present. It is not an error for no such timer to be present.
         *
         * @return  The removed timer, or nullptr if the list contains no matching timer.
         */
        Timer * Remove(TimerCompleteCallback onComplete, void * appState);

        /**
         * Remove and return the earliest timer in the list.
         *
         * @return  The earliest timer, or nullptr if the list is empty.
         */
        Timer * PopEarliest();

        /**
         * Remove and return the earliest timer in the list, provided it expires earlier than the given time @a t.
         *
         * @return  The earliest timer expiring before @a t, or nullptr if there is no such timer.
         */
        Timer * PopIfEarlier(Clock::MonotonicMilliseconds t);

        /**
         * Remove and return all timers that expire before the given time @a t.
         *
         * @return  An ordered linked list (by `mNextTimer`) of all timers that expire before @a t, or nullptr if there are none.
         */
        Timer * ExtractEarlier(Clock::MonotonicMilliseconds t);

        /**
         * Get the earliest timer in the list.
         */
        Timer * Earliest() const { return mHead; }

    protected:
        Timer * mHead;
        List(const List &) = delete;
        List & operator=(const List &) = delete;
    };
    /**
     * List of timers ordered by completion time.
     *
     * This extends Timer::List to lock all access to the list.
     */
    class MutexedList : private List
    {
    public:
        MutexedList() = default;
        CHIP_ERROR Init();
        bool Empty() const
        {
            std::lock_guard<Mutex> lock(mMutex);
            return mHead == nullptr;
        }
        Timer * Add(Timer * add)
        {
            std::lock_guard<Mutex> lock(mMutex);
            return List::Add(add);
        }
        Timer * Remove(Timer * remove)
        {
            std::lock_guard<Mutex> lock(mMutex);
            return List::Remove(remove);
        }
        Timer * Remove(TimerCompleteCallback onComplete, void * appState)
        {
            std::lock_guard<Mutex> lock(mMutex);
            return List::Remove(onComplete, appState);
        }
        Timer * PopEarliest()
        {
            std::lock_guard<Mutex> lock(mMutex);
            return List::PopEarliest();
        }
        Timer * PopIfEarlier(Clock::MonotonicMilliseconds t)
        {
            std::lock_guard<Mutex> lock(mMutex);
            return List::PopIfEarlier(t);
        }
        Timer * ExtractEarlier(Clock::MonotonicMilliseconds t)
        {
            std::lock_guard<Mutex> lock(mMutex);
            return List::ExtractEarlier(t);
        }
        Timer * Earliest() const
        {
            std::lock_guard<Mutex> lock(mMutex);
            return mHead;
        }

    private:
        mutable Mutex mMutex;
        MutexedList(const MutexedList &) = delete;
        MutexedList & operator=(const MutexedList &) = delete;
    };

    Timer() = default;

    /**
     * Obtain a new timer from the system object pool.
     */
    static Timer * New(System::Layer & systemLayer, uint32_t delayMilliseconds, TimerCompleteCallback onComplete, void * appState);

    /**
     * Return the expiration time.
     */
    Clock::MonotonicMilliseconds AwakenTime() const { return mAwakenTime; }

    /**
     * Fire the timer.
     *
     * This method is called by the underlying timer mechanism provided by the platform when the timer fires.
     * It invalidates this timer object, calls Object::Release() on it, and invokes the callback.
     */
    void HandleComplete();

    /**
     * Invalidate the timer fields. This is intended for timer cancellation, and typically this will be followed by
     * an object Release().
     *
     * @note
     *  The Timer owner is responsible for ensuring this timer is not in use, e.g. in a List or by a platform timer implementation.
     */
    void Clear();

    /**
     * Read timer pool statistics.
     */
    static void GetStatistics(chip::System::Stats::count_t & aNumInUse, chip::System::Stats::count_t & aHighWatermark)
    {
        sPool.GetStatistics(aNumInUse, aHighWatermark);
    }

private:
    friend class LayerImplLwIP;
    static ObjectPool<Timer, CHIP_SYSTEM_CONFIG_NUM_TIMERS> sPool;

    TimerCompleteCallback mOnComplete;
    Clock::MonotonicMilliseconds mAwakenTime;
    Timer * mNextTimer;

    Layer * mSystemLayer;

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    friend class LayerImplSelect;
    dispatch_source_t mTimerSource = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    // Not defined
    Timer(const Timer &) = delete;
    Timer & operator=(const Timer &) = delete;
};

#endif // CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

} // namespace System
} // namespace chip
