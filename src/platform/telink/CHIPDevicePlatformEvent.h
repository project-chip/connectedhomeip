/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides platform-specific event types and data for the chip
 *          Device Layer on Telink platform, by including Zephyr
 *          platform definitions.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

#include <zephyr/bluetooth/bluetooth.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Telink platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates Telink platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformTelinkEvent = kRange_InternalPlatformSpecific,
    kPlatformTelinkBleConnected,
    kPlatformTelinkBleDisconnected,
    kPlatformTelinkBleDisconnectRequest,
    kPlatformTelinkBleCCCWrite,
    kPlatformTelinkBleRXWrite,
    kPlatformTelinkBleTXComplete,
    kPlatformTelinkBleOutOfBuffersEvent,
};

} // namespace DeviceEventType

struct BleConnEventType
{
    uint16_t connHandle;
    uint8_t HciResult;
};

struct BleCCCWriteEventType
{
    uint16_t connHandle;
    uint16_t Value;
};

struct BleRXWriteEventType
{
    uint16_t connHandle;
    ::chip::System::PacketBuffer * Data;
};

struct BleTXCompleteEventType
{
    uint16_t connHandle;
};

/**
 * Represents platform-specific event information for Zephyr platforms.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        BleConnEventType BleConnEvent;
        BleCCCWriteEventType BleCCCWriteEvent;
        BleRXWriteEventType BleRXWriteEvent;
        BleTXCompleteEventType BleTXCompleteEvent;
    };
};

} // namespace DeviceLayer
} // namespace chip
