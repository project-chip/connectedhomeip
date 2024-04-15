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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"

#include "sl_status.h"
#include <cmsis_os2.h>

#include "silabs_utils.h"
#include "wfx_host_events.h"
#include "rsi_driver.h"
#include "rsi_wlan_non_rom.h"
#include "rsi_bootup_config.h"
#include "rsi_common_apis.h"
#include "rsi_data_types.h"
#include "rsi_error.h"
#include "rsi_nwk.h"
#include "rsi_socket.h"
#include "rsi_utils.h"
#include "rsi_wlan.h"
#include "rsi_wlan_apis.h"
#include "rsi_wlan_config.h"

#include "dhcp_client.h"
#include "lwip/nd6.h"
#include "wfx_rsi.h"

// TODO convert this file to cpp and use CodeUtils.h
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define WFX_QUEUE_SIZE 10

/* Rsi driver Task will use as its stack */
StackType_t driverRsiTaskStack[WFX_RSI_WLAN_TASK_SZ] = { 0 };

/* Structure that will hold the TCB of the wfxRsi Task being created. */
StaticTask_t driverRsiTaskBuffer;

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
uint32_t retryInterval = WLAN_MIN_RETRY_TIMER_MS;

#if (RSI_BLE_ENABLE)
extern rsi_semaphore_handle_t sl_rs_ble_init_sem;
#endif

// DHCP Poll timer
static osTimerId_t sDHCPTimer;
static osMessageQueueId_t sWifiEventQueue = NULL;
/*
 * This file implements the interface to the RSI SAPIs
 */
static uint8_t wfx_rsi_drv_buf[WFX_RSI_BUF_SZ];
static wfx_wifi_scan_ext_t temp_reset;

static void DHCPTimerEventHandler(void * arg)
{
    WfxEvent_t event;
    event.eventType = WFX_EVT_DHCP_POLL;
    WfxPostEvent(&event);
}

static void CancelDHCPTimer()
{
    osStatus_t status;

    // Check if timer started
    if (!osTimerIsRunning(sDHCPTimer))
    {
        SILABS_LOG("CancelDHCPTimer: timer not running");
        return;
    }

    status = osTimerStop(sDHCPTimer);
    if (status != osOK)
    {
        SILABS_LOG("CancelDHCPTimer: failed to stop timer with status: %d", status);
    }
}

static void StartDHCPTimer(uint32_t timeout)
{
    osStatus_t status;

    // Cancel timer if already started
    CancelDHCPTimer();

    status = osTimerStart(sDHCPTimer, pdMS_TO_TICKS(timeout));
    if (status != osOK)
    {
        SILABS_LOG("StartDHCPTimer: failed to start timer with status: %d", status);
    }
}

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
    int32_t status;
    uint8_t rssi;
    ap->security = wfx_rsi.sec.security;
    ap->chan     = wfx_rsi.ap_chan;
    memcpy(&ap->bssid[0], &wfx_rsi.ap_mac.octet[0], BSSID_MAX_STR_LEN);
    status = rsi_wlan_get(RSI_RSSI, &rssi, sizeof(rssi));
    if (status == RSI_SUCCESS)
    {
        ap->rssi = (-1) * rssi;
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
    int32_t status;
    uint8_t buff[RSI_RESPONSE_MAX_SIZE] = { 0 };
    status                              = rsi_wlan_get(RSI_WLAN_EXT_STATS, buff, sizeof(buff));
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("Failed, Error Code : 0x%lX", status);
    }
    else
    {
        rsi_wlan_ext_stats_t * test   = (rsi_wlan_ext_stats_t *) buff;
        extra_info->beacon_lost_count = test->beacon_lost_count - temp_reset.beacon_lost_count;
        extra_info->beacon_rx_count   = test->beacon_rx_count - temp_reset.beacon_rx_count;
        extra_info->mcast_rx_count    = test->mcast_rx_count - temp_reset.mcast_rx_count;
        extra_info->mcast_tx_count    = test->mcast_tx_count - temp_reset.mcast_tx_count;
        extra_info->ucast_rx_count    = test->ucast_rx_count - temp_reset.ucast_rx_count;
        extra_info->ucast_tx_count    = test->ucast_tx_count - temp_reset.ucast_tx_count;
        extra_info->overrun_count     = test->overrun_count - temp_reset.overrun_count;
    }
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
    int32_t status;
    uint8_t buff[RSI_RESPONSE_MAX_SIZE] = { 0 };
    status                              = rsi_wlan_get(RSI_WLAN_EXT_STATS, buff, sizeof(buff));
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("Failed, Error Code : 0x%lX", status);
    }
    else
    {
        rsi_wlan_ext_stats_t * test  = (rsi_wlan_ext_stats_t *) buff;
        temp_reset.beacon_lost_count = test->beacon_lost_count;
        temp_reset.beacon_rx_count   = test->beacon_rx_count;
        temp_reset.mcast_rx_count    = test->mcast_rx_count;
        temp_reset.mcast_tx_count    = test->mcast_tx_count;
        temp_reset.ucast_rx_count    = test->ucast_rx_count;
        temp_reset.ucast_tx_count    = test->ucast_tx_count;
        temp_reset.overrun_count     = test->overrun_count;
    }
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
    int32_t status = rsi_wlan_disconnect();
    return status;
}

