/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/wiseconnect-interface/WiseconnectWifiInterface.h>

extern WfxRsi_t wfx_rsi;

namespace {

// Thread for the WLAN RSI
osThreadId_t sWlanThread;
constexpr uint32_t kWlanTaskSize = 2048;
uint8_t wlanStack[kWlanTaskSize];
osThread_t sWlanTaskControlBlock;
constexpr osThreadAttr_t kWlanTaskAttr = { .name       = "wlan_rsi",
                                           .attr_bits  = osThreadDetached,
                                           .cb_mem     = &sWlanTaskControlBlock,
                                           .cb_size    = osThreadCbSize,
                                           .stack_mem  = wlanStack,
                                           .stack_size = kWlanTaskSize,
                                           .priority   = osPriorityAboveNormal7 };

} // namespace

CHIP_ERROR GetMacAddress(sl_wfx_interface_t interface, chip::MutableByteSpan & address)
{
    VerifyOrReturnError(address.size() >= kWifiMacAddressLength, CHIP_ERROR_BUFFER_TOO_SMALL);

#ifdef SL_WFX_CONFIG_SOFTAP
    chip::ByteSpan byteSpan((interface == SL_WFX_SOFTAP_INTERFACE) ? wfx_rsi.softap_mac : wfx_rsi.sta_mac);
#else
    chip::ByteSpan byteSpan(wfx_rsi.sta_mac);
#endif

    return CopySpanToMutableSpan(byteSpan, address);
}

