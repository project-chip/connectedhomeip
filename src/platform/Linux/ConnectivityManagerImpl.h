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
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#else
#include <platform/internal/GenericConnectivityManagerImpl_NoWiFi.h>
#endif
#include <support/FlagUtils.hpp>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/Linux/dbus/DBusWpa.h>
#include <platform/Linux/dbus/DBusWpaInterface.h>
#include <platform/Linux/dbus/DBusWpaNetwork.h>
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

    WpaFiW1Wpa_supplicant1 * proxy;
    WpaFiW1Wpa_supplicant1Interface * iface;
    gchar * interfacePath;
    gchar * networkPath;
};
#endif

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
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
                                      public Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>,
#else
                                      public Internal::GenericConnectivityManagerImpl_NoWiFi<ConnectivityManagerImpl>,
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

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    bool _IsWiFiStationConnected(void);
    bool _CanStartWiFiScan();
    static void _OnWpaProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data);
    static void _OnWpaInterfaceRemoved(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                       gpointer user_data);
    static void _OnWpaInterfaceAdded(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties, gpointer user_data);
    static void _OnWpaInterfaceReady(GObject * source_object, GAsyncResult * res, gpointer user_data);
    static void _OnWpaInterfaceProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data);

    static uint16_t mConnectivityFlag;
    static struct GDBusWpaSupplicant mWpaSupplicant;
#endif

    // ===== Members for internal use by the following friends.

    friend ConnectivityManager & ConnectivityMgr(void);
    friend ConnectivityManagerImpl & ConnectivityMgrImpl(void);

    static ConnectivityManagerImpl sInstance;
};

inline bool ConnectivityManagerImpl::_HaveServiceConnectivity(void)
{
    return _HaveServiceConnectivityViaThread();
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
