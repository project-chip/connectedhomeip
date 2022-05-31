/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides an implementation of the system time support
 *          for Open IOT SDK platform.
 */

#include <inttypes.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "cmsis_os2.h"

namespace chip {
namespace System {
namespace Clock {

namespace Internal {

ClockImpl gClockImpl;

} // namespace Internal

const uint32_t TICKS_PER_SECOND = osKernelGetTickFreq();
const uint32_t US_PER_TICK      = 1000000 / TICKS_PER_SECOND;
const uint32_t MS_PER_TICK      = US_PER_TICK / 1000;

extern "C" uint64_t GetTick(void);
extern "C" void SetTick(uint64_t newTick);

Clock::Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    uint64_t ret = GetTick() * US_PER_TICK;
    return Clock::Microseconds64(ret);
}

Clock::Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return Clock::Milliseconds64(GetTick() * MS_PER_TICK);
}

uint64_t GetClock_Monotonic(void)
{
    return (GetTick() * US_PER_TICK);
}

uint64_t GetClock_MonotonicMS(void)
{
    return (GetTick() * MS_PER_TICK);
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Clock::Microseconds64 & aCurTime)
{
    aCurTime = Clock::Microseconds64(GetClock_Monotonic());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Clock::Milliseconds64 & aCurTime)
{
    aCurTime = Clock::Milliseconds64(GetClock_MonotonicMS());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Clock::Microseconds64 aNewCurTime)
{
    uint64_t newTickCount;
    newTickCount = static_cast<uint64_t>(aNewCurTime.count() / UINT64_C(1000000)) * TICKS_PER_SECOND;
    SetTick(newTickCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR InitClock_RealTime()
{
    // Use CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD as the initial value of RealTime.
    // Then the RealTime obtained from GetClock_RealTime will be always valid.
    Clock::Microseconds64 curTime =
        Clock::Microseconds64((static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * UINT64_C(1000000)));
    return System::SystemClock().SetClock_RealTime(curTime);
}

} // namespace Clock
} // namespace System
} // namespace chip
