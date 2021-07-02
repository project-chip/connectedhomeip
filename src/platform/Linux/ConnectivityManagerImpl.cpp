/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <cstdlib>
#include <new>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.cpp>
#endif

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace {
const char kWpaSupplicantServiceName[] = "fi.w1.wpa_supplicant1";
const char kWpaSupplicantObjectPath[]  = "/fi/w1/wpa_supplicant1";

constexpr uint16_t kWiFi_BAND_2_4_GHZ = 2400;
constexpr uint16_t kWiFi_BAND_5_0_GHZ = 5000;

uint16_t Map2400MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inChannel >= 1 && inChannel <= 13)
    {
        frequency = static_cast<uint16_t>(2412 + ((inChannel - 1) * 5));
    }
    else if (inChannel == 14)
    {
        frequency = 2484;
    }

    return frequency;
}

uint16_t Map5000MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    switch (inChannel)
    {

    case 183:
        frequency = 4915;
        break;
    case 184:
        frequency = 4920;
        break;
    case 185:
        frequency = 4925;
        break;
    case 187:
        frequency = 4935;
        break;
    case 188:
        frequency = 4940;
        break;
    case 189:
        frequency = 4945;
        break;
    case 192:
        frequency = 4960;
        break;
    case 196:
        frequency = 4980;
        break;
    case 7:
        frequency = 5035;
        break;
    case 8:
        frequency = 5040;
        break;
    case 9:
        frequency = 5045;
        break;
    case 11:
        frequency = 5055;
        break;
    case 12:
        frequency = 5060;
        break;
    case 16:
        frequency = 5080;
        break;
    case 34:
        frequency = 5170;
        break;
    case 36:
        frequency = 5180;
        break;
    case 38:
        frequency = 5190;
        break;
    case 40:
        frequency = 5200;
        break;
    case 42:
        frequency = 5210;
        break;
    case 44:
        frequency = 5220;
        break;
    case 46:
        frequency = 5230;
        break;
    case 48:
        frequency = 5240;
        break;
    case 52:
        frequency = 5260;
        break;
    case 56:
        frequency = 5280;
        break;
    case 60:
        frequency = 5300;
        break;
    case 64:
        frequency = 5320;
        break;
    case 100:
        frequency = 5500;
        break;
    case 104:
        frequency = 5520;
        break;
    case 108:
        frequency = 5540;
        break;
    case 112:
        frequency = 5560;
        break;
    case 116:
        frequency = 5580;
        break;
    case 120:
        frequency = 5600;
        break;
    case 124:
        frequency = 5620;
        break;
    case 128:
        frequency = 5640;
        break;
    case 132:
        frequency = 5660;
        break;
    case 136:
        frequency = 5680;
        break;
    case 140:
        frequency = 5700;
        break;
    case 149:
        frequency = 5745;
        break;
    case 153:
        frequency = 5765;
        break;
    case 157:
        frequency = 5785;
        break;
    case 161:
        frequency = 5805;
        break;
    case 165:
        frequency = 5825;
        break;
    }

    return frequency;
}

static uint16_t MapFrequency(const uint16_t inBand, const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inBand == kWiFi_BAND_2_4_GHZ)
    {
        frequency = Map2400MHz(inChannel);
    }
    else if (inBand == kWiFi_BAND_5_0_GHZ)
    {
        frequency = Map5000MHz(inChannel);
    }

    return frequency;
}
} // namespace
#endif

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mWiFiStationMode                = kWiFiStationMode_Disabled;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
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

BitFlags<Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>::ConnectivityFlags>
    ConnectivityManagerImpl::mConnectivityFlag;
struct GDBusWpaSupplicant ConnectivityManagerImpl::mWpaSupplicant;

bool ConnectivityManagerImpl::_HaveIPv4InternetConnectivity()
{
    return mConnectivityFlag.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
}

bool ConnectivityManagerImpl::_HaveIPv6InternetConnectivity()
{
    return mConnectivityFlag.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
}

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        mWiFiStationMode = (mWpaSupplicant.iface != nullptr) ? kWiFiStationMode_Enabled : kWiFiStationMode_Disabled;
    }

    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;
exit:
    return err;
}

