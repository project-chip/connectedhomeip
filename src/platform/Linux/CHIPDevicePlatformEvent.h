/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Linux platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Linux platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates Linux platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformLinuxEvent = kRange_InternalPlatformSpecific,
    kPlatformLinuxBLECentralConnected,
    kPlatformLinuxBLECentralConnectFailed,
    kPlatformLinuxBLEWriteComplete,
    kPlatformLinuxBLESubscribeOpComplete,
    kPlatformLinuxBLEIndicationReceived,
    kPlatformLinuxBLEC1WriteEvent,
    kPlatformLinuxBLEOutOfBuffersEvent,
    kPlatformLinuxBLEPeripheralRegisterAppComplete,
    kPlatformLinuxBLEPeripheralAdvConfiguredComplete,
    kPlatformLinuxBLEPeripheralAdvStartComplete,
    kPlatformLinuxBLEPeripheralAdvStopComplete
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Linux platforms.
 */
struct ChipDevicePlatformEvent
{
    union
    {
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
        } BLECentralConnected;
        struct
        {
            CHIP_ERROR mError;
        } BLECentralConnectFailed;
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
        } BLEWriteComplete;
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
            bool mIsSubscribed;
        } BLESubscribeOpComplete;
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
            chip::System::PacketBuffer * mData;
        } BLEIndicationReceived;
        struct
        {
            bool mIsSuccess;
            void * mpAppstate;
        } BLEPeripheralRegisterAppComplete;
        struct
        {
            bool mIsSuccess;
            void * mpAppstate;
        } BLEPeripheralAdvConfiguredComplete;
        struct
        {
            bool mIsSuccess;
            void * mpAppstate;
        } BLEPeripheralAdvStartComplete;
        struct
        {
            bool mIsSuccess;
            void * mpAppstate;
        } BLEPeripheralAdvStopComplete;
    };
};

} // namespace DeviceLayer
} // namespace chip
