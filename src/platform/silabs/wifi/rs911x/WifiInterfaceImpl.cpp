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

#include "sl_status.h"
#include <cmsis_os2.h>
#include <inet/IPAddress.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <platform/silabs/wifi/WifiInterface.h>
#include <platform/silabs/wifi/lwip-support/dhcp_client.h>
#include <platform/silabs/wifi/lwip-support/ethernetif.h>
#include <platform/silabs/wifi/lwip-support/lwip_netif.h>
#include <platform/silabs/wifi/rs911x/WifiInterfaceImpl.h>

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

using namespace chip::DeviceLayer::Silabs;

namespace {

osThreadId_t sDrvThread;
constexpr uint32_t kDrvTaskSize = 1792;
uint8_t drvStack[kDrvTaskSize];
osThread_t sDrvTaskControlBlock;
osThreadAttr_t kDrvTaskAttr = { .name       = "drv_rsi",
                                .attr_bits  = osThreadDetached,
                                .cb_mem     = &sDrvTaskControlBlock,
                                .cb_size    = osThreadCbSize,
                                .stack_mem  = drvStack,
                                .stack_size = kDrvTaskSize,
                                .priority   = osPriorityHigh };

osMessageQueueId_t sWifiEventQueue = NULL;

//  DHCP Polling interval for the IPv4/IPv6
constexpr uint32_t kDhcpPollIntervalMs = 250;

uint8_t wfx_rsi_drv_buf[WFX_RSI_BUF_SZ];
wfx_wifi_scan_ext_t temp_reset;

/******************************************************************
 * @fn   void rsi_wireless_driver_task_wrapper(void * argument)
 * @brief
 *       wrapper thread for the driver task
 * @param[in] argument: argument
 * @return
 *       None
 *********************************************************************/
void rsi_wireless_driver_task_wrapper(void * argument)
{
    rsi_wireless_driver_task();
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
void wfx_rsi_wlan_pkt_cb(uint16_t status, uint8_t * buf, uint32_t len)
{
    if (status != RSI_SUCCESS)
    {
        return;
    }
    wfx_host_received_sta_frame_cb(buf, len);
}

/***************************************************************************************
 * @fn   static void wfx_rsi_save_ap_info(void)
 * @brief
 *       Saving the details of the AP
 * @param[in]  None
 * @return
 *       None
 *******************************************************************************************/
void wfx_rsi_save_ap_info(void) // translation
{
    int32_t status;
    rsi_rsp_scan_t rsp;

    status = rsi_wlan_scan_with_bitmap_options((int8_t *) &wfx_rsi.credentials.ssid[0], AP_CHANNEL_NO_0, &rsp, sizeof(rsp),
                                               SCAN_BITMAP_OPTN_1);
    if (status != RSI_SUCCESS)
    {
        /*
         * Scan is done - failed
         */
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        wfx_rsi.credentials.security = WFX_SEC_WPA3;
#else  /* !WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        wfx_rsi.credentials.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        ChipLogProgress(DeviceLayer, "warn: scan failed: %ld", status);
        return;
    }
    wfx_rsi.credentials.security = WFX_SEC_UNSPECIFIED;
    wfx_rsi.ap_chan              = rsp.scan_info->rf_channel;
    memcpy(wfx_rsi.ap_mac.data(), &rsp.scan_info->bssid[0], kWifiMacAddressLength);

    switch (rsp.scan_info->security_mode)
    {
    case SME_OPEN:
        wfx_rsi.credentials.security = WFX_SEC_NONE;
        break;
    case SME_WPA:
    case SME_WPA_ENTERPRISE:
        wfx_rsi.credentials.security = WFX_SEC_WPA;
        break;
    case SME_WPA2:
    case SME_WPA2_ENTERPRISE:
        wfx_rsi.credentials.security = WFX_SEC_WPA2;
        break;
    case SME_WPA_WPA2_MIXED_MODE:
        wfx_rsi.credentials.security = WFX_SEC_WPA_WPA2_MIXED;
        break;
    case SME_WEP:
        wfx_rsi.credentials.security = WFX_SEC_WEP;
        break;
    case SME_WPA3_PERSONAL_TRANSITION:
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
    case SME_WPA3_PERSONAL:
        wfx_rsi.credentials.security = WFX_SEC_WPA3;
#else
        wfx_rsi.credentials.security = WFX_SEC_WPA2;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
        break;
    default:
        wfx_rsi.credentials.security = WFX_SEC_UNSPECIFIED;
        break;
    }

    ChipLogProgress(DeviceLayer, "wfx_rsi_save_ap_info: connecting to %s, sec=%d, status=%ld", &wfx_rsi.credentials.ssid[0],
                    wfx_rsi.credentials.security, status);
}

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {

WifiInterfaceImpl WifiInterfaceImpl::mInstance;

WifiInterface & WifiInterface::GetInstance()
{
    return WifiInterfaceImpl::GetInstance();
}

WiseconnectWifiInterface & WiseconnectWifiInterface::GetInstance()
{
    return WifiInterfaceImpl::GetInstance();
}

sl_status_t WifiInterfaceImpl::TriggerPlatformWifiDisconnection()
{
    VerifyOrReturnError(rsi_wlan_disconnect() == RSI_SUCCESS, SL_STATUS_FAIL);
    return SL_STATUS_OK;
}

CHIP_ERROR WifiInterfaceImpl::GetAccessPointInfo(wfx_wifi_scan_result_t & info)
{
    int32_t status = RSI_SUCCESS;
    uint8_t rssi   = 0;

    info.security = wfx_rsi.credentials.security;
    info.chan     = wfx_rsi.ap_chan;
    memcpy(&(info.bssid[0]), wfx_rsi.ap_mac.data(), kWifiMacAddressLength);

    status = rsi_wlan_get(RSI_RSSI, &rssi, sizeof(rssi));
    VerifyOrReturnError(status = RSI_SUCCESS, CHIP_ERROR_INTERNAL);

    info.rssi = (-1) * rssi;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info)
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

CHIP_ERROR WifiInterfaceImpl::ResetCounters()
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
CHIP_ERROR WifiInterfaceImpl::ConfigurePowerSave()
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

CHIP_ERROR WifiInterfaceImpl::ConfigureBroadcastFilter(bool enableBroadcastFilter)
{
    // TODO: Implement Broadcast filtering. We do a silent failure to avoid causing problems in higher layers.
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

CHIP_ERROR WifiInterfaceImpl::InitWiFiStack(void)
{
    int32_t status = Rs911xPlatformInit();
    VerifyOrReturnError(status == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "Rs911xPlatformInit failed: %lx", status));
    return CHIP_NO_ERROR;
}

void WifiInterfaceImpl::PostWifiPlatformEvent(WifiPlatformEvent event)
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
void WifiInterfaceImpl::ProcessEvent(WifiPlatformEvent event)
{
    // Process event
    switch (event)
    {
    case WiseconnectWifiInterface::WifiPlatformEvent::kStationConnect: {
        ChipLogDetail(DeviceLayer, "WiseconnectWifiInterface::WifiPlatformEvent::kStationConnect");
        wfx_rsi.dev_state.Set(WifiInterface::WifiState::kStationConnected);
        ResetConnectivityNotificationFlags();
        chip::DeviceLayer::Silabs::Lwip::SetLwipStationLinkUp();
    }
    break;
    case WiseconnectWifiInterface::WifiPlatformEvent::kStationDisconnect: {
        ChipLogDetail(DeviceLayer, "WiseconnectWifiInterface::WifiPlatformEvent::kStationDisconnect");

        wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationReady)
            .Clear(WifiInterface::WifiState::kStationConnecting)
            .Clear(WifiInterface::WifiState::kStationConnected)
            .Clear(WifiInterface::WifiState::kStationDhcpDone);

        /* TODO: Implement disconnect notify */
        ResetConnectivityNotificationFlags();
        chip::DeviceLayer::Silabs::Lwip::SetLwipStationLinkDown();

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
        NotifyIPv4Change(false);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        NotifyIPv6Change(false);
    }
    break;
    case WiseconnectWifiInterface::WifiPlatformEvent::kAPStart:
        // TODO: Currently unimplemented
        break;
    case WiseconnectWifiInterface::WifiPlatformEvent::kScan: {
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

            VerifyOrDie(sizeof(ap.bssid) == kWifiMacAddressLength);
            VerifyOrDie(sizeof(scan.bssid) == kWifiMacAddressLength);

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
    case WiseconnectWifiInterface::WifiPlatformEvent::kStationStartJoin: {
        // saving the AP related info
        wfx_rsi_save_ap_info();
        // Joining to the network
        JoinWifiNetwork();
    }
    break;
    case WiseconnectWifiInterface::WifiPlatformEvent::kConnectionComplete: {
        StartDHCPTimer(kDhcpPollIntervalMs);
    }
    break;
    case WiseconnectWifiInterface::WifiPlatformEvent::kStationDhcpDone: {
        CancelDHCPTimer();
    }
    break;
    case WiseconnectWifiInterface::WifiPlatformEvent::kStationDhcpPoll: {
        HandleDHCPPolling();
    }
    break;
    default:
        break;
    }
}

void WifiInterfaceImpl::JoinCallback(uint16_t status, const uint8_t * buf, const uint16_t len)
{
    wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationConnecting);
    if (status != RSI_SUCCESS)
    {
        WifiInterfaceImpl::GetInstance().ScheduleConnectionAttempt();
        return;
    }

    /*
     * Join was complete - Do the DHCP
     */
    ChipLogProgress(DeviceLayer, "JoinCallback: success");
    memset(&temp_reset, 0, sizeof(wfx_wifi_scan_ext_t));
    WifiPlatformEvent event = WiseconnectWifiInterface::WifiPlatformEvent::kStationConnect;
    WifiInterfaceImpl::GetInstance().PostWifiPlatformEvent(event);
}

void WifiInterfaceImpl::JoinFailCallback(uint16_t status, uint8_t * buf, uint32_t len)
{
    ChipLogError(DeviceLayer, "JoinFailCallback: status: %d", status);

    wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationConnecting).Clear(WifiInterface::WifiState::kStationConnected);

    WiseconnectWifiInterface::WifiPlatformEvent event = WiseconnectWifiInterface::WifiPlatformEvent::kStationStartJoin;
    WifiInterfaceImpl::GetInstance().PostWifiPlatformEvent(event);
}

int32_t WifiInterfaceImpl::Rs911xPlatformInit(void)
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
    sWifiEventQueue = osMessageQueueNew(WFX_QUEUE_SIZE, sizeof(WiseconnectWifiInterface::WifiPlatformEvent), NULL);
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
    if ((status = rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, WifiInterfaceImpl::JoinFailCallback)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_wlan_register_callbacks failed: %ld", status);
        return status;
    }
    if ((status = rsi_wlan_register_callbacks(RSI_WLAN_DATA_RECEIVE_NOTIFY_CB, wfx_rsi_wlan_pkt_cb)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_wlan_register_callbacks failed: %ld", status);
        return status;
    }

    wfx_rsi.dev_state.Set(WifiInterface::WifiState::kStationInit);
    return RSI_SUCCESS;
}

void WifiInterfaceImpl::JoinWifiNetwork(void)
{
    sl_status_t status = SL_STATUS_OK;
    rsi_security_mode_t connect_security_mode;

    VerifyOrReturn(
        !wfx_rsi.dev_state.HasAny(WifiInterface::WifiState::kStationConnecting, WifiInterface::WifiState::kStationConnected));

    switch (wfx_rsi.credentials.security)
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
        ChipLogError(DeviceLayer, "JoinWifiNetwork: error: unknown security type.");
        return;
    }

