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
#include <support/DLLUtil.h>

#include <system/SystemError.h>

namespace chip {
namespace System {

enum
{
    kTimerFactor_nano_per_micro  = 1000, /** Number of nanoseconds in a microsecond. */
    kTimerFactor_micro_per_milli = 1000, /** Number of microseconds in a millisecond. */
    kTimerFactor_milli_per_unit  = 1000, /** Number of milliseconds in a second. */

    kTimerFactor_nano_per_milli = 1000000, /** Number of nanoseconds in a millisecond. */
    kTimerFactor_micro_per_unit = 1000000  /** Number of microseconds in a second. */
};

namespace Platform {
namespace Layer {

/**
 * @brief
 *   Platform-specific function for getting monotonic system time in microseconds.
 *
 * This function is expected to return elapsed time in microseconds since an arbitrary, platform-defined
 * epoch.  Platform implementations are obligated to return a value that is ever-increasing (i.e. never
 * wraps) between reboots of the system.  Additionally, the underlying time source is required to tick
 * continuously during any system sleep modes that do not entail a restart upon wake.
 *
 * The epoch for time returned by this function is *not* required to be the same that for any of the
 * other GetClock... functions, including GetClock_MonotonicMS().
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::GetClock_Monotonic().
 *
 * @returns             Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
extern uint64_t GetClock_Monotonic();

/**
 * @brief
 *   Platform-specific function for getting monotonic system time in milliseconds.
 *
 * This function is expected to return elapsed time in milliseconds since an arbitrary, platform-defined
 * epoch.  Platform implementations are obligated to return a value that is ever-increasing (i.e. never
 * wraps) between reboots of the system.  Additionally, the underlying time source is required to tick
 * continuously during any system sleep modes that do not entail a restart upon wake.
 *
 * The epoch for time returned by this function is *not* required to be the same as that for any of the
 * other GetClock... functions, including GetClock_Monotonic().
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::GetClock_MonotonicMS().
 *
 * @returns             Elapsed time in milliseconds since an arbitrary, platform-defined epoch.
 */
extern uint64_t GetClock_MonotonicMS();

/**
 * @brief
 *   Platform-specific function for getting high-resolution monotonic system time in microseconds.
 *
 * This function is expected to return elapsed time in microseconds since an arbitrary, platform-defined
 * epoch.  Values returned by GetClock_MonotonicHiRes() are required to be ever-increasing (i.e. never
 * wrap).  However, the underlying timer is *not* required to tick continuously during system
 * deep-sleep states.
 *
 * Platform are encouraged to implement GetClock_MonotonicHiRes() using a high-resolution timer
 * that is not subject to gradual clock adjustments (slewing).  On platforms without such a timer,
 * GetClock_MonotonicHiRes() can return the same value as GetClock_Monotonic().
 *
 * The epoch for time returned by this function is not required to be the same that for any of the
 * other GetClock... functions.
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::GetClock_MonotonicHiRes().
 *
 * @returns             Elapsed time in microseconds since an arbitrary, platform-defined epoch.
 */
extern uint64_t GetClock_MonotonicHiRes();

/**
 * @brief
 *   Platform-specific function for getting the current real (civil) time in microsecond Unix time
 *   format.
 *
 * This function is expected to return the local platform's notion of current real time, expressed
 * as a Unix time value scaled to microseconds.  The underlying clock is required to tick at a
 * rate of least at whole seconds (values of 1,000,000), but may tick faster.
 *
 * On those platforms that are capable of tracking real time, GetClock_RealTime() must return the
 * error CHIP_ERROR_REAL_TIME_NOT_SYNCED whenever the system is unsynchronized with real time.
 *
 * Platforms that are incapable of tracking real time should not implement the GetClock_RealTime()
 * function, thereby forcing link-time failures of features that depend on access to real time.
 * Alternatively, such platforms may supply an implementation of GetClock_RealTime() that returns
 * the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::GetClock_RealTime().
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
extern CHIP_ERROR GetClock_RealTime(uint64_t & curTime);

/**
 * @brief
 *   Platform-specific function for getting the current real (civil) time in millisecond Unix time
 *   format.
 *
 * This function is expected to return the local platform's notion of current real time, expressed
 * as a Unix time value scaled to milliseconds.
 *
 * See the documentation for GetClock_RealTime() for details on the expected behavior.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::GetClock_RealTimeMS().
 *
 * @param[out] curTimeMS               The current time, expressed as Unix time scaled to milliseconds.
 *
 * @retval #CHIP_NO_ERROR       If the method succeeded.
 * @retval #CHIP_ERROR_REAL_TIME_NOT_SYNCED
 *                                      If the platform is capable of tracking real time, but is
 *                                      is currently unsynchronized.
 * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *                                      If the platform is incapable of tracking real time.
 */
extern CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTimeMS);

/**
 * @brief
 *   Platform-specific function for setting the current real (civil) time.
 *
 * CHIP calls this function to set the local platform's notion of current real time.  The new current
 * time is expressed as a Unix time value scaled to microseconds.
 *
 * Once set, underlying platform clock is expected to track real time with a granularity of at least whole
 * seconds.
 *
 * On platforms that support tracking real time, the SetClock_RealTime() function must return the error
 * CHIP_ERROR_ACCESS_DENIED if the calling application does not have the privilege to set the
 * current time.
 *
 * Platforms that are incapable of tracking real time, or do not offer the ability to set real time,
 * should not implement the SetClock_RealTime() function, thereby forcing link-time failures of features
 * that depend on setting real time.  Alternatively, such platforms may supply an implementation of
 * SetClock_RealTime() that returns the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
 *
 * This function is expected to be thread-safe on any platform that employs threading.
 *
 * @note
 *   This function is reserved for internal use by the CHIP System Layer.  Users of the CHIP System
 *   Layer should call System::Layer::GetClock_RealTimeMS().
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
extern CHIP_ERROR SetClock_RealTime(uint64_t newCurTime);

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
