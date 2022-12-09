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

/* Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "em_bus.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_ldma.h"
#include "em_usart.h"
#include "gpiointerrupt.h"

#include "wifi_config.h"

#include "AppConfig.h"
#include "sl_wfx_board.h"
#include "sl_wfx_host.h"
#include "sl_wfx_task.h"
#include "wfx_host_events.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#include "dhcp_client.h"
#include "ethernetif.h"
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

/* wfxRsi Task will use as its stack */
StackType_t wfxEventTaskStack[1024] = { 0 };

/* Structure that will hold the TCB of the wfxRsi Task being created. */
StaticTask_t wfxEventTaskBuffer;

/* Declare a variable to hold the data associated with the created event group. */
StaticEventGroup_t wfxEventGroup;

EventGroupHandle_t sl_wfx_event_group;
TaskHandle_t wfx_events_task_handle;
static sl_wfx_mac_address_t ap_mac;
static uint32_t sta_ip;
static wfx_wifi_scan_result_t ap_info;

// Set Scan Parameters
#define ACTIVE_CHANNEL_TIME 110
#define PASSIVE_CHANNEL_TIME 0
#define NUM_PROBE_REQUEST 2

// wfx_fmac_driver context
sl_wfx_context_t wifiContext;
static uint8_t wifi_extra;

/*****************************************************************************
 * macros
 ******************************************************************************/
#define WE_ST_STARTED 1
#define WE_ST_STA_CONN 2
#define WE_ST_HW_STARTED 4

#ifdef SL_WFX_CONFIG_SOFTAP
// Connection parameters
char softap_ssid[32]                   = SOFTAP_SSID_DEFAULT;
char softap_passkey[64]                = SOFTAP_PASSKEY_DEFAULT;
sl_wfx_security_mode_t softap_security = SOFTAP_SECURITY_DEFAULT;
uint8_t softap_channel                 = SOFTAP_CHANNEL_DEFAULT;
#endif

/* station network interface structures */
struct netif * sta_netif;
wfx_wifi_provision_t wifi_provision;
sl_wfx_get_counters_cnf_t * counters;
sl_wfx_get_counters_cnf_t * Tempcounters;
#define PUT_COUNTER(name) SILABS_LOG("%-24s %lu\r\n", #name, (unsigned long) counters->body.count_##name);

bool hasNotifiedIPV6             = false;
bool hasNotifiedIPV4             = false;
bool hasNotifiedWifiConnectivity = false;
static uint8_t retryJoin         = 0;
bool retryInProgress             = false;

#ifdef SL_WFX_CONFIG_SCAN
static struct scan_result_holder
{
    struct scan_result_holder * next;
    wfx_wifi_scan_result scan;
} * scan_save;
static uint8_t scan_count = 0;
static void (*scan_cb)(wfx_wifi_scan_result_t *); /* user-callback - when scan is done */
static char * scan_ssid;                          /* Which one are we scanning for */
static void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t * scan_result);
static void sl_wfx_scan_complete_callback(uint32_t status);
#endif /* SL_WFX_CONFIG_SCAN */

static void wfx_events_task(void * p_arg);

/* WF200 host callbacks */
static void sl_wfx_connect_callback(sl_wfx_connect_ind_body_t connect_indication_body);
static void sl_wfx_disconnect_callback(uint8_t * mac, uint16_t reason);
static void sl_wfx_generic_status_callback(sl_wfx_generic_ind_t * frame);

#ifdef SL_WFX_CONFIG_SOFTAP
static void sl_wfx_start_ap_callback(uint32_t status);
static void sl_wfx_stop_ap_callback(void);
static void sl_wfx_client_connected_callback(uint8_t * mac);
static void sl_wfx_ap_client_disconnected_callback(uint32_t status, uint8_t * mac);
static void sl_wfx_ap_client_rejected_callback(uint32_t status, uint8_t * mac);
#endif

extern uint32_t gOverrunCount;

/***************************************************************************
 * @brief
 * Creates WFX events processing task.
 ******************************************************************************/
static void wfx_events_task_start()
{
    /* create an event group to track Wi-Fi events */
    sl_wfx_event_group = xEventGroupCreateStatic(&wfxEventGroup);

    wfx_events_task_handle = xTaskCreateStatic(wfx_events_task, "wfx_events", WLAN_TASK_STACK_SIZE, NULL, WLAN_TASK_PRIORITY,
                                               wfxEventTaskStack, &wfxEventTaskBuffer);
    if (NULL == wfx_events_task_handle)
    {
        SILABS_LOG("Failed to create WFX wfx_events");
    }
}

/****************************************************************************
 * @brief
 * Called when the driver needs to post an event
 * @param[in]  event_payload:
 * @returns Returns SL_STATUS_OK if successful,
 *SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t sl_wfx_host_process_event(sl_wfx_generic_message_t * event_payload)
{
    switch (event_payload->header.id)
    {
    /******** INDICATION ********/
    case SL_WFX_STARTUP_IND_ID: {
        SILABS_LOG("WFX Startup Completed\r\n");
        PlatformMgrImpl().HandleWFXSystemEvent(WIFI_EVENT, event_payload);
        break;
    }
    case SL_WFX_CONNECT_IND_ID: {
        sl_wfx_connect_ind_t * connect_indication = (sl_wfx_connect_ind_t *) event_payload;
        sl_wfx_connect_callback(connect_indication->body);
        break;
    }
    case SL_WFX_DISCONNECT_IND_ID: {
        sl_wfx_disconnect_ind_t * disconnect_indication = (sl_wfx_disconnect_ind_t *) event_payload;
        sl_wfx_disconnect_callback(disconnect_indication->body.mac, disconnect_indication->body.reason);
        break;
    }
    case SL_WFX_RECEIVED_IND_ID: {
        sl_wfx_received_ind_t * ethernet_frame = (sl_wfx_received_ind_t *) event_payload;
        if (ethernet_frame->body.frame_type == ETH_FRAME)
        {
            sl_wfx_host_received_frame_callback(ethernet_frame);
        }
        break;
    }
