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

#ifndef _WFX_RSI_H_
#define _WFX_RSI_H_

#include "event_groups.h"

/*
 * Interface to RSI Sapis
 */

#define WFX_RSI_WLAN_TASK_SZ (1024 + 512 + 256) /* Stack for the WLAN task	 	*/
#define WFX_RSI_TASK_SZ (1024 + 1024)           /* Stack for the WFX/RSI task		*/
#define WFX_RSI_BUF_SZ (1024 * 10)              /* May need tweak 			*/
#define WFX_RSI_CONFIG_MAX_JOIN (5)             /* Max join retries			*/

/*
 * Various events fielded by the wfx_rsi task
 * Make sure that we only use 8 bits (otherwise freeRTOS - may need some changes)
 */
typedef enum
{
    WFX_EVT_STA_CONN       = (1 << 0),
    WFX_EVT_STA_DISCONN    = (1 << 1),
    WFX_EVT_AP_START       = (1 << 2),
    WFX_EVT_AP_STOP        = (1 << 3),
    WFX_EVT_SCAN           = (1 << 4), /* This is used as scan result and start */
    WFX_EVT_STA_START_JOIN = (1 << 5),
    WFX_EVT_STA_DO_DHCP    = (1 << 6),
    WFX_EVT_STA_DHCP_DONE  = (1 << 7)
} WfxEventType_e;

typedef enum
{
    WFX_RSI_ST_DEV_READY       = (1 << 0),
    WFX_RSI_ST_AP_READY        = (1 << 1),
    WFX_RSI_ST_STA_PROVISIONED = (1 << 2),
    WFX_RSI_ST_STA_CONNECTING  = (1 << 3),
    WFX_RSI_ST_STA_CONNECTED   = (1 << 4),
    WFX_RSI_ST_STA_DHCP_DONE   = (1 << 6), /* Requested to do DHCP after conn	*/
    WFX_RSI_ST_STA_MODE        = (1 << 7), /* Enable Station Mode			*/
    WFX_RSI_ST_AP_MODE         = (1 << 8), /* Enable AP Mode			*/
    WFX_RSI_ST_STA_READY       = (WFX_RSI_ST_STA_CONNECTED | WFX_RSI_ST_STA_DHCP_DONE),
    WFX_RSI_ST_STARTED         = (1 << 9),  /* RSI task started			*/
    WFX_RSI_ST_SCANSTARTED     = (1 << 10), /* Scan Started				*/
    WFX_RSI_ST_SLEEP_READY     = (1 << 11)  /* Notify the M4 to go to sleep*/
} WfxStateType_e;

typedef struct wfx_rsi_s
{
    // TODO: Change tp WfxEventType_e once the event queue is implemented
    EventGroupHandle_t events;
    TaskHandle_t drv_task;
    TaskHandle_t wlan_task;
    TaskHandle_t init_task;
    TaskHandle_t ble_task;
    uint16_t dev_state;
    uint16_t ap_chan; /* The chan our STA is using	*/
    wfx_wifi_provision_t sec;
#ifdef SL_WFX_CONFIG_SCAN
    void (*scan_cb)(wfx_wifi_scan_result_t *);
    char * scan_ssid; /* Which one are we scanning for */
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
#ifdef __cplusplus
extern "C" {
#endif
void wfx_rsidev_init(void);
void wfx_rsi_task(void * arg);
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_rsi_reset_count();
int32_t wfx_rsi_disconnect();
int32_t wfx_wifi_rsi_init(void);
#if SL_ICD_ENABLED
void sl_wfx_host_si91x_sleep_wakeup();
int32_t wfx_rsi_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state);
#endif /* SL_ICD_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* _WFX_RSI_H_ */
