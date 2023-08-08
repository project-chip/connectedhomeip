/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sl_status.h"
#include "silabs_utils.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "sl_board_configuration.h"
#include "sl_net.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_types.h"
#include "sl_wifi_callback_framework.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_types.h"
#include "sl_wlan_config.h"
#include "task.h"
#include "wfx_host_events.h"

#include "ble_config.h"

#include "dhcp_client.h"
#include "lwip/nd6.h"
#include "sl_wifi.h"
#include "wfx_host_events.h"
#include "wfx_rsi.h"
#define ADV_SCAN_THRESHOLD           -40
#define ADV_RSSI_TOLERANCE_THRESHOLD 5
#define ADV_ACTIVE_SCAN_DURATION     15
#define ADV_PASSIVE_SCAN_DURATION    20
#define ADV_MULTIPROBE               1
#define ADV_SCAN_PERIODICITY         10

struct wfx_rsi wfx_rsi;

/* Declare a variable to hold the data associated with the created event group. */
StaticEventGroup_t rsiDriverEventGroup;

bool hasNotifiedIPV6 = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
bool hasNotifiedIPV4 = false;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
bool hasNotifiedWifiConnectivity = false;

/* Declare a flag to differentiate between after boot-up first IP connection or reconnection */
bool is_wifi_disconnection_event = false;

/* Declare a variable to hold connection time intervals */
uint32_t retryInterval              = WLAN_MIN_RETRY_TIMER_MS;
volatile bool scan_results_complete = false;
#define WIFI_SCAN_TIMEOUT 10000

extern osSemaphoreId_t sl_rs_ble_init_sem;

/*
 * This file implements the interface to the wifi sdk
 */

wfx_wifi_scan_ext_t * temp_reset;

volatile sl_status_t callback_status = SL_STATUS_OK;

/******************************************************************
 * @fn   int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t *ap)
 * @brief
 *       Getting the AP details
 * @param[in] ap: access point
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap)
{
    sl_status_t status            = SL_STATUS_OK;
    uint8_t rssi = 0;
    ap->security = wfx_rsi.sec.security;
    ap->chan     = wfx_rsi.ap_chan;
    memcpy(&ap->bssid[0], &wfx_rsi.ap_mac.octet[0], BSSID_MAX_STR_LEN);
    sl_wifi_get_signal_strength(SL_WIFI_CLIENT_INTERFACE, &rssi);
    if(status == SL_STATUS_OK)
    {
        ap->rssi =  rssi;
    }
    return status;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t *extra_info)
 * @brief
 *       Getting the AP extra details
 * @param[in] extra info: access point extra information
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info)
{
    sl_status_t status        = SL_STATUS_OK;
    sl_wifi_statistics_t test = { 0 };
    status                    = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VERIFY_STATUS_AND_RETURN(status);
    extra_info->beacon_lost_count = test.beacon_lost_count - temp_reset->beacon_lost_count;
    extra_info->beacon_rx_count   = test.beacon_rx_count - temp_reset->beacon_rx_count;
    extra_info->mcast_rx_count    = test.mcast_rx_count - temp_reset->mcast_rx_count;
    extra_info->mcast_tx_count    = test.mcast_tx_count - temp_reset->mcast_tx_count;
    extra_info->ucast_rx_count    = test.ucast_rx_count - temp_reset->ucast_rx_count;
    extra_info->ucast_tx_count    = test.ucast_tx_count - temp_reset->ucast_tx_count;
    extra_info->overrun_count     = test.overrun_count - temp_reset->overrun_count;
    return status;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_reset_count()
 * @brief
 *       Getting the driver reset count
 * @param[in] None
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_reset_count()
{
    sl_wifi_statistics_t test = { 0 };
    sl_status_t status        = SL_STATUS_OK;
    status                    = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VERIFY_STATUS_AND_RETURN(status);
    temp_reset->beacon_lost_count = test.beacon_lost_count;
    temp_reset->beacon_rx_count   = test.beacon_rx_count;
    temp_reset->mcast_rx_count    = test.mcast_rx_count;
    temp_reset->mcast_tx_count    = test.mcast_tx_count;
    temp_reset->ucast_rx_count    = test.ucast_rx_count;
    temp_reset->ucast_tx_count    = test.ucast_tx_count;
    temp_reset->overrun_count     = test.overrun_count;
    return status;
}

/******************************************************************
 * @fn   wfx_rsi_disconnect()
 * @brief
 *       Getting the driver disconnect status
 * @param[in] None
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_disconnect()
{
    return sl_wifi_disconnect(SL_WIFI_CLIENT_INTERFACE);
}

sl_status_t join_callback_handler(sl_wifi_event_t event, char * result, uint32_t result_length, void * arg)
{
    temp_reset = (wfx_wifi_scan_ext_t *) malloc(sizeof(wfx_wifi_scan_ext_t));
    memset(temp_reset, 0, sizeof(wfx_wifi_scan_ext_t));
    if (CHECK_IF_EVENT_FAILED(event))
    {
        SILABS_LOG("F: Join Event received with %u bytes payload\n", result_length);
        callback_status = *(sl_status_t *) result;
        wfx_rsi.join_retries += 1;
        wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED);
        wfx_retry_interval_handler(is_wifi_disconnection_event, wfx_rsi.join_retries++);
        if (is_wifi_disconnection_event || wfx_rsi.join_retries <= WFX_RSI_CONFIG_MAX_JOIN)
        {
            xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_START_JOIN);
        }
        is_wifi_disconnection_event = true;
        return SL_STATUS_FAIL;
    }
    /*
     * Join was complete - Do the DHCP
     */
    SILABS_LOG("%s: join completed.", __func__);
    SILABS_LOG("%c: Join Event received with %u bytes payload\n", *result, result_length);
    xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_CONN);
    wfx_rsi.join_retries = 0;
    retryInterval        = WLAN_MIN_RETRY_TIMER_MS;
    if (is_wifi_disconnection_event)
        is_wifi_disconnection_event = false;
    callback_status = SL_STATUS_OK;
    return SL_STATUS_OK;
}

