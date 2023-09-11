/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the
 *          CHIP Device Layer on the PSoC6.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates platform-specific event types that are visible to the application.
 */
enum
{
    kP6SystemEvent = kRange_PublicPlatformSpecific,
};

/**
 * Enumerates internal platform-specific event types.
 */
enum InternalPlatformSpecificEventTypes
{
    // TODO: maybe need remove this and handle BLEEnabledEvt direct
    //      from BLEManagerImpl::BLEManagerCallback.
    kP6BLEEnabledEvt = kRange_InternalPlatformSpecific,
    kP6BLEDisabledEvt
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for the PSoC6 platform.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        struct
        {
            int32_t event_type;
        } P6SystemEvent;
    };
};

} // namespace DeviceLayer
} // namespace chip