uint32_t ConnectivityManagerImpl::_GetWiFiStationReconnectIntervalMS()
{
    return mWiFiStationReconnectIntervalMS;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectIntervalMS(uint32_t val)
{
    mWiFiStationReconnectIntervalMS = val;

    return CHIP_NO_ERROR;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled()
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected()
{
    bool ret            = false;
    const gchar * state = nullptr;

    if (mWpaSupplicant.state != GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: _IsWiFiStationConnected: interface not connected");
        return false;
    }

    state = wpa_fi_w1_wpa_supplicant1_interface_get_state(mWpaSupplicant.iface);
    if (g_strcmp0(state, "completed") == 0)
    {
        mConnectivityFlag.Set(ConnectivityFlags::kHaveIPv4InternetConnectivity)
            .Set(ConnectivityFlags::kHaveIPv6InternetConnectivity);
        ret = true;
    }

    return ret;
}

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled()
{
    return mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned()
{
    bool ret          = false;
    const gchar * bss = nullptr;

    if (mWpaSupplicant.state != GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: _IsWiFiStationProvisioned: interface not connected");
        return false;
    }

    bss = wpa_fi_w1_wpa_supplicant1_interface_get_current_bss(mWpaSupplicant.iface);
    if (g_str_match_string("BSSs", bss, true))
    {
        ret = true;
    }

    return ret;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision()
{
    if (mWpaSupplicant.state != GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: _ClearWiFiStationProvision: interface not connected");
        return;
    }

    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        GError * err = nullptr;
        wpa_fi_w1_wpa_supplicant1_interface_call_remove_all_networks_sync(mWpaSupplicant.iface, nullptr, &err);

        if (err != nullptr)
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to remove all networks with error: %s",
                            err ? err->message : "unknown error");
            g_error_free(err);
        }
    }
}

bool ConnectivityManagerImpl::_CanStartWiFiScan()
{
    return mWpaSupplicant.state == GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED &&
        mWpaSupplicant.scanState == GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
        mWiFiAPMode = val;

        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }

exit:
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP()
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: Demand start WiFi AP");
        mLastAPDemandTime = System::Layer::GetClock_MonotonicMS();
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: Demand start WiFi AP ignored, mode: %s", WiFiAPModeToStr(mWiFiAPMode));
    }
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP()
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: Demand stop WiFi AP");
        mLastAPDemandTime = 0;
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: Demand stop WiFi AP ignored, mode: %s", WiFiAPModeToStr(mWiFiAPMode));
    }
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP()
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        if (mWiFiAPState == kWiFiAPState_Active)
        {
            mLastAPDemandTime = System::Layer::GetClock_MonotonicMS();
        }
    }
}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeoutMS(uint32_t val)
{
    mWiFiAPIdleTimeoutMS = val;
    SystemLayer.ScheduleWork(DriveAPState, NULL);
}

void ConnectivityManagerImpl::_OnWpaInterfaceProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err                            = nullptr;
    WpaFiW1Wpa_supplicant1Interface * iface = wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus_finish(res, &err);

    if (mWpaSupplicant.iface)
    {
        g_object_unref(mWpaSupplicant.iface);
        mWpaSupplicant.iface = nullptr;
    }

    if (iface != nullptr && err == nullptr)
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

    if (err != nullptr)
        g_error_free(err);
}

void ConnectivityManagerImpl::_OnWpaInterfaceReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = nullptr;

    gboolean result =
        wpa_fi_w1_wpa_supplicant1_call_get_interface_finish(mWpaSupplicant.proxy, &mWpaSupplicant.interfacePath, res, &err);
    if (result)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_GOT_INTERFACE_PATH;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface: %s", mWpaSupplicant.interfacePath);

        wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                              mWpaSupplicant.interfacePath, nullptr, _OnWpaInterfaceProxyReady,
                                                              nullptr);
    }
    else
    {
        GError * error  = nullptr;
        GVariant * args = nullptr;
        GVariantBuilder builder;

        ChipLogProgress(DeviceLayer, "wpa_supplicant: can't find interface %s: %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME,
                        err ? err->message : "unknown error");

        ChipLogProgress(DeviceLayer, "wpa_supplicant: try to create interface %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME);

        g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
        g_variant_builder_add(&builder, "{sv}", "Ifname", g_variant_new_string(CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME));
        args = g_variant_builder_end(&builder);

        result = wpa_fi_w1_wpa_supplicant1_call_create_interface_sync(mWpaSupplicant.proxy, args, &mWpaSupplicant.interfacePath,
                                                                      nullptr, &error);

        if (result)
        {
            mWpaSupplicant.state = GDBusWpaSupplicant::WPA_GOT_INTERFACE_PATH;
            ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface: %s", mWpaSupplicant.interfacePath);

            wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                                  kWpaSupplicantServiceName, mWpaSupplicant.interfacePath, nullptr,
                                                                  _OnWpaInterfaceProxyReady, nullptr);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to create interface %s: %s",
                            CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, error ? error->message : "unknown error");

            mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NO_INTERFACE_PATH;

            if (mWpaSupplicant.interfacePath)
            {
                g_free(mWpaSupplicant.interfacePath);
                mWpaSupplicant.interfacePath = nullptr;
            }
        }

        if (error != nullptr)
            g_error_free(error);
    }

    if (err != nullptr)
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
                                                              mWpaSupplicant.interfacePath, nullptr, _OnWpaInterfaceProxyReady,
                                                              nullptr);
    }
}

