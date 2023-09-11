/*
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on webOS platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates webOS platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates webOS platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformWebOSEvent = kRange_InternalPlatformSpecific,
    kPlatformWebOSBLECentralConnected,
    kPlatformWebOSBLECentralConnectFailed,
    kPlatformWebOSBLEWriteComplete,
    kPlatformWebOSBLESubscribeOpComplete,
    kPlatformWebOSBLEIndicationReceived,
    kPlatformWebOSBLEC1WriteEvent,
    kPlatformWebOSBLEOutOfBuffersEvent,
    kPlatformWebOSBLEPeripheralRegisterAppComplete,
    kPlatformWebOSBLEPeripheralAdvConfiguredComplete,
    kPlatformWebOSBLEPeripheralAdvStartComplete,
    kPlatformWebOSBLEPeripheralAdvStopComplete
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for webOS platforms.
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
