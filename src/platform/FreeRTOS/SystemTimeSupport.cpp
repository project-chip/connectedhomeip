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
 *          time/clock functions based on the FreeRTOS tick counter.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/TimeUtils.h>

#include "FreeRTOS.h"

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

namespace {

constexpr uint32_t kTicksOverflowShift = (configUSE_16_BIT_TICKS) ? 16 : 32;

uint64_t sBootTimeUS = 0;

#ifdef __CORTEX_M
BaseType_t sNumOfOverflows;
#endif
} // unnamed namespace

/**
 * Returns the number of FreeRTOS ticks since the system booted.
 *
 * NOTE: The default implementation of this function uses FreeRTOS's
 * vTaskSetTimeOutState() function to get the total number of ticks,
 * irrespective of tick counter overflows.  Unfortunately, this function cannot
 * be called in interrupt context, no equivalent ISR function exists, and
 * FreeRTOS provides no portable way of determining whether a function is being
 * called in an interrupt context.  Adaptations that need to use the Chip
 * Get/SetClock methods from within an interrupt handler must override this
 * function with a suitable alternative that works on the target platform.  The
 * provided version is safe to call on ARM Cortex platforms with CMSIS
 * libraries.
 */

uint64_t FreeRTOSTicksSinceBoot(void) __attribute__((weak));

uint64_t FreeRTOSTicksSinceBoot(void)
{
    TimeOut_t timeOut;

#ifdef __CORTEX_M
    if (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) // running in an interrupt context
    {
        // Note that sNumOverflows may be quite stale, and under those
        // circumstances, the function may violate monotonicity guarantees
        timeOut.xTimeOnEntering = xTaskGetTickCountFromISR();
        timeOut.xOverflowCount  = sNumOfOverflows;
    }
    else
    {
#endif

        vTaskSetTimeOutState(&timeOut);

#ifdef __CORTEX_M
        // BaseType_t is supposed to be atomic
        sNumOfOverflows = timeOut.xOverflowCount;
    }
#endif

    return static_cast<uint64_t>(timeOut.xTimeOnEntering) + (static_cast<uint64_t>(timeOut.xOverflowCount) << kTicksOverflowShift);
}

uint64_t GetClock_Monotonic(void)
{
    return (FreeRTOSTicksSinceBoot() * kMicrosecondsPerSecond) / configTICK_RATE_HZ;
}

uint64_t GetClock_MonotonicMS(void)
{
    return (FreeRTOSTicksSinceBoot() * kMillisecondPerSecond) / configTICK_RATE_HZ;
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