CHIP_ERROR StartNetworkScan(chip::ByteSpan ssid, ScanCallback callback)
{
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!wfx_rsi.dev_state.Has(WifiState::kScanStarted), CHIP_ERROR_IN_PROGRESS);

    // SSID Max Length that is supported by the Wi-Fi SDK is 32
    VerifyOrReturnError(ssid.size() <= WFX_MAX_SSID_LENGTH, CHIP_ERROR_INVALID_STRING_LENGTH);

    if (ssid.empty()) // Scan all networks
    {
        wfx_rsi.scan_ssid_length = 0;
        wfx_rsi.scan_ssid        = nullptr;
    }
    else // Scan specific SSID
    {
        wfx_rsi.scan_ssid_length = ssid.size();
        wfx_rsi.scan_ssid        = reinterpret_cast<uint8_t *>(chip::Platform::MemoryAlloc(wfx_rsi.scan_ssid_length));
        VerifyOrReturnError(wfx_rsi.scan_ssid != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::MutableByteSpan scanSsidSpan(wfx_rsi.scan_ssid, wfx_rsi.scan_ssid_length);
        chip::CopySpanToMutableSpan(ssid, scanSsidSpan);
    }
    wfx_rsi.scan_cb = callback;

    // TODO: We should be calling the start function directly instead of doing it asynchronously
    WifiPlatformEvent event = WifiPlatformEvent::kScan;
    sl_matter_wifi_post_event(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR StartWifiTask()
{
    // Verify that the Wifi task has not already been started.
    VerifyOrReturnError(!(wfx_rsi.dev_state.Has(WifiState::kStationStarted)), CHIP_NO_ERROR);
    wfx_rsi.dev_state.Set(WifiState::kStationStarted);

    // Creating a Wi-Fi task thread
    sWlanThread = osThreadNew(sl_matter_wifi_task, NULL, &kWlanTaskAttr);
    VerifyOrReturnError(sWlanThread != NULL, CHIP_ERROR_NO_MEMORY, ChipLogError(DeviceLayer, "Unable to create the WifiTask."););

    return CHIP_NO_ERROR;
}

void ConfigureStationMode()
{
    wfx_rsi.dev_state.Set(WifiState::kStationMode);
}

bool IsStationModeEnabled()
{
    return wfx_rsi.dev_state.Has(WifiState::kStationMode);
}

bool IsStationConnected()
{
    return wfx_rsi.dev_state.Has(WifiState::kStationConnected);
}

/*********************************************************************
 * @fn  void wfx_set_wifi_provision(wfx_wifi_provision_t *cfg)
 * @brief
 *      Driver set the wifi provision
 * @param[in]  cfg:  wifi configuration
 * @return
 *       None
 ***********************************************************************/
void wfx_set_wifi_provision(wfx_wifi_provision_t * cfg)
{
    VerifyOrReturn(cfg != nullptr);
    wfx_rsi.sec = *cfg;
    wfx_rsi.dev_state.Set(WifiState::kStationProvisioned);
}

/*********************************************************************
 * @fn  bool wfx_get_wifi_provision(wfx_wifi_provision_t *wifiConfig)
 * @brief
 *      Driver get the wifi provision
 * @param[in]  wifiConfig:  wifi configuration
 * @return  return false if successful,
 *        true otherwise
 ***********************************************************************/
bool wfx_get_wifi_provision(wfx_wifi_provision_t * wifiConfig)
{
    VerifyOrReturnError(wifiConfig != nullptr, false);
    VerifyOrReturnError(wfx_rsi.dev_state.Has(WifiState::kStationProvisioned), false);
    *wifiConfig = wfx_rsi.sec;
    return true;
}

/*********************************************************************
 * @fn  void wfx_clear_wifi_provision(void)
 * @brief
 *      Driver is clear the wifi provision
 * @param[in]  None
 * @return  None
 ***********************************************************************/
void wfx_clear_wifi_provision(void)
{
    memset(&wfx_rsi.sec, 0, sizeof(wfx_rsi.sec));
    wfx_rsi.dev_state.Clear(WifiState::kStationProvisioned);
    ChipLogProgress(DeviceLayer, "Clear WiFi Provision");
}

/*************************************************************************
 * @fn sl_status_t wfx_connect_to_ap(void)
 * @brief
 * Start a JOIN command to the AP - Done by the wfx_rsi task
 * @param[in]   None
 * @return  returns SL_STATUS_OK if successful
 ****************************************************************************/
sl_status_t wfx_connect_to_ap(void)
{
    VerifyOrReturnError(wfx_rsi.dev_state.Has(WifiState::kStationProvisioned), SL_STATUS_INVALID_CONFIGURATION);
    VerifyOrReturnError(wfx_rsi.sec.ssid_length, SL_STATUS_INVALID_CREDENTIALS);
    VerifyOrReturnError(wfx_rsi.sec.ssid_length <= WFX_MAX_SSID_LENGTH, SL_STATUS_HAS_OVERFLOWED);
    ChipLogProgress(DeviceLayer, "connect to access point: %s", wfx_rsi.sec.ssid);

    WifiPlatformEvent event = WifiPlatformEvent::kStationStartJoin;
    sl_matter_wifi_post_event(event);
    return SL_STATUS_OK;
}

/*********************************************************************
 * @fn  void wfx_setup_ip6_link_local(sl_wfx_interface_t whichif)
 * @brief
 *      Implement the ipv6 setup
 * @param[in]  whichif:
 * @return  None
 ***********************************************************************/
void wfx_setup_ip6_link_local(sl_wfx_interface_t whichif)
{
    /*
     * TODO: Implement IPV6 setup, currently in sl_matter_wifi_task()
     * This is hooked with MATTER code.
     */
}

/*********************************************************************
 * @fn  wifi_mode_t wfx_get_wifi_mode(void)
 * @brief
 *      get the wifi mode
 * @param[in]  None
 * @return  return WIFI_MODE_NULL if successful,
 *          WIFI_MODE_STA otherwise
 ***********************************************************************/
wifi_mode_t wfx_get_wifi_mode(void)
{
    if (wfx_rsi.dev_state.Has(WifiState::kStationInit))
        return WIFI_MODE_STA;
    return WIFI_MODE_NULL;
}

/*********************************************************************
 * @fn  sl_status_t sl_matter_wifi_disconnect(void)
 * @brief
 *      called fuction when STA disconnected
 * @param[in]  None
 * @return  return SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t sl_matter_wifi_disconnect(void)
{
    sl_status_t status;
    status = sl_wifi_platform_disconnect();
    wfx_rsi.dev_state.Clear(WifiState::kStationConnected);
    return status;
}
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/*********************************************************************
 * @fn  bool wfx_have_ipv4_addr(sl_wfx_interface_t which_if)
 * @brief
 *      called fuction when driver have ipv4 address
 * @param[in]  which_if:
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_have_ipv4_addr(sl_wfx_interface_t which_if)
{
    VerifyOrReturnError(which_if == SL_WFX_STA_INTERFACE, false);
    return wfx_rsi.dev_state.Has(WifiState::kStationDhcpDone);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/*********************************************************************
 * @fn  bool wfx_have_ipv6_addr(sl_wfx_interface_t which_if)
 * @brief
 *      called fuction when driver have ipv6 address
 * @param[in]  which_if:
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_have_ipv6_addr(sl_wfx_interface_t which_if)
{
    VerifyOrReturnError(which_if == SL_WFX_STA_INTERFACE, false);
    // TODO: WifiState::kStationConnected does not guarantee SLAAC IPv6 LLA, maybe use a different FLAG
    return wfx_rsi.dev_state.Has(WifiState::kStationConnected);
}

/*********************************************************************
 * @fn  bool wfx_hw_ready(void)
 * @brief
 *      called fuction when driver ready
 * @param[in]  None
 * @return  returns ture if successful,
 *          false otherwise
 ***********************************************************************/
bool wfx_hw_ready(void)
{
    return wfx_rsi.dev_state.Has(WifiState::kStationInit);
}

/*********************************************************************
 * @fn  int32_t wfx_get_ap_info(wfx_wifi_scan_result_t *ap)
 * @brief
 *      get the access point information
 * @param[in]  ap: access point
 * @return
 *      access point information
 ***********************************************************************/
int32_t wfx_get_ap_info(wfx_wifi_scan_result_t * ap)
{
    return wfx_rsi_get_ap_info(ap);
}

/*********************************************************************
 * @fn   int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t *extra_info)
 * @brief
 *      get the access point extra information
 * @param[in]  extra_info:access point extra information
 * @return
 *      access point extra information
 ***********************************************************************/
int32_t wfx_get_ap_ext(wfx_wifi_scan_ext_t * extra_info)
{
    return wfx_rsi_get_ap_ext(extra_info);
}

/***************************************************************************
 * @fn   int32_t wfx_reset_counts(void)
 * @brief
 *      get the driver reset count
 * @param[in]  None
 * @return
 *      reset count
 *****************************************************************************/
int32_t wfx_reset_counts(void)
{
    return wfx_rsi_reset_count();
}

/***************************************************************************
 * @fn   void wfx_cancel_scan(void)
 * @brief
 *      called function when driver cancel scaning
 * @param[in]  None
 * @return
 *      None
 *****************************************************************************/
void wfx_cancel_scan(void)
{
    /* Not possible */
    ChipLogError(DeviceLayer, "cannot cancel scan");
}
