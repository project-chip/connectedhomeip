/*
 *    Copyright (c) 2022 Project CHIP Authors
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

namespace chip {
namespace DeviceLayer {

enum
{
    kWiFiOnInitDone = DeviceEventType::kRange_PublicPlatformSpecific,
    kWiFiOnScanDone,
    kWiFiOnConnected,
    kGotIpAddress,
    kGotIpv6Address,
    kWiFiOnDisconnected,
};

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
    friend void netif_status_callback(struct netif * netif);

public:
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    bool _IsWiFiStationConnected(void);
    WiFiStationState GetWiFiStationState(void);
    void ChangeWiFiStationState(WiFiStationState newState);
    void OnWiFiStationStateChanged(void);
    void OnWiFiStationConnected(void);
    void OnWiFiStationDisconnected(void);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    void OnConnectivityChanged(struct netif * interface);
    void OnIPv4AddressAvailable();
    void OnIPv6AddressAvailable();
#endif

private:
    // ===== Members that implement the ConnectivityManager abstract interface.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    WiFiStationMode mWiFiStationMode;
    WiFiStationState mWiFiStationState;

    BitFlags<GenericConnectivityManagerImpl_WiFi::ConnectivityFlags> mConnectivityFlag;

    bool _IsWiFiStationEnabled(void);
    ConnectivityManager::WiFiStationMode _GetWiFiStationMode();
    CHIP_ERROR _SetWiFiStationMode(WiFiStationMode val);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision();
    void _OnWiFiStationProvisionChange();
    CHIP_ERROR ConnectProvisionedWiFiNetwork();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    enum class ConnectivityFlags : uint16_t{
        kHaveIPv4InternetConnectivity = 0x0001,
        kHaveIPv6InternetConnectivity = 0x0002,
        kAwaitingConnectivity         = 0x0010,
    };
    BitFlags<ConnectivityFlags> mConnectivityFlag;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
    ip4_addr_t m_ip4addr;
    ip6_addr_t m_ip6addr[LWIP_IPV6_NUM_ADDRESSES];
#endif

    void DriveStationState(void);
    static void DriveStationState(::chip::System::Layer * aLayer, void * aAppState);
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

    // ===== Members for internal use by the following friends.
    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;
};

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
inline ConnectivityManager::WiFiStationState ConnectivityManagerImpl::GetWiFiStationState(void)
{
    return mWiFiStationState;
}

inline bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    return mWiFiStationState == kWiFiStationState_Connected;
}

inline ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    return kWiFiStationMode_Enabled;
}
#endif

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
 * that are specific to the Bouffalo Lab platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
