/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#pragma once

#include <platform/silabs/wifi/wiseconnect-interface/WiseconnectWifiInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

/**
 * @brief WifiInterface implementation for the SiWx platform
 *
 */
class WifiInterfaceImpl final : public WiseconnectWifiInterface
{
public:
    static WifiInterfaceImpl & GetInstance() { return mInstance; }

    WifiInterfaceImpl(const WifiInterfaceImpl &)             = delete;
    WifiInterfaceImpl & operator=(const WifiInterfaceImpl &) = delete;

    /*
     * WifiInterface impl
     */

    CHIP_ERROR InitWiFiStack(void) override;
    CHIP_ERROR GetAccessPointInfo(wfx_wifi_scan_result_t & info) override;
    CHIP_ERROR GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info) override;
    CHIP_ERROR ResetCounters() override;
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter) override;
    CHIP_ERROR ConfigurePowerSave() override;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    /**
     * @brief
     *
     * TODO: Current inheritance structure with the task creation in the parent forces this function to be public when it shouldn't
     *       be. This isn't the best practice and we should try to move this to protected.
     *
     * @param event
     */
    void ProcessEvent(WiseconnectWifiInterface::WifiPlatformEvent event);

protected:
    /*
     * WiseconnectWifiInterface impl
     */

    sl_status_t TriggerPlatformWifiDisconnection() override;
    void PostWifiPlatformEvent(WifiPlatformEvent event) override;

private:
    WifiInterfaceImpl()  = default;
    ~WifiInterfaceImpl() = default;

    /**
     * @brief Callback function on a succesfull join operation
     *
     *        See the third_party/silabs/wiseconnect-wifi-bt-sdk/sapi/wlan/rsi_wlan_apis.c
     *        rsi_wlan_connect_async documentation for the argument definitions
     */
    static void JoinCallback(uint16_t status, const uint8_t * buf, const uint16_t len);

    /**
     * @brief Callback function for a failed join operation
     *
     *        See the third_party/silabs/wiseconnect-wifi-bt-sdk/sapi/wlan/rsi_wlan_apis.c
     *        rsi_wlan_register_callbacks documentation for the argument definitions
     */
    static void JoinFailCallback(uint16_t status, uint8_t * buf, uint32_t len);

    /**
     * @brief Platform Init for the RS911x platform
     *
     * @return int32_t RSI_SUCCESS, on succesfull init
     *                 RSI_FAILURE, otherwise
     */
    int32_t Rs911xPlatformInit();

    /**
     * @brief Triggers a connection attempt to the Wi-Fi network with the stored credentials
     */
    void JoinWifiNetwork();

    /**
     * @brief Processing function responsible of executing the DHCP polling operation
     *        until we have an IPv6 or IPv4 address
     */
    void HandleDHCPPolling();

    /**
     * @brief Function cancels the DHCP timer if it is running.
     *        If the timer isn't running, function doesn't do anything.
     */
    void CancelDHCPTimer();

    /**
     * @brief Function starts the DHCP timer with the given timeout.
     *
     * TODO: change input to milliseconds type
     *
     * @param timeout timer duration in milliseconds
     */
    void StartDHCPTimer(uint32_t timeout);

    /**
     * @brief Function creates the DHCP timer
     *
     *
     * @return sl_status_t SL_STATUS_OK, the timer was successfully created
     */
    sl_status_t CreateDHCPTimer();

    /**
     * @brief Callback function for the DHCP timer event.
     */
    static void DHCPTimerEventHandler(void * arg);

    osTimerId_t mDHCPTimer;
    static WifiInterfaceImpl mInstance;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
