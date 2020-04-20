/*
 *
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

#ifndef CONNECTIVITY_MANAGER_IMPL_H
#define CONNECTIVITY_MANAGER_IMPL_H

#include <Weave/DeviceLayer/ConnectivityManager.h>
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl.h>
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_NoThread.h>
#include <Weave/Profiles/network-provisioning/NetworkProvisioning.h>
#include <Weave/Profiles/weave-tunneling/WeaveTunnelCommon.h>
#include <Weave/Profiles/weave-tunneling/WeaveTunnelConnectionMgr.h>
#include <Weave/Support/FlagUtils.hpp>

#include "esp_event.h"

namespace nl {
namespace Inet {
class IPAddress;
} // namespace Inet
} // namespace nl

namespace nl {
namespace Weave {
namespace DeviceLayer {

class PlatformManagerImpl;

namespace Internal {

class NetworkProvisioningServerImpl;
template<class ImplClass> class GenericNetworkProvisioningServerImpl;

} // namespace Internal

/**
 * Concrete implementation of the ConnectivityManager singleton object for the ESP32 platform.
 */
class ConnectivityManagerImpl final
    : public ConnectivityManager,
      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>
{
    using TunnelConnNotifyReasons = ::nl::Weave::Profiles::WeaveTunnel::WeaveTunnelConnectionMgr::TunnelConnNotifyReasons;

    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

private:

    // ===== Members that implement the ConnectivityManager abstract interface.

    WiFiStationMode _GetWiFiStationMode(void);
    WEAVE_ERROR _SetWiFiStationMode(WiFiStationMode val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationConnected(void);
    uint32_t _GetWiFiStationReconnectIntervalMS(void);
    WEAVE_ERROR _SetWiFiStationReconnectIntervalMS(uint32_t val);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    WiFiAPMode _GetWiFiAPMode(void);
    WEAVE_ERROR _SetWiFiAPMode(WiFiAPMode val);
    bool _IsWiFiAPActive(void);
    bool _IsWiFiAPApplicationControlled(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    uint32_t _GetWiFiAPIdleTimeoutMS(void);
    void _SetWiFiAPIdleTimeoutMS(uint32_t val);
    WEAVE_ERROR _GetAndLogWifiStatsCounters(void);
    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
    ServiceTunnelMode _GetServiceTunnelMode(void);
    WEAVE_ERROR _SetServiceTunnelMode(ServiceTunnelMode val);
    bool _IsServiceTunnelConnected(void);
    bool _IsServiceTunnelRestricted(void);
    bool _HaveServiceConnectivityViaTunnel(void);
    bool _HaveServiceConnectivity(void);
    WEAVE_ERROR _Init(void);
    void _OnPlatformEvent(const WeaveDeviceEvent * event);
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();
    static const char * _WiFiStationModeToStr(WiFiStationMode mode);
    static const char * _WiFiAPModeToStr(WiFiAPMode mode);
    static const char * _ServiceTunnelModeToStr(ServiceTunnelMode mode);

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum WiFiStationState
    {
        kWiFiStationState_NotConnected,
        kWiFiStationState_Connecting,
        kWiFiStationState_Connecting_Succeeded,
        kWiFiStationState_Connecting_Failed,
        kWiFiStationState_Connected,
        kWiFiStationState_Disconnecting,
    };

    enum WiFiAPState
    {
        kWiFiAPState_NotActive,
        kWiFiAPState_Activating,
        kWiFiAPState_Active,
        kWiFiAPState_Deactivating,
    };

    enum Flags
    {
        kFlag_HaveIPv4InternetConnectivity      = 0x0001,
        kFlag_HaveIPv6InternetConnectivity      = 0x0002,
        kFlag_ServiceTunnelStarted              = 0x0004,
        kFlag_ServiceTunnelUp                   = 0x0008,
        kFlag_AwaitingConnectivity              = 0x0010,
    };

    uint64_t mLastStationConnectFailTime;
    uint64_t mLastAPDemandTime;
    WiFiStationMode mWiFiStationMode;
    WiFiStationState mWiFiStationState;
    WiFiAPMode mWiFiAPMode;
    WiFiAPState mWiFiAPState;
    ServiceTunnelMode mServiceTunnelMode;
    uint32_t mWiFiStationReconnectIntervalMS;
    uint32_t mWiFiAPIdleTimeoutMS;
    uint16_t mFlags;

    void DriveStationState(void);
    void OnStationConnected(void);
    void OnStationDisconnected(void);
    void ChangeWiFiStationState(WiFiStationState newState);
    static void DriveStationState(::nl::Weave::System::Layer * aLayer, void * aAppState, ::nl::Weave::System::Error aError);

    void DriveAPState(void);
    WEAVE_ERROR ConfigureWiFiAP(void);
    void ChangeWiFiAPState(WiFiAPState newState);
    static void DriveAPState(::nl::Weave::System::Layer * aLayer, void * aAppState, ::nl::Weave::System::Error aError);

    void UpdateInternetConnectivityState(void);
    void OnStationIPv4AddressAvailable(const system_event_sta_got_ip_t & got_ip);
    void OnStationIPv4AddressLost(void);
    void OnIPv6AddressAvailable(const system_event_got_ip6_t & got_ip);

    void DriveServiceTunnelState(void);
    static void DriveServiceTunnelState(::nl::Weave::System::Layer * aLayer, void * aAppState, ::nl::Weave::System::Error aError);

    static const char * WiFiStationStateToStr(WiFiStationState state);
    static const char * WiFiAPStateToStr(WiFiAPState state);
    static void RefreshMessageLayer(void);
    static void HandleServiceTunnelNotification(TunnelConnNotifyReasons reason, WEAVE_ERROR err, void *appCtxt);
};

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

inline uint32_t ConnectivityManagerImpl::_GetWiFiStationReconnectIntervalMS(void)
{
    return mWiFiStationReconnectIntervalMS;
}

inline ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::_GetWiFiAPMode(void)
{
    return mWiFiAPMode;
}

inline bool ConnectivityManagerImpl::_IsWiFiAPActive(void)
{
    return mWiFiAPState == kWiFiAPState_Active;
}

inline uint32_t ConnectivityManagerImpl::_GetWiFiAPIdleTimeoutMS(void)
{
    return mWiFiAPIdleTimeoutMS;
}

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return ::nl::GetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity);
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return ::nl::GetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity);
}

inline ConnectivityManager::ServiceTunnelMode ConnectivityManagerImpl::_GetServiceTunnelMode(void)
{
    return mServiceTunnelMode;
}

inline bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return mWiFiStationState != kWiFiStationState_Connecting;
}

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return HaveServiceConnectivityViaTunnel() || HaveServiceConnectivityViaThread();
}


/**
 * Returns the public interface of the ConnectivityManager singleton object.
 *
 * Weave applications should use this to access features of the ConnectivityManager object
 * that are common to all platforms.
 */
inline ConnectivityManager & ConnectivityMgr(void)
{
    return ConnectivityManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ConnectivityManager singleton object.
 *
 * Weave applications can use this to gain access to features of the ConnectivityManager
 * that are specific to the ESP32 platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // CONNECTIVITY_MANAGER_IMPL_H
