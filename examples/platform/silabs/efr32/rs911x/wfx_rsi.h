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
/*
 * Interface to RSI Sapis
 */

#define WFX_RSI_WLAN_TASK_SZ (1024 + 512 + 256) /* Unknown how big this should be 	*/
#define WFX_RSI_TASK_SZ (1024 + 1024)           /* Stack for the WFX/RSI task		*/
#define WFX_RSI_BUF_SZ (1024 * 10)              /* May need tweak 			*/
#define WFX_RSI_CONFIG_MAX_JOIN 5               /* Max join retries			*/
#define WFX_RSI_NUM_TIMERS 2                    /* Number of RSI timers to alloc	*/

/*
 * Various events fielded by the wfx_rsi task
 * Make sure that we only use 8 bits (otherwise freeRTOS - may need some changes)
 */
#define WFX_EVT_STA_CONN 0x01
#define WFX_EVT_STA_DISCONN 0x02
#define WFX_EVT_AP_START 0x04
#define WFX_EVT_AP_STOP 0x08
#define WFX_EVT_SCAN 0x10 /* This is used as scan result and start */
#define WFX_EVT_STA_START_JOIN 0x20
#define WFX_EVT_STA_DO_DHCP 0x40
#define WFX_EVT_STA_DHCP_DONE 0x80

#define WFX_RSI_ST_DEV_READY 0x01
#define WFX_RSI_ST_AP_READY 0x02
#define WFX_RSI_ST_STA_PROVISIONED 0x04
#define WFX_RSI_ST_STA_CONNECTING 0x08
#define WFX_RSI_ST_STA_CONNECTED 0x10
#define WFX_RSI_ST_STA_DHCP_DONE 0x40 /* Requested to do DHCP after conn	*/
#define WFX_RSI_ST_STA_MODE 0x80      /* Enable Station Mode			*/
#define WFX_RSI_ST_AP_MODE 0x100      /* Enable AP Mode			*/
#define WFX_RSI_ST_STA_READY (WFX_RSI_ST_STA_CONNECTED | WFX_RSI_ST_STA_DHCP_DONE)
#define WFX_RSI_ST_STARTED 0x200     /* RSI task started			*/
#define WFX_RSI_ST_SCANSTARTED 0x400 /* Scan Started				*/

struct wfx_rsi
{
    EventGroupHandle_t events;
    TaskHandle_t drv_task;
    TaskHandle_t wlan_task;
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
};
#define RSI_SCAN_RESP_SZ 54

extern struct wfx_rsi wfx_rsi;
#ifdef __cplusplus
extern "C" {
#endif
void wfx_rsidev_init(void);
void wfx_rsi_task(void * arg);
void efr32Log(const char * aFormat, ...);
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_rsi_reset_count();
int32_t wfx_rsi_disconnect();
#define WFX_RSI_LOG(...) efr32Log(__VA_ARGS__);

#ifdef __cplusplus
}
#endif

#endif /* _WFX_RSI_H_ */
