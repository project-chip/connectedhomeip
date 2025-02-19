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

#include "lwip/nd6.h"
#include "silabs_utils.h"
#include "sl_status.h"
#include <cmsis_os2.h>
#include <inet/IPAddress.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>
#include <platform/silabs/wifi/lwip-support/dhcp_client.h>
#include <platform/silabs/wifi/lwip-support/ethernetif.h>
#include <platform/silabs/wifi/wiseconnect-interface/WiseconnectWifiInterface.h>

extern "C" {
#include "rsi_bootup_config.h"
#include "rsi_common_apis.h"
#include "rsi_data_types.h"
#include "rsi_driver.h"
#include "rsi_error.h"
#include "rsi_nwk.h"
#include "rsi_socket.h"
#include "rsi_utils.h"
#include "rsi_wlan.h"
#include "rsi_wlan_apis.h"
#include "rsi_wlan_config.h"
#include "rsi_wlan_non_rom.h"
}

using WifiStateFlags = chip::BitFlags<WifiState>;

#define WFX_QUEUE_SIZE 10
#define WFX_RSI_BUF_SZ (1024 * 10)
#define RSI_RESPONSE_MAX_SIZE (28)
#define RSI_RESPONSE_HOLD_BUFF_SIZE (128)
#define RSI_DRIVER_STATUS (0)
#define OPER_MODE_0 (0)
#define COEX_MODE_0 (0)
#define RESP_BUFF_SIZE (6)
#define AP_CHANNEL_NO_0 (0)
#define SCAN_BITMAP_OPTN_1 (1)

WfxRsi_t wfx_rsi;

static osThreadId_t sDrvThread;
constexpr uint32_t kDrvTaskSize = 1792;
static uint8_t drvStack[kDrvTaskSize];
static osThread_t sDrvTaskControlBlock;
osThreadAttr_t kDrvTaskAttr = { .name       = "drv_rsi",
                                .attr_bits  = osThreadDetached,
                                .cb_mem     = &sDrvTaskControlBlock,
                                .cb_size    = osThreadCbSize,
                                .stack_mem  = drvStack,
                                .stack_size = kDrvTaskSize,
                                .priority   = osPriorityHigh };

static osMessageQueueId_t sWifiEventQueue = NULL;
/*
 * This file implements the interface to the RSI SAPIs
 */
static uint8_t wfx_rsi_drv_buf[WFX_RSI_BUF_SZ];
static wfx_wifi_scan_ext_t temp_reset;

sl_status_t TriggerPlatformWifiDisconnection()
{
    VerifyOrReturnError(rsi_wlan_disconnect() == RSI_SUCCESS, SL_STATUS_FAIL);
    return SL_STATUS_OK;
}

