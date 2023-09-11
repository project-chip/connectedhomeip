/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on mbed platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace System {
class PacketBuffer;
} // namespace System
} // namespace chip

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Zephyr platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates mbed platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{

};

} // namespace DeviceEventType

struct BleConnEventType
{
};

struct BleCCCWriteEventType
{
};

struct BleC1WriteEventType
{
};

struct BleC2IndDoneEventType
{
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
        BleC1WriteEventType BleC1WriteEvent;
        BleC2IndDoneEventType BleC2IndDoneEvent;
    };
};

} // namespace DeviceLayer
} // namespace chip