#ifdef SL_WFX_CONFIG_SCAN
    case SL_WFX_SCAN_RESULT_IND_ID: {
        sl_wfx_scan_result_ind_t * scan_result = (sl_wfx_scan_result_ind_t *) event_payload;
        sl_wfx_scan_result_callback(&scan_result->body);
        break;
    }
    case SL_WFX_SCAN_COMPLETE_IND_ID: {
        sl_wfx_scan_complete_ind_t * scan_complete = (sl_wfx_scan_complete_ind_t *) event_payload;
        sl_wfx_scan_complete_callback(scan_complete->body.status);
        break;
    }
#endif /* SL_WFX_CONFIG_SCAN */
#ifdef SL_WFX_CONFIG_SOFTAP
    case SL_WFX_START_AP_IND_ID: {
        sl_wfx_start_ap_ind_t * start_ap_indication = (sl_wfx_start_ap_ind_t *) event_payload;
        sl_wfx_start_ap_callback(start_ap_indication->body.status);
        break;
    }
    case SL_WFX_STOP_AP_IND_ID: {
        sl_wfx_stop_ap_callback();
        break;
    }
    case SL_WFX_AP_CLIENT_CONNECTED_IND_ID: {
        sl_wfx_ap_client_connected_ind_t * client_connected_indication = (sl_wfx_ap_client_connected_ind_t *) event_payload;
        sl_wfx_client_connected_callback(client_connected_indication->body.mac);
        break;
    }
    case SL_WFX_AP_CLIENT_REJECTED_IND_ID: {
        sl_wfx_ap_client_rejected_ind_t * ap_client_rejected_indication = (sl_wfx_ap_client_rejected_ind_t *) event_payload;
        sl_wfx_ap_client_rejected_callback(ap_client_rejected_indication->body.reason, ap_client_rejected_indication->body.mac);
        break;
    }
    case SL_WFX_AP_CLIENT_DISCONNECTED_IND_ID: {
        sl_wfx_ap_client_disconnected_ind_t * ap_client_disconnected_indication =
            (sl_wfx_ap_client_disconnected_ind_t *) event_payload;
        sl_wfx_ap_client_disconnected_callback(ap_client_disconnected_indication->body.reason,
                                               ap_client_disconnected_indication->body.mac);
        break;
    }
#endif /* SL_WFX_CONFIG_SOFTAP */
#ifdef SL_WFX_USE_SECURE_LINK
    case SL_WFX_SECURELINK_EXCHANGE_PUB_KEYS_IND_ID: {
        if (host_context.waited_event_id != SL_WFX_SECURELINK_EXCHANGE_PUB_KEYS_IND_ID)
        {
            memcpy((void *) &sl_wfx_context->secure_link_exchange_ind, (void *) event_payload, event_payload->header.length);
        }
        break;
    }
#endif
    case SL_WFX_GENERIC_IND_ID: {
        sl_wfx_generic_ind_t * generic_status = (sl_wfx_generic_ind_t *) event_payload;
        sl_wfx_generic_status_callback(generic_status);
        break;
    }
    case SL_WFX_EXCEPTION_IND_ID: {
        sl_wfx_exception_ind_t * firmware_exception = (sl_wfx_exception_ind_t *) event_payload;
        uint8_t * exception_tmp                     = (uint8_t *) firmware_exception;
        SILABS_LOG("firmware exception\r\n");
        for (uint16_t i = 0; i < firmware_exception->header.length; i += 16)
        {
            SILABS_LOG("hif: %.8x:", i);
            for (uint8_t j = 0; (j < 16) && ((i + j) < firmware_exception->header.length); j++)
            {
                SILABS_LOG(" %.2x", *exception_tmp);
                exception_tmp++;
            }
            SILABS_LOG("\r\n");
        }
        break;
    }
    case SL_WFX_ERROR_IND_ID: {
        sl_wfx_error_ind_t * firmware_error = (sl_wfx_error_ind_t *) event_payload;
        uint8_t * error_tmp                 = (uint8_t *) firmware_error;
        SILABS_LOG("firmware error %lu\r\n", firmware_error->body.type);
        for (uint16_t i = 0; i < firmware_error->header.length; i += 16)
        {
            SILABS_LOG("hif: %.8x:", i);
            for (uint8_t j = 0; (j < 16) && ((i + j) < firmware_error->header.length); j++)
            {
                SILABS_LOG(" %.2x", *error_tmp);
                error_tmp++;
            }
            SILABS_LOG("\r\n");
        }
        break;
    }
    }

    return SL_STATUS_OK;
}

#ifdef SL_WFX_CONFIG_SCAN
/****************************************************************************
 * @brief
 * Callback for individual scan result
 * @param[in] scan_result: Scan result of all SSID's
 *****************************************************************************/
