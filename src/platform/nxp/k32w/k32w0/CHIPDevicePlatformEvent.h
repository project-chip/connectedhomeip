/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on K32W061 platforms using the NXP SDK.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates K32W061 platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates K32W061 platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    /* None currently defined */
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for NXP K32W061 platforms.
 */

struct ChipDevicePlatformEvent final
{
};

} // namespace DeviceLayer
} // namespace chip
