/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on webOS platforms.
 */

#pragma once

#include <lib/core/CHIPError.h>
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
    kPlatformLinuxEvent = kRange_InternalPlatformSpecific,
    kPlatformLinuxBLEAdapterAdded,
    kPlatformLinuxBLEAdapterRemoved,
    kPlatformLinuxBLECentralConnected,
    kPlatformLinuxBLECentralConnectFailed,
    kPlatformLinuxBLEWriteComplete,
    kPlatformLinuxBLESubscribeOpComplete,
    kPlatformLinuxBLEIndicationReceived,
    kPlatformLinuxBLEC1WriteEvent,
    kPlatformLinuxBLEOutOfBuffersEvent,
    kPlatformLinuxBLEPeripheralRegisterAppComplete,
    kPlatformLinuxBLEPeripheralAdvStartComplete,
    kPlatformLinuxBLEPeripheralAdvStopComplete,
    kPlatformLinuxBLEPeripheralAdvReleased,
};

enum InternalPlatformSpecificWbsEventTypes
{
    kPlatformWebOSEvent = kRange_InternalPlatformSpecific,
    kPlatformWebOSBLEAdapterAdded,
    kPlatformWebOSBLEAdapterRemoved,
    kPlatformWebOSBLECentralConnected,
    kPlatformWebOSBLECentralConnectFailed,
    kPlatformWebOSBLEWriteComplete,
    kPlatformWebOSBLESubscribeOpComplete,
    kPlatformWebOSBLEIndicationReceived,
    kPlatformWebOSBLEC1WriteEvent,
    kPlatformWebOSBLEOutOfBuffersEvent,
    kPlatformWebOSBLEPeripheralRegisterAppComplete,
    kPlatformWebOSBLEPeripheralAdvStartComplete,
    kPlatformWebOSBLEPeripheralAdvStopComplete,
    kPlatformWebOSBLEPeripheralAdvReleased,
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
            unsigned int mAdapterId;
            char mAdapterAddress[18];
        } BLEAdapter;
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
            CHIP_ERROR mError;
        } BLEPeripheralRegisterAppComplete;
        struct
        {
            CHIP_ERROR mError;
        } BLEPeripheralAdvStartComplete;
        struct
        {
            CHIP_ERROR mError;
        } BLEPeripheralAdvStopComplete;
    };
};

} // namespace DeviceLayer
} // namespace chip
