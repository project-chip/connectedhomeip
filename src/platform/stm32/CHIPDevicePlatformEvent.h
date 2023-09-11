/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <platform/stm32/CHIPPlatformConfig.h>

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
    kSTMBLEConnected = kRange_InternalPlatformSpecific,
    kSTMBLEDisconnected,
    kCHIPoBLECCCWriteEvent,
    kCHIPoBLERXCharWriteEvent,
    kCHIPoBLETXCharWriteEvent,
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
        } STMBLEConnected;
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
    };
};

} // namespace DeviceLayer
} // namespace chip
