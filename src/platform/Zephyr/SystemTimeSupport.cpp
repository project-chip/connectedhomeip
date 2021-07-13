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
namespace Clock {

static uint64_t sBootTimeUS = 0;

uint64_t GetMonotonicMicroseconds(void)
{
    return k_ticks_to_us_floor64(k_uptime_ticks());
}

uint64_t GetMonotonicMilliseconds(void)
{
    return k_uptime_get();
}

CHIP_ERROR GetUnixTimeMicroseconds(uint64_t & curTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = sBootTimeUS + GetMonotonicMicroseconds();
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetUnixTimeMicroseconds(uint64_t newCurTime)
{
    uint64_t timeSinceBootUS = GetMonotonicMicroseconds();
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

} // namespace Clock
} // namespace Platform
} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
