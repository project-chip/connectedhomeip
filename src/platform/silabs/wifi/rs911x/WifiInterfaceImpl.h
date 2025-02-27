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
     * @brief
     *
     * @param status
     * @param buf
     * @param len
     */
    static void JoinCallback(uint16_t status, const uint8_t * buf, const uint16_t len);

    /**
     * @brief
     *
     * @param status
     * @param buf
     * @param len
     */
    static void JoinFailCallback(uint16_t status, uint8_t * buf, uint32_t len);

    /**
     * @brief
     *
     * @return int32_t
     */
    int32_t Rs911xPlatformInit();

    /**
     * @brief Triggers a connection attempt to the Wi-Fi network with the stored credentials
     */
    void JoinWifiNetwork();

    /**
     * @brief
     *
     */
    void HandleDHCPPolling();

    static WifiInterfaceImpl mInstance;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