#if SL_ICD_ENABLED
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
#ifdef RSI_BLE_ENABLE
    status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, RSI_MAX_PSP);
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("BT Powersave Config Failed, Error Code : 0x%lX", status);
        return status;
    }
#endif /* RSI_BLE_ENABLE */

    status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, RSI_MAX_PSP);
    if (status != RSI_SUCCESS)
    {
        SILABS_LOG("Powersave Config Failed, Error Code : 0x%lX", status);
        return status;
    }
    SILABS_LOG("Powersave Config Success");
    return status;
}
#endif /* SL_ICD_ENABLED */

/******************************************************************
 * @fn   wfx_rsi_join_cb(uint16_t status, const uint8_t *buf, const uint16_t len)
 * @brief
 *       called when driver join with cb
 * @param[in] status:
 * @param[in] buf:
 * @param[in] len:
 * @return
 *        None
 *********************************************************************/
static void wfx_rsi_join_cb(uint16_t status, const uint8_t * buf, const uint16_t len)
{
    WfxEvent_t WfxEvent;
    wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_CONNECTING;
    if (status != RSI_SUCCESS)
    {
        /*
         * We should enable retry.. (Need config variable for this)
         */
        SILABS_LOG("%s: failed. retry: %d", __func__, wfx_rsi.join_retries);
        wfx_retry_interval_handler(is_wifi_disconnection_event, wfx_rsi.join_retries++);
        if (is_wifi_disconnection_event || wfx_rsi.join_retries <= WFX_RSI_CONFIG_MAX_JOIN)
        {
            WfxEvent.eventType = WFX_EVT_STA_START_JOIN;
            WfxPostEvent(&WfxEvent);
        }
    }
    else
    {
        /*
         * Join was complete - Do the DHCP
         */
        memset(&temp_reset, 0, sizeof(wfx_wifi_scan_ext_t));
        SILABS_LOG("%s: join completed.", __func__);
        WfxEvent.eventType = WFX_EVT_STA_CONN;
        WfxPostEvent(&WfxEvent);
        wfx_rsi.join_retries = 0;
        retryInterval        = WLAN_MIN_RETRY_TIMER_MS;
    }
}

/******************************************************************
 * @fn  wfx_rsi_join_fail_cb(uint16_t status, uint8_t *buf, uint32_t len)
 * @brief
 *       called when driver fail to join with cb
 * @param[in] status:
 * @param[in] buf:
 * @param[in] len:
 * @return
 *        None
 *********************************************************************/
