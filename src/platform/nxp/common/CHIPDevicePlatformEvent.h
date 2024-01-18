/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
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
 *          Device Layer on NXP platforms using the NXP SDK.
 */

#pragma once
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <sys/atomic.h>
#include <toolchain.h>
#include <zephyr/bluetooth/bluetooth.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */
#include "wlan.h"
#if defined(__cplusplus)
}
#endif
#endif

#include <platform/CHIPDeviceEvent.h>

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
    kPlatformZephyrEvent = kRange_InternalPlatformSpecific,
    kPlatformZephyrBleConnected,
    kPlatformZephyrBleDisconnected,
    kPlatformZephyrBleCCCWrite,
    kPlatformZephyrBleC1WriteEvent,
    kPlatformZephyrBleC2IndDoneEvent,
    kPlatformZephyrBleOutOfBuffersEvent,
    kPlatformNxpWlanEvent,
    kPlatformNxpIpChangeEvent,
    kPlatformNxpStartWlanConnectEvent,
};

} // namespace DeviceEventType

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
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

#endif

/**
 * Represents platform-specific event information for Zephyr platforms.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
        BleConnEventType BleConnEvent;
        BleCCCWriteEventType BleCCCWriteEvent;
        BleC1WriteEventType BleC1WriteEvent;
        BleC2IndDoneEventType BleC2IndDoneEvent;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
        enum wlan_event_reason WlanEventReason;
        struct wlan_network * pNetworkDataEvent;
#endif
    };
};

} // namespace DeviceLayer
} // namespace chip