static void sl_wfx_scan_result_callback(sl_wfx_scan_result_ind_body_t * scan_result)
{
    struct scan_result_holder * ap;

    SILABS_LOG("# %2d %2d  %03d %02X:%02X:%02X:%02X:%02X:%02X  %s", scan_count, scan_result->channel,
               ((int16_t)(scan_result->rcpi - 220) / 2), scan_result->mac[0], scan_result->mac[1], scan_result->mac[2],
               scan_result->mac[3], scan_result->mac[4], scan_result->mac[5], scan_result->ssid_def.ssid);
    /*Report one AP information*/
    SILABS_LOG("\r\n");
    /* don't save if filter only wants specific ssid */
    if (scan_ssid != (char *) 0)
    {
        if (strcmp(scan_ssid, (char *) &scan_result->ssid_def.ssid[0]) != CMP_SUCCESS)
            return;
    }
    if ((ap = (struct scan_result_holder *) pvPortMalloc(sizeof(*ap))) == (struct scan_result_holder *) 0)
    {
        SILABS_LOG("*ERR*Scan: No Mem");
    }
    else
    {
        ap->next  = scan_save;
        scan_save = ap;
        /* Not checking if scan_result->ssid_length is < 33 */
        memcpy(ap->scan.ssid, scan_result->ssid_def.ssid, scan_result->ssid_def.ssid_length);
        ap->scan.ssid[scan_result->ssid_def.ssid_length] = 0; /* make sure about null terminate */
        /* We do it in this order WPA3 first */
        /* No EAP supported - Is this required */
        if (scan_result->security_mode.wpa3)
        {
            ap->scan.security = WFX_SEC_WPA3;
        }
        else if (scan_result->security_mode.wpa2)
        {
            ap->scan.security = WFX_SEC_WPA2;
        }
        else if (scan_result->security_mode.wpa)
        {
            ap->scan.security = WFX_SEC_WPA;
        }
        else if (scan_result->security_mode.wep)
        {
            ap->scan.security = WFX_SEC_WEP;
        }
        else
        {
            ap->scan.security = WFX_SEC_NONE;
        }
        ap->scan.chan = scan_result->channel;
        ap->scan.rssi = scan_result->rcpi;
        memcpy(&ap->scan.bssid[0], &scan_result->mac[0], BSSID_MAX_STR_LEN);
        scan_count++;
    }
}

/****************************************************************************
 * @brief
 * Callback for scan complete
 * @param[in] status:Status of WLAN scan api
 *****************************************************************************/
/* ARGSUSED */
static void sl_wfx_scan_complete_callback(uint32_t status)
{
    (void) (status);
    /* Use scan_count value and reset it */
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_SCAN_COMPLETE);
}
#endif /* SL_WFX_CONFIG_SCAN */

/****************************************************************************
 * @brief
 * Callback when station connects
 * @param[in]  mac: MAC address of device
 * @param[in]  status: Status of connect call
 *****************************************************************************/
static void sl_wfx_connect_callback(sl_wfx_connect_ind_body_t connect_indication_body)
{
    uint8_t * mac   = connect_indication_body.mac;
    uint32_t status = connect_indication_body.status;
    ap_info.chan    = connect_indication_body.channel;
    memcpy(&ap_info.security, &wifi_provision.security, sizeof(wifi_provision.security));
    switch (status)
    {
    case WFM_STATUS_SUCCESS: {
        SILABS_LOG("STA-Connected\r\n");
        memcpy(&ap_mac.octet[0], mac, MAC_ADDRESS_FIRST_OCTET);
        sl_wfx_context->state =
            static_cast<sl_wfx_state_t>(static_cast<int>(sl_wfx_context->state) | static_cast<int>(SL_WFX_STA_INTERFACE_CONNECTED));
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_CONNECT);
        break;
    }
    case WFM_STATUS_NO_MATCHING_AP: {
        SILABS_LOG("WFX Connection failed, access point not found\r\n");
        break;
    }
    case WFM_STATUS_CONNECTION_ABORTED: {
        SILABS_LOG("WFX Connection aborted\r\n");
        break;
    }
    case WFM_STATUS_CONNECTION_TIMEOUT: {
        SILABS_LOG("WFX Connection timeout\r\n");
        break;
    }
    case WFM_STATUS_CONNECTION_REJECTED_BY_AP: {
        SILABS_LOG("WFX Connection rejected by the access point\r\n");
        break;
    }
    case WFM_STATUS_CONNECTION_AUTH_FAILURE: {
        SILABS_LOG("WFX Connection authentication failure\r\n");
        break;
    }
    default: {
        SILABS_LOG("WF Connection attempt error\r\n");
    }
    }

    if ((status != WFM_STATUS_SUCCESS) && retryJoin < MAX_JOIN_RETRIES_COUNT)
    {
        retryJoin += 1;
        retryInProgress = false;
        SILABS_LOG("WFX Retry to connect to network count: %d", retryJoin);
        sl_wfx_context->state =
            static_cast<sl_wfx_state_t>(static_cast<int>(sl_wfx_context->state) & ~static_cast<int>(SL_WFX_STARTED));
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_RETRY_CONNECT);
    }
}

/****************************************************************************
 * @brief
 * Callback for station disconnect
 * @param[in]  mac: MAC address of device
 * @param[in]  reason: Reason code of disconnection
 *****************************************************************************/
static void sl_wfx_disconnect_callback(uint8_t * mac, uint16_t reason)
{
    (void) (mac);
    SILABS_LOG("WFX Disconnected %d\r\n", reason);
    sl_wfx_context->state =
        static_cast<sl_wfx_state_t>(static_cast<int>(sl_wfx_context->state) & ~static_cast<int>(SL_WFX_STA_INTERFACE_CONNECTED));
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_DISCONNECT);
}

#ifdef SL_WFX_CONFIG_SOFTAP
/****************************************************************************
 * @brief
 * Callback for AP started
 * @param[in]  status: Status of wfx start ap api
 *****************************************************************************/
static void sl_wfx_start_ap_callback(uint32_t status)
{
    if (status == AP_START_SUCCESS)
    {
        SILABS_LOG("AP started\r\n");
        sl_wfx_context->state =
            static_cast<sl_wfx_state_t>(static_cast<int>(sl_wfx_context->state) | static_cast<int>(SL_WFX_AP_INTERFACE_UP));
        xEventGroupSetBits(sl_wfx_event_group, SL_WFX_START_AP);
    }
    else
    {
        SILABS_LOG("AP start failed\r\n");
        strcpy(event_log, "AP start failed");
    }
}