#if CHIP_DEVICE_CONFIG_ENABLE_SED
/******************************************************************
 * @fn   wfx_rsi_power_save()
 * @brief
 *       Setting the RS911x in DTIM sleep based mode
 *
 * @param[in] None
 * @return
 *        None
 *********************************************************************/
int32_t wfx_rsi_power_save()
{
    int32_t status;
    status = rsi_bt_power_save_profile(2, 0);
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("BT Powersave Config Failed, Error Code : 0x%lX", status);
        return status;
    }

    sl_wifi_performance_profile_t wifi_profile = { ASSOCIATED_POWER_SAVE };
    status = sl_wifi_set_performance_profile(&wifi_profile);
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("Powersave Config Failed, Error Code : 0x%lX", status);
        return status;
    }
    SILABS_LOG("Powersave Config Success");
    return status;
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_SED */

/*************************************************************************************
 * @fn  static int32_t wfx_wifi_rsi_init(void)
 * @brief
 *      Wifi initialization called from app main
 * @param[in]  None
 * @return
 *        None
 *****************************************************************************************/
int32_t wfx_wifi_rsi_init(void)
{
  SILABS_LOG("wfx_wifi_rsi_init #1 ");
  sl_status_t status;
  status = sl_wifi_init(&config  , default_wifi_event_handler);
  SILABS_LOG("wfx_wifi_rsi_init #2 ");
  if(status != SL_STATUS_OK){
    SILABS_LOG("wfx_wifi_rsi_init failed %x", status);
    return status;
  }
  return status;
}

/*************************************************************************************
 * @fn  static int32_t wfx_rsi_init(void)
 * @brief
 *      driver initialization
 * @param[in]  None
 * @return
 *        None
 *****************************************************************************************/
static int32_t wfx_rsi_init(void)
{
    sl_status_t status;
    status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, (sl_mac_address_t *) &wfx_rsi.sta_mac.octet[0]);
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("sl_wifi_get_mac_address failed: %x", status);
        return status;
    }

    wfx_rsi.events = xEventGroupCreateStatic(&rsiDriverEventGroup);
    wfx_rsi.dev_state |= WFX_RSI_ST_DEV_READY;
    osSemaphoreRelease(sl_rs_ble_init_sem);
    return status;
}

