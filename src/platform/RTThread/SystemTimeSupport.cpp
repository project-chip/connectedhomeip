/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *          time/clock functions based on the RT-Thread tick counter.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>

#include <rtthread.h>

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

namespace {

static uint64_t sBootTimeUS = 0;

uint64_t RTThreadTicksSinceBoot(void)
{
    rt_tick_t tick = rt_tick_get();
    return static_cast<uint64_t>(tick) * static_cast<uint64_t>(RT_TICK_PER_SECOND) * UINT64_C(1000000) / RT_TICK_PER_SECOND;
}

} // unnamed namespace

Clock::Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return Clock::Microseconds64(RTThreadTicksSinceBoot());
}

Clock::Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return Clock::Milliseconds64(RTThreadTicksSinceBoot() / 1000);
}

uint64_t GetClock_Monotonic(void)
{
    return RTThreadTicksSinceBoot();
}

uint64_t GetClock_MonotonicMS(void)
{
    return RTThreadTicksSinceBoot() / 1000;
}

uint64_t GetClock_MonotonicHiRes(void)
{
    return GetClock_Monotonic();
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Clock::Microseconds64 & aCurTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    aCurTime = Clock::Microseconds64(sBootTimeUS + GetClock_Monotonic());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Clock::Milliseconds64 & aCurTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    aCurTime = Clock::Milliseconds64((sBootTimeUS + GetClock_Monotonic()) / 1000);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Clock::Microseconds64 aNewCurTime)
{
    uint64_t timeSinceBootUS = GetClock_Monotonic();
    if (aNewCurTime.count() > timeSinceBootUS)
    {
        sBootTimeUS = aNewCurTime.count() - timeSinceBootUS;
    }
    else
    {
        sBootTimeUS = 0;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR InitClock_RealTime()
{
    Clock::Microseconds64 curTime = Clock::Microseconds64((static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * UINT64_C(1000000)));
    // Use CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD as the initial value of RealTime.
    // Then the RealTime obtained from GetClock_RealTime will always be valid.
    return System::SystemClock().SetClock_RealTime(curTime);
}

} // namespace Clock
} // namespace System
} // namespace chip