/****************************************************************************
 * @brief
 * Callback for AP stopped
 *****************************************************************************/
static void sl_wfx_stop_ap_callback(void)
{
    // TODO
    // dhcpserver_clear_stored_mac();
    SILABS_LOG("SoftAP stopped\r\n");
    sl_wfx_context->state =
        static_cast<sl_wfx_state_t>(static_cast<int>(sl_wfx_context->state) & ~static_cast<int>(SL_WFX_AP_INTERFACE_UP));
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_STOP_AP);
}

/****************************************************************************
 * @brief
 * Callback for client connect to AP
 * @param[in]  mac: MAC adress of device
 *****************************************************************************/
static void sl_wfx_client_connected_callback(uint8_t * mac)
{
    SILABS_LOG("Client connected, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // TODO
    SILABS_LOG("Open a web browser and go to http://%d.%d.%d.%d\r\n", ap_ip_addr0, ap_ip_addr1, ap_ip_addr2, ap_ip_addr3);
}

/****************************************************************************
 * @brief
 * Callback for client rejected from AP
 * @param[in] status: Status of ap rejected
 * @param[in] mac: MAC adress of device
 *****************************************************************************/
static void sl_wfx_ap_client_rejected_callback(uint32_t status, uint8_t * mac)
{
    SILABS_LOG("Client rejected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", (int) status, mac[0], mac[1], mac[2], mac[3],
               mac[4], mac[5]);
}

/****************************************************************************
 * @brief
 * Callback for AP client disconnect
 * @param[in] status: Status of ap dissconnect
 * @param[in]  mac:
 *****************************************************************************/
static void sl_wfx_ap_client_disconnected_callback(uint32_t status, uint8_t * mac)
{
    // TODO
    SILABS_LOG("Client disconnected, reason: %d, MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", (int) status, mac[0], mac[1], mac[2],
               mac[3], mac[4], mac[5]);
}
#endif /* SL_WFX_CONFIG_SOFTAP */

/****************************************************************************
 * @brief
 * Callback for generic status received
 * @param[in] farme:
 *****************************************************************************/
static void sl_wfx_generic_status_callback(sl_wfx_generic_ind_t * frame)
{
    (void) (frame);
    SILABS_LOG("WFX Generic status received\r\n");
}

/***************************************************************************
 * @brief
 * WFX events processing task.
 * @param[in] p_arg:
 *  ******************************************************************************/
static void wfx_events_task(void * p_arg)
{
    TickType_t last_dhcp_poll, now;
    EventBits_t flags;
    (void) p_arg;

    sta_netif      = wfx_get_netif(SL_WFX_STA_INTERFACE);
    last_dhcp_poll = xTaskGetTickCount();
    while (true)
    {
        flags = xEventGroupWaitBits(sl_wfx_event_group,
                                    SL_WFX_CONNECT | SL_WFX_DISCONNECT
#ifdef SL_WFX_CONFIG_SOFTAP
                                        | SL_WFX_START_AP | SL_WFX_STOP_AP
#endif /* SL_WFX_CONFIG_SOFTAP */
#ifdef SL_WFX_CONFIG_SCAN
                                        | SL_WFX_SCAN_START | SL_WFX_SCAN_COMPLETE
#endif /* SL_WFX_CONFIG_SCAN */
                                        | BITS_TO_WAIT,
                                    pdTRUE, pdFALSE, pdMS_TO_TICKS(250)); /* 250 msec delay converted to ticks */
        if (flags & SL_WFX_RETRY_CONNECT)
        {
            if (!retryInProgress)
            {
                SILABS_LOG("WFX sending the connect command");
                wfx_connect_to_ap();
                retryInProgress = true;
            }
        }

        if (wifi_extra & WE_ST_STA_CONN)
        {
            if ((now = xTaskGetTickCount()) > (last_dhcp_poll + pdMS_TO_TICKS(250)))
            {
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
                uint8_t dhcp_state = dhcpclient_poll(&sta_netif);

                if ((dhcp_state == DHCP_ADDRESS_ASSIGNED) && !hasNotifiedIPV4)
                {
                    wfx_dhcp_got_ipv4((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
                    hasNotifiedIPV4 = true;
                    if (!hasNotifiedWifiConnectivity)
                    {
                        SILABS_LOG("WIFI: Has Notified Wifi Connectivity");
                        wfx_connected_notify(CONNECTION_STATUS_SUCCESS, &ap_mac);
                        hasNotifiedWifiConnectivity = true;
                    }
                }
                else if (dhcp_state == DHCP_OFF)
                {
                    wfx_ip_changed_notify(IP_STATUS_FAIL);
                    hasNotifiedIPV4 = false;
                }
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
                if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !hasNotifiedIPV6)
                {
                    wfx_ipv6_notify(1);
                    hasNotifiedIPV6 = true;
                    if (!hasNotifiedWifiConnectivity)
                    {
                        wfx_connected_notify(CONNECTION_STATUS_SUCCESS, &ap_mac);
                        hasNotifiedWifiConnectivity = true;
                    }
                }
                last_dhcp_poll = now;
            }
        }

        if (flags & SL_WFX_CONNECT)
        {
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
            wfx_ip_changed_notify(IP_STATUS_FAIL);
            hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
            wfx_ipv6_notify(GET_IPV6_FAIL);
            hasNotifiedIPV6             = false;
            hasNotifiedWifiConnectivity = false;
            SILABS_LOG("WIFI: Connected to AP");
            wifi_extra |= WE_ST_STA_CONN;
            wfx_lwip_set_sta_link_up();
#ifdef SLEEP_ENABLED
            if (!(wfx_get_wifi_state() & SL_WFX_AP_INTERFACE_UP))
            {
                // Enable the power save
                sl_wfx_set_power_mode(WFM_PM_MODE_PS, WFM_PM_POLL_UAPSD, BEACON_1);
                sl_wfx_enable_device_power_save();
            }
#endif // SLEEP_ENABLED
        }

        if (flags & SL_WFX_DISCONNECT)
        {

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
            wfx_ip_changed_notify(IP_STATUS_FAIL);
            hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
            wfx_ipv6_notify(GET_IPV6_FAIL);
            hasNotifiedIPV6             = false;
            hasNotifiedWifiConnectivity = false;
            wifi_extra &= ~WE_ST_STA_CONN;
            wfx_lwip_set_sta_link_down();
        }

#ifdef SL_WFX_CONFIG_SCAN
        if (flags & SL_WFX_SCAN_START)
        {
            /*
             * Start the Scan
             */
            sl_wfx_ssid_def_t ssid, *sp;
            uint16_t num_ssid, slen;
            if (scan_ssid)
            {
                memset(&ssid, 0, sizeof(ssid));
                slen = strlen(scan_ssid);
                memcpy(&ssid.ssid[0], scan_ssid, slen);
                ssid.ssid_length = slen;
                num_ssid         = 1;
                sp               = &ssid;
            }
            else
            {
                num_ssid = 0;
                sp       = (sl_wfx_ssid_def_t *) 0;
            }

            SILABS_LOG("WIFI Scan Paramter set to Active channel time %d, Passive "
                       "Channel Time: %d, Number of prob: %d",
                       ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
            (void) sl_wfx_set_scan_parameters(ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
            (void) sl_wfx_send_scan_command(WFM_SCAN_MODE_ACTIVE, CHANNEL_LIST, /* Channel list */
                                            CHANNEL_COUNT,                      /* Scan all chans */
                                            sp, num_ssid, IE_DATA,              /* IE we're looking for */
                                            IE_DATA_LENGTH, BSSID_SCAN);
        }
        if (flags & SL_WFX_SCAN_COMPLETE)
        {
            struct scan_result_holder *hp, *next;

            SILABS_LOG("WIFI: Return %d scan results", scan_count);
            for (hp = scan_save; hp; hp = next)
            {
                next = hp->next;
                (*scan_cb)(&hp->scan);
                vPortFree(hp);
            }
            (*scan_cb)((wfx_wifi_scan_result *) 0);
            scan_save  = (struct scan_result_holder *) 0;
            scan_count = 0;
            if (scan_ssid)
            {
                vPortFree(scan_ssid);
                scan_ssid = (char *) 0;
            }
            /* Terminate scan */
            scan_cb = 0;
        }
#endif /* SL_WFX_CONFIG_SCAN */
    }
}

/****************************************************************************
 * @brief
 * Initialize the WF200 used by the two interfaces
 *****************************************************************************/
static sl_status_t wfx_init(void)
{
    /* Initialize the WF200 used by the two interfaces */
    wfx_events_task_start();
    sl_status_t status = sl_wfx_init(&wifiContext);
    SILABS_LOG("FMAC Driver version    %s", FMAC_DRIVER_VERSION_STRING);
    switch (status)
    {
    case SL_STATUS_OK:
        SILABS_LOG("WF200 FW ver:%d.%d.%d [MAC %02x:%02x:%02x-%02x:%02x:%02x]", wifiContext.firmware_major,
                   wifiContext.firmware_minor, wifiContext.firmware_build, wifiContext.mac_addr_0.octet[0],
                   wifiContext.mac_addr_0.octet[1], wifiContext.mac_addr_0.octet[2], wifiContext.mac_addr_0.octet[3],
                   wifiContext.mac_addr_0.octet[4], wifiContext.mac_addr_0.octet[5]);
        SILABS_LOG("WF200 Init OK");

        if (wifiContext.state == SL_WFX_STA_INTERFACE_CONNECTED)
        {
            sl_wfx_send_disconnect_command();
        }

        break;
    case SL_STATUS_WIFI_INVALID_KEY:
        SILABS_LOG("*ERR*WF200: F/W keyset invalid");
        break;
    case SL_STATUS_WIFI_FIRMWARE_DOWNLOAD_TIMEOUT:
        SILABS_LOG("*ERR*WF200: F/W download timo");
        break;
    case SL_STATUS_TIMEOUT:
        SILABS_LOG("*ERR*WF200: Poll for value timo");
        break;
    case SL_STATUS_FAIL:
        SILABS_LOG("*ERR*WF200: Error");
        break;
    default:
        SILABS_LOG("*ERR*WF200: Unknown");
    }

    return status;
}

/*****************************************************************************
 * @brief
 *   tcp ip, wfx and lwip stack and start dhcp client.
 * @return
 *    sl_status_t Shows init succes or error.
 ******************************************************************************/
static void wfx_wifi_hw_start(void)
{
    sl_status_t status;

    if (wifi_extra & WE_ST_HW_STARTED)
        return;
    SILABS_LOG("STARTING WF200\n");
    wifi_extra |= WE_ST_HW_STARTED;

    sl_wfx_host_gpio_init();
    if ((status = wfx_init()) == SL_STATUS_OK)
    {
        /* Initialize the LwIP stack */
        SILABS_LOG("WF200:Start LWIP");
        wfx_lwip_start();
        wifiContext.state = SL_WFX_STARTED; /* Really this is a bit mask */
        SILABS_LOG("WF200:ready..");
    }
    else
    {
        SILABS_LOG("*ERR*WF200:init failed");
    }
}

/***********************************************************************
 * @brief
 * Get AP info
 * @param[in]  ap: access point information
 * @return returns -1
 **************************************************************************/
int32_t wfx_get_ap_info(wfx_wifi_scan_result_t * ap)
{
    int32_t signal_strength;
    SILABS_LOG("WIFI:SSID:: %s", &ap_info.ssid[0]);
    memcpy(ap->ssid, ap_info.ssid, sizeof(ap_info.ssid));
    SILABS_LOG("WIFI:Mac addr:: %02x:%02x:%02x:%02x:%02x:%02x", ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
               ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);
    memcpy(ap->bssid, ap_info.bssid, sizeof(ap_info.bssid));
    ap->security = ap_info.security;
    SILABS_LOG("WIFI:security:: %d", ap->security);
    ap->chan = ap_info.chan;
    SILABS_LOG("WIFI:Channel:: to %d", ap->chan);

    sl_status_t status = sl_wfx_get_signal_strength((uint32_t *) &signal_strength);

    if (status == SL_STATUS_OK)
    {
        SILABS_LOG("status SL_STATUS_OK & signal_strength:: %d", signal_strength);
        ap->rssi = (signal_strength - 220) / 2;
    }
    return status;
}

/************************************************************************
 * @brief
 * Get AP extra info
 * @param[in]  extra_info: access point extra information
 * @return returns -1
 **************************************************************************/
int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t * extra_info)
{
    int32_t status;
    status = get_all_counters();
    if (status != SL_STATUS_OK)
    {
        SILABS_LOG("Failed to get the couters");
    }
    else
    {
        extra_info->beacon_lost_count = counters->body.count_miss_beacon;
        extra_info->beacon_rx_count   = counters->body.count_rx_beacon;
        extra_info->mcast_rx_count    = counters->body.count_rx_multicast_frames;
        extra_info->mcast_tx_count    = counters->body.count_tx_multicast_frames;
        extra_info->ucast_rx_count    = counters->body.count_rx_packets;
        extra_info->ucast_tx_count    = counters->body.count_tx_packets;
        extra_info->overrun_count     = gOverrunCount;
    }
    return status;
}

sl_status_t get_all_counters(void)
{
    sl_status_t result;
    uint8_t command_id         = 0x05;
    uint16_t mib_id            = 0x2035;
    sl_wfx_mib_req_t * request = NULL;
    uint32_t request_length    = SL_WFX_ROUND_UP_EVEN(sizeof(sl_wfx_header_mib_t) + sizeof(sl_wfx_mib_req_body_t));

    result =
        sl_wfx_allocate_command_buffer((sl_wfx_generic_message_t **) &request, command_id, SL_WFX_CONTROL_BUFFER, request_length);

    if (request == NULL)
    {
    }

    request->body.mib_id      = mib_id;
    request->header.interface = 0x2;
    request->header.encrypted = 0x0;

    result = sl_wfx_send_request(command_id, (sl_wfx_generic_message_t *) request, request_length);
    SL_WFX_ERROR_CHECK(result);

    result = sl_wfx_host_wait_for_confirmation(command_id, SL_WFX_DEFAULT_REQUEST_TIMEOUT_MS, (void **) &counters);
    SL_WFX_ERROR_CHECK(result);

    SILABS_LOG("%-24s %12s \r\n", "", "Debug Counters Content");
    SILABS_LOG("%-24s %lu\r\n", "rcpi", (unsigned long) counters->body.rcpi);
    PUT_COUNTER(plcp_errors);
    PUT_COUNTER(fcs_errors);
    PUT_COUNTER(tx_packets);
    PUT_COUNTER(rx_packets);
    PUT_COUNTER(rx_packet_errors);
    PUT_COUNTER(rx_decryption_failures);
    PUT_COUNTER(rx_mic_failures);
    PUT_COUNTER(rx_no_key_failures);
    PUT_COUNTER(tx_multicast_frames);
    PUT_COUNTER(tx_frames_success);
    PUT_COUNTER(tx_frame_failures);
    PUT_COUNTER(tx_frames_retried);
    PUT_COUNTER(tx_frames_multi_retried);
    PUT_COUNTER(rx_frame_duplicates);
    PUT_COUNTER(rts_success);
    PUT_COUNTER(rts_failures);
    PUT_COUNTER(ack_failures);
    PUT_COUNTER(rx_multicast_frames);
    PUT_COUNTER(rx_frames_success);
    PUT_COUNTER(rx_cmacicv_errors);
    PUT_COUNTER(rx_cmac_replays);
    PUT_COUNTER(rx_mgmt_ccmp_replays);
    PUT_COUNTER(rx_bipmic_errors);
    PUT_COUNTER(rx_beacon);
    PUT_COUNTER(miss_beacon);

error_handler:

    if (result == SL_STATUS_TIMEOUT)
    {
        if (sl_wfx_context->used_buffers > 0)
        {
            sl_wfx_context->used_buffers--;
        }
    }
    if (request != NULL)
    {
        sl_wfx_free_command_buffer((sl_wfx_generic_message_t *) request, command_id, SL_WFX_CONTROL_BUFFER);
    }

    return result;
}

/************************************************************************
 * @brief
 *    reset the count
 * @return returns -1
 **************************************************************************/
int32_t wfx_reset_counts()
{
    /* TODO */
    return -1;
}

/*************************************************************************
 * @brief
 * I think that this is getting called before FreeRTOS threads are ready
 * @return  returns SL_STATUS_OK
 **************************************************************************/
sl_status_t wfx_wifi_start(void)
{
    if (wifi_extra & WE_ST_STARTED)
    {
        SILABS_LOG("WIFI: Already started");
        return SL_STATUS_OK;
    }
    wifi_extra |= WE_ST_STARTED;
    wfx_soft_init();
    wfx_wifi_hw_start();

    return SL_STATUS_OK;
}

/****************************************************************************
 * @brief
 *      get the wifi state
 * @return returns wificonetext state
 *****************************************************************************/
sl_wfx_state_t wfx_get_wifi_state(void)
{
    return wifiContext.state;
}

/****************************************************************************
 * @brief
 *      getnetif using interface
 * @param[in]  interface:
 * @return returns selectedNetif
 *****************************************************************************/
struct netif * wfx_GetNetif(sl_wfx_interface_t interface)
{
    struct netif * SelectedNetif = NULL;
    if (interface == SL_WFX_STA_INTERFACE)
    {
        SelectedNetif = sta_netif;
    }
#ifdef SL_WFX_CONFIG_SOFTAP
    else if (interface == SL_WFX_SOFTAP_INTERFACE)
    {
        // no ap currently
    }
#endif
    return SelectedNetif;
}

/****************************************************************************
 * @brief
 * get the wifi mac address using interface
 * @param[in] interface:
 * @return  returns wificontext.mac_addr_o if successful,
 *          wificontext.mac_addr_1 otherwise
 *****************************************************************************/
sl_wfx_mac_address_t wfx_get_wifi_mac_addr(sl_wfx_interface_t interface)
{
    // return Mac address used by WFX SL_WFX_STA_INTERFACE or SL_WFX_SOFTAP_INTERFACE,
    return (interface == SL_WFX_STA_INTERFACE) ? wifiContext.mac_addr_0 : wifiContext.mac_addr_1;
}

/****************************************************************************
 * @brief
 * set the wifi provision
 * @param[in] wifiConfig: configuration of wifi
 *****************************************************************************/
void wfx_set_wifi_provision(wfx_wifi_provision_t * wifiConfig)
{
    memcpy(wifi_provision.ssid, wifiConfig->ssid, sizeof(wifiConfig->ssid));
    memcpy(wifi_provision.passkey, wifiConfig->passkey, sizeof(wifiConfig->passkey));
    SILABS_LOG("WIFI: Provision SSID=%s", &wifi_provision.ssid[0]);

    /* Not very good - To be improved */
    switch (wifiConfig->security)
    {
    case WFX_SEC_WPA:
        wifi_provision.security = static_cast<uint8_t>(sl_wfx_security_mode_e::WFM_SECURITY_MODE_WPA2_WPA1_PSK);
        break;
    case WFX_SEC_WPA3:
        wifi_provision.security = WFM_SECURITY_MODE_WPA3_SAE;
        break;
    case WFX_SEC_WPA2:
        wifi_provision.security = static_cast<uint8_t>(sl_wfx_security_mode_e::WFM_SECURITY_MODE_WPA2_WPA1_PSK);
        break;
    case WFX_SEC_WPA_WPA2_MIXED:
        wifi_provision.security = static_cast<uint8_t>(sl_wfx_security_mode_e::WFM_SECURITY_MODE_WPA2_WPA1_PSK);
        break;
    default:
        wifi_provision.security = WFM_SECURITY_MODE_WPA2_PSK;
        break;
    }
}

/****************************************************************************
 * @brief
 * get the wifi provision
 * @param[in] wifiConfig: configuration of wifi
 * @return returns true if successful,
 *         false otherwise
 *****************************************************************************/
bool wfx_get_wifi_provision(wfx_wifi_provision_t * wifiConfig)
{
    if (wifiConfig == NULL)
    {
        return false;
    }
    memcpy(wifiConfig, &wifi_provision, sizeof(wfx_wifi_provision_t));

    return true;
}

/****************************************************************************
 * @brief
 * clear the wifi provision
 * @return returns true if successful,
 *         false otherwise
 *****************************************************************************/
void wfx_clear_wifi_provision(void)
{
    memset(&wifi_provision, 0, sizeof(wifi_provision));
}

/****************************************************************************
 * @brief
 *      driver STA provisioned
 * @return returns true if successful,
 *         false otherwise
 *****************************************************************************/
bool wfx_is_sta_provisioned(void)
{
    return (wifi_provision.ssid[0]) ? true : false;
}

/****************************************************************************
 * @fn  sl_status_t wfx_connect_to_ap(void)
 * @brief
 *  driver connect to ap
 * @return returns SL_STATUS_NOT_AVAILABLE
 *****************************************************************************/
sl_status_t wfx_connect_to_ap(void)
{
    sl_status_t result;

    if (wifi_provision.ssid[0] == 0)
    {
        return SL_STATUS_NOT_AVAILABLE;
    }
    SILABS_LOG("WIFI:JOIN to %s", &wifi_provision.ssid[0]);

    SILABS_LOG("WIFI Scan Paramter set to Active channel time %d, Passive Channel "
               "Time: %d, Number of prob: %d",
               ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
    (void) sl_wfx_set_scan_parameters(ACTIVE_CHANNEL_TIME, PASSIVE_CHANNEL_TIME, NUM_PROBE_REQUEST);
    result =
        sl_wfx_send_join_command((uint8_t *) wifi_provision.ssid, strlen(wifi_provision.ssid), NULL, CHANNEL_0,
                                 static_cast<sl_wfx_security_mode_t>(wifi_provision.security), PREVENT_ROAMING, DISABLE_PMF_MODE,
                                 (uint8_t *) wifi_provision.passkey, strlen(wifi_provision.passkey), NULL, IE_DATA_LENGTH);

    return result;
}

/****************************************************************************
 * @brief
 * get the wifi mac addresss
 * @param[in] interface:
 * @param[in] addr : address
 *****************************************************************************/
void wfx_get_wifi_mac_addr(sl_wfx_interface_t interface, sl_wfx_mac_address_t * addr)
{
    sl_wfx_mac_address_t * mac;

#ifdef SL_WFX_CONFIG_SOFTAP
    mac = (interface == SL_WFX_SOFTAP_INTERFACE) ? &wifiContext.mac_addr_1 : &wifiContext.mac_addr_0;
#else
    mac = &wifiContext.mac_addr_0;
#endif
    *addr = *mac;
    SILABS_LOG("WLAN:Get WiFi Mac addr %02x:%02x:%02x:%02x:%02x:%02x", mac->octet[0], mac->octet[1], mac->octet[2], mac->octet[3],
               mac->octet[4], mac->octet[5]);
    memcpy(&ap_info.bssid[0], &mac->octet[0], 6);
}

/****************************************************************************
 * @brief
 *     function called when driver have ipv4 address
 * @param[in]  which_if:
 * @return returns false if sucessful,
 *         true otherwise
 *****************************************************************************/
bool wfx_have_ipv4_addr(sl_wfx_interface_t which_if)
{
    if (which_if == SL_WFX_STA_INTERFACE)
    {
        return (sta_ip == STA_IP_FAIL) ? false : true;
    }
    else
    {
        return false; /* TODO */
    }
}

/****************************************************************************
 * @brief
 *     function called when driver have ipv6 address
 * @param[in]  which_if:
 * @return returns false if sucessful,
 *         true otherwise
 *****************************************************************************/
bool wfx_have_ipv6_addr(sl_wfx_interface_t which_if)
{
    SILABS_LOG("%s: started.", __func__);
    bool status = false;
    if (which_if == SL_WFX_STA_INTERFACE)
    {
        status = wfx_is_sta_connected();
    }
    else
    {
        status = false; /* TODO */
    }
    SILABS_LOG("%s: status: %d", __func__, status);
    return status;
}

/****************************************************************************
 * @brief
 * Disconnect station mode from connected AP
 * @returns Returns SL_STATUS_OK if successful,
 *             SL_STATUS_FAIL otherwise
 *****************************************************************************/
sl_status_t wfx_sta_discon(void)
{
    SILABS_LOG("STA-Disconnecting");
    int32_t status = sl_wfx_send_disconnect_command();
    wifi_extra &= ~WE_ST_STA_CONN;
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_RETRY_CONNECT);
    return status;
}

/****************************************************************************
 * @brief
 *     enable the STA mode
 * @return returns true
 *****************************************************************************/
bool wfx_is_sta_mode_enabled(void)
{
    return true; /* It always is */
}

/****************************************************************************
 * @brief
 *     fuction called when driver is STA connected
 * @return returns true if sucessful,
 *         false otherwise
 *****************************************************************************/
bool wfx_is_sta_connected(void)
{
    bool val;

    val = (wifi_extra & WE_ST_STA_CONN) ? true : false;

    SILABS_LOG("WLAN: STA %s connected", (val ? "IS" : "NOT"));

    return val;
}

/****************************************************************************
 * @brief
 *      It is automatically done when lwip link up
 * @return returns true if sucessful,
 *         false otherwise
 *****************************************************************************/
void wfx_setup_ip6_link_local(sl_wfx_interface_t whichif)
{
    SILABS_LOG("Setup-IP6: TODO"); /* It is automatically done when lwip link up */
}

/****************************************************************************
 * @brief
 *      get the wifi mode
 * @return returns WIFI_MODE_NULL if sucessful,
 *         WIFI_MODE_STA otherwise
 *****************************************************************************/
wifi_mode_t wfx_get_wifi_mode()
{
    if (wifiContext.state & SL_WFX_STARTED)
        return WIFI_MODE_STA;
    return WIFI_MODE_NULL;
}

/*****************************************************************************
 * @brief
 * This is called from the context of AppTask
 * For WF200 - Start WIFI here
 * @return returns true if sucessful,
 *         false otherwise
 ******************************************************************************/
bool wfx_hw_ready(void)
{
    return (wifiContext.state & SL_WFX_STARTED) ? true : false;
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/*****************************************************************************
 * @brief
 *    function called when dhcp got ipv4
 * @param[in]  ip : internet protocol
 ******************************************************************************/
void wfx_dhcp_got_ipv4(uint32_t ip)
{
    /* Acquire the new IP address
     */
    sta_ip = ip;
    wfx_ip_changed_notify(IP_STATUS_SUCCESS);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/*****************************************************************************
 * @brief
 *    function called from connectivityManager
 ******************************************************************************/
void wfx_enable_sta_mode(void)
{
    /* Nothing to do - default is that it is
       place holder */
}

/****************************************************************************
 * @brief
 * driver scan start
 * @param[in]  callback: Callback from the wifi scan  results
 * @return returns true if sucessful,
 *         false otherwise
 *****************************************************************************/
#ifdef SL_WFX_CONFIG_SCAN
bool wfx_start_scan(char * ssid, void (*callback)(wfx_wifi_scan_result_t *))
{
    int sz;

    if (scan_cb)
        return false; /* Already in progress */
    if (ssid)
    {
        sz = strlen(ssid);
        if ((scan_ssid = (char *) pvPortMalloc(sz + 1)) == (char *) 0)
        {
            return false;
        }
        strcpy(scan_ssid, ssid);
    }
    scan_cb = callback;
    xEventGroupSetBits(sl_wfx_event_group, SL_WFX_SCAN_START);

    return true;
}

/****************************************************************************
 * @brief
 * driver scan cancelation
 *****************************************************************************/
void wfx_cancel_scan(void)
{
    struct scan_result_holder *hp, *next;
    /* Not possible */
    if (!scan_cb)
    {
        return;
    }
    sl_wfx_send_stop_scan_command();
    for (hp = scan_save; hp; hp = next)
    {
        next = hp->next;
        vPortFree(hp);
    }
    scan_save  = (struct scan_result_holder *) 0;
    scan_count = 0;
    if (scan_ssid)
    {
        vPortFree(scan_ssid);
        scan_ssid = (char *) 0;
    }
    scan_cb = 0;
}
#endif /* SL_WFX_CONFIG_SCAN */
