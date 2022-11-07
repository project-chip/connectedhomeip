/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
// Android WPA APIs
#endif

namespace chip {
namespace Inet {
class IPAddress;
} // namespace Inet
} // namespace chip

namespace chip {
namespace DeviceLayer {

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
struct GDBusWpaSupplicant
{
    enum
    {
        INIT,
        WPA_CONNECTING,
        WPA_CONNECTED,
        WPA_NOT_CONNECTED,
        WPA_NO_INTERFACE_PATH,
        WPA_GOT_INTERFACE_PATH,
        WPA_INTERFACE_CONNECTED,
    } state;

    enum
    {
        WIFI_SCANNING_IDLE,
        WIFI_SCANNING,
    } scanState;
};
#endif

/**
 * Concrete implementation of the ConnectivityManager singleton object for Android platforms.
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
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

public:
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    void StartWiFiManagement();
#endif

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    CHIP_ERROR _Init();
    void _OnPlatformEvent(const ChipDeviceEvent * event);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    WiFiStationMode _GetWiFiStationMode();
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    System::Clock::Timeout _GetWiFiStationReconnectInterval();
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool _IsWiFiStationEnabled();
    bool _IsWiFiStationConnected();
    bool _IsWiFiStationApplicationControlled();
    bool _IsWiFiStationProvisioned();
    void _ClearWiFiStationProvision();
    bool _CanStartWiFiScan();

    WiFiAPMode _GetWiFiAPMode();
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPActive();
    bool _IsWiFiAPApplicationControlled();
    void _DemandStartWiFiAP();
    void _StopOnDemandWiFiAP();
    void _MaintainOnDemandWiFiAP();
    System::Clock::Timeout _GetWiFiAPIdleTimeout();
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);

    static BitFlags<ConnectivityFlags> mConnectivityFlag;
#endif

    // ==================== ConnectivityManager Private Methods ====================

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    void DriveAPState();
    CHIP_ERROR ConfigureWiFiAP();
    void ChangeWiFiAPState(WiFiAPState newState);
    static void DriveAPState(::chip::System::Layer * aLayer, void * aAppState);
#endif

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr();
    friend ConnectivityManagerImpl & ConnectivityMgrImpl();

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    ConnectivityManager::WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    System::Clock::Timestamp mLastAPDemandTime;
    System::Clock::Timeout mWiFiStationReconnectInterval;
    System::Clock::Timeout mWiFiAPIdleTimeout;
};

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
inline ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode()
{
    return mWiFiAPMode;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPActive()
{
    return mWiFiAPState == kWiFiAPState_Active;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPApplicationControlled()
{
    return mWiFiAPMode == kWiFiAPMode_ApplicationControlled;
}

inline System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiAPIdleTimeout()
{
    return mWiFiAPIdleTimeout;
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
