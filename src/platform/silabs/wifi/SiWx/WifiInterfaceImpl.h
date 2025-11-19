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

#include <platform/silabs/wifi/WifiInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

/**
 * @brief WifiInterface implementation for the SiWx platform
 *
 */
class WifiInterfaceImpl final : public WifiInterface
{
public:
    enum class WifiPlatformEvent : uint8_t
    {
        kStationConnect     = 0,
        kStationDisconnect  = 1,
        kAPStart            = 2,
        kAPStop             = 3,
        kStationStartJoin   = 5,
        kConnectionComplete = 6, /* This combines the DHCP and Notify */
        kStationDhcpDone    = 7,
        kStationDhcpPoll    = 8,
    };

    static WifiInterfaceImpl & GetInstance() { return mInstance; }

    WifiInterfaceImpl(const WifiInterfaceImpl &)             = delete;
    WifiInterfaceImpl & operator=(const WifiInterfaceImpl &) = delete;

    /*
     * WifiInterface impl
     */

    CHIP_ERROR GetMacAddress(sl_wfx_interface_t interface, chip::MutableByteSpan & addr) override;
    CHIP_ERROR StartNetworkScan(chip::ByteSpan ssid, ScanCallback callback) override;
    CHIP_ERROR StartWifiTask() override;
    void ConfigureStationMode() override;
    bool IsStationConnected() override;
    bool IsStationModeEnabled() override;
    bool IsStationReady() override;
    CHIP_ERROR TriggerDisconnection() override;
    void ClearWifiCredentials() override;
    void SetWifiCredentials(const WifiCredentials & credentials) override;
    CHIP_ERROR GetWifiCredentials(WifiCredentials & credentials) override;
    CHIP_ERROR ConnectToAccessPoint(void) override;
    bool HasAnIPv4Address() override;
    bool HasAnIPv6Address() override;
    void CancelScanNetworks() override;
    bool IsWifiProvisioned() override;
    CHIP_ERROR InitWiFiStack(void) override;
    CHIP_ERROR GetAccessPointInfo(wfx_wifi_scan_result_t & info) override;
    CHIP_ERROR GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info) override;
    CHIP_ERROR ResetCounters() override;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter) override;
    CHIP_ERROR ConfigurePowerSave(PowerSaveInterface::PowerSaveConfiguration configuration, uint32_t listenInterval) override;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    /**
     * @brief Processes the wifi platform events for the SiWx platform
     *
     * @param event
     */
    void ProcessEvent(WifiPlatformEvent event);

protected:
    /**
     * @brief Function calls the underlying platforms disconnection API.
     *
     * @return sl_status_t SL_STATUS_OK, the Wi-Fi disconnection was succesfully triggered
     *                     SL_STATUS_FAILURE, otherwise
     */
    sl_status_t TriggerPlatformWifiDisconnection();
    /**
     * @brief Posts an event to the Wi-Fi task
     *
     * @param[in] event Event to process.
     */
    void PostWifiPlatformEvent(WifiPlatformEvent event);
    /**
     * @brief Main worker function for the Matter Wi-Fi task responsible of processing Wi-Fi platform events.
     *        Function is used in the StartWifiTask.
     *
     * @param[in] arg context pointer
     */
    static void MatterWifiTask(void * arg);

    /**
     * @brief Notify the application about the connectivity status if it has not been notified yet.
     */
    void NotifyConnectivity(void);

    /**
     * @brief Function resets the IP and connectiity flags and triggers the DHCP operation
     *
     */
    void ResetConnectivityNotificationFlags();

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

    bool mHasNotifiedWifiConnectivity = false;

    static WifiInterfaceImpl mInstance;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
