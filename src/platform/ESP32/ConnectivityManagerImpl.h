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

#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoBLE.h>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_NoThread.h>
#include <support/BitFlags.h>

#include "esp_event.h"

namespace Inet {
class IPAddress;
} // namespace Inet

namespace chip {
namespace DeviceLayer {

class PlatformManagerImpl;

/**
 * Concrete implementation of the ConnectivityManager singleton object for the ESP32 platform.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
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
    using Flags = GenericConnectivityManagerImpl_WiFi::ConnectivityFlags;
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
    BitFlags<Flags> mFlags;

    void DriveStationState(void);
    void OnStationConnected(void);
    void OnStationDisconnected(void);
    void ChangeWiFiStationState(WiFiStationState newState);
    static void DriveStationState(::chip::System::Layer * aLayer, void * aAppState, ::CHIP_ERROR aError);

    void DriveAPState(void);
    CHIP_ERROR ConfigureWiFiAP(void);
    void ChangeWiFiAPState(WiFiAPState newState);
    static void DriveAPState(::chip::System::Layer * aLayer, void * aAppState, ::CHIP_ERROR aError);

    void UpdateInternetConnectivityState(void);
    void OnStationIPv4AddressAvailable(const ip_event_got_ip_t & got_ip);
    void OnStationIPv4AddressLost(void);
    void OnIPv6AddressAvailable(const ip_event_got_ip6_t & got_ip);

    static void RefreshMessageLayer(void);
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
    return mFlags.Has(Flags::kHaveIPv4InternetConnectivity);
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return mFlags.Has(Flags::kHaveIPv6InternetConnectivity);
}

inline bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return mWiFiStationState != kWiFiStationState_Connecting;
}

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return HaveServiceConnectivityViaThread();
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
