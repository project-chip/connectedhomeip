/*
 *
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
 *          time/clock functions that are suitable for use on the Posix platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <chrono>
#include <errno.h>
#include <inttypes.h>
#include <sys/time.h>

namespace chip {
namespace System {

namespace Internal {
ClockImpl gClockImpl;
} // namespace Internal

Clock::MonotonicMicroseconds ClockImpl::GetMonotonicMicroseconds()
{
    std::chrono::microseconds epoch =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch());
    // count() is nominally signed, but for a monotonic clock it cannot be negative.
    return static_cast<uint64_t>(epoch.count());
}

Clock::MonotonicMilliseconds ClockImpl::GetMonotonicMilliseconds()
{
    std::chrono::milliseconds epoch =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    // count() is nominally signed, but for a monotonic clock it cannot be negative.
    return static_cast<uint64_t>(epoch.count());
}

} // namespace System
} // namespace chip
