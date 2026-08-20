/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <memory>

#include <stdint.h>

#include <lib/support/FixedBuffer.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.h>
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/NetworkCommissioning.h>

#include "ConnectivityManagerImpl_NetworkManagementDelegate.h"

namespace chip {
namespace DeviceLayer {

namespace Internal {

// Forward Declarations

class NetworkManagementInterface;
class WiFiPafInterface;

} // namespace Internal

/**
 *  This provides a concrete implementation of the public Connectivity
 *  Manager Curiously-recurring Template Pattern (CRTP) singleton
 *  object interface for Linux platforms.
 *
 *  This fans out those CRTP interface methods into one of two
 *  abstract virtual interfaces:
 *
 *    1. High-level network management, if any, that owns network
 *       policy and state and orchestrates connectivity decisions and
 *       cross-interface policy.
 *    2. Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized
 *       Service Discovery (USD) / Public Action Frame (PAF)
 *       commissioning transport management.
 *
 *  The implementations of which are allocated, instantiated, and
 *  initialized in the #Init method.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                                      public Internal::GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
                                      public Internal::NetworkManagementDelegate
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

public:
    ConnectivityManagerImpl() = default;
    ~ConnectivityManagerImpl();

    // Singleton

    static ConnectivityManagerImpl & GetDefaultInstance();

    using NetworkStatusChangeCallback = NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback;
    using OneShotScanCallback         = NetworkCommissioning::WiFiDriver::ScanCallback;
    using OneShotConnectCallback      = NetworkCommissioning::Internal::WirelessDriver::ConnectCallback;

    void SetOneShotConnectCallback(OneShotConnectCallback * inOneShotConnectCallback) noexcept;
    void SetOneShotScanCallback(OneShotScanCallback * inOneShotScanCallback) noexcept;
    void SetNetworkStatusChangeCallback(NetworkStatusChangeCallback * inStatusChangeCallback) noexcept;

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    // Ethernet Control Plane Management

    // Observation

    const char * GetEthernetIfName();

    // Helper

    void UpdateEthernetNetworkingStatus();
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Wi-Fi Control Plane Management

    // Wi-Fi Station Control Plane Management

    // Introspection

    bool IsWiFiStationConnecting() const noexcept;
    bool IsWiFiStationScanning() const noexcept;
    bool IsWiFiManagementStarted();

    // Observation

    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & outNetwork);
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & outBssId);
    const char * GetWiFiIfName();
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & outSecurityType);
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & outVersion);

    // Worker

    CHIP_ERROR ConnectWiFiNetworkAsync(ByteSpan inSsid, ByteSpan inCredentials,
                                       NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    CHIP_ERROR ConnectWiFiNetworkWithPDCAsync(ByteSpan inSsid, ByteSpan inNetworkIdentity, ByteSpan inClientIdentity,
                                              const Crypto::P256Keypair & inClientIdentityKeypair,
                                              NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * inConnectCallback);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    CHIP_ERROR CommitConfig();
    void StartWiFiManagement();
    void StopWiFiManagement();
    CHIP_ERROR StartWiFiScan(ByteSpan inSsid, NetworkCommissioning::WiFiDriver::ScanCallback * inScanCallback);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    // Network Commissioning Action Delegation Methods

    void OnScanFinished(NetworkCommissioning::Status inStatus, CharSpan inDebugText,
                        NetworkCommissioning::WiFiScanResponseIterator * inNetworks) noexcept;
    void OnConnectResult(NetworkCommissioning::Status inCommissioningError, CharSpan inDebugText, int32_t inConnectStatus) noexcept;
    void OnStatusChange(NetworkCommissioning::Status inCommissioningError, Optional<ByteSpan> inNetworkId,
                        Optional<int32_t> inConnectStatus) noexcept;

private:
    // Curiously-recurring template pattern (CRTP) members that
    // implement the public Connectivity Manager interface which, on
    // Linux, are further dispatched via one of two private
    // implmentation pointer (PImpl) interfaces.

    // Initialization

    CHIP_ERROR _Init();

    // Event Handling

    void _OnPlatformEvent(const ChipDeviceEvent * inDeviceEvent);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Wi-Fi Control Plane Management

    // Wi-Fi Station Control Plane Management

    // Introspection

    bool _IsWiFiStationApplicationControlled();
    bool _IsWiFiStationConnected();
    bool _IsWiFiStationEnabled();

    // Observation

    WiFiStationMode _GetWiFiStationMode();

    // Mutation

    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode inWiFiStationMode);

    // Wi-Fi Soft Access Point Control Plane Management

    // Observation

    WiFiAPMode _GetWiFiAPMode();

    // Mutation

    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode inWiFiAPMode);

    // Control

    void _DemandStartWiFiAP();
    void _MaintainOnDemandWiFiAP();
    void _StopOnDemandWiFiAP();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    // Wi-Fi Neighbor Awareness Networking (NAN) Unsynchronized
    // Service Discovery (USD) / Public Action Frame (PAF)
    // Commissioning Transport

    // Initialization

    CHIP_ERROR _WiFiPAFShutdown(uint32_t id, WiFiPAF::WiFiPafRole inWiFiPafRole);

    // Introspection

    bool _WiFiPAFResourceAvailable();

    // Mutation

    CHIP_ERROR _SetWiFiPAFAdvertisingEnabled(bool inEnabled, uint32_t & inOutPublishId);
    void _WiFiPafSetApFreq(const uint16_t inFrequency);
    void _WiFiPAFSetParam(const WiFiPAFAdvertiseParam & inWiFiPAFAdvertiseParams);

    // Publish-and-subscribe

    CHIP_ERROR _WiFiPAFPublish(WiFiPAFAdvertiseParam & inWiFiPafAdvertiseParams);
    CHIP_ERROR _WiFiPAFCancelPublish(uint32_t inPublishId);
    CHIP_ERROR _WiFiPAFSubscribe(const uint16_t & inConnectionDiscriminator, void * inContext,
                                 OnConnectionCompleteFunct inOnSuccessFunc, OnConnectionErrorFunct inOnErrorFunc);
    CHIP_ERROR _WiFiPAFCancelSubscribe(uint32_t inSubscribeId);
    CHIP_ERROR _WiFiPAFCancelIncompleteSubscribe();

    // Data Transmission

    CHIP_ERROR _WiFiPAFSend(const WiFiPAF::WiFiPAFSession & inWiFiPafSession, chip::System::PacketBufferHandle && inMessageBuffer);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

    // Network Management Delegate Method

    void OnWiFiMediumAvailable(Internal::NetworkManagementBasis & inOutNetworkManagement, bool inAvailable) override final;

private:
    template <typename T>
    struct Deleter
    {
        void operator()(T * inPointer) const noexcept;
    };

    using NetworkManagementUniquePtr =
        std::unique_ptr<Internal::NetworkManagementInterface, Deleter<Internal::NetworkManagementInterface>>;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    using WiFiPafUniquePtr = std::unique_ptr<Internal::WiFiPafInterface, Deleter<Internal::WiFiPafInterface>>;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

    /**
     *  A callback through which, when non-null, the Wi-Fi driver
     *  'OnFinished' method is invoked after a Wi-Fi scan is
     *  complete. The semantics of this callback are one-shot in that
     *  it is set-scan-invoke-and-clear.
     *
     *  A non-null value implies that a Wi-Fi scan is in progress.
     */
    OneShotScanCallback * mpOneShotScanCallback;

    /**
     *  A callback through which, when non-null, the wireless driver
     *  'OnResult' method is invoked after a Wi-Fi association is
     *  complete. The semantics of this callback are one-shot in that
     *  it is set-associate-invoke-and-clear.
     *
     *  A non-null value implies that a Wi-Fi association is in
     *  progress.
     */
    OneShotConnectCallback * mpOneShotConnectCallback;
    NetworkStatusChangeCallback * mpStatusChangeCallback;
    NetworkManagementUniquePtr mNetworkManagementImplementation;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    WiFiPafUniquePtr mWiFiPafImplementation;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
};

} // namespace DeviceLayer
} // namespace chip
