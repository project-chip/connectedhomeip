/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Darwin platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformDarwinEvent = kRange_InternalPlatformSpecific,
    kPlatformDarwinBleC1WriteEvent,
    kPlatformDarwinBleOutOfBuffersEvent,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Darwin platforms.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        struct
        {
            uint16_t ConnId;
            ::chip::System::PacketBuffer * Data;
        } BleC1WriteEvent;
    };
};

} // namespace DeviceLayer
} // namespace chip