    ChipLogProgress(DeviceLayer, "JoinWifiNetwork: connecting to %s, sec=%d", &wfx_rsi.credentials.ssid[0],
                    wfx_rsi.credentials.security);

    /*
     * Join the network
     */
    /* TODO - make the WFX_SECURITY_xxx - same as RSI_xxx
     * Right now it's done by hand - we need something better
     */
    wfx_rsi.dev_state.Set(WifiInterface::WifiState::kStationConnecting);

    if ((status = rsi_wlan_register_callbacks(RSI_JOIN_FAIL_CB, WifiInterfaceImpl::JoinFailCallback)) != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "JoinWifiNetwork: rsi_wlan_register_callbacks failed: %ld", status);
    }

    /* Try to connect Wifi with given Credentials
     * until there is a success or maximum number of tries allowed
     */
    if ((status = rsi_wlan_connect_async((int8_t *) &wfx_rsi.credentials.ssid[0], connect_security_mode,
                                         &wfx_rsi.credentials.passkey[0], JoinCallback)) != RSI_SUCCESS)
    {
        wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationConnecting);
        ScheduleConnectionAttempt();
    }
}

void WifiInterfaceImpl::DHCPTimerEventHandler(void * arg)
{
    WifiPlatformEvent event = WiseconnectWifiInterface::WifiPlatformEvent::kStationDhcpPoll;
    WifiInterfaceImpl::GetInstance().PostWifiPlatformEvent(event);
}

