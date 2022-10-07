/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          time/clock functions based on the Zephyr tick counter.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <system/SystemError.h>

#include <zephyr/zephyr.h>

#if !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

namespace chip {
namespace System {
namespace Clock {

namespace Internal {

ClockImpl gClockImpl;

} // namespace Internal

namespace {

constexpr Microseconds64 kUnknownRealTime = Seconds64::zero();

// Unix epoch time of boot event
Microseconds64 gBootRealTime = kUnknownRealTime;

} // namespace

Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    return Microseconds64(k_ticks_to_us_floor64(k_uptime_ticks()));
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    return Milliseconds64(k_uptime_get());
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    // The real time can be configured by an application if it has access to a reliable time source.
    // Otherwise, just return an error so that Matter stack can fallback to Last Known UTC Time.
    ReturnErrorCodeIf(gBootRealTime == kUnknownRealTime, CHIP_ERROR_INCORRECT_STATE);

    aCurTime = gBootRealTime + GetMonotonicMicroseconds64();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    Microseconds64 curTimeUs;

    ReturnErrorOnFailure(GetClock_RealTime(curTimeUs));
    aCurTime = std::chrono::duration_cast<Milliseconds64>(curTimeUs);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    gBootRealTime = aNewCurTime - GetMonotonicMicroseconds64();

    return CHIP_NO_ERROR;
}

} // namespace Clock
} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