static void wfx_rsi_join_fail_cb(uint16_t status, uint8_t * buf, uint32_t len)
{
    SILABS_LOG("%s: error: failed status: %02x", __func__, status);
    WfxEvent_t WfxEvent;
    wfx_rsi.join_retries += 1;
    wfx_rsi.dev_state &= ~(WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED);
    is_wifi_disconnection_event = true;
    WfxEvent.eventType = WFX_EVT_STA_START_JOIN;
    WfxPostEvent(&WfxEvent);
}
/*************************************************************************************
 * @fn  wfx_rsi_wlan_pkt_cb(uint16_t status, uint8_t *buf, uint32_t len)
 * @brief
 *      Got RAW WLAN data pkt
 * @param[in]  status:
 * @param[in]  buf:
 * @param[in]  len:
 * @return
 *        None
 *****************************************************************************************/
static void wfx_rsi_wlan_pkt_cb(uint16_t status, uint8_t * buf, uint32_t len)
{
    if (status != RSI_SUCCESS)
    {
        return;
    }
    wfx_host_received_sta_frame_cb(buf, len);
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
    int32_t status;
    uint8_t buf[RSI_RESPONSE_HOLD_BUFF_SIZE];
    extern void rsi_hal_board_init(void);

    SILABS_LOG("%s: starting(HEAP_SZ = %d)", __func__, SL_HEAP_SIZE);
    //! Driver initialization
    status = rsi_driver_init(wfx_rsi_drv_buf, WFX_RSI_BUF_SZ);
    if ((status < RSI_DRIVER_STATUS) || (status > WFX_RSI_BUF_SZ))
    {
        SILABS_LOG("%s: error: RSI Driver initialization failed with status: %02x", __func__, status);
        return status;
    }

    SILABS_LOG("%s: rsi_device_init", __func__);
    /* ! Redpine module intialisation */
    if ((status = rsi_device_init(LOAD_NWP_FW)) != RSI_SUCCESS)
    {
        SILABS_LOG("%s: error: rsi_device_init failed with status: %02x", __func__, status);
        return status;
    }
    SILABS_LOG("%s: start wireless drv task", __func__);
    /*
     * Create the driver task
     */
    wfx_rsi.drv_task = xTaskCreateStatic((TaskFunction_t) rsi_wireless_driver_task, "rsi_drv", WFX_RSI_WLAN_TASK_SZ, NULL,
                                         WLAN_TASK_PRIORITY, driverRsiTaskStack, &driverRsiTaskBuffer);
    if (NULL == wfx_rsi.drv_task)
    {
        SILABS_LOG("%s: error: rsi_wireless_driver_task failed", __func__);
        return RSI_ERROR_INVALID_PARAM;
    }

#if (RSI_BLE_ENABLE)
    if ((status = rsi_wireless_init(OPER_MODE_0, RSI_OPERMODE_WLAN_BLE)) != RSI_SUCCESS)
    {
#else
    if ((status = rsi_wireless_init(OPER_MODE_0, COEX_MODE_0)) != RSI_SUCCESS)
    {
#endif
        SILABS_LOG("%s: error: Initialize WiSeConnect failed with status: %02x", __func__, status);
        return status;
    }

    SILABS_LOG("%s: get FW version..", __func__);
    /*
     * Get the MAC and other info to let the user know about it.
     */
    if (rsi_wlan_get(RSI_FW_VERSION, buf, sizeof(buf)) != RSI_SUCCESS)
    {
        SILABS_LOG("%s: error: rsi_wlan_get(RSI_FW_VERSION) failed with status: %02x", __func__, status);
        return status;
    }

    buf[sizeof(buf) - 1] = 0;
    SILABS_LOG("%s: RSI firmware version: %s", __func__, buf);
    //! Send feature frame
    if ((status = rsi_send_feature_frame()) != RSI_SUCCESS)
    {
        SILABS_LOG("%s: error: rsi_send_feature_frame failed with status: %02x", __func__, status);
        return status;
    }

    SILABS_LOG("%s: sent rsi_send_feature_frame", __func__);
    /* initializes wlan radio parameters and WLAN supplicant parameters.
     */
    (void) rsi_wlan_radio_init(); /* Required so we can get MAC address */
    if ((status = rsi_wlan_get(RSI_MAC_ADDRESS, &wfx_rsi.sta_mac.octet[0], RESP_BUFF_SIZE)) != RSI_SUCCESS)
    {
        SILABS_LOG("%s: error: rsi_wlan_get failed with status: %02x", __func__, status);
        return status;
    }

    SILABS_LOG("%s: WLAN: MAC %02x:%02x:%02x %02x:%02x:%02x", __func__, wfx_rsi.sta_mac.octet[0], wfx_rsi.sta_mac.octet[1],
               wfx_rsi.sta_mac.octet[2], wfx_rsi.sta_mac.octet[3], wfx_rsi.sta_mac.octet[4], wfx_rsi.sta_mac.octet[5]);
    
    // Create the message queue
    sWifiEventQueue = osMessageQueueNew(WFX_QUEUE_SIZE, sizeof(WfxEvent_t), NULL);
    if (sWifiEventQueue == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }

    // Create timer for DHCP polling
    // TODO: Use LWIP timer instead of creating a new one here
    sDHCPTimer = osTimerNew(DHCPTimerEventHandler, osTimerPeriodic, NULL, NULL);
    if (sDHCPTimer == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }

    /*
     * Register callbacks - We are only interested in the connectivity CBs
     */
    if ((status = rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, wfx_rsi_join_fail_cb)) != RSI_SUCCESS)
    {
        SILABS_LOG("%s: RSI callback register join failed with status: %02x", __func__, status);
        return status;
    }
    if ((status = rsi_wlan_register_callbacks(RSI_WLAN_DATA_RECEIVE_NOTIFY_CB, wfx_rsi_wlan_pkt_cb)) != RSI_SUCCESS)
    {
        SILABS_LOG("%s: RSI callback register data-notify failed with status: %02x", __func__, status);
        return status;
    }

#if (RSI_BLE_ENABLE)
    rsi_semaphore_post(&sl_rs_ble_init_sem);
#endif

    wfx_rsi.dev_state |= WFX_RSI_ST_DEV_READY;
    SILABS_LOG("%s: RSI: OK", __func__);
    return RSI_SUCCESS;
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
    int32_t status;
    rsi_rsp_scan_t rsp;

    status =
        rsi_wlan_scan_with_bitmap_options((int8_t *) &wfx_rsi.sec.ssid[0], AP_CHANNEL_NO_0, &rsp, sizeof(rsp), SCAN_BITMAP_OPTN_1);
    if (status)
    {
        /*
         * Scan is done - failed
         */
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else  /* !WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        SILABS_LOG("%s: warn: failed with status: %02x", __func__, status);
        return;
    }
    wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
    wfx_rsi.ap_chan      = rsp.scan_info->rf_channel;
    memcpy(&wfx_rsi.ap_mac.octet[0], &rsp.scan_info->bssid[0], BSSID_MAX_STR_LEN);

    switch (rsp.scan_info->security_mode)
    {
    case SME_OPEN:
        wfx_rsi.sec.security = WFX_SEC_NONE;
        break;
    case SME_WPA:
    case SME_WPA_ENTERPRISE:
        wfx_rsi.sec.security = WFX_SEC_WPA;
        break;
    case SME_WPA2:
    case SME_WPA2_ENTERPRISE:
        wfx_rsi.sec.security = WFX_SEC_WPA2;
        break;
    case SME_WEP:
        wfx_rsi.sec.security = WFX_SEC_WEP;
        break;
    case SME_WPA3_TRANSITION:
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case SME_WPA3:
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        break;
    default:
        wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
        break;
    }

    SILABS_LOG("%s: WLAN: connecting to %s, sec=%d, status=%02x", __func__, &wfx_rsi.sec.ssid[0], wfx_rsi.sec.security, status);
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
    rsi_security_mode_t connect_security_mode;

    if (wfx_rsi.dev_state & (WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED))
    {
        SILABS_LOG("%s: not joining - already in progress", __func__);
    }
    else
    {

        switch (wfx_rsi.sec.security)
        {
        case WFX_SEC_WEP:
            connect_security_mode = RSI_WEP;
            break;
        case WFX_SEC_WPA:
        case WFX_SEC_WPA2:
            connect_security_mode = RSI_WPA_WPA2_MIXED;
            break;
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        case WFX_SEC_WPA3:
            connect_security_mode = RSI_WPA3_TRANSITION;
            break;
#endif // WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        case WFX_SEC_NONE:
            connect_security_mode = RSI_OPEN;
            break;
        default:
            SILABS_LOG("%s: error: unknown security type.", __func__);
            return;
        }

        SILABS_LOG("%s: WLAN: connecting to %s, sec=%d", __func__, &wfx_rsi.sec.ssid[0], wfx_rsi.sec.security);

        /*
         * Join the network
         */
        /* TODO - make the WFX_SECURITY_xxx - same as RSI_xxx
         * Right now it's done by hand - we need something better
         */
        wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTING;

        if ((status = rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, wfx_rsi_join_fail_cb)) != RSI_SUCCESS)
        {
            SILABS_LOG("%s: RSI callback register join failed with status: %02x", __func__, status);
        }

        /* Try to connect Wifi with given Credentials
         * untill there is a success or maximum number of tries allowed
         */
        while (is_wifi_disconnection_event || wfx_rsi.join_retries <= WFX_RSI_CONFIG_MAX_JOIN)
        {
            /* Call rsi connect call with given ssid and password
             * And check there is a success
             */
            if ((status = rsi_wlan_connect_async((int8_t *) &wfx_rsi.sec.ssid[0], connect_security_mode, &wfx_rsi.sec.passkey[0],
                                                 wfx_rsi_join_cb)) != RSI_SUCCESS)
            {

                wfx_rsi.dev_state &= ~WFX_RSI_ST_STA_CONNECTING;
                SILABS_LOG("%s: rsi_wlan_connect_async failed with status: %02x on try %d", __func__, status, wfx_rsi.join_retries);

                wfx_retry_interval_handler(is_wifi_disconnection_event, wfx_rsi.join_retries);
                wfx_rsi.join_retries++;
            }
            else
            {
                SILABS_LOG("%s: starting JOIN to %s after %d tries\n", __func__, (char *) &wfx_rsi.sec.ssid[0],
                           wfx_rsi.join_retries);
                break; // exit while loop
            }
        }
    }
}

