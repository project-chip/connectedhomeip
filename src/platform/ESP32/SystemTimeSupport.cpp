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

#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <esp_timer.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

uint64_t GetClock_Monotonic(void)
{
    return (uint64_t)::esp_timer_get_time();
}

uint64_t GetClock_MonotonicMS(void)
{
    return (uint64_t)::esp_timer_get_time() / 1000;
}

uint64_t GetClock_MonotonicHiRes(void)
{
    return (uint64_t)::esp_timer_get_time();
}

CHIP_ERROR GetClock_RealTime(uint64_t & curTime)
{
    struct timeval tv;
    int res = gettimeofday(&tv, NULL);
    if (res != 0)
    {
        return MapErrorPOSIX(errno);
    }
    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (tv.tv_sec * UINT64_C(1000000)) + tv.tv_usec;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTime)
{
    struct timeval tv;
    int res = gettimeofday(&tv, NULL);
    if (res != 0)
    {
        return MapErrorPOSIX(errno);
    }
    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (tv.tv_sec * UINT64_C(1000)) + (tv.tv_usec / 1000);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetClock_RealTime(uint64_t newCurTime)
{
    struct timeval tv;
    tv.tv_sec  = static_cast<time_t>(newCurTime / UINT64_C(1000000));
    tv.tv_usec = static_cast<long>(newCurTime % UINT64_C(1000000));
    int res    = settimeofday(&tv, NULL);
    if (res != 0)
    {
        return (errno == EPERM) ? CHIP_ERROR_ACCESS_DENIED : MapErrorPOSIX(errno);
    }
#if CHIP_PROGRESS_LOGGING
    {
        uint16_t year;
        uint8_t month, dayOfMonth, hour, minute, second;
        SecondsSinceEpochToCalendarTime(tv.tv_sec, year, month, dayOfMonth, hour, minute, second);
        ChipLogProgress(DeviceLayer,
                        "Real time clock set to %ld (%04" PRId16 "/%02" PRId8 "/%02" PRId8 " %02" PRId8 ":%02" PRId8 ":%02" PRId8
                        " UTC)",
                        tv.tv_sec, year, month, dayOfMonth, hour, minute, second);
    }
#endif // CHIP_PROGRESS_LOGGING
    return CHIP_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
