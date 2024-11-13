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
#include <platform/silabs/wifi/wfx_msgs.h>
#include <sl_cmsis_os2_common.h>

#include "sl_status.h"
#include <stdbool.h>

/* LwIP includes. */
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"

#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#include "rsi_common_apis.h"
#include "sl_si91x_types.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_device.h"

/*
 * Interface to RSI Sapis
 */

#define WFX_RSI_DHCP_POLL_INTERVAL (250) /* Poll interval in ms for DHCP		*/

// MAX SSID LENGTH excluding NULL character
#define WFX_MAX_SSID_LENGTH (32)
// MAX PASSKEY LENGTH including NULL character
#define WFX_MAX_PASSKEY_LENGTH (SL_WIFI_MAX_PSK_LENGTH)
#define SL_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME_MS (1000)
#else
// MAX SSID LENGTH excluding NULL character
#define WFX_MAX_SSID_LENGTH (32)
// MAX PASSKEY LENGTH including NULL character
#define WFX_MAX_PASSKEY_LENGTH (64)
#endif // (SLI_SI91X_MCU_INTERFACE  | EXP_BOARD)

#define MAX_JOIN_RETRIES_COUNT (5)
#define BSSID_LEN (6)
#define MAC_ADDRESS_FIRST_OCTET (6)
#define CONNECTION_STATUS_SUCCESS (1)
#define IP_STATUS_FAIL (0)
#define GET_IPV6_FAIL (0)
#define IP_STATUS_SUCCESS (1)

#define SL_WFX_STARTUP_IND_ID (1)
#define SL_WFX_CONNECT_IND_ID (2)
#define SL_WFX_DISCONNECT_IND_ID (3)
#define SL_WFX_SCAN_COMPLETE_ID (4)

// TASK and Interrupt Macros
#define SUCCESS_STATUS (1)

enum class WifiState : uint16_t
{
    kStationInit        = (1 << 0),
    kAPReady            = (1 << 1),
    kStationProvisioned = (1 << 2),
    kStationConnecting  = (1 << 3),
    kStationConnected   = (1 << 4),
    kStationDhcpDone    = (1 << 6), /* Requested to do DHCP after conn */
    kStationMode        = (1 << 7), /* Enable Station Mode */
    kAPMode             = (1 << 8), /* Enable AP Mode */
    kStationReady       = (kStationConnected | kStationDhcpDone),
    kStationStarted     = (1 << 9),  /* RSI task started */
    kScanStarted        = (1 << 10), /* Scan Started */
};

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

typedef enum
{
    WIFI_EVENT,
    IP_EVENT,
} wfx_event_base_t;

/* Note that these are same as RSI_security */
typedef enum
{
    WFX_SEC_UNSPECIFIED    = 0,
    WFX_SEC_NONE           = 1,
    WFX_SEC_WEP            = 2,
    WFX_SEC_WPA            = 3,
    WFX_SEC_WPA2           = 4,
    WFX_SEC_WPA3           = 5,
    WFX_SEC_WPA_WPA2_MIXED = 6,
} wfx_sec_t;

typedef struct
{
    char ssid[WFX_MAX_SSID_LENGTH + 1];
    size_t ssid_length;
    char passkey[WFX_MAX_PASSKEY_LENGTH + 1];
    size_t passkey_length;
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
    char ssid[WFX_MAX_SSID_LENGTH + 1];
    size_t ssid_length;
    wfx_sec_t security;
    uint8_t bssid[BSSID_LEN];
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
    IP_EVENT_STA_GOT_IP,
    IP_EVENT_GOT_IP6,
    IP_EVENT_STA_LOST_IP,
} ip_event_id_t;

#ifdef RS911X_WIFI
/*
 * This Sh%t is here to support WFXUtils - and the Matter stuff that uses it
 * We took it from the SDK (for WF200)
 */