/**  NotifyConnectivity
 *   @brief Notify the application about the connectivity status if it has not been notified yet.
 *          Helper function for HandleDHCPPolling.
 */
void NotifyConnectivity()
{
    if (!hasNotifiedWifiConnectivity)
    {
        wfx_connected_notify(CONNECTION_STATUS_SUCCESS, &wfx_rsi.ap_mac);
        hasNotifiedWifiConnectivity = true;
    }
}

void HandleDHCPPolling()
{
    struct netif * sta_netif;
    WfxEvent_t event;

    sta_netif = wfx_get_netif(SL_WFX_STA_INTERFACE);
    if (sta_netif == NULL)
    {
        // TODO: Notify the application that the interface is not set up or Chipdie here because we are in an unkonwn state
        SILABS_LOG("HandleDHCPPolling: failed to get STA netif");
        return;
    }
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    uint8_t dhcp_state = dhcpclient_poll(sta_netif);
    if (dhcp_state == DHCP_ADDRESS_ASSIGNED && !hasNotifiedIPV4)
    {
        wfx_dhcp_got_ipv4((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
        hasNotifiedIPV4 = true;
        NotifyConnectivity();
    }
    else if (dhcp_state == DHCP_OFF)
    {
        wfx_ip_changed_notify(IP_STATUS_FAIL);
        hasNotifiedIPV4 = false;
    }
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
    /* Checks if the assigned IPv6 address is preferred by evaluating
     * the first block of IPv6 address ( block 0)
     */
    if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !hasNotifiedIPV6)
    {
        wfx_ipv6_notify(GET_IPV6_SUCCESS);
        hasNotifiedIPV6 = true;
        event.eventType = WFX_EVT_STA_DHCP_DONE;
        WfxPostEvent(&event);
        NotifyConnectivity();
    }
}

/** ResetDHCPNotificationFlags
 *  @brief Reset the flags that are used to notify the application about DHCP connectivity
 *         and emits a WFX_EVT_STA_DO_DHCP event to trigger DHCP polling checks. Helper function for ProcessEvent.
 */ 
void ResetDHCPNotificationFlags()
{
    WfxEvent_t outEvent;

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
    hasNotifiedIPV6             = false;
    hasNotifiedWifiConnectivity = false;

    outEvent.eventType = WFX_EVT_STA_DO_DHCP;
    WfxPostEvent(&outEvent);
}

/** 
 * @brief Post the WfxEvent to tue WiFiEventQueue to be process by the wfx_rsi_task
 */
void WfxPostEvent(WfxEvent_t * event)
{
    sl_status_t status = osMessageQueuePut(sWifiEventQueue, event, 0, 0);

    if (status != osOK)
    {
        SILABS_LOG("WfxPostEvent: failed to post event with status: %d", status);
        // TODO: Handle error, requeue event depending on queue size or notify relevant task, Chipdie, etc.
    }
}

void ProcessEvent(WfxEvent_t inEvent)
{
    // Process event
    switch (inEvent.eventType)
    {
    case WFX_EVT_STA_CONN:
        SILABS_LOG("%s: starting LwIP STA", __func__);
        wfx_rsi.dev_state |= WFX_RSI_ST_STA_CONNECTED;
        ResetDHCPNotificationFlags();
        wfx_lwip_set_sta_link_up();
        /* We need to get AP Mac - TODO */
        // Uncomment once the hook into MATTER is moved to IP connectivty instead
        // of AP connectivity.
        // wfx_connected_notify(0, &wfx_rsi.ap_mac); // This
        // is independant of IP connectivity.
        break;
    case WFX_EVT_STA_DISCONN:
        // TODO: This event is not being posted anywhere, seems to be a dead code or we are missing something
        wfx_rsi.dev_state &=
            ~(WFX_RSI_ST_STA_READY | WFX_RSI_ST_STA_CONNECTING | WFX_RSI_ST_STA_CONNECTED | WFX_RSI_ST_STA_DHCP_DONE);
        SILABS_LOG("%s: disconnect notify", __func__);
        /* TODO: Implement disconnect notify */
        ResetDHCPNotificationFlags();
        wfx_lwip_set_sta_link_down(); // Internally dhcpclient_poll(netif) ->
                                      // wfx_ip_changed_notify(0) for IPV4
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
        wfx_ip_changed_notify(IP_STATUS_FAIL);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        wfx_ipv6_notify(GET_IPV6_FAIL);
        break;
    case WFX_EVT_AP_START:
        // TODO: Currently unimplemented
        break;
    case WFX_EVT_AP_STOP:
        // TODO: Currently unimplemented
        break;
    case WFX_EVT_SCAN:
#ifdef SL_WFX_CONFIG_SCAN
        rsi_rsp_scan_t scan_rsp = { 0 };
        int32_t status          = rsi_wlan_bgscan_profile(1, &scan_rsp, sizeof(scan_rsp));

        if (status)
        {
            SILABS_LOG("SSID scan failed: %02x ", status);
        }
        else
        {
            rsi_scan_info_t * scan;
            wfx_wifi_scan_result_t ap;
            for (int x = 0; x < scan_rsp.scan_count[0]; x++)
            {
                scan = &scan_rsp.scan_info[x];
                // is it a scan all or target scan
                if (!wfx_rsi.scan_ssid ||
                    (wfx_rsi.scan_ssid && strcmp(wfx_rsi.scan_ssid, (char *) scan->ssid) == CMP_SUCCESS))
                {
                    strncpy(ap.ssid, (char *) scan->ssid, MIN(sizeof(ap.ssid), sizeof(scan->ssid)));
                    ap.security = scan->security_mode;
                    ap.rssi     = (-1) * scan->rssi_val;
                    configASSERT(sizeof(ap.bssid) >= BSSID_MAX_STR_LEN);
                    configASSERT(sizeof(scan->bssid) >= BSSID_MAX_STR_LEN);
                    memcpy(ap.bssid, scan->bssid, BSSID_MAX_STR_LEN);
                    (*wfx_rsi.scan_cb)(&ap);

                    if (wfx_rsi.scan_ssid)
                    {
                        break; // we found the targeted ssid.
                    }
                }
            }
        }

        /* Terminate with end of scan which is no ap sent back */
        (*wfx_rsi.scan_cb)((wfx_wifi_scan_result_t *) 0);
        wfx_rsi.scan_cb = (void (*)(wfx_wifi_scan_result_t *)) 0;

        if (wfx_rsi.scan_ssid)
        {
            vPortFree(wfx_rsi.scan_ssid);
            wfx_rsi.scan_ssid = (char *) 0;
        }
        break;
#endif /* SL_WFX_CONFIG_SCAN */
    case WFX_EVT_STA_START_JOIN:
        // saving the AP related info
        wfx_rsi_save_ap_info();
        // Joining to the network
        wfx_rsi_do_join();
        break;
    case WFX_EVT_STA_DO_DHCP:
        StartDHCPTimer(WFX_RSI_DHCP_POLL_INTERVAL);
        break;
    case WFX_EVT_STA_DHCP_DONE:
        CancelDHCPTimer();
        break;
    case WFX_EVT_DHCP_POLL:
        HandleDHCPPolling();
    default:
        break;
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
    (void) arg;
    uint32_t rsi_status = wfx_rsi_init();
    if (rsi_status != RSI_SUCCESS)
    {
        SILABS_LOG("%s: error: wfx_rsi_init with status: %02x", __func__, rsi_status);
        return;
    }
    WfxEvent_t wfxEvent;
    wfx_lwip_start();
    wfx_started_notify();

    SILABS_LOG("Starting event loop");
    for (;;)
    {
        osStatus_t status = osMessageQueueGet(sWifiEventQueue, &wfxEvent, NULL, osWaitForever);
        if (status == osOK)
        {
            ProcessEvent(wfxEvent);
        }
        else
        {
            SILABS_LOG("Failed to get event with status: %x", status);
        }
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
void * wfx_rsi_alloc_pkt()
{
    rsi_pkt_t * pkt;

    // Allocate packet to send data
    if ((pkt = rsi_pkt_alloc(&rsi_driver_cb->wlan_cb->wlan_tx_pool)) == NULL)
    {
        return (void *) 0;
    }

    return (void *) pkt;
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
    rsi_pkt_t * pkt;

    pkt = (rsi_pkt_t *) p;
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
    register uint8_t * host_desc;
    rsi_pkt_t * pkt;

    pkt       = (rsi_pkt_t *) p;
    host_desc = pkt->desc;
    memset(host_desc, 0, RSI_HOST_DESC_LENGTH);
    rsi_uint16_to_2bytes(host_desc, (len & 0xFFF));

    // Fill packet type
    host_desc[1] |= (RSI_WLAN_DATA_Q << 4);
    host_desc[2] |= 0x01;

    rsi_enqueue_pkt(&rsi_driver_cb->wlan_tx_q, pkt);

#ifndef RSI_SEND_SEM_BITMAP
    rsi_driver_cb_non_rom->send_wait_bitmap |= BIT(0);
#endif
    // Set TX packet pending event
    rsi_set_event(RSI_TX_EVENT);

    if (rsi_wait_on_wlan_semaphore(&rsi_driver_cb_non_rom->send_data_sem, RSI_SEND_DATA_RESPONSE_WAIT_TIME) != RSI_ERROR_NONE)
    {
        return RSI_ERROR_RESPONSE_TIMEOUT;
    }
    status = rsi_wlan_get_status();

    return status;
}

WfxRsi_t wfx_rsi;
