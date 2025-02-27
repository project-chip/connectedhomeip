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
 *    distributed under the License is distributed on an "AS IS"
 *    BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language
 *    governing permissions and limitations under the License.
 */
#pragma once

#include <platform/silabs/wifi/WifiInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

/**
 * @brief Wifi Interface implementation
 *
 */
class WifiInterfaceImpl final : public WifiInterface
{
public:
    static WifiInterfaceImpl & GetInstance() { return mInstance; }

    WifiInterfaceImpl(const WifiInterfaceImpl &)             = delete;
    WifiInterfaceImpl & operator=(const WifiInterfaceImpl &) = delete;

    /*
     * WifiInterface Impl
     */

    CHIP_ERROR InitWiFiStack(void) override;
    CHIP_ERROR GetMacAddress(sl_wfx_interface_t interface, chip::MutableByteSpan & addr) override;
    CHIP_ERROR StartNetworkScan(chip::ByteSpan ssid, ScanCallback callback) override;
    CHIP_ERROR StartWifiTask() override;
    void ConfigureStationMode() override;
    bool IsStationConnected() override;
    bool IsStationModeEnabled() override;
    bool IsStationReady() override;
    CHIP_ERROR TriggerDisconnection() override;
    CHIP_ERROR GetAccessPointInfo(wfx_wifi_scan_result_t & info) override;
    CHIP_ERROR GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info) override;
    CHIP_ERROR ResetCounters() override;
    void ClearWifiCredentials() override;
    void SetWifiCredentials(const WifiCredentials & credentials) override;
    CHIP_ERROR GetWifiCredentials(WifiCredentials & credentials) override;
    bool IsWifiProvisioned() override;
    CHIP_ERROR ConnectToAccessPoint(void) override;
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    CHIP_ERROR ConfigurePowerSave() override;
    CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter) override;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
    bool HasAnIPv4Address() override;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
    bool HasAnIPv6Address() override;
    void CancelScanNetworks() override;

    /**
     * @brief Callback function passed to the Wi-Fi stack to notify the application of a connection event.
     *        Function is public to allow the Wi-Fi stack to call it. In practice, it should be private but the callback structure
     *        needs to be reworked first.
     *
     * @param connect_indication_body Connection callback data
     */
    void ConnectionEventCallback(sl_wfx_connect_ind_body_t connect_indication_body);

    /**
     * @brief Callback function to the Wi-Fi stack to notify the application of a disconnection event.
     *        Function is public to allow the Wi-Fi stack to call it. In practice, it should be private but the callback
     *        structure needs to be reworked first.
     *
     * @note See the sl_wfx_disconnect_ind_body_t structure for the reason values.
     *
     * @param[in] mac Access Point MAC Address
     * @param[in] reason disconnection reason
     */
    void DisconnectionEventCallback(uint8_t * mac, uint16_t reason);

private:
    WifiInterfaceImpl()  = default;
    ~WifiInterfaceImpl() = default;

    /**
     * @brief Initialize the WF200 platform
     *        Function triggers the platform int and the LwIP init.
     *
     * @return sl_status_t SL_STATUS_OK if the platform is initialized successfully or if it is already initialized.
     *                     otherwise platform error code.
     */
    sl_status_t InitWf200Platform();

    /**
     * @brief Wi-Fi Task process fonctions
     *
     * @param[in] arg
     */
    static void ProcessEvents(void * arg);

    /**
     * @brief Creates a new task to process Wi-Fi events
     */
    void StartWifiProcessTask(void);

    /**
     * @brief Wf200 Wifi Init
     *
     * TODO: We have multiple init function when we could have only one.
     *
     * @return sl_status_t SL_STATUS_OK if the platform is initialized successfully,
     *                     SL_STATUS_WIFI_INVALID_KEY, If the firmware keyset is invalid
     *                     SL_STATUS_WIFI_FIRMWARE_DOWNLOAD_TIMEOUT, If the firmware download times out
     *                     SL_STATUS_TIMEOUT, If the firmware download times out
     *                     SL_STATUS_FAIL, If there is an internal error
     */
    sl_status_t wfx_init(void);

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    /**
     * @brief Returns IPv4 Notification state
     *
     * TODO: This function is necessary because the ProcessEvents is static and does all the processing as well.
     *        Once the processing function is reworked, it won't be necessary anymore
     */
    bool HasNotifiedIPv4() { return mHasNotifiedIPv4; }
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4

    /**
     * @brief Returns IPv6 Notification state
     *
     * TODO: This function is necessary because the ProcessEvents is static and does all the processing as well.
     *        Once the processing function is reworked, it won't be necessary anymore
     */
    bool HasNotifiedIPv6() { return mHasNotifiedIPv6; }

    static WifiInterfaceImpl mInstance;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
