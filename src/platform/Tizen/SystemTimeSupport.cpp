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
 *          time/clock functions that are suitable for use on the Tizen platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

uint64_t GetClock_Monotonic()
{
    std::chrono::microseconds epoch =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    // count() is nominally signed, but for a monotonic clock it cannot be
    // negative.
    return static_cast<uint64_t>(epoch.count());
}

uint64_t GetClock_MonotonicMS()
{
    std::chrono::milliseconds epoch =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    // count() is nominally signed, but for a monotonic clock it cannot be
    // negative.
    return static_cast<uint64_t>(epoch.count());
}

uint64_t GetClock_MonotonicHiRes()
{
    std::chrono::microseconds epoch =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    // count() is nominally signed, but for a monotonic clock it cannot be
    // negative.
    return static_cast<uint64_t>(epoch.count());
}

CHIP_ERROR GetClock_RealTime(uint64_t & curTime)
{
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
    curTime = (static_cast<uint64_t>(tv.tv_sec) * UINT64_C(1000000)) + static_cast<uint64_t>(tv.tv_usec);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTime)
{
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
    curTime = (static_cast<uint64_t>(tv.tv_sec) * UINT64_C(1000)) + (static_cast<uint64_t>(tv.tv_usec) / 1000);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetClock_RealTime(uint64_t newCurTime)
{
    struct timeval tv;
    tv.tv_sec  = static_cast<time_t>(newCurTime / UINT64_C(1000000));
    tv.tv_usec = static_cast<long>(newCurTime % UINT64_C(1000000));
    if (settimeofday(&tv, nullptr) != 0)
    {
        return (errno == EPERM) ? CHIP_ERROR_ACCESS_DENIED : CHIP_ERROR_POSIX(errno);
    }
#if CHIP_PROGRESS_LOGGING
    {
        const time_t timep = tv.tv_sec;
        struct tm calendar;
        localtime_r(&timep, &calendar);
        ChipLogProgress(DeviceLayer, "Real time clock set to %ld (%04" PRId16 "/%02d/%02d %02d:%02d:%02d UTC)", tv.tv_sec,
                        calendar.tm_year, calendar.tm_mon, calendar.tm_mday, calendar.tm_hour, calendar.tm_min, calendar.tm_sec);
    }
#endif // CHIP_PROGRESS_LOGGING
    return CHIP_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
