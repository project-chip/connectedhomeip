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
 *          time/clock functions that are suitable for use on the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/SystemTimeSupport.h>

#include <esp_timer.h>

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return Clock::Microseconds64(::esp_timer_get_time());
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
#ifdef CONFIG_ENABLE_SNTP_TIME_SYNC
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
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
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_ENABLE_SNTP_TIME_SYNC
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
        return (errno == EPERM) ? CHIP_ERROR_ACCESS_DENIED : CHIP_ERROR_POSIX(errno);
    }
#if CHIP_PROGRESS_LOGGING
    {
        const time_t timep = tv.tv_sec;
        struct tm calendar;
        localtime_r(&timep, &calendar);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S UTC", &calendar);
        ChipLogProgress(DeviceLayer, "Real time clock set to %lld (%s)", static_cast<long long>(tv.tv_sec), time_str);
    }
#endif // CHIP_PROGRESS_LOGGING
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
