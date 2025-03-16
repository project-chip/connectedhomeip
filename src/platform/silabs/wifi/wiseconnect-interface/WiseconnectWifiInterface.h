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
#pragma once

#include <platform/silabs/wifi/WifiInterface.h>
#include <sl_status.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

/**
 * @brief Wiseconnect Wifi Interface implementation.
 *        Common implementations for Wiseconnect platforms.
 */
class WiseconnectWifiInterface : public WifiInterface
{
public:
    enum class WifiPlatformEvent : uint8_t
    {
        kStationConnect     = 0,
        kStationDisconnect  = 1,
        kAPStart            = 2,
        kAPStop             = 3,
        kScan               = 4, /* This combines the scan start and scan result events  */
        kStationStartJoin   = 5,
        kConnectionComplete = 6, /* This combines the DHCP for RS9116 and Notify for SiWx917 */
        kStationDhcpDone    = 7,
        kStationDhcpPoll    = 8,
    };

    virtual ~WiseconnectWifiInterface() = default;

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
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
    bool HasAnIPv4Address() override;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
    bool HasAnIPv6Address() override;
    void CancelScanNetworks() override;
    bool IsWifiProvisioned() override;

protected:
    /**
     * @brief Function calls the underlying platforms disconnection API.
     *
     * @note This abstraction layer here is used to reduce the duplication for wiseconnect platforms.
     *       Since the only difference is the disconnection API, the common implementation is in the WiseconnectWifiInterface
     *       which calls this abstraction function that is implemented by the different platforms.
     *
     * @return sl_status_t SL_STATUS_OK, the Wi-Fi disconnection was succesfully triggered
     *                     SL_STATUS_FAILURE, otherwise
     */
    virtual sl_status_t TriggerPlatformWifiDisconnection() = 0;

    /**
     * @brief Posts an event to the Wi-Fi task
     *
     * TODO: Move the implementations the rs9116 and SiWx implementations to the interface
     *       Remote the pure virutal once it is done
     *
     * @param[in] event Event to process.
     */
    virtual void PostWifiPlatformEvent(WifiPlatformEvent event) = 0;

    /**
     * @brief Main worker function for the Matter Wi-Fi task responsible of processing Wi-Fi platform events.
     *        Function is used in the StartWifiTask.
     *
     * @note Function must be implemented by the child classes
     *
     * @param[in] arg context pointer
     */
    static void MatterWifiTask(void * arg);

    /**
     * @brief Notify the application about the connectivity status if it has not been notified yet.
     */
    void NotifyConnectivity(void);

    /**
     * @brief Updates the IPv4 address in the Wi-Fi interface and notifies the application layer about the new IP address.
     *
     * @param[in] ip New IPv4 address
     */
    void GotIPv4Address(uint32_t ip);

    /**
     * @brief Function resets the IP and connectiity flags and triggers the DHCP operation
     *
     */
    void ResetConnectivityNotificationFlags();

private:
    /**
     * @brief Returns the singleton instance of the Wiseconnect WiFi interface
     *
     *  @note This function needs to be implemented in the child classes sources file
     *
     * @return WiseconnectWifiInterface&
     */
    static WiseconnectWifiInterface & GetInstance();

    bool mHasNotifiedWifiConnectivity = false;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