CHIP_ERROR GetAccessPointInfo(wfx_wifi_scan_result_t & info)
{
    int32_t status = RSI_SUCCESS;
    uint8_t rssi   = 0;

    info.security = wfx_rsi.sec.security;
    info.chan     = wfx_rsi.ap_chan;
    memcpy(&(info.bssid[0]), wfx_rsi.ap_mac.data(), kWifiMacAddressLength);

    status = rsi_wlan_get(RSI_RSSI, &rssi, sizeof(rssi));
    VerifyOrReturnError(status = RSI_SUCCESS, CHIP_ERROR_INTERNAL);

    info.rssi = (-1) * rssi;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info)
{
    rsi_wlan_ext_stats_t stats = { 0 };

    int32_t status = rsi_wlan_get(RSI_WLAN_EXT_STATS, reinterpret_cast<uint8_t *>(&stats), sizeof(stats));
    VerifyOrReturnError(status == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Failed, Error Code : 0x%lX", status));

    info.beacon_lost_count = stats.beacon_lost_count - temp_reset.beacon_lost_count;
    info.beacon_rx_count   = stats.beacon_rx_count - temp_reset.beacon_rx_count;
    info.mcast_rx_count    = stats.mcast_rx_count - temp_reset.mcast_rx_count;
    info.mcast_tx_count    = stats.mcast_tx_count - temp_reset.mcast_tx_count;
    info.ucast_rx_count    = stats.ucast_rx_count - temp_reset.ucast_rx_count;
    info.ucast_tx_count    = stats.ucast_tx_count - temp_reset.ucast_tx_count;
    info.overrun_count     = stats.overrun_count - temp_reset.overrun_count;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ResetCounters()
{
    rsi_wlan_ext_stats_t stats = { 0 };

    int32_t status = rsi_wlan_get(RSI_WLAN_EXT_STATS, reinterpret_cast<uint8_t *>(&stats), sizeof(stats));
    VerifyOrReturnError(status == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Failed, Error Code : 0x%lX", status));

    temp_reset.beacon_lost_count = stats.beacon_lost_count;
    temp_reset.beacon_rx_count   = stats.beacon_rx_count;
    temp_reset.mcast_rx_count    = stats.mcast_rx_count;
    temp_reset.mcast_tx_count    = stats.mcast_tx_count;
    temp_reset.ucast_rx_count    = stats.ucast_rx_count;
    temp_reset.ucast_tx_count    = stats.ucast_tx_count;
    temp_reset.overrun_count     = stats.overrun_count;

    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
CHIP_ERROR ConfigurePowerSave()
{
    int32_t status = RSI_SUCCESS;
#ifdef RSI_BLE_ENABLE
    status = rsi_bt_power_save_profile(RSI_SLEEP_MODE_2, RSI_MAX_PSP);
    VerifyOrReturnError(status == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "BT Powersave Config Failed, Error Code : 0x%lX", status));
#endif /* RSI_BLE_ENABLE */

    status = rsi_wlan_power_save_profile(RSI_SLEEP_MODE_2, RSI_MAX_PSP);
    VerifyOrReturnError(status == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "WLAN Powersave Config Failed, Error Code : 0x%lX", status));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter)
{
    // TODO: Implement Broadcast filtering. We do a silent failure to avoid causing problems in higher layers.
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

/******************************************************************
 * @fn   void rsi_wireless_driver_task_wrapper(void * argument)
 * @brief
 *       wrapper thread for the driver task
 * @param[in] argument: argument
 * @return
 *       None
 *********************************************************************/
static void rsi_wireless_driver_task_wrapper(void * argument)
{
    rsi_wireless_driver_task();
}

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
    wfx_rsi.dev_state.Clear(WifiState::kStationConnecting);
    if (status != RSI_SUCCESS)
    {
        /*
         * We should enable retry.. (Need config variable for this)
         */
        ChipLogProgress(DeviceLayer, "wfx_rsi_join_cb: failed. retry: %d", wfx_rsi.join_retries);
        wfx_retry_connection(++wfx_rsi.join_retries);
        return;
    }

    /*
     * Join was complete - Do the DHCP
     */
    ChipLogProgress(DeviceLayer, "wfx_rsi_join_cb: success");
    memset(&temp_reset, 0, sizeof(wfx_wifi_scan_ext_t));
    WifiPlatformEvent event = WifiPlatformEvent::kStationConnect;
    PostWifiPlatformEvent(event);
    wfx_rsi.join_retries = 0;
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
    ChipLogError(DeviceLayer, "wfx_rsi_join_fail_cb: status: %d", status);
    wfx_rsi.join_retries += 1;

    wfx_rsi.dev_state.Clear(WifiState::kStationConnecting).Clear(WifiState::kStationConnected);

    WifiPlatformEvent event = WifiPlatformEvent::kStationStartJoin;
    PostWifiPlatformEvent(event);
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
 * @fn  static int32_t sl_matter_wifi_init(void)
 * @brief
 *      driver initialization
 * @param[in]  None
 * @return
 *        None
 *****************************************************************************************/
static int32_t sl_matter_wifi_init(void)
{
    int32_t status;
    uint8_t buf[RSI_RESPONSE_HOLD_BUFF_SIZE];
    extern void rsi_hal_board_init(void);

    //! Driver initialization
    status = rsi_driver_init(wfx_rsi_drv_buf, WFX_RSI_BUF_SZ);
    if ((status < RSI_DRIVER_STATUS) || (status > WFX_RSI_BUF_SZ))
    {
        ChipLogError(DeviceLayer, "rsi_driver_init failed: %ld", status);
        return status;
    }
    /* ! Redpine module intialisation */
    if ((status = rsi_device_init(LOAD_NWP_FW)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_device_init failed: %ld", status);
        return status;
    }
    /*
     * Create the driver wrapper thread
     */
    sDrvThread = osThreadNew(rsi_wireless_driver_task_wrapper, NULL, &kDrvTaskAttr);
    if (NULL == sDrvThread)
    {
        ChipLogError(DeviceLayer, "failed to create task");
        return RSI_ERROR_INVALID_PARAM;
    }

#if (RSI_BLE_ENABLE)
    if ((status = rsi_wireless_init(OPER_MODE_0, RSI_OPERMODE_WLAN_BLE)) != RSI_SUCCESS)
    {
#else
    if ((status = rsi_wireless_init(OPER_MODE_0, COEX_MODE_0)) != RSI_SUCCESS)
    {
#endif
        ChipLogError(DeviceLayer, "rsi_wireless_init failed: %ld", status);
        return status;
    }

    /*
     * Get the MAC and other info to let the user know about it.
     */
    if (rsi_wlan_get(RSI_FW_VERSION, buf, sizeof(buf)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_wlan_get(RSI_FW_VERSION) failed: %ld", status);
        return status;
    }

    buf[sizeof(buf) - 1] = 0;
    ChipLogProgress(DeviceLayer, "RSI firmware version: %s", buf);
    //! Send feature frame
    if ((status = rsi_send_feature_frame()) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "error: rsi_send_feature_frame failed: %ld", status);
        return status;
    }

    /* initializes wlan radio parameters and WLAN supplicant parameters.
     */
    (void) rsi_wlan_radio_init(); /* Required so we can get MAC address */
    if ((status = rsi_wlan_get(RSI_MAC_ADDRESS, wfx_rsi.sta_mac.data(), RESP_BUFF_SIZE)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_wlan_get(RSI_MAC_ADDRESS) failed: %ld", status);
        return status;
    }

    ChipLogDetail(DeviceLayer, "MAC: %02x:%02x:%02x %02x:%02x:%02x", wfx_rsi.sta_mac.at(0), wfx_rsi.sta_mac.at(1),
                  wfx_rsi.sta_mac.at(2), wfx_rsi.sta_mac.at(3), wfx_rsi.sta_mac.at(4), wfx_rsi.sta_mac.at(5));

    // Create the message queue
    sWifiEventQueue = osMessageQueueNew(WFX_QUEUE_SIZE, sizeof(WifiPlatformEvent), NULL);
    if (sWifiEventQueue == NULL)
    {
        return SL_STATUS_ALLOCATION_FAILED;
    }

    // TODO: Use LWIP timer instead of creating a new one here
    status = CreateDHCPTimer();
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    /*
     * Register callbacks - We are only interested in the connectivity CBs
     */
    if ((status = rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, wfx_rsi_join_fail_cb)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_wlan_register_callbacks failed: %ld", status);
        return status;
    }
    if ((status = rsi_wlan_register_callbacks(RSI_WLAN_DATA_RECEIVE_NOTIFY_CB, wfx_rsi_wlan_pkt_cb)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_wlan_register_callbacks failed: %ld", status);
        return status;
    }

    wfx_rsi.dev_state.Set(WifiState::kStationInit);
    return RSI_SUCCESS;
}

/***************************************************************************************
 * @fn   static void wfx_rsi_save_ap_info(void)
 * @brief
 *       Saving the details of the AP
 * @param[in]  None
 * @return
 *       None
 *******************************************************************************************/
static void wfx_rsi_save_ap_info(void) // translation
{
    int32_t status;
    rsi_rsp_scan_t rsp;

    status =
        rsi_wlan_scan_with_bitmap_options((int8_t *) &wfx_rsi.sec.ssid[0], AP_CHANNEL_NO_0, &rsp, sizeof(rsp), SCAN_BITMAP_OPTN_1);
    if (status != RSI_SUCCESS)
    {
        /*
         * Scan is done - failed
         */
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else  /* !WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        ChipLogProgress(DeviceLayer, "warn: scan failed: %ld", status);
        return;
    }
    wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
    wfx_rsi.ap_chan      = rsp.scan_info->rf_channel;
    memcpy(wfx_rsi.ap_mac.data(), &rsp.scan_info->bssid[0], kWifiMacAddressLength);

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
    case SME_WPA_WPA2_MIXED_MODE:
        wfx_rsi.sec.security = WFX_SEC_WPA_WPA2_MIXED;
        break;
    case SME_WEP:
        wfx_rsi.sec.security = WFX_SEC_WEP;
        break;
    case SME_WPA3_PERSONAL_TRANSITION:
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case SME_WPA3_PERSONAL:
        wfx_rsi.sec.security = WFX_SEC_WPA3;
#else
        wfx_rsi.sec.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        break;
    default:
        wfx_rsi.sec.security = WFX_SEC_UNSPECIFIED;
        break;
    }

    ChipLogProgress(DeviceLayer, "wfx_rsi_save_ap_info: connecting to %s, sec=%d, status=%ld", &wfx_rsi.sec.ssid[0],
                    wfx_rsi.sec.security, status);
}

/********************************************************************************************
 * @fn   static void sl_wifi_platform_join_network(void)
 * @brief
 *        Start an async Join command
 * @return
 *        None
 **********************************************************************************************/
static void sl_wifi_platform_join_network(void)
{
    sl_status_t status = SL_STATUS_OK;
    rsi_security_mode_t connect_security_mode;

    VerifyOrReturn(!wfx_rsi.dev_state.HasAny(WifiState::kStationConnecting, WifiState::kStationConnected));

    switch (wfx_rsi.sec.security)
    {
    case WFX_SEC_WEP:
        connect_security_mode = RSI_WEP;
        break;
    case WFX_SEC_WPA:
    case WFX_SEC_WPA2:
    case WFX_SEC_WPA_WPA2_MIXED:
        connect_security_mode = RSI_WPA_WPA2_MIXED;
        break;
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case WFX_SEC_WPA3:
        connect_security_mode = RSI_WPA3_PERSONAL_TRANSITION;
        break;
#endif // WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case WFX_SEC_NONE:
        connect_security_mode = RSI_OPEN;
        break;
    default:
        ChipLogError(DeviceLayer, "sl_wifi_platform_join_network: error: unknown security type.");
        return;
    }

    ChipLogProgress(DeviceLayer, "sl_wifi_platform_join_network: connecting to %s, sec=%d", &wfx_rsi.sec.ssid[0],
                    wfx_rsi.sec.security);

    /*
     * Join the network
     */
    /* TODO - make the WFX_SECURITY_xxx - same as RSI_xxx
     * Right now it's done by hand - we need something better
     */
    wfx_rsi.dev_state.Set(WifiState::kStationConnecting);

    if ((status = rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, wfx_rsi_join_fail_cb)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "sl_wifi_platform_join_network: rsi_wlan_register_callbacks failed: %ld", status);
    }

    /* Try to connect Wifi with given Credentials
     * until there is a success or maximum number of tries allowed
     */
    if ((status = rsi_wlan_connect_async((int8_t *) &wfx_rsi.sec.ssid[0], connect_security_mode, &wfx_rsi.sec.passkey[0],
                                         wfx_rsi_join_cb)) != RSI_SUCCESS)
    {
        wfx_rsi.dev_state.Clear(WifiState::kStationConnecting);
        ChipLogProgress(DeviceLayer, "sl_wifi_platform_join_network: rsi_wlan_connect_async failed: %ld on try %d", status,
                        wfx_rsi.join_retries);
        wfx_retry_connection(++wfx_rsi.join_retries);
    }
}

void HandleDHCPPolling(void)
{
    struct netif * sta_netif;

    sta_netif = wfx_get_netif(SL_WFX_STA_INTERFACE);
    if (sta_netif == NULL)
    {
        // TODO: Notify the application that the interface is not set up or Chipdie here because we
        // are in an unkonwn state
        ChipLogError(DeviceLayer, "HandleDHCPPolling: failed to get STA netif");
        return;
    }
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    uint8_t dhcp_state = dhcpclient_poll(sta_netif);
    if (dhcp_state == DHCP_ADDRESS_ASSIGNED && !HasNotifiedIPv4Change())
    {
        wfx_dhcp_got_ipv4((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
        NotifyIPv4Change(true);
        NotifyConnectivity();
    }
    else if (dhcp_state == DHCP_OFF)
    {
        NotifyIPv4Change(false);
    }
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
    /* Checks if the assigned IPv6 address is preferred by evaluating
     * the first block of IPv6 address ( block 0)
     */
    if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !HasNotifiedIPv6Change())
    {
        char addrStr[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
        VerifyOrReturn(ip6addr_ntoa_r(netif_ip6_addr(sta_netif, 0), addrStr, sizeof(addrStr)) != nullptr);
        ChipLogProgress(DeviceLayer, "SLAAC OK: linklocal addr: %s", addrStr);
        NotifyIPv6Change(true);
        WifiPlatformEvent event = WifiPlatformEvent::kStationDhcpDone;
        PostWifiPlatformEvent(event);
        NotifyConnectivity();
    }
}

void PostWifiPlatformEvent(WifiPlatformEvent event)
{
    sl_status_t status = osMessageQueuePut(sWifiEventQueue, &event, 0, 0);

    if (status != osOK)
    {
        ChipLogError(DeviceLayer, "PostWifiPlatformEvent: failed to post event with status: %ld", status);
        // TODO: Handle error, requeue event depending on queue size or notify relevant task,
        // Chipdie, etc.
    }
}

/**
 * @brief Process the Wi-Fi event.
 *
 * This function is responsible for processing different types of Wi-Fi events and taking appropriate actions based on the event
 * type.
 *
 * @param event The input Wi-Fi event to be processed.
 */
void ProcessEvent(WifiPlatformEvent event)
{
    // Process event
    switch (event)
    {
    case WifiPlatformEvent::kStationConnect: {
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationConnect");
        wfx_rsi.dev_state.Set(WifiState::kStationConnected);
        ResetDHCPNotificationFlags();
        wfx_lwip_set_sta_link_up();
    }
    break;
    case WifiPlatformEvent::kStationDisconnect: {
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationDisconnect");
        // TODO: This event is not being posted anywhere, seems to be a dead code or we are missing something
        WifiStateFlags flagsToClear = WifiStateFlags(WifiState::kStationReady, WifiState::kStationConnecting,
                                                     WifiState::kStationConnected, WifiState::kStationDhcpDone);
        wfx_rsi.dev_state.Clear(flagsToClear);
        /* TODO: Implement disconnect notify */
        ResetDHCPNotificationFlags();
        wfx_lwip_set_sta_link_down();

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
        NotifyIPv4Change(false);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        NotifyIPv6Change(false);
    }
    break;
    case WifiPlatformEvent::kAPStart:
        // TODO: Currently unimplemented
        break;
    case WifiPlatformEvent::kScan: {
        rsi_rsp_scan_t scan_rsp = { 0 };
        int32_t status          = rsi_wlan_bgscan_profile(1, &scan_rsp, sizeof(scan_rsp));

        VerifyOrReturn(status == RSI_SUCCESS, ChipLogError(DeviceLayer, "rsi_wlan_bgscan_profile failed: %ld", status));
        VerifyOrReturn(wfx_rsi.scan_cb != nullptr, ChipLogError(DeviceLayer, "wfx_rsi.scan_cb is nullptr"));

        uint8_t nbreOfScannedNetworks = scan_rsp.scan_count[0];
        for (int i = 0; i < nbreOfScannedNetworks; i++)
        {
            rsi_scan_info_t scan      = scan_rsp.scan_info[i];
            wfx_wifi_scan_result_t ap = { 0 };

            ap.ssid_length = strnlen(reinterpret_cast<char *>(scan.ssid), WFX_MAX_SSID_LENGTH);

            chip::ByteSpan scannedSsid(scan.ssid, ap.ssid_length);
            chip::MutableByteSpan outputSsid(ap.ssid, WFX_MAX_SSID_LENGTH);
            chip::CopySpanToMutableSpan(scannedSsid, outputSsid);

            // Check if the scanned ssid is the requested Ssid
            chip::ByteSpan requestedSsid(wfx_rsi.scan_ssid, wfx_rsi.scan_ssid_length);
            if (!requestedSsid.empty() && !requestedSsid.data_equal(scannedSsid))
            {
                // Scanned SSID entry does not match the requested SSID. Continue to the next.
                continue;
            }

            // TODO: convert security mode from RSI to WFX
            ap.security = static_cast<wfx_sec_t>(scan.security_mode);
            ap.rssi     = (-1) * scan.rssi_val;

            configASSERT(sizeof(ap.bssid) == kWifiMacAddressLength);
            configASSERT(sizeof(scan.bssid) == kWifiMacAddressLength);

            chip::MutableByteSpan bssidSpan(ap.bssid, kWifiMacAddressLength);
            chip::ByteSpan scanBssidSpan(scan.bssid, kWifiMacAddressLength);
            chip::CopySpanToMutableSpan(scanBssidSpan, bssidSpan);

            wfx_rsi.scan_cb(&ap);

            // If we reach this and the requestedSsid is not empty, it means we found the requested SSID and we can exit
            if (!requestedSsid.empty())
            {
                break;
            }
        }
        // Notify the stack that we have finishes scanning for Networks
        wfx_rsi.scan_cb(nullptr);

        // Clean up
        wfx_rsi.scan_cb = nullptr;
        if (wfx_rsi.scan_ssid)
        {
            chip::Platform::MemoryFree(wfx_rsi.scan_ssid);
            wfx_rsi.scan_ssid = NULL;
        }
    }
    break;
    case WifiPlatformEvent::kStationStartJoin: {
        // saving the AP related info
        wfx_rsi_save_ap_info();
        // Joining to the network
        sl_wifi_platform_join_network();
    }
    break;
    case WifiPlatformEvent::kStationDoDhcp: {
        StartDHCPTimer(WFX_RSI_DHCP_POLL_INTERVAL);
    }
    break;
    case WifiPlatformEvent::kStationDhcpDone: {
        CancelDHCPTimer();
    }
    break;
    case WifiPlatformEvent::kStationDhcpPoll: {
        HandleDHCPPolling();
    }
    break;
    default:
        break;
    }
}

CHIP_ERROR InitWiFiStack(void)
{
    int32_t status = sl_matter_wifi_init();
    VerifyOrReturnError(status == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "sl_matter_wifi_init failed: %lx", status));
    return CHIP_NO_ERROR;
}

void MatterWifiTask(void * arg)
{
    (void) arg;
    WifiPlatformEvent event;
    sl_matter_lwip_start();
    sl_matter_wifi_task_started();

    ChipLogProgress(DeviceLayer, "MatterWifiTask: starting event loop");
    for (;;)
    {
        osStatus_t status = osMessageQueueGet(sWifiEventQueue, &event, NULL, osWaitForever);
        if (status == osOK)
        {
            ProcessEvent(event);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "MatterWifiTask: get event failed: %x", status);
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
    wfx_rsi.ip4_addr[0] = (ip) &0xFF;
    wfx_rsi.ip4_addr[1] = (ip >> 8) & 0xFF;
    wfx_rsi.ip4_addr[2] = (ip >> 16) & 0xFF;
    wfx_rsi.ip4_addr[3] = (ip >> 24) & 0xFF;
    ChipLogProgress(DeviceLayer, "DHCP OK: IP=%d.%d.%d.%d", wfx_rsi.ip4_addr[0], wfx_rsi.ip4_addr[1], wfx_rsi.ip4_addr[2],
                    wfx_rsi.ip4_addr[3]);
    /* Notify the Connectivity Manager - via the app */
    wfx_rsi.dev_state.Set(WifiState::kStationDhcpDone, WifiState::kStationReady);
    NotifyIPv4Change(true);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/*
 * WARNING - Taken from RSI and broken up
 * This is my own RSI stuff for not copying code and allocating an extra
 * level of indirection - when using LWIP buffers
 * see also: int32_t rsi_wlan_send_data_xx(uint8_t *buffer, uint32_t length)
 */
/********************************************************************************************
 * @fn   void *wfx_rsi_alloc_pkt(void)
 * @brief
 *       Allocate packet to send data
 * @param[in] None
 * @return
 *        None
 **********************************************************************************************/
void * wfx_rsi_alloc_pkt(void)
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
    uint8_t * host_desc;
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
