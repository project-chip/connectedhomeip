/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Defines platform-specific event types and data for the
 *          CHIP Device Layer on the ESP32.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

#include <esp_event.h>
#include <esp_netif_types.h>
#include <esp_wifi_types.h>

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates platform-specific event types that are visible to the application.
 */
enum
{
    kESPSystemEvent = kRange_PublicPlatformSpecific,
};

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
/**
 * Enumerates ESP32 platform-specific event types that are internal to the Chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformESP32Event = kRange_InternalPlatformSpecific,
    kPlatformESP32BLECentralConnected,
    kPlatformESP32BLECentralConnectFailed,
    kPlatformESP32BLEWriteComplete,
    kPlatformESP32BLESubscribeOpComplete,
    kPlatformESP32BLEIndicationReceived,
};

#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER
} // namespace DeviceEventType

/**
 * Represents platform-specific event information for the ESP32 platform.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        struct
        {
            esp_event_base_t Base;
            int32_t Id;
            union
            {
                ip_event_t EthGotIp;
                ip_event_got_ip_t IpGotIp;
                ip_event_got_ip6_t IpGotIp6;
                ip_event_ap_staipassigned_t IpApStaIpAssigned;
                wifi_event_sta_scan_done_t WiFiStaScanDone;
                wifi_event_sta_connected_t WiFiStaConnected;
                wifi_event_sta_disconnected_t WiFiStaDisconnected;
                wifi_event_sta_authmode_change_t WiFiStaAuthModeChange;
                wifi_event_sta_wps_er_pin_t WiFiStaWpsErPin;
                wifi_event_sta_wps_fail_reason_t WiFiStaWpsErFailed;
                wifi_event_ap_staconnected_t WiFiApStaConnected;
                wifi_event_ap_stadisconnected_t WiFiApStaDisconnected;
                wifi_event_ap_probe_req_rx_t WiFiApProbeReqRecved;
            } Data;
        } ESPSystemEvent;
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
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
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    };
};

} // namespace DeviceLayer
} // namespace chip
