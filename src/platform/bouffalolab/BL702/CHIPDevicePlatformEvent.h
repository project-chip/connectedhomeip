/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

#pragma once

#include <bluetooth/bluetooth.h>
#include <conn_internal.h>
#include <platform/CHIPDeviceEvent.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates BL702 platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates BL702 platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformZephyrEvent = chip::DeviceLayer::DeviceEventType::kRange_InternalPlatformSpecific,
    kPlatformZephyrBleConnected,
    kPlatformZephyrBleDisconnected,
    kPlatformZephyrBleCCCWrite,
    kPlatformZephyrBleRXWrite,
    kPlatformZephyrBleTXComplete,
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

struct BleRXWriteEventType
{
    bt_conn * BtConn;
    ::chip::System::PacketBuffer * Data;
};

struct BleTXCompleteEventType
{
    bt_conn * BtConn;
};

/**
 * Represents platform-specific event information for the BL702 platform.
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
