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

#include <zephyr.h>

#if !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

namespace chip {
namespace System {
namespace Clock {

namespace Internal {

ClockImpl gClockImpl;

} // namespace Internal

namespace {

// Last known UTC time in Unix format.
#ifdef CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_UNIX_TIME
Microseconds64 gRealTime = Seconds64(CHIP_DEVICE_CONFIG_FIRMWARE_BUILD_UNIX_TIME);
#else
Microseconds64 gRealTime = Seconds64(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD);
#endif

// Monotonic time of setting the last known UTC time.
Microseconds64 gRealTimeSetTime;

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
    aCurTime = GetMonotonicMicroseconds64() - gRealTimeSetTime + gRealTime;

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
    gRealTime        = aNewCurTime;
    gRealTimeSetTime = GetMonotonicMicroseconds64();

    return CHIP_NO_ERROR;
}

} // namespace Clock
} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
