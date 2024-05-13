/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          time/clock functions that are suitable for use on the Mbed-OS platform.
 */
/* this file behaves like a config.h, comes first */
#include <inttypes.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "platform/mbed_rtc_time.h"
#include "platform/mbed_thread.h"
#include "platform/mbed_wait_api.h"
#include <Kernel.h>
#include <drivers/LowPowerTimer.h>

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

namespace {
mbed::LowPowerTimer timer()
{
    static mbed::LowPowerTimer t;
    t.start();
    return t;
}
} // namespace

extern "C" uint64_t get_clock_monotonic()
{
    return timer().elapsed_time().count();
}

// Platform-specific function for getting monotonic system time in microseconds.
// Returns elapsed time in microseconds since an arbitrary, platform-defined epoch.
Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    return Microseconds64(get_clock_monotonic());
}

// Platform-specific function for getting monotonic system time in milliseconds.
// Return elapsed time in milliseconds since an arbitrary, platform-defined epoch.
Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    // TODO(19081): This platform does not properly error out if wall clock has
    //              not been set.  For now, short circuit this.
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#if 0
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }
    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    if (tv.tv_usec < 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    static_assert(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD >= 0, "We might be letting through negative tv_sec values!");
    aCurTime = Microseconds64((static_cast<uint64_t>(tv.tv_sec) * UINT64_C(1000000)) + static_cast<uint64_t>(tv.tv_usec));
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    Microseconds64 curTimeUs;
    auto err = GetClock_RealTime(curTimeUs);
    aCurTime = std::chrono::duration_cast<Milliseconds64>(curTimeUs);
    return err;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    struct timeval tv;
    tv.tv_sec  = static_cast<time_t>(aNewCurTime.count() / UINT64_C(1000000));
    tv.tv_usec = static_cast<long>(aNewCurTime.count() % UINT64_C(1000000));
    if (settimeofday(&tv, nullptr) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    const time_t timep = tv.tv_sec;
    struct tm calendar;
    localtime_r(&timep, &calendar);
    ChipLogProgress(DeviceLayer, "Real time clock set to %lld (%04d/%02d/%02d %02d:%02d:%02d UTC)", tv.tv_sec, calendar.tm_year,
                    calendar.tm_mon, calendar.tm_mday, calendar.tm_hour, calendar.tm_min, calendar.tm_sec);

    return CHIP_NO_ERROR;
}

CHIP_ERROR InitClock_RealTime()
{
    Clock::Microseconds64 curTime =
        Clock::Microseconds64((static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * UINT64_C(1000000)));
    // Use CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD as the initial value of RealTime.
    // Then the RealTime obtained from GetClock_RealTime will be always valid.
    //
    // TODO(19081): This is broken because it causes the platform to report
    //              that it does have wall clock time when it actually doesn't.
    return System::SystemClock().SetClock_RealTime(curTime);
}

} // namespace Clock
} // namespace System
} // namespace chip
