/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Zephyr platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

#include <zephyr/bluetooth/bluetooth.h>

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
 * Enumerates Zephyr platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformZephyrEvent = kRange_InternalPlatformSpecific,
    kPlatformZephyrBleConnected,
    kPlatformZephyrBleDisconnected,
    kPlatformZephyrBleCCCWrite,
    kPlatformZephyrBleC1WriteEvent,
    kPlatformZephyrBleC2IndDoneEvent,
    kPlatformZephyrBleOutOfBuffersEvent,
};

} // namespace DeviceEventType

struct BleConnEventType
{
    bt_conn * BtConn;
    uint8_t HciResult;
};

struct BleCCCWriteEventType
{
    bt_conn * BtConn;
    uint16_t Value;
};

struct BleC1WriteEventType
{
    bt_conn * BtConn;
    ::chip::System::PacketBuffer * Data;
};

struct BleC2IndDoneEventType
{
    bt_conn * BtConn;
    uint8_t Result;
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
