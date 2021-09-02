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
 *      This is an internal header file that defines the interface to a platform-supplied
 *      function for retrieving the current system time.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <lib/support/DLLUtil.h>
#include <lib/support/TimeUtils.h>

#include <system/SystemError.h>

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <sys/time.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace chip {
namespace System {

namespace Platform {
namespace Clock {

/**
 * Platform-specific function for getting monotonic system time in microseconds.
 *
 * This function is expected to return elapsed time in microseconds since an arbitrary, platform-defined
 * epoch.  Platform implementations are obligated to return a value that is ever-increasing (i.e. never
 * wraps) between reboots of the system.  Additionally, the underlying time source is required to tick
 * continuously during any system sleep modes that do not entail a restart upon wake.
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Clock::GetMonotonicMicroseconds().
 *
 * @returns             Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
extern uint64_t GetMonotonicMicroseconds();

/**
 * Platform-specific function for getting monotonic system time in milliseconds.
 *
 * This function is expected to return elapsed time in milliseconds since an arbitrary, platform-defined
 * epoch.  Platform implementations are obligated to return a value that is ever-increasing (i.e. never
 * wraps) between reboots of the system.  Additionally, the underlying time source is required to tick
 * continuously during any system sleep modes that do not entail a restart upon wake.
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Clock::GetMonotonicMilliseconds().
 *
 * @returns             Elapsed time in milliseconds since an arbitrary, platform-defined epoch.
 */
extern uint64_t GetMonotonicMilliseconds();

/**
 * Platform-specific function for getting the current real (civil) time in microsecond Unix time format.
 *
 * This function is expected to return the local platform's notion of current real time, expressed
 * as a Unix time value scaled to microseconds.  The underlying clock is required to tick at a
 * rate of least at whole seconds (values of 1,000,000), but may tick faster.
 *
 * On those platforms that are capable of tracking real time, GetUnixTimeMicroseconds() must return the
 * error CHIP_ERROR_REAL_TIME_NOT_SYNCED whenever the system is unsynchronized with real time.
 *
 * Platforms that are incapable of tracking real time should not implement the GetUnixTimeMicroseconds()
 * function, thereby forcing link-time failures of features that depend on access to real time.
 * Alternatively, such platforms may supply an implementation of GetUnixTimeMicroseconds() that returns
 * the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Clock::GetUnixTimeMicroseconds().
 *
 * @param[out] curTime                  The current time, expressed as Unix time scaled to microseconds.
 *
 * @retval #CHIP_NO_ERROR       If the method succeeded.
 * @retval #CHIP_ERROR_REAL_TIME_NOT_SYNCED
 *                                      If the platform is capable of tracking real time, but is
 *                                      is currently unsynchronized.
 * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *                                      If the platform is incapable of tracking real time.
 */
extern CHIP_ERROR GetUnixTimeMicroseconds(uint64_t & curTime);

/**
 * Platform-specific function for setting the current real (civil) time.
 *
 * CHIP calls this function to set the local platform's notion of current real time.  The new current
 * time is expressed as a Unix time value scaled to microseconds.
 *
 * Once set, underlying platform clock is expected to track real time with a granularity of at least whole
 * seconds.
 *
 * On platforms that support tracking real time, the SetUnixTimeMicroseconds() function must return the error
 * CHIP_ERROR_ACCESS_DENIED if the calling application does not have the privilege to set the
 * current time.
 *
 * Platforms that are incapable of tracking real time, or do not offer the ability to set real time,
 * should not implement the SetUnixTimeMicroseconds() function, thereby forcing link-time failures of features
 * that depend on setting real time.  Alternatively, such platforms may supply an implementation of
 * SetUnixTimeMicroseconds() that returns the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::SetUnixTimeMicroseconds().
 *
 * @param[in] newCurTime                The new current time, expressed as Unix time scaled to microseconds.
 *
 * @retval #CHIP_NO_ERROR       If the method succeeded.
 * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *                                      If the platform is incapable of tracking real time.
 * @retval #CHIP_ERROR_ACCESS_DENIED
 *                                      If the calling application does not have the privilege to set the
 *                                      current time.
 */
extern CHIP_ERROR SetUnixTimeMicroseconds(uint64_t newCurTime);

} // namespace Clock
} // namespace Platform

class Clock
{
public:
    // Note: these provide documentation, not type safety.
    using MonotonicMicroseconds = uint64_t;
    using MonotonicMilliseconds = uint64_t;
    using UnixTimeMicroseconds  = uint64_t;

    /**
     * Returns a monotonic system time in units of microseconds.
     *
     * This function returns an elapsed time in microseconds since an arbitrary, platform-defined
     * epoch.  The value returned is guaranteed to be ever-increasing (i.e. never wrapping) between
     * reboots of the system.  Additionally, the underlying time source is guaranteed to tick
     * continuously during any system sleep modes that do not entail a restart upon wake.
     *
     * Although some platforms may choose to return a value that measures the time since boot for the
     * system, applications must *not* rely on this.
     *
     * @returns             Elapsed time in microseconds since an arbitrary, platform-defined epoch.
     */
    static inline MonotonicMicroseconds GetMonotonicMicroseconds()
    {
        // Current implementation is a simple pass-through to the platform.
        return Platform::Clock::GetMonotonicMicroseconds();
    }

    /**
     * Returns a monotonic system time in units of milliseconds.
     *
     * This function returns an elapsed time in milliseconds since an arbitrary, platform-defined
     * epoch.  The value returned is guaranteed to be ever-increasing (i.e. never wrapping) between
     * reboots of the system.  Additionally, the underlying time source is guaranteed to tick
     * continuously during any system sleep modes that do not entail a restart upon wake.
     *
     * Although some platforms may choose to return a value that measures the time since boot for the
     * system, applications must *not* rely on this.
     *
     * @returns             Elapsed time in milliseconds since an arbitrary, platform-defined epoch.
     */
    static inline MonotonicMilliseconds GetMonotonicMilliseconds()
    {
        // Current implementation is a simple pass-through to the platform.
        return Platform::Clock::GetMonotonicMilliseconds();
    }

    /**
     * Returns the current real (civil) time in microsecond Unix time format.
     *
     * This method returns the local platform's notion of current real time, expressed as a Unix time
     * value scaled to microseconds.  The underlying clock is guaranteed to tick at a rate of least at
     * whole seconds (values of 1,000,000), but on some platforms may tick faster.
     *
     * If the underlying platform is capable of tracking real time, but the system is currently
     * unsynchronized, GetRealTime() will return the error CHIP_ERROR_REAL_TIME_NOT_SYNCED.
     *
     * On platforms that are incapable of tracking real time, the GetRealTime() method may be absent,
     * resulting a link error for any application that references it.  Alternatively, such platforms may
     * supply an implementation of GetRealTime() that always returns the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
     *
     * @param[out] curTime                  The current time, expressed as Unix time scaled to microseconds.
     *
     * @retval #CHIP_NO_ERROR       If the method succeeded.
     * @retval #CHIP_ERROR_REAL_TIME_NOT_SYNCED
     *                                      If the platform is capable of tracking real time, but is
     *                                      is currently unsynchronized.
     * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *                                      If the platform is incapable of tracking real time.
     */
    static inline CHIP_ERROR GetUnixTimeMicroseconds(UnixTimeMicroseconds & curTime)
    {
        // Current implementation is a simple pass-through to the platform.
        return Platform::Clock::GetUnixTimeMicroseconds(curTime);
    }

    /**
     * Sets the platform's notion of current real (civil) time.
     *
     * Applications can call this function to set the local platform's notion of current real time.  The
     * new current time is expressed as a Unix time value scaled to microseconds.
     *
     * Once set, underlying platform clock is guaranteed to track real time with a granularity of at least
     * whole seconds.
     *
     * Some platforms may restrict which applications or processes can set real time.  If the caller is
     * not permitted to change real time, the SetRealTime() function will return the error
     * CHIP_ERROR_ACCESS_DENIED.
     *
     * On platforms that are incapable of tracking real time, or do not offer the ability to set real time,
     * the SetRealTime() function may be absent, resulting a link error for any application that
     * references it.  Alternatively, such platforms may supply an implementation of SetRealTime()
     * that always returns the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
     *
     * This function is guaranteed to be thread-safe on any platform that employs threading.
     *
     * @param[in] newCurTime                The new current time, expressed as Unix time scaled to microseconds.
     *
     * @retval #CHIP_NO_ERROR               If the method succeeded.
     * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *                                      If the platform is incapable of tracking real time.
     * @retval #CHIP_ERROR_ACCESS_DENIED
     *                                      If the calling application does not have the privilege to set the
     *                                      current time.
     */
    static inline CHIP_ERROR SetUnixTimeMicroseconds(UnixTimeMicroseconds newCurTime)
    {
        // Current implementation is a simple pass-through to the platform.
        return Platform::Clock::SetUnixTimeMicroseconds(newCurTime);
    }

    /**
     *  Compares two Clock::MonotonicMilliseconds values and returns true if the first value is earlier than the second value.
     *
     *  @brief
     *      A static API that gets called to compare 2 time values.  This API attempts to account for timer wrap by assuming that
     * the difference between the 2 input values will only be more than half the timestamp scalar range if a timer wrap has occurred
     *      between the 2 samples.
     *
     *  @note
     *      This implementation assumes that Clock::MonotonicMilliseconds is an unsigned scalar type.
     *
     *  @return true if the first param is earlier than the second, false otherwise.
     */
    static bool IsEarlier(const Clock::MonotonicMilliseconds & first, const Clock::MonotonicMilliseconds & second);
};

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS
Clock::MonotonicMilliseconds TimevalToMilliseconds(const timeval & in);
void MillisecondsToTimeval(Clock::MonotonicMilliseconds in, timeval & out);
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace System
} // namespace chip
