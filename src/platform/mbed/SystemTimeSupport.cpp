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
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <Kernel.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

/**
 * Platform-specific function for getting monotonic system time in microseconds.
 *
 * @returns Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
uint64_t GetClock_Monotonic()
{
    return rtos::Kernel::get_ms_count() * UINT64_C(1000);
}

/**
 * Platform-specific function for getting monotonic system time in milliseconds.
 *
 * @returns Elapsed time in milliseconds since an arbitrary, platform-defined epoch.
 */
uint64_t GetClock_MonotonicMS()
{
    return rtos::Kernel::get_ms_count();
}

/**
 * Platform-specific function for getting high-resolution monotonic system time in microseconds.
 *
 * @returns Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
uint64_t GetClock_MonotonicHiRes()
{
    return GetClock_Monotonic();
}

/**
 * Platform-specific function for getting the current real (civil) time in microsecond Unix time
 * format.
 *
 * @param[out] curTime      The current time, expressed as Unix time scaled to microseconds.
 * @returns                 CHIP_SYSTEM_NO_ERROR if the method succeeded.
 */
Error GetClock_RealTime(uint64_t & curTime)
{
    struct timeval tv;
    int res = gettimeofday(&tv, NULL);
    if (res != 0)
    {
        return CHIP_SYSTEM_ERROR_UNEXPECTED_STATE;
    }
    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (tv.tv_sec * UINT64_C(1000000)) + tv.tv_usec;
    return CHIP_SYSTEM_NO_ERROR;
}

/**
 * Platform-specific function for getting the current real (civil) time in millisecond Unix time
 * format.
 *
 * @param[out] curTimeMS    The current time, expressed as Unix time scaled to milliseconds.
 * @returns                 CHIP_SYSTEM_NO_ERROR if the method succeeded.
 */
Error GetClock_RealTimeMS(uint64_t & curTimeMS)
{
    struct timeval tv;
    int res = gettimeofday(&tv, NULL);
    if (res != 0)
    {
        return CHIP_SYSTEM_ERROR_UNEXPECTED_STATE;
    }
    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTimeMS = (tv.tv_sec * UINT64_C(1000)) + (tv.tv_usec / 1000);
    return CHIP_SYSTEM_NO_ERROR;
}

/**
 * Platform-specific function for setting the current real (civil) time.
 *
 * @param[out] newCurTime   The new current time, expressed as Unix time scaled to microseconds.
 * @returns                 CHIP_SYSTEM_NO_ERROR if the method succeeded.
 */
Error SetClock_RealTime(uint64_t newCurTime)
{
    struct timeval tv;
    tv.tv_sec  = static_cast<time_t>(newCurTime / UINT64_C(1000000));
    tv.tv_usec = static_cast<long>(newCurTime % UINT64_C(1000000));
    int res    = settimeofday(&tv, NULL);
    if (res != 0)
    {
        return CHIP_SYSTEM_ERROR_UNEXPECTED_STATE;
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
    return CHIP_SYSTEM_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