void WifiInterfaceImpl::CancelDHCPTimer(void)
{
    VerifyOrReturn(osTimerIsRunning(mDHCPTimer), ChipLogDetail(DeviceLayer, "CancelDHCPTimer: timer not running"));
    VerifyOrReturn(osTimerStop(mDHCPTimer) == osOK, ChipLogError(DeviceLayer, "CancelDHCPTimer: failed to stop timer"));
}

void WifiInterfaceImpl::StartDHCPTimer(uint32_t timeout)
{
    // Cancel timer if already started
    CancelDHCPTimer();

    VerifyOrReturn(osTimerStart(mDHCPTimer, pdMS_TO_TICKS(timeout)) == osOK,
                   ChipLogError(DeviceLayer, "StartDHCPTimer: failed to start timer"));
}

sl_status_t WifiInterfaceImpl::CreateDHCPTimer()
{
    // TODO: Use LWIP timer instead of creating a new one here
    mDHCPTimer = osTimerNew(DHCPTimerEventHandler, osTimerPeriodic, nullptr, nullptr);
    VerifyOrReturnError(mDHCPTimer != nullptr, SL_STATUS_ALLOCATION_FAILED);

    return SL_STATUS_OK;
}

void WifiInterfaceImpl::HandleDHCPPolling(void)
{
    struct netif * sta_netif;

    sta_netif = chip::DeviceLayer::Silabs::Lwip::GetNetworkInterface(SL_WFX_STA_INTERFACE);
    if (sta_netif == NULL)
    {
        // TODO: Notify the application that the interface is not set up or Chipdie here because we
        // are in an unkonwn state
        ChipLogError(DeviceLayer, "HandleDHCPPolling: failed to get STA netif");
        return;
    }
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    uint8_t dhcp_state = dhcpclient_poll(sta_netif);
    if (dhcp_state == DHCP_ADDRESS_ASSIGNED && !mHasNotifiedIPv4)
    {
        GotIPv4Address((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
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
    if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !mHasNotifiedIPv6)
    {
        char addrStr[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
        VerifyOrReturn(ip6addr_ntoa_r(netif_ip6_addr(sta_netif, 0), addrStr, sizeof(addrStr)) != nullptr);
        ChipLogProgress(DeviceLayer, "SLAAC OK: linklocal addr: %s", addrStr);
        NotifyIPv6Change(true);
        WifiPlatformEvent event = WiseconnectWifiInterface::WifiPlatformEvent::kStationDhcpDone;
        PostWifiPlatformEvent(event);
        NotifyConnectivity();
    }
}

void WiseconnectWifiInterface::MatterWifiTask(void * arg)
{
    (void) arg;
    WifiPlatformEvent event;
    chip::DeviceLayer::Silabs::Lwip::InitializeLwip();
    WifiInterfaceImpl::GetInstance().NotifyWifiTaskInitialized();

    ChipLogProgress(DeviceLayer, "MatterWifiTask: starting event loop");
    for (;;)
    {
        osStatus_t status = osMessageQueueGet(sWifiEventQueue, &event, NULL, osWaitForever);
        if (status == osOK)
        {
            WifiInterfaceImpl::GetInstance().ProcessEvent(event);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "MatterWifiTask: get event failed: %x", status);
        }
    }
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
