/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Android platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Android platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates Android platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Android platforms.
 */
struct ChipDevicePlatformEvent
{
};

} // namespace DeviceLayer
} // namespace chip
