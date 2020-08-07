/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>

#include <new>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace {
const char kWpaSupplicantServiceName[] = "fi.w1.wpa_supplicant1";
const char kWpaSupplicantObjectPath[]  = "/fi/w1/wpa_supplicant1";
} // namespace
#endif

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    mConnectivityFlag            = 0;
    mWpaSupplicant.state         = GDBusWpaSupplicant::INIT;
    mWpaSupplicant.scanState     = GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
    mWpaSupplicant.proxy         = NULL;
    mWpaSupplicant.iface         = NULL;
    mWpaSupplicant.interfacePath = NULL;
    mWpaSupplicant.networkPath   = NULL;

    wpa_fi_w1_wpa_supplicant1_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                kWpaSupplicantObjectPath, NULL, _OnWpaProxyReady, NULL);
#endif

    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

uint16_t ConnectivityManagerImpl::mConnectivityFlag;
struct GDBusWpaSupplicant ConnectivityManagerImpl::mWpaSupplicant;

bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity(void)
{
    return ((mConnectivityFlag & kFlag_HaveIPv4InternetConnectivity) != 0);
}

bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity(void)
{
    return ((mConnectivityFlag & kFlag_HaveIPv6InternetConnectivity) != 0);
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    bool ret            = false;
    const gchar * state = NULL;

    if (mWpaSupplicant.state != GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: _IsWiFiStationConnected: interface not connected");
        return false;
    }

    state = wpa_fi_w1_wpa_supplicant1_interface_get_state(mWpaSupplicant.iface);
    ChipLogProgress(DeviceLayer, "wpa_supplicant: wpa_fi_w1_wpa_supplicant1_interface_get_state: %s", state);

    if (g_strcmp0(state, "completed") == 0)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi already connected");
        SetFlag(mConnectivityFlag, kFlag_HaveIPv4InternetConnectivity, true);
        SetFlag(mConnectivityFlag, kFlag_HaveIPv6InternetConnectivity, true);
        ret = true;
    }

    return ret;
}

bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return mWpaSupplicant.state == GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED &&
        mWpaSupplicant.scanState == GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
}

void ConnectivityManagerImpl::_OnWpaInterfaceProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err                            = NULL;
    WpaFiW1Wpa_supplicant1Interface * iface = wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus_finish(res, &err);

    if (mWpaSupplicant.iface)
    {
        g_object_unref(mWpaSupplicant.iface);
        mWpaSupplicant.iface = NULL;
    }

    if (iface != NULL && err == NULL)
    {
        mWpaSupplicant.iface = iface;
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to wpa_supplicant interface proxy");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to create wpa_supplicant1 interface proxy %s: %s",
                        mWpaSupplicant.interfacePath, err ? err->message : "unknown error");

        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NOT_CONNECTED;
    }

    if (err != NULL)
        g_error_free(err);
}

void ConnectivityManagerImpl::_OnWpaInterfaceReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = NULL;

    gboolean result =
        wpa_fi_w1_wpa_supplicant1_call_get_interface_finish(mWpaSupplicant.proxy, &mWpaSupplicant.interfacePath, res, &err);
    if (result)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_GOT_INTERFACE_PATH;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface: %s", mWpaSupplicant.interfacePath);

        wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                              mWpaSupplicant.interfacePath, NULL, _OnWpaInterfaceProxyReady, NULL);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: can't find interface %s: %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME,
                        err ? err->message : "unknown error");

        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NO_INTERFACE_PATH;

        if (mWpaSupplicant.interfacePath)
        {
            g_free(mWpaSupplicant.interfacePath);
            mWpaSupplicant.interfacePath = NULL;
        }
    }

    if (err != NULL)
        g_error_free(err);
}

void ConnectivityManagerImpl::_OnWpaInterfaceAdded(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                                   gpointer user_data)
{
    if (mWpaSupplicant.interfacePath)
        return;

    mWpaSupplicant.interfacePath = const_cast<gchar *>(path);
    if (mWpaSupplicant.interfacePath)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_GOT_INTERFACE_PATH;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface added: %s", mWpaSupplicant.interfacePath);

        wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                              mWpaSupplicant.interfacePath, NULL, _OnWpaInterfaceProxyReady, NULL);
    }
}

void ConnectivityManagerImpl::_OnWpaInterfaceRemoved(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                                     gpointer user_data)
{
    if (mWpaSupplicant.interfacePath == NULL)
        return;

    if (g_strcmp0(mWpaSupplicant.interfacePath, path) == 0)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface removed: %s", path);

        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NO_INTERFACE_PATH;

        if (mWpaSupplicant.interfacePath)
        {
            g_free(mWpaSupplicant.interfacePath);
            mWpaSupplicant.interfacePath = NULL;
        }

        if (mWpaSupplicant.iface)
        {
            g_object_unref(mWpaSupplicant.iface);
            mWpaSupplicant.iface = NULL;
        }

        mWpaSupplicant.scanState = GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
    }
}

void ConnectivityManagerImpl::_OnWpaProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = NULL;

    mWpaSupplicant.proxy = wpa_fi_w1_wpa_supplicant1_proxy_new_for_bus_finish(res, &err);
    if (mWpaSupplicant.proxy != NULL && err == NULL)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_CONNECTED;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to wpa_supplicant proxy");

        g_signal_connect(mWpaSupplicant.proxy, "interface-added", G_CALLBACK(_OnWpaInterfaceAdded), NULL);

        g_signal_connect(mWpaSupplicant.proxy, "interface-removed", G_CALLBACK(_OnWpaInterfaceRemoved), NULL);

        wpa_fi_w1_wpa_supplicant1_call_get_interface(mWpaSupplicant.proxy, CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, NULL,
                                                     _OnWpaInterfaceReady, NULL);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to create wpa_supplicant proxy %s",
                        err ? err->message : "unknown error");
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NOT_CONNECTED;
    }

    if (err != NULL)
        g_error_free(err);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace DeviceLayer
} // namespace chip