void ConnectivityManagerImpl::_OnWpaInterfaceRemoved(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                                     gpointer user_data)
{
    if (mWpaSupplicant.interfacePath == nullptr)
        return;

    if (g_strcmp0(mWpaSupplicant.interfacePath, path) == 0)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface removed: %s", path);

        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NO_INTERFACE_PATH;

        if (mWpaSupplicant.interfacePath)
        {
            g_free(mWpaSupplicant.interfacePath);
            mWpaSupplicant.interfacePath = nullptr;
        }

        if (mWpaSupplicant.iface)
        {
            g_object_unref(mWpaSupplicant.iface);
            mWpaSupplicant.iface = nullptr;
        }

        mWpaSupplicant.scanState = GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
    }
}

void ConnectivityManagerImpl::_OnWpaProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = nullptr;

    mWpaSupplicant.proxy = wpa_fi_w1_wpa_supplicant1_proxy_new_for_bus_finish(res, &err);
    if (mWpaSupplicant.proxy != nullptr && err == nullptr)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_CONNECTED;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to wpa_supplicant proxy");

        g_signal_connect(mWpaSupplicant.proxy, "interface-added", G_CALLBACK(_OnWpaInterfaceAdded), NULL);

        g_signal_connect(mWpaSupplicant.proxy, "interface-removed", G_CALLBACK(_OnWpaInterfaceRemoved), NULL);

        wpa_fi_w1_wpa_supplicant1_call_get_interface(mWpaSupplicant.proxy, CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, nullptr,
                                                     _OnWpaInterfaceReady, nullptr);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to create wpa_supplicant proxy %s",
                        err ? err->message : "unknown error");
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NOT_CONNECTED;
    }

    if (err != nullptr)
        g_error_free(err);
}

void ConnectivityManagerImpl::StartWiFiManagement()
{
    mConnectivityFlag.ClearAll();
    mWpaSupplicant.state         = GDBusWpaSupplicant::INIT;
    mWpaSupplicant.scanState     = GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
    mWpaSupplicant.proxy         = nullptr;
    mWpaSupplicant.iface         = nullptr;
    mWpaSupplicant.interfacePath = nullptr;
    mWpaSupplicant.networkPath   = nullptr;

    wpa_fi_w1_wpa_supplicant1_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                kWpaSupplicantObjectPath, nullptr, _OnWpaProxyReady, nullptr);
}

