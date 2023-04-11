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

#include "stdbool.h"
#include "wfx_msgs.h"

/* LwIP includes. */
#include "lwip/apps/httpd.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"

#include "sl_status.h"

/* Wi-Fi events*/
#define SL_WFX_STARTUP_IND_ID (1)
#define SL_WFX_CONNECT_IND_ID (2)
#define SL_WFX_DISCONNECT_IND_ID (3)
#define SL_WFX_SCAN_COMPLETE_ID (4)
#define WFX_RSI_SSID_SIZE (64)

/* Wi-Fi bitmask events - for the task */
#define SL_WFX_CONNECT (1 << 1)
#define SL_WFX_DISCONNECT (1 << 2)
#define SL_WFX_START_AP (1 << 3)
#define SL_WFX_STOP_AP (1 << 4)
#define SL_WFX_SCAN_START (1 << 5)
#define SL_WFX_SCAN_COMPLETE (1 << 6)
#define SL_WFX_RETRY_CONNECT (1 << 7)

#define WLAN_TASK_STACK_SIZE (1024)
#define WLAN_TASK_PRIORITY (3)
#define WLAN_DRIVER_TASK_PRIORITY (2)
#define MAX_JOIN_RETRIES_COUNT (5)

// WLAN retry time intervals in milli seconds
#define WLAN_MAX_RETRY_TIMER_MS 30000
#define WLAN_MIN_RETRY_TIMER_MS 1000
#define WLAN_RETRY_TIMER_MS 5000
#define CONVERT_MS_TO_SEC(TimeInMS) (TimeInMS / 1000)

// WLAN related Macros
#define ETH_FRAME (0)
#define CMP_SUCCESS (0)
#define BSSID_MAX_STR_LEN (6)
#define MAC_ADDRESS_FIRST_OCTET (6)
#define AP_START_SUCCESS (0)
#define BITS_TO_WAIT (0)
#define CONNECTION_STATUS_SUCCESS (1)
#define IP_STATUS_FAIL (0)
#define GET_IPV6_SUCCESS (1)
#define GET_IPV6_FAIL (0)
#define BEACON_1 (0)
#define CHANNEL_LIST ((const uint8_t *) 0)
#define CHANNEL_COUNT (0)
#define IE_DATA ((const uint8_t *) 0)
#define IE_DATA_LENGTH (0)
#define BSSID_SCAN ((const uint8_t *) 0)
#define CHANNEL_0 (0)
#define PREVENT_ROAMING (1)
#define DISABLE_PMF_MODE (0)
#define STA_IP_FAIL (0)
#define IP_STATUS_SUCCESS (1)
#define ACTIVE_CHANNEL_TIME_100 (100)
#define PASSIVE_CHANNEL_TIME_0 (0)
#define PROBE_NUM_REQ_1 (1)

#define PINOUT_CLEAR_STATUS (0)
#define TICKS_TO_WAIT_0 (0)
#define TICKS_TO_WAIT_3 (3)
#define TICKS_TO_WAIT_1 (1)
#define TICKS_TO_WAIT_500 (500)

// TASK and Interrupt Macros
#define SUCCESS_STATUS (1)
#define LINK_UP (1)
#define LINK_DOWN (0)
#define MAC_48_BIT_SET (1)
#define STRUCT_PBUF ((struct pbuf *) 0)
#define PRIORITY_0 (0)
#define HEX_VALUE_FF (0XFF)

// Timer Delay
#define MAX_XLEN (16)
#define MIN_XLEN (0)
#define PINOUT_CLEAR (0)
#define PINOUT_SET (1)
#define WFX_SPI_NVIC_PRIORITY (5)
#define WFX_GPIO_NVIC_PRIORITY (5)
#define CB_VALUE ((DMADRV_Callback_t) 0)

/* TIMER_TICKS_TO_WAIT Specifies the time, in ticks, that the calling task should
 * be held in the Blocked state to wait for the start command to be successfully
 * sent to the timer command queue.
 */
#define CONVERT_SEC_TO_MSEC (1000)
#define CONVERT_USEC_TO_MSEC (1 / 1000)

#define RSI_RESPONSE_MAX_SIZE (28)
#define RSI_RESPONSE_HOLD_BUFF_SIZE (128)
#define RSI_DRIVER_STATUS (0)
#define OPER_MODE_0 (0)
#define COEX_MODE_0 (0)
#define RESP_BUFF_SIZE (6)
#define AP_CHANNEL_NO_0 (0)
#define SCAN_BITMAP_OPTN_1 (1)
#define IP_CONF_RSP_BUFF_LENGTH_4 (4)
#define STATION (0)
#define BG_SCAN_RES_SIZE (500)