typedef enum
{
    SL_WFX_STA_INTERFACE    = 0, ///< Interface 0, linked to the station
    SL_WFX_SOFTAP_INTERFACE = 1, ///< Interface 1, linked to the softap
} sl_wfx_interface_t;
#endif

typedef struct wfx_rsi_s
{
    chip::BitFlags<WifiState> dev_state;
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

// TODO: We shouldn't need to have access to a global variable in the interface here
extern WfxRsi_t wfx_rsi;

sl_status_t wfx_wifi_start(void);
void wfx_enable_sta_mode(void);
void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t * addr);
void wfx_set_wifi_provision(wfx_wifi_provision_t * wifiConfig);
bool wfx_get_wifi_provision(wfx_wifi_provision_t * wifiConfig);
bool wfx_is_sta_mode_enabled(void);
int32_t wfx_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_reset_counts();
void wfx_clear_wifi_provision(void);
sl_status_t wfx_connect_to_ap(void);
void wfx_setup_ip6_link_local(sl_wfx_interface_t);
bool wfx_is_sta_connected(void);
sl_status_t sl_matter_wifi_disconnect(void);

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
void sl_matter_wifi_task_started(void);
void wfx_connected_notify(int32_t status, sl_wfx_mac_address_t * ap);
void wfx_disconnected_notify(int32_t status);

/* Implemented for LWIP */
void wfx_lwip_set_sta_link_up(void);
void wfx_lwip_set_sta_link_down(void);
void sl_matter_lwip_start(void);
struct netif * wfx_get_netif(sl_wfx_interface_t interface);

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_dhcp_got_ipv4(uint32_t);
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
void wfx_ipv6_notify(int got_ip);
void wfx_retry_connection(uint16_t retryAttempt);

#ifdef RS911X_WIFI
#if !(EXP_BOARD) // for RS9116
void * wfx_rsi_alloc_pkt(void);
/* RSI for LWIP */
void wfx_rsi_pkt_add_data(void * p, uint8_t * buf, uint16_t len, uint16_t off);
int32_t wfx_rsi_send_data(void * p, uint16_t len);
#endif //!(EXP_BOARD)
#endif // RS911X_WIFI

bool wfx_hw_ready(void);

#ifdef RS911X_WIFI // for RS9116, 917 NCP and 917 SoC
/* RSI Power Save */
#if SL_ICD_ENABLED
#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
sl_status_t wfx_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state);
#else
sl_status_t wfx_power_save();
#endif /* (SLI_SI91X_MCU_INTERFACE | EXP_BOARD) */
#endif /* SL_ICD_ENABLED */
#endif /* RS911X_WIFI */

void sl_matter_wifi_task(void * arg);

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void wfx_ip_changed_notify(int got_ip);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap);
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info);
int32_t wfx_rsi_reset_count();
int32_t sl_wifi_platform_disconnect();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if SLI_SI917
int32_t wfx_rsi_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state);
#else
int32_t wfx_rsi_power_save();
#endif /* SLI_SI917 */
#endif /* SL_ICD_ENABLED */

/**
 * @brief Posts an event to the Wi-Fi task
 *
 * @param[in] event Event to process.
 */
void sl_matter_wifi_post_event(WifiEvent event);

#ifdef __cplusplus
extern "C" {
#endif

#if (SLI_SI91X_MCU_INTERFACE)
#if SL_ICD_ENABLED
// TODO : This should be moved outside of the Wifi interface functions
void sl_button_on_change(uint8_t btn, uint8_t btnAction);
#endif /* SL_ICD_ENABLED */
#endif /* SLI_SI91X_MCU_INTERFACE */

#ifdef WF200_WIFI
void sl_wfx_host_gpio_init(void);
void wfx_bus_start(void);
sl_status_t sl_wfx_host_process_event(sl_wfx_generic_message_t * event_payload);
#endif /* WF200_WIFI */

#ifdef __cplusplus
}
#endif
