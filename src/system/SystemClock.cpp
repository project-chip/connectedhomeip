/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *      Provides default implementations for the platform Get/SetClock_ functions
 *      for POSIX and LwIP platforms.
 */

#include <system/SystemClock.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <system/SystemError.h>

#include <limits>
#include <stdint.h>
#include <stdlib.h>

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <errno.h>
#include <time.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/sys.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

namespace chip {
namespace System {
namespace Clock {

namespace Internal {

#if CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
extern ClockImpl gClockImpl;
#else  // CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
ClockImpl gClockImpl;
#endif // CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

ClockBase * gClockBase = &gClockImpl;

} // namespace Internal

Timestamp ClockBase::GetMonotonicTimestamp()
{
    // Below implementation uses `__atomic_*` API which has wider support than
    // <atomic> on embedded platforms, so that embedded platforms can use
    // it by widening the #ifdefs later.
#if CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK
    uint64_t prevTimestamp = __atomic_load_n(&mLastTimestamp, __ATOMIC_SEQ_CST);
    static_assert(sizeof(prevTimestamp) == sizeof(Timestamp), "Must have scalar match between timestamp and uint64_t for atomics.");

    // Force a reorder barrier to prevent GetMonotonicMilliseconds64() from being
    // optimizer-called before prevTimestamp loading, so that newTimestamp acquisition happens-after
    // the prevTimestamp load.
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
#else
    uint64_t prevTimestamp = mLastTimestamp;
#endif // CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK

    Timestamp newTimestamp = GetMonotonicMilliseconds64();

    // Need to guarantee the invariant that monotonic clock never goes backwards, which would break multiple system
    // assumptions which use these clocks.
    VerifyOrDie(newTimestamp.count() >= prevTimestamp);

#if CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK
    // newTimestamp guaranteed to never be < the last timestamp.
    __atomic_store_n(&mLastTimestamp, newTimestamp.count(), __ATOMIC_SEQ_CST);
#else
    mLastTimestamp         = newTimestamp.count();
#endif // CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK

    return newTimestamp;
}

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

// -------------------- Default Get/SetClock Functions for POSIX Systems --------------------

#if !HAVE_CLOCK_GETTIME && !HAVE_GETTIMEOFDAY
#error "CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS requires either clock_gettime() or gettimeofday()"
#endif

#if HAVE_CLOCK_GETTIME

#if defined(HAVE_DECL_CLOCK_BOOTTIME) && HAVE_DECL_CLOCK_BOOTTIME
// CLOCK_BOOTTIME is a Linux-specific option to clock_gettime for a clock which compensates for system sleep.
#define MONOTONIC_CLOCK_ID CLOCK_BOOTTIME
#define MONOTONIC_RAW_CLOCK_ID CLOCK_MONOTONIC_RAW
#else // HAVE_DECL_CLOCK_BOOTTIME
// CLOCK_MONOTONIC is defined in POSIX and hence is the default choice
#define MONOTONIC_CLOCK_ID CLOCK_MONOTONIC
#endif

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    struct timespec ts;
    int res = clock_gettime(MONOTONIC_CLOCK_ID, &ts);
    VerifyOrDie(res == 0);
    return Seconds64(ts.tv_sec) +
        std::chrono::duration_cast<Microseconds64>(std::chrono::duration<uint64_t, std::nano>(ts.tv_nsec));
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

#endif // HAVE_CLOCK_GETTIME

#if HAVE_GETTIMEOFDAY

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    struct timeval tv;
    int res = gettimeofday(&tv, NULL);
    VerifyOrDie(res == 0);
    return TimevalToMicroseconds(tv);
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

#endif // HAVE_GETTIMEOFDAY

#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#if CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

// -------------------- Default Get/SetClock Functions for LwIP Systems --------------------

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    return GetMonotonicMilliseconds64();
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    static volatile uint64_t overflow        = 0;
    static volatile u32_t lastSample         = 0;
    static volatile uint8_t lock             = 0;
    static const uint64_t kOverflowIncrement = static_cast<uint64_t>(0x100000000);

    uint64_t overflowSample;
    u32_t sample;

    // Tracking timer wrap assumes that this function gets called with
    // a period that is less than 1/2 the timer range.
    if (__sync_bool_compare_and_swap(&lock, 0, 1))
    {
        sample = sys_now();

        if (lastSample > sample)
        {
            overflow += kOverflowIncrement;
        }

        lastSample     = sample;
        overflowSample = overflow;

        __sync_bool_compare_and_swap(&lock, 1, 0);
    }
    else
    {
        // a lower priority task is in the block above. Depending where that
        // lower task is blocked can spell trouble in a timer wrap condition.
        // the question here is what this task should use as an overflow value.
        // To fix this race requires a platform api that can be used to
        // protect critical sections.
        overflowSample = overflow;
        sample         = sys_now();
    }

    return Milliseconds64(overflowSample | static_cast<uint64_t>(sample));
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

#endif // CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

Microseconds64 TimevalToMicroseconds(const timeval & tv)
{
    return Seconds64(tv.tv_sec) + Microseconds64(tv.tv_usec);
}

void ToTimeval(Microseconds64 in, timeval & out)
{
    Seconds32 seconds = std::chrono::duration_cast<Seconds32>(in);
    in -= seconds;
    out.tv_sec  = static_cast<time_t>(seconds.count());
    out.tv_usec = static_cast<suseconds_t>(in.count());
}

#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

static_assert(std::numeric_limits<Microseconds64::rep>::is_integer, "Microseconds64 must be an integer type");
static_assert(std::numeric_limits<Microseconds32::rep>::is_integer, "Microseconds32 must be an integer type");
static_assert(std::numeric_limits<Milliseconds64::rep>::is_integer, "Milliseconds64 must be an integer type");
static_assert(std::numeric_limits<Milliseconds32::rep>::is_integer, "Milliseconds32 must be an integer type");
static_assert(std::numeric_limits<Seconds64::rep>::is_integer, "Seconds64 must be an integer type");
static_assert(std::numeric_limits<Seconds32::rep>::is_integer, "Seconds32 must be an integer type");
static_assert(std::numeric_limits<Seconds16::rep>::is_integer, "Seconds16 must be an integer type");

static_assert(std::numeric_limits<Microseconds64::rep>::digits >= 64, "Microseconds64 must be at least 64 bits");
static_assert(std::numeric_limits<Microseconds32::rep>::digits >= 32, "Microseconds32 must be at least 32 bits");
static_assert(std::numeric_limits<Milliseconds64::rep>::digits >= 64, "Milliseconds64 must be at least 64 bits");
static_assert(std::numeric_limits<Milliseconds32::rep>::digits >= 32, "Milliseconds32 must be at least 32 bits");
static_assert(std::numeric_limits<Seconds64::rep>::digits >= 64, "Seconds64 must be at least 64 bits");
static_assert(std::numeric_limits<Seconds32::rep>::digits >= 32, "Seconds32 must be at least 32 bits");
static_assert(std::numeric_limits<Seconds16::rep>::digits >= 16, "Seconds16 must be at least 16 bits");

} // namespace Clock
} // namespace System
} // namespace chip
