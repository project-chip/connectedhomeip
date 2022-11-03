/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
    };
};

} // namespace DeviceLayer
} // namespace chip
