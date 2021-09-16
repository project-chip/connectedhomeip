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
 *          time/clock functions that are suitable for use on the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <esp_timer.h>

namespace chip {
namespace System {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

Clock::MonotonicMicroseconds ClockImpl::GetMonotonicMicroseconds(void)
{
    return (Clock::MonotonicMicroseconds)::esp_timer_get_time();
}

Clock::MonotonicMilliseconds GetMonotonicMilliseconds(void)
{
    return (Clock::MonotonicMilliseconds)::esp_timer_get_time() / kMicrosecondsPerMillisecond;
}

} // namespace System
} // namespace chip
