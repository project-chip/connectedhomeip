/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef CONNECTIVITY_MANAGER_IMPL_H
#define CONNECTIVITY_MANAGER_IMPL_H

#include <platform/ConnectivityManager.h>
#include <platform/DeviceNetworkInfo.h>
#include <platform/internal/GenericConnectivityManagerImpl.h>
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
#include <platform/internal/GenericConnectivityManagerImpl_NoTunnel.h>
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#include <support/FlagUtils.hpp>

#include <DBusWpa.h>
#include <DBusWpaInterface.h>
#include <DBusWpaNetwork.h>

namespace chip {
namespace Inet {
class IPAddress;
} // namespace Inet
} // namespace chip

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the ConnectivityManager singleton object for Linux platforms.
 */
class ConnectivityManagerImpl final : public ConnectivityManager,
                                      public Internal::GenericConnectivityManagerImpl<ConnectivityManagerImpl>,
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
                                      public Internal::GenericConnectivityManagerImpl_NoTunnel<ConnectivityManagerImpl>
{
    // Allow the ConnectivityManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ConnectivityManager;

private:
    // ===== Members that implement the ConnectivityManager abstract interface.

    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
    bool _HaveServiceConnectivity(void);
    CHIP_ERROR _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

    // ===== WiFi interface.

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
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();
    static const char * _WiFiStationModeToStr(WiFiStationMode mode);
    static const char * _WiFiAPModeToStr(WiFiAPMode mode);

    // Wifi station state machine, the struct can only be modified inside _WiFiStateMachine
    struct _WiFiState
    {
        enum
        {
            INIT,
            DISABLED,
            ENABLED,
            DBUS_CONNECTING,
            DBUS_CONNECTED,
            DBUS_GETTING_INTERFACE_PATH,
            DBUS_NO_INTERFACE,
            DBUS_CREATING_INTERFACE,
            DBUS_GOT_INTERFACE_PATH,
            DBUS_GETTING_INTERFACE,
            DBUS_GOT_INTERFACE,
            DBUS_ERROR,
        } state = INIT;

        // two nested state in DBUS_GOT_INTERFACE
        enum
        {
            WIFI_SCANNING_IDLE,
            WIFI_SCANNING,
        } scanState = WIFI_SCANNING_IDLE;

        enum
        {
            WIFI_IDLE,
            WIFI_ADDING_NETWORK,
            WIFI_ADDED_NETWORK,
            WIFI_SELECTING_NETWORK,
            WIFI_CONNECTING,
            WIFI_CONNECTED,
            WIFI_DISCONNECTING,
            WIFI_ERROR,
        } connectState = WIFI_IDLE;

        WeaveDBusFiW1Wpa_supplicant1 * wpa            = nullptr;
        gchar * interfacePath                         = nullptr;
        WeaveDBusFiW1Wpa_supplicant1Interface * iface = nullptr;
        gchar * networkPath                           = nullptr;
    } _WiFiState;
    void _WiFiThread();
    void _WiFiStateMachine();

    friend class ::chip::DeviceLayer::Internal::NetworkProvisioningServerImpl;
    bool wifiProvisioned = false;
    bool wifiScanPending = false;
    ::chip::DeviceLayer::DeviceNetworkInfo wifiNetworkInfo;

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum Flags
    {
        kFlag_HaveIPv4InternetConnectivity = 0x0001,
        kFlag_HaveIPv6InternetConnectivity = 0x0002,
        kFlag_AwaitingConnectivity         = 0x0010,
    };

    WiFiStationMode mWiFiStationMode;
    uint16_t mFlags;
};

inline bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled(void)
{
    return mWiFiStationMode == kWiFiStationMode_ApplicationControlled;
}

inline bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    return _WiFiState.connectState == _WiFiState.WIFI_CONNECTED;
}

inline bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return (mFlags & kFlag_HaveIPv4InternetConnectivity) != 0;
}

inline bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return (mFlags & kFlag_HaveIPv6InternetConnectivity) != 0;
}

inline bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return _WiFiState.state == _WiFiState.DBUS_GOT_INTERFACE && _WiFiState.scanState == _WiFiState.WIFI_SCANNING_IDLE;
}

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
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
 * that are specific to the ESP32 platform.
 */
inline ConnectivityManagerImpl & ConnectivityMgrImpl(void)
{
    return ConnectivityManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CONNECTIVITY_MANAGER_IMPL_H
