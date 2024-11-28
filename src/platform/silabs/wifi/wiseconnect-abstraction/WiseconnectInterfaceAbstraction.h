/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <app/icd/server/ICDServerConfig.h>
#include <cmsis_os2.h>
#include <lib/support/BitFlags.h>
#include <platform/silabs/wifi/WifiInterfaceAbstraction.h>
#include <sl_cmsis_os2_common.h>

#define WFX_RSI_DHCP_POLL_INTERVAL (250) /* Poll interval in ms for DHCP */

enum class WifiPlatformEvent : uint8_t
{
    kStationConnect    = 1,
    kStationDisconnect = 2,
    kAPStart           = 3,
    kAPStop            = 4,
    kScan              = 5, /* This is used as scan result and start */
    kStationStartJoin  = 6,
    kStationDoDhcp     = 7,
    kStationDhcpDone   = 8,
    kStationDhcpPoll   = 9,
};

void sl_matter_wifi_task(void * arg);

int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_rsi_reset_count();
int32_t sl_wifi_platform_disconnect();

sl_status_t sl_matter_wifi_platform_init(void);

/**
 * @brief Posts an event to the Wi-Fi task
 *
 * @param[in] event Event to process.
 */
void sl_matter_wifi_post_event(WifiPlatformEvent event);
