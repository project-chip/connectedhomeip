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
namespace Platform {
namespace Layer {

static uint64_t sBootTimeUS = 0;

uint64_t GetClock_Monotonic(void)
{
    return k_ticks_to_us_floor64(k_uptime_ticks());
}

uint64_t GetClock_MonotonicMS(void)
{
    return k_uptime_get();
}

uint64_t GetClock_MonotonicHiRes(void)
{
    return GetClock_Monotonic();
}

CHIP_ERROR GetClock_RealTime(uint64_t & curTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = sBootTimeUS + GetClock_Monotonic();
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (sBootTimeUS + GetClock_Monotonic()) / 1000;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetClock_RealTime(uint64_t newCurTime)
{
    // FIXME: make thread-safe or update comment in SystemClock.h
    uint64_t timeSinceBootUS = GetClock_Monotonic();
    if (newCurTime > timeSinceBootUS)
    {
        sBootTimeUS = newCurTime - timeSinceBootUS;
    }
    else
    {
        sBootTimeUS = 0;
    }
    return CHIP_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
