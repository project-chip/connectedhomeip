/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    Copyright 2023 NXP
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

#include <lib/dnssd/Constants.h>
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
extern "C" {
#include "wlan.h"
}
#include "event_groups.h"
#include "lwip/netif.h"
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for NXP platforms.
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
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
#endif

                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

public:
    CHIP_ERROR ProvisionWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    void StartWiFiManagement();
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    CHIP_ERROR _SetPollingInterval(System::Clock::Milliseconds32 pollingInterval);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#if CHIP_ENABLE_OPENTHREAD
    Inet::InterfaceId GetExternalInterface();
    Inet::InterfaceId GetThreadInterface();
    const char * GetHostName() { return sInstance.mHostname; }
#endif

#endif

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
    bool _HaveServiceConnectivity(void);
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    void ProcessWlanEvent(enum wlan_event_reason wlanEvent);
    WiFiStationMode _GetWiFiStationMode();
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    ConnectivityManager::WiFiStationState _GetWiFiStationState(void);
    void _SetWiFiStationState(ConnectivityManager::WiFiStationState val);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);

    static void ConnectNetworkTimerHandler(::chip::System::Layer * aLayer, void * context);

    bool _IsWiFiStationEnabled();
    bool _IsWiFiStationConnected();
    bool _IsWiFiStationApplicationControlled();
    CHIP_ERROR _DisconnectNetwork(void);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    ConnectivityManager::WiFiStationState mWiFiStationState;
    ConnectivityManager::WiFiAPMode mWiFiAPMode;
    uint32_t mWiFiStationReconnectIntervalMS;
    bool mBorderRouterInit = false;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    bool mWifiManagerInit = false;

    enum WiFiEventGroup{
        kWiFiEventGroup_WiFiStationModeBit = (1 << 0),
    };

    BitFlags<GenericConnectivityManagerImpl_WiFi::ConnectivityFlags> mFlags;
    static netif_ext_callback_t sNetifCallback;
    static constexpr uint32_t kWlanInitWaitMs = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;

#if CHIP_ENABLE_OPENTHREAD
    static constexpr uint8_t kMaxIp6Addr = 3;

    Inet::InterfaceId mThreadNetIf;
    Inet::InterfaceId mExternalNetIf;

    char mHostname[chip::Dnssd::kHostNameMaxLength + 1] = "";
    otIp6Address mIp6AddrList[kMaxIp6Addr];
    uint32_t mIp6AddrNum = 0;
#endif

    static int _WlanEventCallback(enum wlan_event_reason event, void * data);
    static void _NetifExtCallback(struct netif * netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args);

    void OnStationConnected(void);
    void OnStationDisconnected(void);
    void UpdateInternetConnectivityState(void);
#if CHIP_ENABLE_OPENTHREAD
    void BrHandleStateChange(bool bLinkState);
    void UpdateMdnsHost(void);
    bool UpdateIp6AddrList(void);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_THREAD */
#endif
    /* CHIP_DEVICE_CONFIG_ENABLE_WPA */
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
    return _HaveServiceConnectivity();
}
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
inline ConnectivityManager::WiFiStationState ConnectivityManagerImpl::_GetWiFiStationState(void)
{
    return mWiFiStationState;
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
 * that are specific to the NXP platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
