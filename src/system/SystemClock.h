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

class ClockBase
{
public:
    // Note: these provide documentation, not type safety.
    using MonotonicMicroseconds = uint64_t;
    using MonotonicMilliseconds = uint64_t;
    using UnixTimeMicroseconds  = uint64_t;

    virtual ~ClockBase() = default;

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
     * @returns Elapsed time in microseconds since an arbitrary, platform-defined epoch.
     */
    virtual MonotonicMicroseconds GetMonotonicMicroseconds() = 0;

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
    virtual MonotonicMilliseconds GetMonotonicMilliseconds() = 0;
};

// Currently we have a single implementation class, ClockImpl, whose members are implemented in build-specific files.
class ClockImpl : public ClockBase
{
public:
    ~ClockImpl() = default;
    MonotonicMicroseconds GetMonotonicMicroseconds() override;
    MonotonicMilliseconds GetMonotonicMilliseconds() override;
};

namespace Internal {
// These should only be used via the public Clock:: functions below.
extern ClockImpl gClockImpl;
extern ClockBase * gClockBase;
} // namespace Internal

namespace Clock {
using MonotonicMicroseconds = ClockBase::MonotonicMicroseconds;
using MonotonicMilliseconds = ClockBase::MonotonicMicroseconds;
using UnixTimeMicroseconds  = ClockBase::MonotonicMicroseconds;

inline MonotonicMicroseconds GetMonotonicMicroseconds()
{
    return Internal::gClockBase->GetMonotonicMicroseconds();
}

inline MonotonicMilliseconds GetMonotonicMilliseconds()
{
    return Internal::gClockBase->GetMonotonicMilliseconds();
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
bool IsEarlier(const Clock::MonotonicMilliseconds & first, const Clock::MonotonicMilliseconds & second);

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS
Clock::MonotonicMilliseconds TimevalToMilliseconds(const timeval & in);
void MillisecondsToTimeval(Clock::MonotonicMilliseconds in, timeval & out);
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Clock

} // namespace System
} // namespace chip
