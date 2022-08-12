/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *    @file
 *          Provides implementations of the CHIP System Layer platform
 *          time/clock functions that are suitable for use on the Beken platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "FreeRTOS.h"

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

Clock::Milliseconds64 baseTime;

Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return (Clock::Microseconds64(xTaskGetTickCount() << 1) * kMicrosecondsPerMillisecond);
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return (Clock::Milliseconds64(xTaskGetTickCount() << 1));
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Clock::Microseconds64 & curTime)
{
    curTime = GetMonotonicMicroseconds64();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Clock::Milliseconds64 & curTime)
{
    curTime = baseTime + GetMonotonicMilliseconds64();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Clock::Microseconds64 aNewCurTime)
{
    baseTime = Clock::Milliseconds64(aNewCurTime.count() / kMicrosecondsPerMillisecond);

    return CHIP_NO_ERROR;
}

CHIP_ERROR InitClock_RealTime()
{
    baseTime =
        Clock::Milliseconds64((static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * kMicrosecondsPerMillisecond));

    // Use CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD as the initial value of RealTime.
    // Then the RealTime obtained from GetClock_RealTime will be always valid.
    //
    // TODO(19081): This is broken because it causes the platform to report
    //              that it does have wall clock time when it actually doesn't.
    return System::SystemClock().SetClock_RealTime(baseTime);
}

} // namespace Clock
} // namespace System
} // namespace chip
