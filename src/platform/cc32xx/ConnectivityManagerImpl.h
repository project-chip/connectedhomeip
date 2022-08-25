/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/**
 *    @file
 *          Platform-specific connectivity manager class
 *          for the Texas Instruments CC32XX platform.
 *
 * NOTE: currently a bare-bones implementation to allow for building.
 */

#pragma once

#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.h>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>

extern "C" {
#include "lwip_if.h"
}

namespace chip {
namespace Inet {

class IPAddress;

} // namespace Inet
} // namespace chip

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for the CC32XX platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
                                      public Internal::GenericConnectivityManagerImpl_UDP<ConnectivityManagerImpl>,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                      public Internal::GenericConnectivityManagerImpl_TCP<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>

{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(WiFiStationMode val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationConnected(void);
    uint32_t _GetWiFiStationReconnectIntervalMS(void);
    CHIP_ERROR _SetWiFiStationReconnectIntervalMS(uint32_t val);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    WiFiAPMode _GetWiFiAPMode(void);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPActive(void);
    bool _IsWiFiAPApplicationControlled(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    uint32_t _GetWiFiAPIdleTimeoutMS(void);
    void _SetWiFiAPIdleTimeoutMS(uint32_t val);
    CHIP_ERROR _GetAndLogWifiStatsCounters(void);
    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
    bool _HaveServiceConnectivity(void);
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    uint64_t mLastStationConnectFailTime;
    uint64_t mLastAPDemandTime;
    WiFiStationMode mWiFiStationMode;
    WiFiStationState mWiFiStationState;
    WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    uint32_t mWiFiStationReconnectIntervalMS;
    uint32_t mWiFiAPIdleTimeoutMS;
    uint16_t mFlags;

    void OnStationConnected(void);
    void OnStationDisconnected(void);
    void ChangeWiFiStationState(WiFiStationState newState);

    static void _OnLwipEvent(struct netif * pNetIf, NetIfStatus_e status, void * pParams);
    static void _OnIpAcquired(intptr_t arg);
    CHIP_ERROR ConfigureWiFiAP(void);
    void ChangeWiFiAPState(WiFiAPState newState);

    void UpdateInternetConnectivityState(void);
    void OnStationIPv4AddressAvailable();
    void OnStationIPv4AddressLost(void);
    void OnIPv6AddressAvailable();

    static void RefreshMessageLayer(void);
};

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return false;
}

/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * chip applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager & ConnectivityMgr(void)
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * chip applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the CC32XX platform
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
