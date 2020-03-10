/*
 *
 *    <COPYRIGHT>
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

#ifndef SYSTEMTIMER_H
#define SYSTEMTIMER_H

// Include configuration headers
#include <SystemConfig.h>

// Include dependent headers
#include <support/DLLUtil.h>

#include <SystemClock.h>
#include <SystemError.h>
#include <SystemObject.h>
#include <SystemStats.h>

#if CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

namespace chip {
namespace Inet {

class InetLayer;

} // namespace Inet
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

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

    static Epoch GetCurrentEpoch(void);
    static bool IsEarlierEpoch(const Epoch &first, const Epoch &second);

    typedef void (*OnCompleteFunct)(Layer* aLayer, void* aAppState, Error aError);
    OnCompleteFunct OnComplete;

    Error Start(uint32_t aDelayMilliseconds, OnCompleteFunct aOnComplete, void* aAppState);
    Error Cancel(void);

    static void GetStatistics(chip::System::Stats::count_t& aNumInUse, chip::System::Stats::count_t& aHighWatermark);

#if CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    void AttachInetLayer(Inet::InetLayer& aInetLayer, void* aOnCompleteInetLayer, void* aAppStateInetLayer);
    Inet::InetLayer* InetLayer(void) const;
    void* OnCompleteInetLayer(void) const;
    void* AppStateInetLayer(void) const;
#endif // CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

private:
    static ObjectPool<Timer, CHIP_SYSTEM_CONFIG_NUM_TIMERS> sPool;

    Epoch mAwakenEpoch;

#if CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
    Inet::InetLayer* mInetLayer;
    void* mOnCompleteInetLayer;
    void* mAppStateInetLayer;
#endif // CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

    void HandleComplete(void);

    Error ScheduleWork(OnCompleteFunct aOnComplete, void* aAppState);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    Timer *mNextTimer;

    static Error HandleExpiredTimers(Layer& aLayer);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    // Not defined
    Timer(const Timer&);
    Timer& operator =(const Timer&);
};


inline void Timer::GetStatistics(chip::System::Stats::count_t& aNumInUse,
                                 chip::System::Stats::count_t& aHighWatermark)
{
    sPool.GetStatistics(aNumInUse, aHighWatermark);
}

#if CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES
inline void Timer::AttachInetLayer(Inet::InetLayer& aInetLayer, void* aOnCompleteInetLayer, void* aAppStateInetLayer)
{
    this->mInetLayer = &aInetLayer;
    this->mOnCompleteInetLayer = aOnCompleteInetLayer;
    this->mAppStateInetLayer = aAppStateInetLayer;
}

inline Inet::InetLayer* Timer::InetLayer(void) const
{
    return this->mInetLayer;
}

inline void* Timer::OnCompleteInetLayer(void) const
{
    return this->mOnCompleteInetLayer;
}

inline void* Timer::AppStateInetLayer(void) const
{
    return this->mAppStateInetLayer;
}
#endif // CHIP_SYSTEM_CONFIG_PROVIDE_OBSOLESCENT_INTERFACES

} // namespace System
} // namespace chip

#endif // defined(SYSTEMTIMER_H)
