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

#include <bl_timer.h>

#if CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

namespace chip {
namespace System {
namespace Clock {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

static Microseconds64 gBootRealTime = Seconds64::zero();

Microseconds64 ClockImpl::GetMonotonicMicroseconds64(void)
{
    return Clock::Microseconds64(bl_timer_now_us64());
}

Milliseconds64 ClockImpl::GetMonotonicMilliseconds64(void)
{
    return Clock::Milliseconds64(bl_timer_now_us64() / 1000);
}

CHIP_ERROR ClockImpl::GetClock_RealTime(Microseconds64 & aCurTime)
{
    aCurTime = gBootRealTime + GetMonotonicMicroseconds64();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::GetClock_RealTimeMS(Milliseconds64 & aCurTime)
{
    Microseconds64 curTimeUs;

    GetClock_RealTime(curTimeUs);
    aCurTime = std::chrono::duration_cast<Milliseconds64>(curTimeUs);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClockImpl::SetClock_RealTime(Microseconds64 aNewCurTime)
{
    gBootRealTime = aNewCurTime - GetMonotonicMicroseconds64();

    return CHIP_NO_ERROR;
}

} // namespace Clock
} // namespace System
} // namespace chip

#endif
