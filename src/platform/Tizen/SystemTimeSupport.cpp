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
 *          time/clock functions that are suitable for use on the Tizen platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/TimeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

uint64_t GetClock_Monotonic()
{
    return static_cast<uint64_t>(0);
}

uint64_t GetClock_MonotonicMS()
{
    return static_cast<uint64_t>(0);
}

uint64_t GetClock_MonotonicHiRes()
{
    return static_cast<uint64_t>(0);
}

CHIP_ERROR GetClock_RealTime(uint64_t & curTime)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GetClock_RealTimeMS(uint64_t & curTime)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR SetClock_RealTime(uint64_t newCurTime)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip
