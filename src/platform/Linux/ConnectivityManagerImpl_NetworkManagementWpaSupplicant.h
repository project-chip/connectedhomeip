/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "ConnectivityManagerImpl_NetworkManagementBasis.h"
#include "ConnectivityManagerImpl_NetworkManagementInterface.h"
#include "WpaSupplicantClient.h"

namespace chip {
namespace DeviceLayer {

// Forward Declarations

struct ChipDeviceEvent;

/**
 *  Provides an implementation of the ConnectionManager object
 *  for Linux platforms where wpa_supplicant is both the
 *  high-level network manager, that owns Wi-Fi network policy
 *  and state and is the low-level Wi-Fi control plane manager
 *  that drives the Wi-Fi MAC and PHY through the Linux
 *  nl80211 driver.
 *
 */
class ConnectivityManagerImpl_NetworkManagementWpaSupplicant final : public Internal::NetworkManagementBasis,
                                                                     public Internal::NetworkManagementInterface,
                                                                     public Internal::WpaSupplicantClient
{
public:
    // Construction

    ConnectivityManagerImpl_NetworkManagementWpaSupplicant() = default;

    // Destruction

    virtual ~ConnectivityManagerImpl_NetworkManagementWpaSupplicant() = default;

    // Initialization

    CHIP_ERROR Init(ConnectivityManagerImpl & inConnectivityManagerImpl) override final;

    // Observation

    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork) override final;

    // Event Handling

    void OnPlatformEvent(const ChipDeviceEvent & inDeviceEvent) override final;

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    // Ethernet Control Plane Management

    const char * GetEthernetIfName() override final;
    void UpdateEthernetNetworkingStatus() override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Wi-Fi Control Plane Management

    // Observation

    const char * GetWiFiIfName() override final;

    // Control

    void StartNonConcurrentWiFiManagement() override final;
    void StartWiFiManagement() override final;
    void StopWiFiManagement() override final;

    // Wi-Fi Station Control Plane Management

    // Introspection

    bool IsWiFiManagementStarted() override final;
    bool IsWiFiStationApplicationControlled() override final;
    bool IsWiFiStationConnected() override final;
    bool IsWiFiStationEnabled() override final;
    bool IsWiFiStationProvisioned() override final;

    // Observation

    CHIP_ERROR GetWiFiBssId(MutableByteSpan & outBssId) override final;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType) override final;
    ConnectivityManager::WiFiStationMode GetWiFiStationMode() override final;
    System::Clock::Timeout GetWiFiStationReconnectInterval() override final;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion) override final;

    // Mutation

    CHIP_ERROR SetWiFiStationMode(const ConnectivityManager::WiFiStationMode & inWiFiStationMode) override final;
    CHIP_ERROR SetWiFiStationReconnectInterval(const System::Clock::Timeout & inInterval) override final;

    // Worker

    void ClearWiFiStationProvision() override final;
    CHIP_ERROR CommitConfig() override final;
    CHIP_ERROR
    ConnectWiFiNetworkAsync(ByteSpan inSsid, ByteSpan inCredentials,
                            NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) override final;
    CHIP_ERROR ConnectWiFiNetworkWithPDCAsync(
        ByteSpan inSsid, ByteSpan inNetworkIdentity, ByteSpan inClientIdentity, const Crypto::P256Keypair & inClientIdentityKeypair,
        NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback) override final;
    CHIP_ERROR StartWiFiScan(ByteSpan inSsid, NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback) override final;

    // Wi-Fi Soft Access Point (AP) Control Plane Management

    // Observation

    ConnectivityManager::WiFiAPMode GetWiFiApMode() override final;

    // Mutation

    CHIP_ERROR SetWiFiApMode(const ConnectivityManager::WiFiAPMode & inWiFiApMode) override final;
    void SetWiFiApIdleTimeout(const System::Clock::Timeout & inTimeout) override final;

    // Control

    void DemandStartWiFiAp() override final;
    void StopOnDemandWiFiAp() override final;
    void MaintainOnDemandWiFiAp() override final;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

private:
    // Helper Methods

    void ChangeWiFiApState(const ConnectivityManager::WiFiAPState & newState);
    CHIP_ERROR ConfigureWiFiAp();
    CHIP_ERROR ConnectWiFiNetworkAsyncLocked(GVariant * args,
                                             NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback) noexcept;
    void DriveApState();
    static void DriveApState(::chip::System::Layer * aSystemLayer, void * aAppState);
    int32_t GetDisconnectReason();

    // wpa_supplicant Base Class Method Overrides

    void OnWiFiMediumAvailable(WpaSupplicantClient & inOutWpaSupplicantClient, bool inAvailable) override final;
    void PostNetworkConnect() override final;
    void UpdateWiFiNetworkStatus() override final;

private:
    ConnectivityManagerImpl * mConnectivityManagerImpl;
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    System::Clock::Timeout mWiFiStationReconnectInterval;
    ConnectivityManager::WiFiAPMode mWiFiAPMode;
    ConnectivityManager::WiFiAPState mWiFiAPState;
    System::Clock::Timestamp mLastAPDemandTime;
    System::Clock::Timeout mWiFiAPIdleTimeout;
    bool mAssociationStarted = false;
    char mEthernetIfName[Inet::InterfaceId::kMaxIfNameLength];
};

} // namespace DeviceLayer
} // namespace chip
