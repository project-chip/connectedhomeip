/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif

#include "filogic.h"

namespace Inet {
class IPAddress;
} // namespace Inet

namespace chip {
namespace DeviceLayer {

// class PlatformManagerImpl;

/**
 * Concrete implementation of the ConnectivityManager singleton object for MediaTek Genio platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>
#endif
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
#if 0 // CHIP_DEVICE_CONFIG_ENABLE_THREAD
    bool _HaveServiceConnectivity(void);
#endif
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    void OnInternetConnectivityChangeEvent(const ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    CHIP_ERROR WiFiInit(void);
    WiFiStationMode GetFilogicStationMode(void);
    WiFiAPMode GetFilogicAPMode(void);
    filogic_wifi_opmode_t GetFilogicNextOpMode(WiFiStationMode wifiStationMode, WiFiAPMode wifiAPMode);
    void SetFlogicNextMode(filogic_wifi_opmode_t nextMode);
    void _OnWiFiPlatformEvent(const ChipDeviceEvent * event);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    using Flags = GenericConnectivityManagerImpl_WiFi::ConnectivityFlags;
    WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(WiFiStationMode val);
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    CHIP_ERROR _GetAndLogWifiStatsCounters(void);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationConnected(void);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();
    System::Clock::Timeout _GetWiFiStationReconnectInterval(void);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    void ChangeWiFiAPState(WiFiAPState newState);
    void DriveAPState(void);
    CHIP_ERROR ConfigureWiFiAP(void);
    static void DriveAPState(::chip::System::Layer * aLayer, void * aAppState);
    WiFiAPMode _GetWiFiAPMode(void);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPApplicationControlled(void);
    bool _IsWiFiAPActive(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    System::Clock::Timeout _GetWiFiAPIdleTimeout(void);
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);
#endif
    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    static ConnectivityManagerImpl sInstance;
#endif

    // ===== Private members reserved for use by this class only.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    System::Clock::Timestamp mLastStationConnectFailTime;
    WiFiStationMode mWiFiStationMode;
    WiFiStationState mWiFiStationState;
    System::Clock::Timeout mWiFiStationReconnectInterval;
    BitFlags<Flags> mFlags;

    void DriveStationState(void);
    void OnStationConnected(void);
    void OnStationDisconnected(void);
    void ChangeWiFiStationState(WiFiStationState newState);
    static void DriveStationState(::chip::System::Layer * aLayer, void * aAppState);

    void UpdateInternetConnectivityState(bool haveIPv4Conn, bool haveIPv6Conn, const uint8_t * ipAddr);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    System::Clock::Timeout mWiFiAPIdleTimeout;
    System::Clock::Timestamp mLastAPDemandTime;
#endif
    void * mFilogicCtx;
};

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    return mFlags.Has(Flags::kHaveIPv4InternetConnectivity);
#else
    return false;
#endif
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    return mFlags.Has(Flags::kHaveIPv6InternetConnectivity);
#else
    return false;
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
inline bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled(void)
{
    return mWiFiStationMode == kWiFiStationMode_ApplicationControlled;
}

inline bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    return mWiFiStationState == kWiFiStationState_Connected;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval(void)
{
    return mWiFiStationReconnectInterval;
}

inline bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return mWiFiStationState != kWiFiStationState_Connecting;
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
inline ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode(void)
{
    return mWiFiAPMode;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPApplicationControlled(void)
{
    return mWiFiAPMode == kWiFiAPMode_ApplicationControlled;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPActive(void)
{
    return mWiFiAPState == kWiFiAPState_Active;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiAPIdleTimeout(void)
{
    return mWiFiAPIdleTimeout;
}
#endif

#if 0 // CHIP_DEVICE_CONFIG_ENABLE_THREAD
inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return _HaveServiceConnectivityViaThread();
}
#endif

/**
 *
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
 * that are specific to the Genio platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
