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

#include <lib/support/TimeUtils.h>

#include "FreeRTOS.h"

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

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

Clock::Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return Clock::Microseconds64((FreeRTOSTicksSinceBoot() * kMicrosecondsPerSecond) / configTICK_RATE_HZ);
}

Clock::Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return Clock::Milliseconds64((FreeRTOSTicksSinceBoot() * kMillisecondsPerSecond) / configTICK_RATE_HZ);
}

uint64_t GetClock_Monotonic(void)
{
    return (FreeRTOSTicksSinceBoot() * kMicrosecondsPerSecond) / configTICK_RATE_HZ;
}

uint64_t GetClock_MonotonicMS(void)
{
    return (FreeRTOSTicksSinceBoot() * kMillisecondsPerSecond) / configTICK_RATE_HZ;
}

uint64_t GetClock_MonotonicHiRes(void)
{
    return GetClock_Monotonic();
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Clock::Microseconds64 & aCurTime)
{
    // TODO(19081): This platform does not properly error out if wall clock has
    //              not been set.  For now, short circuit this.
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#if 0
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    aCurTime = Clock::Microseconds64(sBootTimeUS + GetClock_Monotonic());
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Clock::Milliseconds64 & aCurTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    aCurTime = Clock::Milliseconds64((sBootTimeUS + GetClock_Monotonic()) / 1000);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Clock::Microseconds64 aNewCurTime)
{
    uint64_t timeSinceBootUS = GetClock_Monotonic();
    if (aNewCurTime.count() > timeSinceBootUS)
    {
        sBootTimeUS = aNewCurTime.count() - timeSinceBootUS;
    }
    else
    {
        sBootTimeUS = 0;
    }
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
