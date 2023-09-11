/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on Tizen platforms.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Tizen platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates Tizen platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformTizenEvent = kRange_InternalPlatformSpecific,
    kPlatformTizenBLEPeripheralGATTServerRegisterComplete,
    kPlatformTizenBLEPeripheralAdvConfiguredComplete,
    kPlatformTizenBLEPeripheralAdvStartComplete,
    kPlatformTizenBLEPeripheralAdvStopComplete,
    kPlatformTizenBLECentralConnected,
    kPlatformTizenBLECentralConnectFailed,
    kPlatformTizenBLEWriteComplete,
    kPlatformTizenBLESubscribeOpComplete,
    kPlatformTizenBLEIndicationReceived
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Tizen platforms.
 */
struct ChipDevicePlatformEvent
{
    union
    {
        struct
        {
            bool mIsSuccess;
            void * mpAppstate;
        } BLEPeripheralGATTServerRegisterComplete;
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
    };
};

} // namespace DeviceLayer
} // namespace chip
