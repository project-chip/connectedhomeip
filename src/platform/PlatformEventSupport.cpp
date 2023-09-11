/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides implementations of the CHIP System Layer platform
 *          event functions that are suitable for use on all platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

namespace chip {
namespace System {

using namespace ::chip::DeviceLayer;

CHIP_ERROR PlatformEventing::ScheduleLambdaBridge(System::Layer & aLayer, LambdaBridge && bridge)
{
    ChipDeviceEvent event;
    event.Type        = DeviceEventType::kChipLambdaEvent;
    event.LambdaEvent = std::move(bridge);

    return PlatformMgr().PostEvent(&event);
}

CHIP_ERROR PlatformEventing::StartTimer(System::Layer & aLayer, System::Clock::Timeout delay)
{
    return PlatformMgr().StartChipTimer(delay);
}

} // namespace System
} // namespace chip