/*************************************************************************************
 * @fn  void wfx_show_err(char *msg)
 * @brief
 *      driver shows error message
 * @param[in]  msg
 * @return
 *        None
 *****************************************************************************************/
void wfx_show_err(char * msg)
{
    SILABS_LOG("%s: message: %d", __func__, msg);
}

sl_status_t scan_callback_handler(sl_wifi_event_t event, sl_wifi_scan_result_t * scan_result, uint32_t result_length, void * arg)
{
    if (CHECK_IF_EVENT_FAILED(event))
    {
        callback_status       = *(sl_status_t *) scan_result;
        scan_results_complete = true;
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        return SL_STATUS_FAIL;
    }
    wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
    wfx_rsi.ap_chan      = scan_result->scan_info[0].rf_channel;
    memcpy(&wfx_rsi.ap_mac.octet[0], scan_result->scan_info[0].bssid[0], BSSID_MAX_STR_LEN);
    switch (scan_result->scan_info[0].security_mode)
    {
    case SL_WIFI_OPEN:
        wfx_rsi.sec.security = WFX_SEC_NONE;
        break;
    case SL_WIFI_WPA:
    case SL_WIFI_WPA_ENTERPRISE:
        wfx_rsi.sec.security = WFX_SEC_WPA;
        break;
    case SL_WIFI_WPA2:
    case SL_WIFI_WPA2_ENTERPRISE:
        wfx_rsi.sec.security = WFX_SEC_WPA2;
        break;
    case SL_WIFI_WEP:
        wfx_rsi.sec.security = WFX_SEC_WEP;
        break;
    case SL_WIFI_WPA3_TRANSITION:
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case SL_WIFI_WPA3:
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        break;
    default:
        wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
        break;
    }
    scan_results_complete = true;
    return SL_STATUS_OK;
}
sl_status_t show_scan_results(sl_wifi_scan_result_t *scan_result)
{
  ARGS_CHECK_NULL_POINTER(scan_result);
  int x;
  wfx_wifi_scan_result_t ap;
  if (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED)
  {
    for (x = 0; x < scan_result->scan_count; x++)
    {
      strcpy(&ap.ssid[0], (char *) &scan_result->scan_info[x].ssid);
      uint8_t *bssid = (uint8_t *)&scan_result->scan_info[x].bssid;
      if (wfx_rsi.scan_ssid)
      {
        SILABS_LOG("SCAN SSID: %s , ap scan: %s", wfx_rsi.scan_ssid, ap.ssid);
        if (strcmp(wfx_rsi.scan_ssid, ap.ssid) == CMP_SUCCESS)
        {
          ap.security = scan_result->scan_info[x].security_mode;
          ap.rssi     = (-1) * scan_result->scan_info[x].rssi_val;
          memcpy(&ap.bssid[0], &scan_result->scan_info[x].bssid[0], BSSID_MAX_STR_LEN);
          (*wfx_rsi.scan_cb)(&ap);
        }
      }
      else
      {
        ap.security = scan_result->scan_info[x].security_mode;
        ap.rssi     = (-1) * scan_result->scan_info[x].rssi_val;
        memcpy(&ap.bssid[0], &scan_result->scan_info[x].bssid[0], BSSID_MAX_STR_LEN);
        (*wfx_rsi.scan_cb)(&ap);
      }
    }
  }
  wfx_rsi.dev_state &= ~WFX_RSI_ST_SCANSTARTED;
  (*wfx_rsi.scan_cb)((wfx_wifi_scan_result_t *) 0);
  wfx_rsi.scan_cb = (void (*)(wfx_wifi_scan_result_t *)) 0;
  if (wfx_rsi.scan_ssid)
  {
    vPortFree(wfx_rsi.scan_ssid);
    wfx_rsi.scan_ssid = (char *) 0;
  }
  return SL_STATUS_OK;
}
sl_status_t bg_scan_callback_handler(sl_wifi_event_t event,
                                  sl_wifi_scan_result_t *result,
                                  uint32_t result_length,
                                  void *arg)
{
    callback_status = show_scan_results(result);
    scan_results_complete = true;
    return SL_STATUS_OK;
}
/***************************************************************************************
 * @fn   static void wfx_rsi_save_ap_info()
 * @brief
 *       Saving the details of the AP
 * @param[in]  None
 * @return
 *       None
 *******************************************************************************************/
