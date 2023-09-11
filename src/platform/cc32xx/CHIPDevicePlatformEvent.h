/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          for the Texas Instruments CC32XX platform.
 *
 * NOTE: currently a bare-bones implementation to allow for building.
 */

#ifndef CHIP_DEVICE_PLATFORM_EVENT_H
#define CHIP_DEVICE_PLATFORM_EVENT_H

#include <platform/CHIPDeviceEvent.h>

// XXX: Seth, what device events do we need

namespace chip {
namespace DeviceLayer {

/**
 * Represents platform-specific event information
 */
struct ChipDevicePlatformEvent final
{
};

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_PLATFORM_EVENT_H
