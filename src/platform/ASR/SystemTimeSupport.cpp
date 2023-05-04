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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/logging/CHIPLogging.h>
#include <platform/ASR/SystemTimeSupport.h>

#include <lega_rtos_api.h>

#if CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return Clock::Microseconds64(lega_rtos_get_time() * 1000l);
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return std::chrono::duration_cast<Milliseconds64>(GetMonotonicMicroseconds64());
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    // TODO(19081): This platform does not properly error out if wall clock has
    //              not been set.  For now, short circuit this.
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    Microseconds64 curTimeUs;
    auto err = GetClock_RealTime(curTimeUs);
    aCurTime = std::chrono::duration_cast<Milliseconds64>(curTimeUs);
    return err;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    struct timeval tv;
    tv.tv_sec  = static_cast<time_t>(aNewCurTime.count() / UINT64_C(1000000));
    tv.tv_usec = static_cast<long>(aNewCurTime.count() % UINT64_C(1000000));

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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

#endif
