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
    CHIP_ERROR ConfigurePowerSave(rsi_power_save_profile_mode_t sl_si91x_ble_state,
                                  sl_si91x_performance_profile_t sl_si91x_wifi_state, uint32_t listenInterval) override;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    /**
     * @brief Processes the wifi platform events for the SiWx platform
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
     * @brief Callback function for the SL_WIFI_JOIN_EVENTS group
     *
     * This callback handler will be invoked when any event within join event group occurs, providing the event details and any
     * associated data The callback doesn't get called when we join a network using the sl net APIs
     *
     * @note In case of failure, the 'result' parameter will be of type sl_status_t, and the 'resultLenght' parameter should be
     * ignored
     *
     * @param[in] event sl_wifi_event_t that triggered the callback
     * @param[in] result Pointer to the response data received
     * @param[in] result_length Length of the data received in bytes
     * @param[in] arg Optional user provided argument
     *
     * @return sl_status_t Returns the status of the operation
     */
    static sl_status_t JoinCallback(sl_wifi_event_t event, char * result, uint32_t resultLenght, void * arg);

    /**
     * @brief Triggers a synchronous connection attempt to the stored Wifi crendetials
     *
     * @return sl_status_t SL_STATUS_IN_PROGRESS, if the device is already connected or connecting.
     *                     SL_STATUS_OK, on connection success.
     *                     other error on failure, otherwise
     */
    sl_status_t JoinWifiNetwork();

    /**
     * @brief Processing function responsible for notifying the upper layers of a succesful connection attempt.
     *
     */
    void NotifySuccessfulConnection();

    static WifiInterfaceImpl mInstance;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
