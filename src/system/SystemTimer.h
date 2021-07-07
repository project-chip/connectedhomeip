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
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <support/DLLUtil.h>

#include <system/SystemClock.h>
#include <system/SystemError.h>
#include <system/SystemObject.h>
#include <system/SystemStats.h>

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif

namespace chip {
namespace System {

class Layer;

/**
 * @class Timer
 *
 * @brief
 *  This is an internal class to CHIP System Layer, used to represent an in-progress one-shot timer. There is no real public
 *  interface available for the application layer. The static public methods used to acquire current system time are intended for
 *  internal use.
 *
 */
class DLL_EXPORT Timer : public Object
{
    friend class Layer;

public:
    /**
     *  Represents an epoch in the local system timescale, usually the POSIX timescale.
     *
     *  The units are dependent on the context. If used with values returned by GetCurrentEpoch, the units are milliseconds.
     */
    typedef uint64_t Epoch;

    static Epoch GetCurrentEpoch();
    static bool IsEarlierEpoch(const Epoch & first, const Epoch & second);

    typedef void (*OnCompleteFunct)(Layer * aLayer, void * aAppState, CHIP_ERROR aError);
    OnCompleteFunct OnComplete;

    CHIP_ERROR Start(uint32_t aDelayMilliseconds, OnCompleteFunct aOnComplete, void * aAppState);
    CHIP_ERROR Cancel();

    static void GetStatistics(chip::System::Stats::count_t & aNumInUse, chip::System::Stats::count_t & aHighWatermark);

private:
    static ObjectPool<Timer, CHIP_SYSTEM_CONFIG_NUM_TIMERS> sPool;

    Epoch mAwakenEpoch;

    void HandleComplete();

    CHIP_ERROR ScheduleWork(OnCompleteFunct aOnComplete, void * aAppState);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    Timer * mNextTimer;

    static CHIP_ERROR HandleExpiredTimers(Layer & aLayer);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_source_t mTimerSource = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    // Not defined
    Timer(const Timer &) = delete;
    Timer & operator=(const Timer &) = delete;
};

inline void Timer::GetStatistics(chip::System::Stats::count_t & aNumInUse, chip::System::Stats::count_t & aHighWatermark)
{
    sPool.GetStatistics(aNumInUse, aHighWatermark);
}

} // namespace System
} // namespace chip
