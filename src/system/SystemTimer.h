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
#include <support/DLLUtil.h>

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

namespace Timers {

typedef void (*OnCompleteFunct)(Layer * aLayer, void * appState);

} // namespace Timers

#if CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

/**
 * This is an Object-pool based class that System::Layer implementations can use to assist in providing timer functions.
 */
class DLL_EXPORT Timer : public Object
{
public:
    class List
    {
    public:
        List() : mHead(nullptr) {}
        List(Timer * head) : mHead(head) {}
        bool Empty() const { return mHead == nullptr; }
        Timer * Add(Timer * add);
        Timer * Remove(Timer * remove);
        Timer * Remove(Timers::OnCompleteFunct onComplete, void * appState);
        Timer * PopEarliest();
        Timer * PopIfEarlier(Clock::MonotonicMilliseconds t);
        Timer * ExtractEarlier(Clock::MonotonicMilliseconds t);
        Timer * Earliest() const { return mHead; }

    protected:
        Timer * mHead;
        List(const List &) = delete;
        List & operator=(const List &) = delete;
    };
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
        Timer * Remove(Timers::OnCompleteFunct onComplete, void * appState)
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

    static Timer * New(System::Layer & systemLayer, uint32_t delayMilliseconds, Timers::OnCompleteFunct onComplete,
                       void * appState);
    void Clear();

    static void GetStatistics(chip::System::Stats::count_t & aNumInUse, chip::System::Stats::count_t & aHighWatermark)
    {
        sPool.GetStatistics(aNumInUse, aHighWatermark);
    }

private:
    friend class WatchableEventManager;

    static ObjectPool<Timer, CHIP_SYSTEM_CONFIG_NUM_TIMERS> sPool;

    void HandleComplete();

    Timers::OnCompleteFunct mOnComplete;
    Clock::MonotonicMilliseconds mAwakenTime;
    Timer * mNextTimer;

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_source_t mTimerSource = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    // Not defined
    Timer(const Timer &) = delete;
    Timer & operator=(const Timer &) = delete;
};

#endif // CHIP_SYSTEM_CONFIG_USE_TIMER_POOL

} // namespace System
} // namespace chip
