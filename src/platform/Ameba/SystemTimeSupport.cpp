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

#include <support/TimeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "task.h"
#include <time.h>

extern void rtc_init(void);
extern time_t rtc_read(void);
extern void rtc_write(time_t t);

struct rtkTimeVal
{
    uint32_t tv_sec;  /* seconds */
    uint32_t tv_usec; /* microseconds */
};

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

uint64_t GetClock_Monotonic(void)
{
    return xTaskGetTickCount() * 1000;
}

uint64_t GetClock_MonotonicMS(void)
{
    return xTaskGetTickCount();
}

uint64_t GetClock_MonotonicHiRes(void)
{
    return xTaskGetTickCount() * 1000;
}

CHIP_ERROR GetClock_RealTime(uint64_t & curTime)
{
    time_t seconds;
    struct rtkTimeVal tv;

    seconds = rtc_read();

    tv.tv_sec  = (uint32_t) seconds;
    tv.tv_usec = 0;

    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (tv.tv_sec * UINT64_C(1000000)) + tv.tv_usec;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTime)
{
    time_t seconds;
    struct rtkTimeVal tv;

    seconds = rtc_read();

    tv.tv_sec  = (uint32_t) seconds;
    tv.tv_usec = 0;

    if (tv.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }

    curTime = (tv.tv_sec * UINT64_C(1000)) + (tv.tv_usec / 1000);

    return CHIP_NO_ERROR;
}

CHIP_ERROR SetClock_RealTime(uint64_t newCurTime)
{
    rtc_init();
    rtc_write(newCurTime);

    return CHIP_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
