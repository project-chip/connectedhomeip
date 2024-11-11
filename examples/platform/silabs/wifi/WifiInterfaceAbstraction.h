/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <app/icd/server/ICDServerConfig.h>
#include <cmsis_os2.h>
#include <lib/support/BitFlags.h>
#include <sl_cmsis_os2_common.h>
#include <wfx_host_events.h>

/*
 * Interface to RSI Sapis
 */

#define WFX_RSI_DHCP_POLL_INTERVAL (250) /* Poll interval in ms for DHCP		*/

enum class WifiEvent : uint8_t
{
    kStationConnect    = 0,
    kStationDisconnect = 1,
    kAPStart           = 2,
    kAPStop            = 3,
    kScan              = 4, /* This is used as scan result and start */
    kStationStartJoin  = 5,
    kStationDoDhcp     = 6,
    kStationDhcpDone   = 7,
    kStationDhcpPoll   = 8
};

enum class WifiState : uint16_t
{
    kStationInit        = (1 << 0),
    kAPReady            = (1 << 1),
    kStationProvisioned = (1 << 2),
    kStationConnecting  = (1 << 3),
    kStationConnected   = (1 << 4),
    kStationDhcpDone    = (1 << 6), /* Requested to do DHCP after conn	*/
    kStationMode        = (1 << 7), /* Enable Station Mode			*/
    kAPMode             = (1 << 8), /* Enable AP Mode			*/
    kStationReady       = (kStationConnected | kStationDhcpDone),
    kStationStarted     = (1 << 9),  /* RSI task started			*/
    kScanStarted        = (1 << 10), /* Scan Started					*/
};
using WifiStateFlags = chip::BitFlags<WifiState>;

typedef struct wfx_rsi_s
{
    WifiStateFlags dev_state;
    uint16_t ap_chan; /* The chan our STA is using	*/
    wfx_wifi_provision_t sec;
#ifdef SL_WFX_CONFIG_SCAN
    void (*scan_cb)(wfx_wifi_scan_result_t *);
    char * scan_ssid; /* Which one are we scanning for */
    size_t scan_ssid_length;
#endif
#ifdef SL_WFX_CONFIG_SOFTAP
    sl_wfx_mac_address_t softap_mac;
#endif
    sl_wfx_mac_address_t sta_mac;
    sl_wfx_mac_address_t ap_mac;   /* To which our STA is connected */
    sl_wfx_mac_address_t ap_bssid; /* To which our STA is connected */
    uint16_t join_retries;
    uint8_t ip4_addr[4]; /* Not sure if this is enough */
} WfxRsi_t;

extern WfxRsi_t wfx_rsi;

void sl_matter_wifi_task(void * arg);

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_rsi_reset_count();
int32_t sl_wifi_platform_disconnect();

// TODO : this needs to be extern otherwise we get a linking error. We need to figure out why in the header clean up
// NCP files are including this while being c files
#ifdef __cplusplus
extern "C" {
#endif
sl_status_t sl_matter_wifi_platform_init(void);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if SLI_SI917
int32_t wfx_rsi_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state);
#else
int32_t wfx_rsi_power_save();
#endif /* SLI_SI917 */
#endif /* SL_ICD_ENABLED */
#ifdef __cplusplus
}
#endif

/**
 * @brief Posts an event to the Wi-Fi task
 *
 * @param[in] event Event to process.
 */
void sl_matter_wifi_post_event(WifiEvent event);