#define SPI_CONFIG_SUCCESS (0)

typedef enum
{
    WIFI_EVENT,
    IP_EVENT,
} wfx_event_base_t;

typedef enum
{
    IP_EVENT_STA_GOT_IP,
    IP_EVENT_GOT_IP6,
    IP_EVENT_STA_LOST_IP,
} ip_event_id_t;

/* Note that these are same as RSI_security */
typedef enum
{
    WFX_SEC_UNSPECIFIED = 0,
    WFX_SEC_NONE        = 1,
    WFX_SEC_WEP         = 2,
    WFX_SEC_WPA         = 3,
    WFX_SEC_WPA2        = 4,
    WFX_SEC_WPA3        = 5
} wfx_sec_t;

typedef struct
{
    char ssid[32 + 1];
    char passkey[64 + 1];
    wfx_sec_t security;
} wfx_wifi_provision_t;

typedef enum
{
    WIFI_MODE_NULL = 0,
    WIFI_MODE_STA,
    WIFI_MODE_AP,
    WIFI_MODE_APSTA,
    WIFI_MODE_MAX,
} wifi_mode_t;

typedef struct wfx_wifi_scan_result
{
    char ssid[32 + 1];
    wfx_sec_t security;
    uint8_t bssid[6];
    uint8_t chan;
    int16_t rssi; /* I suspect this is in dBm - so signed */
} wfx_wifi_scan_result_t;

typedef struct wfx_wifi_scan_ext
{
    uint32_t beacon_lost_count;
    uint32_t beacon_rx_count;
    uint32_t mcast_rx_count;
    uint32_t mcast_tx_count;
    uint32_t ucast_rx_count;
    uint32_t ucast_tx_count;
    uint32_t overrun_count;
} wfx_wifi_scan_ext_t;

typedef enum
{
    SL_WFX_STA_INTERFACE    = 0, ///< Interface 0, linked to the station
    SL_WFX_SOFTAP_INTERFACE = 1, ///< Interface 1, linked to the softap
} sl_wfx_interface_t;

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t wfx_wifi_start(void);
void wfx_enable_sta_mode(void);
void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t * addr);
void wfx_set_wifi_provision(wfx_wifi_provision_t * wifiConfig);
bool wfx_get_wifi_provision(wfx_wifi_provision_t * wifiConfig);
bool wfx_is_sta_mode_enabled(void);
int32_t wfx_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_reset_counts();
int32_t wfx_rsi_platform();

void wfx_clear_wifi_provision(void);
sl_status_t wfx_connect_to_ap(void);
void wfx_setup_ip6_link_local(sl_wfx_interface_t);
bool wfx_is_sta_connected(void);
sl_status_t wfx_sta_discon(void);
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
bool wfx_have_ipv4_addr(sl_wfx_interface_t);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
bool wfx_have_ipv6_addr(sl_wfx_interface_t);
wifi_mode_t wfx_get_wifi_mode(void);
bool wfx_start_scan(char * ssid, void (*scan_cb)(wfx_wifi_scan_result_t *)); /* true returned if successfully started */
void wfx_cancel_scan(void);

/*
 * Call backs into the Matter Platform code
 */
void wfx_started_notify(void);
void wfx_connected_notify(int32_t status, sl_wfx_mac_address_t * ap);
void wfx_disconnected_notify(int32_t status);
/* Implemented for LWIP */
void wfx_host_received_sta_frame_cb(uint8_t * buf, int len);
void wfx_lwip_set_sta_link_up(void);
void wfx_lwip_set_sta_link_down(void);
void wfx_lwip_start(void);
struct netif * wfx_get_netif(sl_wfx_interface_t interface);
bool wfx_hw_ready(void);

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_dhcp_got_ipv4(uint32_t);
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

void wfx_ipv6_notify(int got_ip);

/* RSI for LWIP */
void * wfx_rsi_alloc_pkt(void);
void wfx_rsi_pkt_add_data(void * p, uint8_t * buf, uint16_t len, uint16_t off);
int32_t wfx_rsi_send_data(void * p, uint16_t len);

void wfx_retry_interval_handler(bool is_wifi_disconnection_event, uint16_t retryJoin);

#ifdef __cplusplus
}
#endif
