/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *          EFR32 System Layer clocks backed by Sleeptimer. The configured
 *          counter must run in every energy mode that resumes without reset.
 *          Wall clock time is valid only after synchronization in this boot.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/silabs/SystemTimeSupport.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>

#include "sl_core.h"
#include "sl_sleeptimer.h"

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

namespace {

// Unix microseconds at monotonic zero. Zero means no synchronization this boot.
uint64_t sBootTimeUS = 0;

/**
 * Converts a Sleeptimer tick count to microseconds without overflowing.
 *
 * A naive (ticks * kMicrosecondsPerSecond) overflows uint64 after roughly 18
 * years at the typical 32768 Hz LFXO rate, so the quotient and remainder are
 * scaled separately.
 */
uint64_t TicksToMicroseconds(uint64_t ticks, uint32_t frequency)
{
    VerifyOrDie(frequency != 0);

    const uint64_t seconds   = ticks / frequency;
    const uint64_t remainder = ticks % frequency;

    return (seconds * kMicrosecondsPerSecond) + ((remainder * kMicrosecondsPerSecond) / frequency);
}

uint64_t GetMonotonicMicroseconds()
{
    return TicksToMicroseconds(sl_sleeptimer_get_tick_count64(), sl_sleeptimer_get_timer_frequency());
}

/**
 * Returns true if the given Unix time is recent enough to be plausible.
 *
 * Anything below the configured threshold (2000-01-01 by default) is treated
 * as "no real time"; the threshold itself is valid.
 */
bool IsPlausibleRealTime(uint64_t unixTimeUS)
{
    return unixTimeUS >= (static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * kMicrosecondsPerSecond);
}

} // unnamed namespace

Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    return Microseconds64(GetMonotonicMicroseconds());
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

uint64_t GetClock_Monotonic()
{
    return GetMonotonicMicroseconds();
}

uint64_t GetClock_MonotonicMS()
{
    return GetMonotonicMicroseconds() / kMicrosecondsPerMillisecond;
}

uint64_t GetClock_MonotonicHiRes()
{
    return GetMonotonicMicroseconds();
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    const uint64_t bootTimeUS = sBootTimeUS;
    CORE_EXIT_CRITICAL();

    VerifyOrReturnError(bootTimeUS != 0, CHIP_ERROR_REAL_TIME_NOT_SYNCED);

    aCurTime = Microseconds64(bootTimeUS + GetMonotonicMicroseconds());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    Microseconds64 currentTimeUS;
    ReturnErrorOnFailure(GetClock_RealTime(currentTimeUS));

    aCurTime = std::chrono::duration_cast<Milliseconds64>(currentTimeUS);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    VerifyOrReturnError(IsPlausibleRealTime(aNewCurTime.count()), CHIP_ERROR_INVALID_TIME);

    const uint64_t monotonicUS = GetMonotonicMicroseconds();
    VerifyOrReturnError(aNewCurTime.count() > monotonicUS, CHIP_ERROR_INVALID_TIME);

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    sBootTimeUS = aNewCurTime.count() - monotonicUS;
    CORE_EXIT_CRITICAL();

    return CHIP_NO_ERROR;
}

CHIP_ERROR InitClock_RealTime()
{
    VerifyOrReturnError(sl_sleeptimer_get_timer_frequency() != 0, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Invalid Sleeptimer frequency"));

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_CRITICAL();
    sBootTimeUS = 0;
    CORE_EXIT_CRITICAL();

    return CHIP_NO_ERROR;
}

} // namespace Clock
} // namespace System
} // namespace chip
