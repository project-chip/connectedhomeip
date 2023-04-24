/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
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
#endif

#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif
#include <platform/NetworkCommissioning.h>
#include <platform/nxp/mw320/NetworkCommissioningDriver.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for NXP K32W platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif

                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>,
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

public:
    void
    SetNetworkStatusChangeCallback(NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback)
    {
        mpStatusChangeCallback = statusChangeCallback;
    }
    CHIP_ERROR ConnectWiFiNetworkAsync(ByteSpan ssid, ByteSpan credentials,
                                       NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * connectCallback);
    CHIP_ERROR CommitConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    void StartWiFiManagement();
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & value);
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType);
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion);
#endif
    CHIP_ERROR GetConfiguredNetwork(NetworkCommissioning::Network & network);
    CHIP_ERROR StartWiFiScan(ByteSpan ssid, NetworkCommissioning::WiFiDriver::ScanCallback * callback);

    static const char * GetWiFiIfName() { return "mlan0"; }

    void UpdateNetworkStatus();

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
    bool _HaveServiceConnectivity(void);
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    WiFiStationMode _GetWiFiStationMode();
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);

    bool _IsWiFiStationEnabled();
    bool _IsWiFiStationConnected();
    bool _IsWiFiStationApplicationControlled();
#endif
    static std::string to_hex_string(const std::string & input);
    static int _OnWpaInterfaceScanDone(unsigned int count);
    static bool _GetBssInfo(const uint8_t sid, NetworkCommissioning::WiFiScanResponse & result);
    NetworkCommissioning::Internal::BaseDriver::NetworkStatusChangeCallback * mpStatusChangeCallback = nullptr;

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    ConnectivityManager::WiFiAPMode mWiFiAPMode;
    uint32_t mWiFiStationReconnectIntervalMS;

    static uint8_t sInterestedSSID[Internal::kMaxWiFiSSIDLength];
    static uint8_t sInterestedSSIDLen;
    // Configured SSID
    static uint8_t sCfgSSID[Internal::kMaxWiFiSSIDLength];
    static uint8_t sCfgSSIDLen;

    static NetworkCommissioning::WiFiDriver::ScanCallback * mpScanCallback;
    static NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * mpConnectCallback;
};

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return false;
}

/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * Chip applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager & ConnectivityMgr(void)
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * Chip applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the ESP32 platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
