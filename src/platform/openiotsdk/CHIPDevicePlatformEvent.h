/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on the Open IOT SDK platform.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Open IoT SDK platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates Open IoT SDK platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{

};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Open IoT SDK platform.
 */
struct ChipDevicePlatformEvent final
{
};

} // namespace DeviceLayer
} // namespace chip
