/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Beken platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {
namespace DeviceEventType {

/**
 * Enumerates platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    kBKSystemEvent = kRange_PublicPlatformSpecific,
};

/**
 * Enumerates platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kQorvoBLEConnected = kRange_InternalPlatformSpecific,
    kQorvoBLEDisconnected,
    kCHIPoBLECCCWriteEvent,
    kCHIPoBLERXCharWriteEvent,
    kCHIPoBLETXCharWriteEvent,
    kRtkWiFiStationConnectedEvent,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information.
 */
struct ChipDevicePlatformEvent final
{
    // TODO - add platform specific definition extension
    union
    {
        struct
        {
            uint8_t dummy;
        } QorvoBLEConnected;
        struct
        {
            uint8_t dummy;
        } CHIPoBLECCCWriteEvent;
        struct
        {
            uint8_t dummy;
        } CHIPoBLERXCharWriteEvent;
        struct
        {
            uint8_t dummy;
        } CHIPoBLETXCharWriteEvent;
        struct
        {
            union
            {
                uint16_t WiFiStaDisconnected;
            } Data;
        } BKSystemEvent;
    };
};

} // namespace DeviceLayer
} // namespace chip