static void wfx_rsi_save_ap_info() // translation
{
    sl_status_t status                                   = SL_STATUS_OK;
    sl_wifi_scan_configuration_t wifi_scan_configuration = { 0 };
    wifi_scan_configuration                              = default_wifi_scan_configuration;
    sl_wifi_ssid_t ssid_arg;
    ssid_arg.length = strlen(wfx_rsi.sec.ssid);
    memcpy(ssid_arg.value, (int8_t *) &wfx_rsi.sec.ssid[0], ssid_arg.length);
    sl_wifi_set_scan_callback(scan_callback_handler, NULL);
    status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &ssid_arg, &wifi_scan_configuration);
    if (SL_STATUS_IN_PROGRESS == status)
    {
        const uint32_t start = osKernelGetTickCount();
        while (!scan_results_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT)
        {
            osThreadYield();
        }
        status = scan_results_complete ? callback_status : SL_STATUS_TIMEOUT;
    }
    return status;
}

/********************************************************************************************
 * @fn   static void wfx_rsi_do_join(void)
 * @brief
 *        Start an async Join command
 * @return
 *        None
 **********************************************************************************************/
static void wfx_rsi_do_join(void)
{
    int32_t status;
    sl_wifi_security_t connect_security_mode;
    switch (wfx_rsi.sec.security)
    {
    case WFX_SEC_WEP:
        connect_security_mode = SL_WIFI_WEP;
        break;
    case WFX_SEC_WPA:
    case WFX_SEC_WPA2:
        connect_security_mode = SL_WIFI_WPA_WPA2_MIXED;
        break;
    case WFX_SEC_WPA3:
        connect_security_mode = SL_WIFI_WPA3;
        break;
    case WFX_SEC_NONE:
        connect_security_mode = SL_WIFI_OPEN;
        break;
    default:
        SILABS_LOG("%s: error: unknown security type.");
        return;
    }

    if (wfx_rsi.dev_state & (WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED))
    {
        SILABS_LOG("%s: not joining - already in progress", __func__);
    }
    else
    {
        SILABS_LOG("%s: WLAN: connecting to %s==%s, sec=%d", __func__, &wfx_rsi.sec.ssid[0], &wfx_rsi.sec.passkey[0],
                   wfx_rsi.sec.security);

        /*
         * Join the network
         */
        /* TODO - make the WFX_SECURITY_xxx - same as RSI_xxx
         * Right now it's done by hand - we need something better
         */
        wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTING;

        sl_wifi_set_join_callback(join_callback_handler, NULL);

        /* Try to connect Wifi with given Credentials
         * untill there is a success or maximum number of tries allowed
         */

        /* Call rsi connect call with given ssid and password
         * And check there is a success
         */
        sl_wifi_credential_t cred = { 0 };
        cred.type                 = SL_WIFI_CRED_PSK;
        memcpy(cred.psk.value, &wfx_rsi.sec.passkey[0], strlen(wfx_rsi.sec.passkey));
        sl_wifi_credential_id_t id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID;
        status = sl_net_set_credential(id, SL_NET_WIFI_PSK, &wfx_rsi.sec.passkey[0], strlen(wfx_rsi.sec.passkey));
        if (SL_STATUS_OK != status)
        {
            return status;
        }

        sl_wifi_client_configuration_t ap = { 0 };
        uint32_t timeout_ms               = 0;

        ap.ssid.length = strlen(wfx_rsi.sec.ssid);
        memcpy(ap.ssid.value, (int8_t *) &wfx_rsi.sec.ssid[0], ap.ssid.length);
        ap.security      = connect_security_mode;
        ap.encryption    = SL_WIFI_NO_ENCRYPTION;
        ap.credential_id = id;
        if ((status = sl_wifi_connect(SL_WIFI_CLIENT_INTERFACE, &ap, timeout_ms)) == SL_STATUS_IN_PROGRESS)
        {
            callback_status = SL_STATUS_IN_PROGRESS;
            while (callback_status == SL_STATUS_IN_PROGRESS)
            {
                osThreadYield();
            }
            status = callback_status;
        }
        else
        {
            if (is_wifi_disconnection_event || wfx_rsi.join_retries <= WFX_RSI_CONFIG_MAX_JOIN)
            {
                SILABS_LOG("%s: failed. retry: %d", __func__, wfx_rsi.join_retries);
                SILABS_LOG("%s: starting JOIN to %s after %d tries\n", __func__, (char *) &wfx_rsi.sec.ssid[0],
                           wfx_rsi.join_retries);
                wfx_rsi.join_retries += 1;
                wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED);
                if (wfx_rsi.join_retries <= MAX_JOIN_RETRIES_COUNT)
                {
                    xEventGroupSetBits(wfx_rsi.events, WFX_EVT_STA_START_JOIN);
                }
                wfx_retry_interval_handler(is_wifi_disconnection_event, wfx_rsi.join_retries);
            }
        }
    }
}