void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;
    uint64_t now;
    uint32_t apTimeout;

    // If the AP interface is not under application control...
    if (mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        // Determine the target (desired) state for AP interface...

        // The target state is 'NotActive' if the application has expressly disabled the AP interface.
        if (mWiFiAPMode == kWiFiAPMode_Disabled)
        {
            targetState = kWiFiAPState_NotActive;
        }

        // The target state is 'Active' if the application has expressly enabled the AP interface.
        else if (mWiFiAPMode == kWiFiAPMode_Enabled)
        {
            targetState = kWiFiAPState_Active;
        }

        // The target state is 'Active' if the AP mode is 'On demand, when no station is available'
        // and the station interface is not provisioned or the application has disabled the station
        // interface.
        else if (mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision &&
                 (!IsWiFiStationProvisioned() || GetWiFiStationMode() == kWiFiStationMode_Disabled))
        {
            targetState = kWiFiAPState_Active;
        }

        // The target state is 'Active' if the AP mode is one of the 'On demand' modes and there
        // has been demand for the AP within the idle timeout period.
        else if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
        {
            now = System::Layer::GetClock_MonotonicMS();

            if (mLastAPDemandTime != 0 && now < (mLastAPDemandTime + mWiFiAPIdleTimeoutMS))
            {
                targetState = kWiFiAPState_Active;

                // Compute the amount of idle time before the AP should be deactivated and
                // arm a timer to fire at that time.
                apTimeout = (uint32_t)((mLastAPDemandTime + mWiFiAPIdleTimeoutMS) - now);
                err       = SystemLayer.StartTimer(apTimeout, DriveAPState, NULL);
                SuccessOrExit(err);
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " s", apTimeout / 1000);
            }
            else
            {
                targetState = kWiFiAPState_NotActive;
            }
        }

        // Otherwise the target state is 'NotActive'.
        else
        {
            targetState = kWiFiAPState_NotActive;
        }

        // If the current AP state does not match the target state...
        if (mWiFiAPState != targetState)
        {
            if (targetState == kWiFiAPState_Active)
            {
                err = ConfigureWiFiAP();
                SuccessOrExit(err);

                ChangeWiFiAPState(kWiFiAPState_Active);
            }
            else
            {
                if (mWpaSupplicant.networkPath)
                {
                    GError * error = nullptr;

                    gboolean result = wpa_fi_w1_wpa_supplicant1_interface_call_remove_network_sync(
                        mWpaSupplicant.iface, mWpaSupplicant.networkPath, nullptr, &error);

                    if (result)
                    {
                        ChipLogProgress(DeviceLayer, "wpa_supplicant: removed network: %s", mWpaSupplicant.networkPath);
                        g_free(mWpaSupplicant.networkPath);
                        mWpaSupplicant.networkPath = nullptr;
                        ChangeWiFiAPState(kWiFiAPState_NotActive);
                    }
                    else
                    {
                        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to stop AP mode with error: %s",
                                        error ? error->message : "unknown error");
                        err = CHIP_ERROR_INTERNAL;
                    }

                    if (error != nullptr)
                        g_error_free(error);
                }
            }
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        SetWiFiAPMode(kWiFiAPMode_Disabled);
        ChipLogError(DeviceLayer, "Drive AP state failed: %s", ErrorStr(err));
    }
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    CHIP_ERROR ret  = CHIP_NO_ERROR;
    GError * err    = nullptr;
    GVariant * args = nullptr;
    GVariantBuilder builder;

    uint16_t channel       = 1;
    uint16_t discriminator = 0;
    char ssid[32];

    channel = MapFrequency(kWiFi_BAND_2_4_GHZ, CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL);

    if (ConfigurationMgr().GetSetupDiscriminator(discriminator) != CHIP_NO_ERROR)
        discriminator = 0;

    snprintf(ssid, 32, "%s%04u", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX, discriminator);

    ChipLogProgress(DeviceLayer, "wpa_supplicant: ConfigureWiFiAP, ssid: %s, channel: %d", ssid, channel);

    // Clean up current network if exists
    if (mWpaSupplicant.networkPath)
    {
        g_object_unref(mWpaSupplicant.networkPath);
        mWpaSupplicant.networkPath = nullptr;
    }

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "ssid", g_variant_new_string(ssid));
    g_variant_builder_add(&builder, "{sv}", "key_mgmt", g_variant_new_string("NONE"));
    g_variant_builder_add(&builder, "{sv}", "mode", g_variant_new_int32(2));
    g_variant_builder_add(&builder, "{sv}", "frequency", g_variant_new_int32(channel));
    args = g_variant_builder_end(&builder);

    gboolean result = wpa_fi_w1_wpa_supplicant1_interface_call_add_network_sync(mWpaSupplicant.iface, args,
                                                                                &mWpaSupplicant.networkPath, nullptr, &err);

    if (result)
    {
        GError * error = nullptr;

        ChipLogProgress(DeviceLayer, "wpa_supplicant: added network: SSID: %s: %s", ssid, mWpaSupplicant.networkPath);

        result = wpa_fi_w1_wpa_supplicant1_interface_call_select_network_sync(mWpaSupplicant.iface, mWpaSupplicant.networkPath,
                                                                              nullptr, &error);
        if (result)
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: succeeded to start softAP: SSID: %s", ssid);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to start softAP: SSID: %s: %s", ssid,
                            error ? error->message : "unknown error");

            ret = CHIP_ERROR_INTERNAL;
        }

        if (error != nullptr)
            g_error_free(error);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to add network: %s: %s", ssid, err ? err->message : "unknown error");

        if (mWpaSupplicant.networkPath)
        {
            g_object_unref(mWpaSupplicant.networkPath);
            mWpaSupplicant.networkPath = nullptr;
        }

        ret = CHIP_ERROR_INTERNAL;
    }

    if (err != nullptr)
        g_error_free(err);

    return ret;
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    if (mWiFiAPState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", WiFiAPStateToStr(mWiFiAPState), WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}

void ConnectivityManagerImpl::DriveAPState(::chip::System::Layer * aLayer, void * aAppState, ::CHIP_ERROR aError)
{
    sInstance.DriveAPState();
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    CHIP_ERROR ret  = CHIP_NO_ERROR;
    GError * err    = nullptr;
    GVariant * args = nullptr;
    GVariantBuilder builder;
    gboolean result;

    // Clean up current network if exists
    if (mWpaSupplicant.networkPath)
    {
        GError * error = nullptr;

        result = wpa_fi_w1_wpa_supplicant1_interface_call_remove_network_sync(mWpaSupplicant.iface, mWpaSupplicant.networkPath,
                                                                              nullptr, &error);

        if (result)
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: removed network: %s", mWpaSupplicant.networkPath);
            g_free(mWpaSupplicant.networkPath);
            mWpaSupplicant.networkPath = nullptr;
        }
        else
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to stop AP mode with error: %s",
                            error ? error->message : "unknown error");
            ret = CHIP_ERROR_INTERNAL;
        }

        if (error != nullptr)
            g_error_free(error);

        SuccessOrExit(ret);
    }

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "ssid", g_variant_new_string(ssid));
    g_variant_builder_add(&builder, "{sv}", "psk", g_variant_new_string(key));
    g_variant_builder_add(&builder, "{sv}", "key_mgmt", g_variant_new_string("WPA-PSK"));
    args = g_variant_builder_end(&builder);

    result = wpa_fi_w1_wpa_supplicant1_interface_call_add_network_sync(mWpaSupplicant.iface, args, &mWpaSupplicant.networkPath,
                                                                       nullptr, &err);

    if (result)
    {
        GError * error = nullptr;

        ChipLogProgress(DeviceLayer, "wpa_supplicant: added network: SSID: %s: %s", ssid, mWpaSupplicant.networkPath);

        result = wpa_fi_w1_wpa_supplicant1_interface_call_select_network_sync(mWpaSupplicant.iface, mWpaSupplicant.networkPath,
                                                                              nullptr, &error);
        if (result)
        {
            GError * gerror = nullptr;

            ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to network: SSID: %s", ssid);

            result = wpa_fi_w1_wpa_supplicant1_interface_call_save_config_sync(mWpaSupplicant.iface, nullptr, &gerror);

            if (result)
            {
                ChipLogProgress(DeviceLayer, "wpa_supplicant: save config succeeded!");
            }
            else
            {
                ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to save config: %s",
                                gerror ? gerror->message : "unknown error");
            }

            if (gerror != nullptr)
                g_error_free(gerror);

            // Iterate on the network interface to see if we already have beed assigned addresses.
            // The temporary hack for getting IP address change on linux for network provisioning in the rendezvous session.
            // This should be removed or find a better place once we depercate the rendezvous session.
            for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
            {
                char ifName[chip::Inet::InterfaceIterator::kMaxIfNameLength];
                if (it.IsUp() && CHIP_NO_ERROR == it.GetInterfaceName(ifName, sizeof(ifName)) &&
                    strncmp(ifName, CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, sizeof(ifName)) == 0)
                {
                    chip::Inet::IPAddress addr = it.GetAddress();
                    if (addr.IsIPv4())
                    {
                        ChipDeviceEvent event;
                        event.Type                            = DeviceEventType::kInternetConnectivityChange;
                        event.InternetConnectivityChange.IPv4 = kConnectivity_Established;
                        event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
                        addr.ToString(event.InternetConnectivityChange.address);

                        ChipLogDetail(DeviceLayer, "Got IP address on interface: %s IP: %s", ifName,
                                      event.InternetConnectivityChange.address);

                        PlatformMgr().PostEvent(&event);
                    }
                }
            }

            // Run dhclient for IP on WiFi.
            // TODO: The wifi can be managed by networkmanager on linux so we don't have to care about this.
            char cmdBuffer[128];
            sprintf(cmdBuffer, "dhclient -nw %s", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME);
            int dhclientSystemRet = system(cmdBuffer);
            if (dhclientSystemRet != 0)
            {
                ChipLogError(DeviceLayer, "Failed to run dhclient, system() returns %d", dhclientSystemRet);
            }
            else
            {
                ChipLogProgress(DeviceLayer, "dhclient is running on the %s interface.", CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME);
            }

            // Return success as long as the device is connected to the network
            ret = CHIP_NO_ERROR;
        }
        else
        {
            ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to connect to network: SSID: %s: %s", ssid,
                            error ? error->message : "unknown error");

            ret = CHIP_ERROR_INTERNAL;
        }

        if (error != nullptr)
            g_error_free(error);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to add network: %s: %s", ssid, err ? err->message : "unknown error");

        if (mWpaSupplicant.networkPath)
        {
            g_object_unref(mWpaSupplicant.networkPath);
            mWpaSupplicant.networkPath = nullptr;
        }

        ret = CHIP_ERROR_INTERNAL;
    }

exit:
    if (err != nullptr)
        g_error_free(err);

    return ret;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

} // namespace DeviceLayer
} // namespace chip
