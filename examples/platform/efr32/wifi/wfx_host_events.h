/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * Copyright 2019, Silicon Laboratories Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the
 *"License"); you may not use this file except in
 *compliance with the License. You may obtain a copy of
 *the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in
 *writing, software distributed under the License is
 *distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing
 *permissions and limitations under the License.
 *****************************************************************************/
#pragma once

#ifdef WF200_WIFI
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "sl_wfx_cmd_api.h"
#include "sl_wfx_constants.h"
#else /* RS911x */
#include "wfx_msgs.h"

/* Wi-Fi events*/
#define SL_WFX_STARTUP_IND_ID 1
#define SL_WFX_CONNECT_IND_ID 2
#define SL_WFX_DISCONNECT_IND_ID 3
#define SL_WFX_SCAN_COMPLETE_ID 4
#define WFX_RSI_SSID_SIZE	64

#endif /* WF200 */

#ifndef RS911X_SOCKETS
/* LwIP includes. */
#include "lwip/apps/httpd.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"

/* Wi-Fi bitmask events - for the task */
#define SL_WFX_CONNECT (1 << 1)
#define SL_WFX_DISCONNECT (1 << 2)
#define SL_WFX_START_AP (1 << 3)
#define SL_WFX_STOP_AP (1 << 4)
#define SL_WFX_SCAN_COMPLETE (1 << 5)

#endif /* RS911X_SOCKETS */

#include "sl_status.h"

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
typedef enum {
      WFX_SEC_NONE = 0,
      WFX_SEC_WPA = 1,
      WFX_SEC_WPA2 = 2,
      WFX_SEC_WEP = 3,
      WFX_SEC_WPA_EAP = 4,
      WFX_SEC_WPA2_EAP = 5,
      WFX_SEC_WPA_WPA2_MIXED = 6,
      WFX_SEC_WPA_PMK = 7,
      WFX_SEC_WPA2_PMK = 8,
      WFX_SEC_WPS_PIN = 9,
      WFX_SEC_GEN_WPS_PIN = 10,
      WFX_SEC_PUSH_BTN = 11,
      WFX_SEC_WPA3 = 11,
} wfx_sec_t;
typedef struct
{
    char ssid [64 + 1];
    char passkey [64+1];
    uint8_t security;
} wfx_wifi_provision_t;

typedef enum
{
    WIFI_MODE_NULL = 0,
    WIFI_MODE_STA,
    WIFI_MODE_AP,
    WIFI_MODE_APSTA,
    WIFI_MODE_MAX,
} wifi_mode_t;
#ifdef RS911X_WIFI
/*
 * This Sh%t is here to support WFXUtils - and the Matter stuff that uses it
 * We took it from the SDK (for WF200)
 */
typedef enum {
      SL_WFX_NOT_INIT = 0,
      SL_WFX_STARTED   = 1,
      SL_WFX_STA_INTERFACE_CONNECTED = 2,
      SL_WFX_AP_INTERFACE_UP         = 3,
      SL_WFX_SLEEPING                = 4,
      SL_WFX_POWER_SAVE_ACTIVE       = 5,
} sl_wfx_state_t;

typedef enum {
  SL_WFX_STA_INTERFACE    = 0,   ///< Interface 0, linked to the station
  SL_WFX_SOFTAP_INTERFACE = 1,   ///< Interface 1, linked to the softap
} sl_wfx_interface_t;

#endif /* RS911X_WIFI */
#ifdef __cplusplus
extern "C" {
#endif

void sl_wfx_host_gpio_init(void);
sl_status_t wfx_wifi_start(void);
void wfx_enable_sta_mode (void);
sl_wfx_state_t wfx_get_wifi_state(void);
void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t *addr);
void wfx_set_wifi_provision(wfx_wifi_provision_t *wifiConfig);
bool wfx_is_sta_provisioned (void);
bool wfx_is_sta_mode_enabled (void);

void wfx_clear_wifi_provision(void);
sl_status_t wfx_connect_to_ap(void);
void wfx_setup_ip6_link_local (sl_wfx_interface_t);
bool wfx_is_sta_connected (void);
sl_status_t wfx_sta_discon (void);
bool wfx_have_ipv4_addr (sl_wfx_interface_t);
wifi_mode_t wfx_get_wifi_mode (void);

  /*
   * Call backs into the Matter Platform code
   */
void wfx_started_notify (void);
void wfx_connected_notify (int32_t status, sl_wfx_mac_address_t *ap);
void wfx_disconnected_notify (int32_t status);
    /* Implemented for LWIP */
void wfx_host_received_sta_frame_cb (uint8_t *buf, int len);
void wfx_lwip_set_sta_link_up(void);
void wfx_lwip_set_sta_link_down(void);
void wfx_lwip_start (void);
struct netif *wfx_get_netif (sl_wfx_interface_t interface);
void wfx_dhcp_got_ipv4 (uint32_t);
bool wfx_hw_ready (void);
void wfx_ip_changed_notify (int got_ip);

#ifdef RS911X_WIFI
/* RSI for LWIP */
void *wfx_rsi_alloc_pkt (void);
void wfx_rsi_pkt_add_data (void *p, uint8_t *buf, uint16_t len, uint16_t off);
int32_t wfx_rsi_send_data (void *p, uint16_t len);
#endif /* RS911X_WIFI */

#ifdef WF200_WIFI
void wfx_bus_start(void);
void sl_wfx_host_gpio_init(void);
sl_status_t sl_wfx_host_process_event(sl_wfx_generic_message_t * event_payload);
#endif
#ifdef __cplusplus
}
#endif