/*********************************************************************************
 * @fn  void wfx_rsi_task(void *arg)
 * @brief
 * The main WLAN task - started by wfx_wifi_start () that interfaces with RSI.
 * The rest of RSI stuff come in call-backs.
 * The initialization has been already done.
 * @param[in] arg:
 * @return
 *       None
 **********************************************************************************/
/* ARGSUSED */
void wfx_rsi_task(void * arg)
{
    EventBits_t flags;
    TickType_t last_dhcp_poll, now;
    struct netif * sta_netif;
    (void) arg;
    uint32_t rsi_status = wfx_rsi_init();
    if (rsi_status != RSI_SUCCESS)
    {
        SILABS_LOG("%s: error: wfx_rsi_init with status: %02x", __func__, rsi_status);
        return;
    }
    wfx_lwip_start();
    last_dhcp_poll = xTaskGetTickCount();
    sta_netif      = wfx_get_netif(SL_WFX_STA_INTERFACE);
    wfx_started_notify();

    SILABS_LOG("%s: starting event wait", __func__);
    for (;;)
    {
        /*
         * This is the main job of this task.
         * Wait for commands from the ConnectivityManager
         * Make state changes (based on call backs)
         */
        flags = xEventGroupWaitBits(wfx_rsi.events,
                                    WFX_EVT_STA_CONN | WFX_EVT_STA_DISCONN | WFX_EVT_STA_START_JOIN
#ifdef SL_WFX_CONFIG_SOFTAP
                                        | WFX_EVT_AP_START | WFX_EVT_AP_STOP
#endif /* SL_WFX_CONFIG_SOFTAP */
#ifdef SL_WFX_CONFIG_SCAN
                                        | WFX_EVT_SCAN
#endif /* SL_WFX_CONFIG_SCAN */
                                        | 0,
                                    pdTRUE,              /* Clear the bits */
                                    pdFALSE,             /* Wait for any bit */
                                    pdMS_TO_TICKS(250)); /* 250 mSec */

        if (flags)
        {
            SILABS_LOG("%s: wait event encountered: %x", __func__, flags);
        }
        /*
         * Let's handle DHCP polling here
         */
        if (wfx_rsi.dev_state & WFX_RSI_ST_STA_CONNECTED)
        {
            if ((now = xTaskGetTickCount()) > (last_dhcp_poll + pdMS_TO_TICKS(250)))
            {
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
                uint8_t dhcp_state = dhcpclient_poll(sta_netif);
                if (dhcp_state == DHCP_ADDRESS_ASSIGNED && !hasNotifiedIPV4)
                {
                    wfx_dhcp_got_ipv4((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
                    hasNotifiedIPV4 = true;
                    if (!hasNotifiedWifiConnectivity)
                    {
                        wfx_connected_notify(CONNECTION_STATUS_SUCCESS, &wfx_rsi.ap_mac);
                        hasNotifiedWifiConnectivity = true;
                    }
                }
                else if (dhcp_state == DHCP_OFF)
                {
                    wfx_ip_changed_notify(IP_STATUS_FAIL);
                    hasNotifiedIPV4 = false;
                }
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
                /*
                 * Checks if the IPv6 event has been notified, if not invoke the nd6_tmr,
                 * which starts the duplicate address detectation.
                 */
                if (!hasNotifiedIPV6)
                {
                    nd6_tmr();
                }
                /* Checks if the assigned IPv6 address is preferred by evaluating
                 * the first block of IPv6 address ( block 0)
                 */
                if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !hasNotifiedIPV6)
                {
                    wfx_ipv6_notify(GET_IPV6_SUCCESS);
                    hasNotifiedIPV6 = true;
                    if (!hasNotifiedWifiConnectivity)
                    {
                        wfx_connected_notify(CONNECTION_STATUS_SUCCESS, &wfx_rsi.ap_mac);
                        hasNotifiedWifiConnectivity = true;
                    }
                }
                last_dhcp_poll = now;
            }
        }
        if (flags & WFX_EVT_STA_START_JOIN)
        {
            // saving the AP related info
            wfx_rsi_save_ap_info();
            // Joining to the network
            wfx_rsi_do_join();
        }
        if (flags & WFX_EVT_STA_CONN)
        {
            SILABS_LOG("%s: starting LwIP STA", __func__);
            wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTED;
            hasNotifiedWifiConnectivity = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
            hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
            hasNotifiedIPV6 = false;
            wfx_lwip_set_sta_link_up();
            /* We need to get AP Mac - TODO */
            // Uncomment once the hook into MATTER is moved to IP connectivty instead
            // of AP connectivity. wfx_connected_notify(0, &wfx_rsi.ap_mac); // This
            // is independant of IP connectivity.
        }
        if (flags & WFX_EVT_STA_DISCONN)
        {
            wfx_rsi.dev_state &=
                ~(WFX_RSI_ST_STA_READY | WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED | WFX_RSI_ST_STA_DHCP_DONE);
            SILABS_LOG("%s: disconnect notify", __func__);
            /* TODO: Implement disconnect notify */
            wfx_lwip_set_sta_link_down(); // Internally dhcpclient_poll(netif) ->
                                          // wfx_ip_changed_notify(0) for IPV4
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
            wfx_ip_changed_notify(IP_STATUS_FAIL);
            hasNotifiedIPV4 = false;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
            wfx_ipv6_notify(GET_IPV6_FAIL);
            hasNotifiedIPV6             = false;
            hasNotifiedWifiConnectivity = false;
        }
#ifdef SL_WFX_CONFIG_SCAN
        if (flags & WFX_EVT_SCAN)
        {
            if (!(wfx_rsi.dev_state & WFX_RSI_ST_SCANSTARTED))
            {
                SILABS_LOG("%s: start SSID scan", __func__);
                int x;
                sl_wifi_scan_configuration_t wifi_scan_configuration              = { 0 };
                wfx_wifi_scan_result_t ap;
                // TODO: Add scan logic
                sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = { 0 };
                int32_t status;
                advanced_scan_configuration.active_channel_time  = ADV_ACTIVE_SCAN_DURATION;
                advanced_scan_configuration.passive_channel_time = ADV_PASSIVE_SCAN_DURATION;
                advanced_scan_configuration.trigger_level        = ADV_SCAN_THRESHOLD;
                advanced_scan_configuration.trigger_level_change = ADV_RSSI_TOLERANCE_THRESHOLD;
                advanced_scan_configuration.enable_multi_probe   = ADV_MULTIPROBE;
                status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);
                VERIFY_STATUS_AND_RETURN(status);
                /* Terminate with end of scan which is no ap sent back */
                wifi_scan_configuration.type                   = SL_WIFI_SCAN_TYPE_ADV_SCAN;
                wifi_scan_configuration.periodic_scan_interval = ADV_SCAN_PERIODICITY;
                sl_wifi_set_scan_callback(bg_scan_callback_handler, NULL);
                status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, NULL, &wifi_scan_configuration);
                if (SL_STATUS_IN_PROGRESS == status) {
                    printf("Scanning...\r\n");
                    const uint32_t start = osKernelGetTickCount();
                    while (!scan_results_complete && (osKernelGetTickCount() - start) <= WIFI_SCAN_TIMEOUT) {
                       osThreadYield();
                    }
                    status = scan_results_complete ? callback_status : SL_STATUS_TIMEOUT;
                }
            }
        }
#endif /* SL_WFX_CONFIG_SCAN */
#ifdef SL_WFX_CONFIG_SOFTAP
        /* TODO */
        if (flags & WFX_EVT_AP_START)
        {
        }
        if (flags & WFX_EVT_AP_STOP)
        {
        }
#endif /* SL_WFX_CONFIG_SOFTAP */
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/********************************************************************************************
 * @fn   void wfx_dhcp_got_ipv4(uint32_t ip)
 * @brief
 *        Acquire the new ip address
 * @param[in] ip: internet protocol
 * @return
 *        None
 **********************************************************************************************/
void wfx_dhcp_got_ipv4(uint32_t ip)
{
    /*
     * Acquire the new IP address
     */
    wfx_rsi.ip4_addr[0] = (ip) &HEX_VALUE_FF;
    wfx_rsi.ip4_addr[1] = (ip >> 8) & HEX_VALUE_FF;
    wfx_rsi.ip4_addr[2] = (ip >> 16) & HEX_VALUE_FF;
    wfx_rsi.ip4_addr[3] = (ip >> 24) & HEX_VALUE_FF;
    SILABS_LOG("%s: DHCP OK: IP=%d.%d.%d.%d", __func__, wfx_rsi.ip4_addr[0], wfx_rsi.ip4_addr[1], wfx_rsi.ip4_addr[2],
               wfx_rsi.ip4_addr[3]);
    /* Notify the Connectivity Manager - via the app */
    wfx_rsi.dev_state |= WFX_RSI_ST_STA_DHCP_DONE;
    wfx_ip_changed_notify(IP_STATUS_SUCCESS);
    wfx_rsi.dev_state |= WFX_RSI_ST_STA_READY;
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/*
 * WARNING - Taken from RSI and broken up
 * This is my own RSI stuff for not copying code and allocating an extra
 * level of indirection - when using LWIP buffers
 * see also: int32_t rsi_wlan_send_data_xx(uint8_t *buffer, uint32_t length)
 */
/********************************************************************************************
 * @fn   void *wfx_rsi_alloc_pkt()
 * @brief
 *       Allocate packet to send data
 * @param[in] None
 * @return
 *        None
 **********************************************************************************************/
void * wfx_rsi_alloc_pkt(uint16_t data_length)
{
    sl_wifi_buffer_t * buffer;
    sl_si91x_packet_t * packet;
    sl_status_t status = SL_STATUS_OK;

    /* Confirm if packet is allocated */

    status = sl_si91x_allocate_command_buffer(&buffer, (void **) &packet, sizeof(sl_si91x_packet_t) + data_length,
                                              SL_WIFI_ALLOCATE_COMMAND_BUFFER_WAIT_TIME);
    //    VERIFY_STATUS_AND_RETURN(status);
    if (packet == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }
    return (void *) packet;
}

/********************************************************************************************
 * @fn   void wfx_rsi_pkt_add_data(void *p, uint8_t *buf, uint16_t len, uint16_t off)
 * @brief
 *       add the data into packet
 * @param[in]  p:
 * @param[in]  buf:
 * @param[in]  len:
 * @param[in]  off:
 * @return
 *        None
 **********************************************************************************************/
void wfx_rsi_pkt_add_data(void * p, uint8_t * buf, uint16_t len, uint16_t off)
{
    sl_si91x_packet_t * pkt;
    pkt = (sl_si91x_packet_t *) p;
    memcpy(((char *) pkt->data) + off, buf, len);
}

/********************************************************************************************
 * @fn   int32_t wfx_rsi_send_data(void *p, uint16_t len)
 * @brief
 *       Driver send a data
 * @param[in]  p:
 * @param[in]  len:
 * @return
 *        None
 **********************************************************************************************/
int32_t wfx_rsi_send_data(void * p, uint16_t len)
{
    int32_t status;
    sl_wifi_buffer_t * buffer;
    buffer = (sl_wifi_buffer_t *) p;

    if (sl_si91x_driver_send_data_packet(SI91X_WLAN_CMD_QUEUE, buffer, RSI_SEND_RAW_DATA_RESPONSE_WAIT_TIME))
    {
        SILABS_LOG("*ERR*EN-RSI:Send fail");
        return ERR_IF;
    }
    return status;
}
