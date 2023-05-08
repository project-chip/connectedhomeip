/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif

#include <lib/support/BitFlags.h>

#include <wifi_conf.h>

namespace chip {

namespace Inet {
class IPAddress;
} // namespace Inet

namespace DeviceLayer {

class PlatformManagerImpl;

class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                                      public Internal::GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>
#else
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>
#endif
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

public:
    void ChangeWiFiStationState(WiFiStationState newState);

private:
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    using Flags = GenericConnectivityManagerImpl_WiFi::ConnectivityFlags;
    // ===== Members that implement the ConnectivityManager abstract interface.

    WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(WiFiStationMode val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationConnected(void);
    System::Clock::Timeout _GetWiFiStationReconnectInterval(void);
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    WiFiAPMode _GetWiFiAPMode(void);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPActive(void);
    bool _IsWiFiAPApplicationControlled(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    System::Clock::Timeout _GetWiFiAPIdleTimeout(void);
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);
    CHIP_ERROR _GetAndLogWiFiStatsCounters(void);
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();

    // ===== Private members reserved for use by this class only.

    System::Clock::Timestamp mLastStationConnectFailTime;
    System::Clock::Timestamp mLastAPDemandTime;
    WiFiStationMode mWiFiStationMode;
    WiFiStationState mWiFiStationState;
    WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    System::Clock::Timeout mWiFiStationReconnectInterval;
    System::Clock::Timeout mWiFiAPIdleTimeout;
    BitFlags<Flags> mFlags;

    void DriveStationState(void);
    void OnStationConnected(void);
    void OnStationDisconnected(void);
    static void DriveStationState(::chip::System::Layer * aLayer, void * aAppState);

    void DriveAPState(void);
    CHIP_ERROR ConfigureWiFiAP(void);
    void ChangeWiFiAPState(WiFiAPState newState);
    static void DriveAPState(::chip::System::Layer * aLayer, void * aAppState);

    void UpdateInternetConnectivityState(void);
    void OnStationIPv4AddressAvailable(void);
    void OnStationIPv4AddressLost(void);
    void OnIPv6AddressAvailable(void);

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;
    static void RefreshMessageLayer(void);
    static void RtkWiFiStationConnectedHandler(char * buf, int buf_len, int flags, void * userdata);
    static void RtkWiFiStationDisconnectedHandler(char * buf, int buf_len, int flags, void * userdata);
    static void RtkWiFiScanCompletedHandler(void);
    void DHCPProcess(void);
    static void DHCPProcessThread(void * param);
    static int conn_callback_dispatcher(void * object);
};

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
inline bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled(void)
{
    return mWiFiStationMode == kWiFiStationMode_ApplicationControlled;
}

inline bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    return mWiFiStationState == kWiFiStationState_Connected;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPApplicationControlled(void)
{
    return mWiFiAPMode == kWiFiAPMode_ApplicationControlled;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval(void)
{
    return mWiFiStationReconnectInterval;
}

inline ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode(void)
{
    return mWiFiAPMode;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPActive(void)
{
    return mWiFiAPState == kWiFiAPState_Active;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiAPIdleTimeout(void)
{
    return mWiFiAPIdleTimeout;
}

inline bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return mWiFiStationState != kWiFiStationState_Connecting;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

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
 * that are specific to the Ameba platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
