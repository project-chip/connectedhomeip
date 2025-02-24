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

#include <cmsis_os2.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/wiseconnect-interface/WiseconnectWifiInterface.h>
#include <sl_cmsis_os2_common.h>

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

osTimerId_t sDHCPTimer;
bool hasNotifiedWifiConnectivity = false;

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
    PostWifiPlatformEvent(event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR StartWifiTask()
{
    // Verify that the Wifi task has not already been started.
    VerifyOrReturnError(!(wfx_rsi.dev_state.Has(WifiState::kStationStarted)), CHIP_NO_ERROR);
    wfx_rsi.dev_state.Set(WifiState::kStationStarted);

    // Creating a Wi-Fi task thread
    sWlanThread = osThreadNew(MatterWifiTask, NULL, &kWlanTaskAttr);
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

bool IsStationReady()
{
    return wfx_rsi.dev_state.Has(WifiState::kStationInit);
}

CHIP_ERROR TriggerDisconnection()
{
    VerifyOrReturnError(TriggerPlatformWifiDisconnection() == SL_STATUS_OK, CHIP_ERROR_INTERNAL);
    wfx_rsi.dev_state.Clear(WifiState::kStationConnected);

    return CHIP_NO_ERROR;
}

void DHCPTimerEventHandler(void * arg)
{
    WifiPlatformEvent event = WifiPlatformEvent::kStationDhcpPoll;
    PostWifiPlatformEvent(event);
}

void CancelDHCPTimer(void)
{
    VerifyOrReturn(osTimerIsRunning(sDHCPTimer), ChipLogDetail(DeviceLayer, "CancelDHCPTimer: timer not running"));
    VerifyOrReturn(osTimerStop(sDHCPTimer) == osOK, ChipLogError(DeviceLayer, "CancelDHCPTimer: failed to stop timer"));
}

void StartDHCPTimer(uint32_t timeout)
{
    // Cancel timer if already started
    CancelDHCPTimer();

    VerifyOrReturn(osTimerStart(sDHCPTimer, pdMS_TO_TICKS(timeout)) == osOK,
                   ChipLogError(DeviceLayer, "StartDHCPTimer: failed to start timer"));
}

void NotifyConnectivity(void)
{
    VerifyOrReturn(!hasNotifiedWifiConnectivity);

    NotifyConnection(wfx_rsi.ap_mac);
    hasNotifiedWifiConnectivity = true;
}

sl_status_t CreateDHCPTimer()
{
    // TODO: Use LWIP timer instead of creating a new one here
    sDHCPTimer = osTimerNew(DHCPTimerEventHandler, osTimerPeriodic, nullptr, nullptr);
    VerifyOrReturnError(sDHCPTimer != nullptr, SL_STATUS_ALLOCATION_FAILED);

    return SL_STATUS_OK;
}

/**
 * @brief Reset the flags that are used to notify the application about DHCP connectivity
 *        and emits a WifiPlatformEvent::kStationDoDhcp event to trigger DHCP polling checks.
 *
 * TODO: This function should be moved to the protected section once the class structure is done.
 */
void ResetDHCPNotificationFlags(void)
{

    ResetIPNotificationStates();
    hasNotifiedWifiConnectivity = false;

    WifiPlatformEvent event = WifiPlatformEvent::kStationDoDhcp;
    PostWifiPlatformEvent(event);
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
void GotIPv4Address(uint32_t ip)
{
    // Acquire the new IP address
    for (int i = 0; i < 4; ++i)
    {
        wfx_rsi.ip4_addr[i] = (ip >> (i * 8)) & 0xFF;
    }

    ChipLogDetail(DeviceLayer, "DHCP OK: IP=%d.%d.%d.%d", wfx_rsi.ip4_addr[0], wfx_rsi.ip4_addr[1], wfx_rsi.ip4_addr[2],
                  wfx_rsi.ip4_addr[3]);

    // Notify the Connectivity Manager - via the app
    wfx_rsi.dev_state.Set(WifiState::kStationDhcpDone).Set(WifiState::kStationReady);
    NotifyIPv4Change(true);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

void ClearWifiCredentials()
{
    ChipLogProgress(DeviceLayer, "Clear WiFi Provision");

    wfx_rsi.credentials.Clear();
    wfx_rsi.dev_state.Clear(WifiState::kStationProvisioned);
}

CHIP_ERROR GetWifiCredentials(WifiCredentials & credentials)
{
    VerifyOrReturnError(wfx_rsi.dev_state.Has(WifiState::kStationProvisioned), CHIP_ERROR_INCORRECT_STATE);
    credentials = wfx_rsi.credentials;

    return CHIP_NO_ERROR;
}

bool IsWifiProvisioned()
{
    return wfx_rsi.dev_state.Has(WifiState::kStationProvisioned);
}

void SetWifiCredentials(const WifiCredentials & credentials)
{
    wfx_rsi.credentials = credentials;
    wfx_rsi.dev_state.Set(WifiState::kStationProvisioned);
}

CHIP_ERROR ConnectToAccessPoint()
{
    VerifyOrReturnError(IsWifiProvisioned(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(wfx_rsi.credentials.ssidLength, CHIP_ERROR_INCORRECT_STATE);

    // TODO: We should move this validation to where we set the credentials. It is too late here.
    VerifyOrReturnError(wfx_rsi.credentials.ssidLength <= WFX_MAX_SSID_LENGTH, CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, "connect to access point: %s", wfx_rsi.credentials.ssid);

    WifiPlatformEvent event = WifiPlatformEvent::kStationStartJoin;
    PostWifiPlatformEvent(event);

    return CHIP_NO_ERROR;
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
bool HasAnIPv4Address()
{
    return wfx_rsi.dev_state.Has(WifiState::kStationDhcpDone);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

bool HasAnIPv6Address()
{
    // TODO: WifiState::kStationConnected does not guarantee SLAAC IPv6 LLA, maybe use a different FLAG
    // Once connect is sync instead of async, this should be fine
    return wfx_rsi.dev_state.Has(WifiState::kStationConnected);
}

void CancelScanNetworks()
{
    // TODO: Implement cancel scan
}
