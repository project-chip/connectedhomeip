
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
 *          time/clock functions that are suitable for use on the Ameba platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/Ameba/SystemTimeSupport.h>
#include <support/logging/CHIPLogging.h>

#include "task.h"
#include <chip_porting.h>
#include <time.h>

struct rtkTimeVal
{
    uint32_t tv_sec;  /* seconds */
    uint32_t tv_usec; /* microseconds */
};

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return Clock::Microseconds64(ameba_get_clock_time());
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Clock::Microseconds64 & curTime)
{
#if CONFIG_ENABLE_AMEBA_SNTP
    time_t seconds = 0, uSeconds = 0;

    if (matter_sntp_rtc_is_sync()) // if RTC is already sync with SNTP, read directly from RTC
    {
        seconds = matter_rtc_read(); // ameba rtc precission is in seconds only
    }
    else // read from SNTP and sync RTC with SNTP
    {
        matter_sntp_get_current_time(&seconds, &uSeconds);
    }

    if (seconds < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    if (uSeconds < 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    static_assert(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD >= 0, "We might be letting through negative uSeconds values!");
    curTime = Microseconds64((static_cast<uint64_t>(seconds) * UINT64_C(1000000)) + static_cast<uint64_t>(uSeconds));

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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
    struct rtkTimeVal tv;
    tv.tv_sec  = static_cast<uint32_t>(aNewCurTime.count() / UINT64_C(1000000));
    tv.tv_usec = static_cast<uint32_t>(aNewCurTime.count() % UINT64_C(1000000));
    matter_rtc_write(tv.tv_sec);

    return CHIP_NO_ERROR;
}

CHIP_ERROR InitClock_RealTime()
{
    Clock::Microseconds64 curTime =
        Clock::Microseconds64((static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * UINT64_C(1000000)));
    // Use CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD as the initial value of RealTime.
    // Then the RealTime obtained from GetClock_RealTime will be always valid.
#if CONFIG_ENABLE_AMEBA_SNTP
    time_t seconds = 0, uSeconds = 0;

    matter_sntp_init();
    matter_sntp_get_current_time(&seconds, &uSeconds); // try to read from SNTP and sync RTC with SNTP
    if ((seconds > CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) && (uSeconds > 0))
    {
        curTime = Microseconds64((static_cast<uint64_t>(seconds) * UINT64_C(1000000)) + static_cast<uint64_t>(uSeconds));
    }
#endif
    return System::SystemClock().SetClock_RealTime(curTime);
}

} // namespace Clock
} // namespace System
} // namespace chip
