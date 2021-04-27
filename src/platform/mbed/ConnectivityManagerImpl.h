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

#pragma once

#include <platform/ConnectivityManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
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
#include "netsocket/WiFiInterface.h"
#include <inet/IPAddress.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for mbed platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
                                      public Internal::GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoBLE<ConnectivityManagerImpl>,
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                                      public Internal::GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoThread<ConnectivityManagerImpl>
#endif

{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

public:
    CHIP_ERROR ProvisionWiFiNetwork(const char * ssid, const char * key);
    void StartWiFiManagement() {}

private:
    // ===== Members that implement the ConnectivityManager abstract interface.
    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
    bool _HaveServiceConnectivity(void);
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    void _ProcessInterfaceChange(nsapi_connection_status_t new_status);
    void OnInterfaceEvent(nsapi_event_t event, intptr_t data);
    WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(WiFiStationMode val);

    bool _IsWiFiStationConnected(void);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    bool _IsWiFiStationApplicationControlled(void);
    CHIP_ERROR _SetWiFiAPMode(WiFiAPMode val);

    CHIP_ERROR OnStationConnected();
    CHIP_ERROR OnStationDisconnected();
    CHIP_ERROR OnStationConnecting();
    const char * status2str(nsapi_connection_status_t sec);
    ::chip::DeviceLayer::Internal::WiFiAuthSecurityType NsapiToNetworkSecurity(nsapi_security_t nsapi_security);
    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;
    WiFiStationMode mWiFiStationMode;
    WiFiStationState mWiFiStationState;
    WiFiAPMode mWiFiAPMode;
    uint32_t mWiFiStationReconnectIntervalMS;
    uint32_t mWiFiAPIdleTimeoutMS;
    WiFiInterface * _interface;
    nsapi_security_t _security = NSAPI_SECURITY_WPA_WPA2;
    Inet::IPAddress mIp4Address;
    Inet::IPAddress mIp6Address;
    bool mIsProvisioned : 1;
};

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return mWiFiStationState == kWiFiStationState_Connected;
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return false;
}

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return mWiFiStationState == kWiFiStationState_Connected;
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
 * that are specific to the mbed platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
