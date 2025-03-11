/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include <cstdint>

#include <inet/InetConfig.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceEvent.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

#include <app-common/zap-generated/cluster-enums.h>

#include "platform/internal/GenericConnectivityManagerImpl.h"
#include "platform/internal/GenericConnectivityManagerImpl_UDP.h"
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include "platform/internal/GenericConnectivityManagerImpl_TCP.h"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include "platform/internal/GenericConnectivityManagerImpl_BLE.h"
#else
#include "platform/internal/GenericConnectivityManagerImpl_NoBLE.h"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "platform/internal/GenericConnectivityManagerImpl_Thread.h"
#else
#include "platform/internal/GenericConnectivityManagerImpl_NoThread.h"
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "platform/internal/GenericConnectivityManagerImpl_WiFi.h"
#else
#include "platform/internal/GenericConnectivityManagerImpl_NoWiFi.h"
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for Tizen platforms.
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    void StartWiFiManagement();
    void StopWiFiManagement();
    bool IsWiFiManagementStarted();
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & value);
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType);
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion);
    const char * GetWiFiIfName() { return (sWiFiIfName[0] == '\0') ? nullptr : sWiFiIfName; }
#endif

    const char * GetEthernetIfName() { return (mEthIfName[0] == '\0') ? nullptr : mEthIfName; }

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    System::Clock::Timeout _GetWiFiStationReconnectInterval(void);
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationConnected(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    bool _CanStartWiFiScan(void);

    WiFiAPMode _GetWiFiAPMode(void);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPActive(void);
    bool _IsWiFiAPApplicationControlled(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    System::Clock::Timeout _GetWiFiAPIdleTimeout(void);
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);
#endif

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    char mEthIfName[Inet::InterfaceId::kMaxIfNameLength];

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    ConnectivityManager::WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    System::Clock::Timestamp mLastAPDemandTime;
    System::Clock::Timeout mWiFiStationReconnectInterval;
    System::Clock::Timeout mWiFiAPIdleTimeout;
    static char sWiFiIfName[Inet::InterfaceId::kMaxIfNameLength];
#endif
};

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
inline bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPApplicationControlled()
{
    return false;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiAPIdleTimeout()
{
    return System::Clock::kZero;
}

#endif

/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * chip applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager & ConnectivityMgr()
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * chip applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the ESP32 platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl()
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
