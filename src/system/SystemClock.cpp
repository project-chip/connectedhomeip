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

// __STDC_LIMIT_MACROS must be defined for UINT8_MAX to be defined for pre-C++11 clib
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif // __STDC_LIMIT_MACROS

// __STDC_CONSTANT_MACROS must be defined for INT64_C and UINT64_C to be defined for pre-C++11 clib
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif // __STDC_CONSTANT_MACROS

// config
#include <system/SystemConfig.h>

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

// module header
#include <system/SystemClock.h>
// common private
#include "SystemLayerPrivate.h"

#include <support/CodeUtils.h>
#include <support/TimeUtils.h>
#include <system/SystemError.h>

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#include <time.h>
#if !(HAVE_CLOCK_GETTIME)
#include <sys/time.h>
#endif
#include <errno.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/sys.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#include <stdint.h>
#include <stdlib.h>

namespace chip {
namespace System {
namespace Platform {
namespace Clock {

// -------------------- Default Get/SetClock Functions for POSIX Systems --------------------

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#if !HAVE_CLOCK_GETTIME && !HAVE_GETTIMEOFDAY
#error "CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS requires either clock_gettime() or gettimeofday()"
#endif

#if HAVE_CLOCK_GETTIME
#if HAVE_DECL_CLOCK_BOOTTIME
// CLOCK_BOOTTIME is a Linux-specific option to clock_gettime for a clock which compensates for system sleep.
#define MONOTONIC_CLOCK_ID CLOCK_BOOTTIME
#define MONOTONIC_RAW_CLOCK_ID CLOCK_MONOTONIC_RAW
#else // HAVE_DECL_CLOCK_BOOTTIME
// CLOCK_MONOTONIC is defined in POSIX and hence is the default choice
#define MONOTONIC_CLOCK_ID CLOCK_MONOTONIC
#endif
#endif // HAVE_CLOCK_GETTIME

uint64_t GetMonotonicMicroseconds()
{
#if HAVE_CLOCK_GETTIME
    struct timespec ts;
    int res = clock_gettime(MONOTONIC_CLOCK_ID, &ts);
    VerifyOrDie(res == 0);
    return (static_cast<uint64_t>(ts.tv_sec) * kMicrosecondsPerSecond) +
        (static_cast<uint64_t>(ts.tv_nsec) / kNanosecondsPerMicrosecond);
#else  // HAVE_CLOCK_GETTIME
    struct timeval tv;
    int res = gettimeofday(&tv, NULL);
    VerifyOrDie(res == 0);
    return (tv.tv_sec * kMicrosecondsPerSecond) + tv.tv_usec;
#endif // HAVE_CLOCK_GETTIME
}

uint64_t GetMonotonicMilliseconds()
{
    return GetMonotonicMicroseconds() / kMicrosecondsPerMillisecond;
}

CHIP_ERROR GetUnixTimeMicroseconds(uint64_t & curTime)
{
#if HAVE_CLOCK_GETTIME
    struct timespec ts;
    int res = clock_gettime(CLOCK_REALTIME, &ts);
    if (res != 0)
    {
        return MapErrorPOSIX(errno);
    }
    if (ts.tv_sec < CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD)
    {
        return CHIP_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (static_cast<uint64_t>(ts.tv_sec) * kMicrosecondsPerSecond) +
        (static_cast<uint64_t>(ts.tv_nsec) / kNanosecondsPerMicrosecond);
    return CHIP_NO_ERROR;
#else  // HAVE_CLOCK_GETTIME
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
    curTime = (tv.tv_sec * kMicrosecondsPerSecond) + tv.tv_usec;
    return CHIP_NO_ERROR;
#endif // HAVE_CLOCK_GETTIME
}

#if HAVE_CLOCK_SETTIME || HAVE_SETTIMEOFDAY

CHIP_ERROR SetUnixTimeMicroseconds(uint64_t newCurTime)
{
#if HAVE_CLOCK_SETTIME
    struct timespec ts;
    ts.tv_sec  = static_cast<time_t>(newCurTime / kMicrosecondsPerSecond);
    ts.tv_nsec = static_cast<long>(newCurTime % kMicrosecondsPerSecond) * kNanosecondsPerMicrosecond;
    int res    = clock_settime(CLOCK_REALTIME, &ts);
    if (res != 0)
    {
        return (errno == EPERM) ? CHIP_ERROR_ACCESS_DENIED : MapErrorPOSIX(errno);
    }
    return CHIP_NO_ERROR;
#else  // HAVE_CLOCK_SETTIME
    struct timeval tv;
    tv.tv_sec  = static_cast<time_t>(newCurTime / kMicrosecondsPerSecond);
    tv.tv_usec = static_cast<long>(newCurTime % kMicrosecondsPerSecond);
    int res    = settimeofday(&tv, NULL);
    if (res != 0)
    {
        return (errno == EPERM) ? CHIP_ERROR_ACCESS_DENIED : MapErrorPOSIX(errno);
    }
    return CHIP_NO_ERROR;
#endif // HAVE_CLOCK_SETTIME
}

#else // !HAVE_CLOCK_SETTTIME

CHIP_ERROR SetUnixTimeMicroseconds(uint64_t newCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

#endif // HAVE_CLOCK_SETTIME || HAVE_SETTIMEOFDAY

#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

// -------------------- Default Get/SetClock Functions for LwIP Systems --------------------

#if CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

uint64_t GetMonotonicMicroseconds(void)
{
    return GetMonotonicMilliseconds() * kMicrosecondsPerMillisecond;
}

uint64_t GetMonotonicMilliseconds(void)
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

    return static_cast<uint64_t>(overflowSample | static_cast<uint64_t>(sample));
}

CHIP_ERROR GetUnixTimeMicroseconds(uint64_t & curTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR SetUnixTimeMicroseconds(uint64_t newCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

} // namespace Clock
} // namespace Platform
} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
