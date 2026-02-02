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
 *          event functions that are suitable for use on all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace System {

using namespace ::chip::DeviceLayer;

CriticalFailure PlatformEventing::ScheduleLambdaBridge(System::Layer & aLayer, LambdaBridge && bridge)
{
    VerifyOrReturnError(bridge.IsInitialized(), CHIP_ERROR_UNINITIALIZED);

    ChipDeviceEvent event{ .Type = DeviceEventType::kChipLambdaEvent };

    event.LambdaEvent.Take(std::move(bridge));

    auto err = PlatformMgr().PostEvent(&event);

    // If the event is successfully posted, the LambdaBridge will be cleaned up either by calling the LambdaBridge or destroy the
    // event scheduler.
    // Otherwise, clean it up immediately.
    if (err != CHIP_NO_ERROR)
    {
        event.LambdaEvent.Destroy();
    }

    return err;
}

CriticalFailure PlatformEventing::StartTimer(System::Layer & aLayer, System::Clock::Timeout delay)
{
    return PlatformMgr().StartChipTimer(delay);
}

} // namespace System
} // namespace chip
