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
 *          time/clock functions that are suitable for use on the Mbed-OS platform.
 */
/* this file behaves like a config.h, comes first */
#include <inttypes.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "platform/mbed_rtc_time.h"
#include "platform/mbed_thread.h"
#include "platform/mbed_wait_api.h"
#include <Kernel.h>
#include <drivers/LowPowerTimer.h>

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

namespace {
mbed::LowPowerTimer timer()
{
    static mbed::LowPowerTimer t;
    t.start();
    return t;
}
} // namespace

extern "C" uint64_t get_clock_monotonic()
{
    return timer().elapsed_time().count();
}

// Platform-specific function for getting monotonic system time in microseconds.
// Returns elapsed time in microseconds since an arbitrary, platform-defined epoch.
Microseconds64 ClockImpl::GetMonotonicMicroseconds64()
{
    return Microseconds64(get_clock_monotonic());
}

// Platform-specific function for getting monotonic system time in milliseconds.
// Return elapsed time in milliseconds since an arbitrary, platform-defined epoch.
Milliseconds64 ClockImpl::GetMonotonicMilliseconds64()
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Clock::Microseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Clock::Milliseconds64 & aCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Clock::Microseconds64 aNewCurTime)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Clock
} // namespace System
} // namespace chip
