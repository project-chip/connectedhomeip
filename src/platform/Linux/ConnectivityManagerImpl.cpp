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
#include <platform/DiagnosticDataProvider.h>
#include <platform/Linux/ConnectivityUtils.h>
#include <platform/internal/BLEManager.h>

#include <cstdlib>
#include <new>

#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/if_link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.cpp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.cpp>
#endif

#ifndef CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD
#define CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD "dhclient -nw %s"
#endif

using namespace ::chip;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace {

enum class EthernetStatsCountType
{
    kEthPacketRxCount,
    kEthPacketTxCount,
    kEthTxErrCount,
    kEthCollisionCount,
    kEthOverrunCount
};

enum class WiFiStatsCountType
{
    kWiFiUnicastPacketRxCount,
    kWiFiUnicastPacketTxCount,
    kWiFiMulticastPacketRxCount,
    kWiFiMulticastPacketTxCount,
    kWiFiOverrunCount
};

CHIP_ERROR GetEthernetStatsCount(EthernetStatsCountType type, uint64_t & count)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET)
            {
                ChipLogProgress(DeviceLayer, "Found the primary Ethernet interface:%s", ifa->ifa_name);
                break;
            }
        }

        if (ifa != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
            {
                struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;
                switch (type)
                {
                case EthernetStatsCountType::kEthPacketRxCount:
                    count = stats->rx_packets;
                    err   = CHIP_NO_ERROR;
                    break;
                case EthernetStatsCountType::kEthPacketTxCount:
                    count = stats->tx_packets;
                    err   = CHIP_NO_ERROR;
                    break;
                case EthernetStatsCountType::kEthTxErrCount:
                    count = stats->tx_errors;
                    err   = CHIP_NO_ERROR;
                    break;
                case EthernetStatsCountType::kEthCollisionCount:
                    count = stats->collisions;
                    err   = CHIP_NO_ERROR;
                    break;
                case EthernetStatsCountType::kEthOverrunCount:
                    count = stats->rx_over_errors;
                    err   = CHIP_NO_ERROR;
                    break;
                default:
                    ChipLogError(DeviceLayer, "Unknown Ethernet statistic metric type");
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR GetWiFiStatsCount(WiFiStatsCountType type, uint64_t & count)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI)
            {
                ChipLogProgress(DeviceLayer, "Found the primary WiFi interface:%s", ifa->ifa_name);
                break;
            }
        }

        if (ifa != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
            {
                // The usecase of this function is embedded devices,on which we can interact with the WiFi
                // driver to get the accurate number of muticast and unicast packets accurately.
                // On Linux simulation, we can only get the total packets received, the total bytes transmitted,
                // the multicast packets received and receiver ring buff overflow.

                struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;
                switch (type)
                {
                case WiFiStatsCountType::kWiFiUnicastPacketRxCount:
                    count = stats->rx_packets;
                    err   = CHIP_NO_ERROR;
                    break;
                case WiFiStatsCountType::kWiFiUnicastPacketTxCount:
                    count = stats->tx_packets;
                    err   = CHIP_NO_ERROR;
                    break;
                case WiFiStatsCountType::kWiFiMulticastPacketRxCount:
                    count = stats->multicast;
                    err   = CHIP_NO_ERROR;
                    break;
                case WiFiStatsCountType::kWiFiMulticastPacketTxCount:
                    count = 0;
                    err   = CHIP_NO_ERROR;
                    break;
                case WiFiStatsCountType::kWiFiOverrunCount:
                    count = stats->rx_over_errors;
                    err   = CHIP_NO_ERROR;
                    break;
                default:
                    ChipLogError(DeviceLayer, "Unknown WiFi statistic metric type");
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}
#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
char ConnectivityManagerImpl::sWiFiIfName[];
#endif

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
#endif

    if (ConnectivityUtils::GetEthInterfaceName(mEthIfName, IFNAMSIZ) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got Ethernet interface: %s", mEthIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get Ethernet interface");
        mEthIfName[0] = '\0';
    }

    if (ResetEthernetStatsCount() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset Ethernet statistic counts");
    }

    // Initialize the generic base classes that require it.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (ConnectivityUtils::GetWiFiInterfaceName(sWiFiIfName, IFNAMSIZ) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got WiFi interface: %s", sWiFiIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get WiFi interface");
        sWiFiIfName[0] = '\0';
    }

    if (ResetWiFiStatsCount() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset WiFi statistic counts");
    }
#endif

    return CHIP_NO_ERROR;
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
std::mutex ConnectivityManagerImpl::mWpaSupplicantMutex;

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

System::Clock::Timeout ConnectivityManagerImpl::_GetWiFiStationReconnectInterval()
{
    return mWiFiStationReconnectInterval;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    mWiFiStationReconnectInterval = val;

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

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

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

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

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
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

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
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    bool ret = mWpaSupplicant.state == GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED &&
        mWpaSupplicant.scanState == GDBusWpaSupplicant::WIFI_SCANNING_IDLE;

    return ret;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
        mWiFiAPMode = val;

        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }

exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiBssId(ByteSpan & value)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    // On Linux simulation, we don't have the DBus API to get the BSSID of connected AP. Use mac address
    // of local WiFi network card instead.
    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        // Walk through linked list, maintaining head pointer so we can free list later.
        for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI)
            {
                if (ConnectivityUtils::GetInterfaceHardwareAddrs(ifa->ifa_name, mWiFiMacAddress, kMaxHardwareAddrSize) !=
                    CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Failed to get WiFi network hardware address");
                }
                else
                {
                    // Set 48-bit IEEE MAC Address
                    value = ByteSpan(mWiFiMacAddress, 6);
                    err   = CHIP_NO_ERROR;
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiSecurityType(uint8_t & securityType)
{
    const gchar * mode = nullptr;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    if (mWpaSupplicant.state != GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED)
    {
        ChipLogError(DeviceLayer, "wpa_supplicant: _GetWiFiSecurityType: interface proxy not connected");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mode = wpa_fi_w1_wpa_supplicant1_interface_get_current_auth_mode(mWpaSupplicant.iface);
    ChipLogProgress(DeviceLayer, "wpa_supplicant: current Wi-Fi security type: %s", mode);

    if (strncmp(mode, "WPA-PSK", 7) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA;
    }
    else if (strncmp(mode, "WPA2-PSK", 8) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA2;
    }
    else if (strncmp(mode, "WPA2-EAP", 8) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA2;
    }
    else if (strncmp(mode, "WPA3-PSK", 8) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA3;
    }
    else if (strncmp(mode, "WEP", 3) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_WEP;
    }
    else if (strncmp(mode, "NONE", 4) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_NONE;
    }
    else if (strncmp(mode, "WPA-NONE", 8) == 0)
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_NONE;
    }
    else
    {
        securityType = EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiVersion(uint8_t & wiFiVersion)
{
    // We don't have driect API to get the WiFi version yet, retrun 802.11n on Linux simulation.
    wiFiVersion = EMBER_ZCL_WI_FI_VERSION_TYPE_802__11N;

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP()
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: Demand start WiFi AP");
        mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
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
        mLastAPDemandTime = System::Clock::kZero;
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
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
            mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        }
    }
}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeout(System::Clock::Timeout val)
{
    mWiFiAPIdleTimeout = val;
    DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
}

void ConnectivityManagerImpl::_OnWpaInterfaceProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = nullptr;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

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
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to create wpa_supplicant interface proxy %s: %s",
                        mWpaSupplicant.interfacePath, err ? err->message : "unknown error");

        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_NOT_CONNECTED;
    }

    if (err != nullptr)
        g_error_free(err);
}

void ConnectivityManagerImpl::_OnWpaBssProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = nullptr;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    WpaFiW1Wpa_supplicant1BSS * bss = wpa_fi_w1_wpa_supplicant1_bss_proxy_new_for_bus_finish(res, &err);

    if (mWpaSupplicant.bss)
    {
        g_object_unref(mWpaSupplicant.bss);
        mWpaSupplicant.bss = nullptr;
    }

    if (bss != nullptr && err == nullptr)
    {
        mWpaSupplicant.bss = bss;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to wpa_supplicant bss proxy");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to create wpa_supplicant bss proxy %s: %s",
                        mWpaSupplicant.interfacePath, err ? err->message : "unknown error");
    }

    if (err != nullptr)
        g_error_free(err);
}

void ConnectivityManagerImpl::_OnWpaInterfaceReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = nullptr;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    gboolean result =
        wpa_fi_w1_wpa_supplicant1_call_get_interface_finish(mWpaSupplicant.proxy, &mWpaSupplicant.interfacePath, res, &err);
    if (result)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_GOT_INTERFACE_PATH;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface: %s", mWpaSupplicant.interfacePath);

        wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                              mWpaSupplicant.interfacePath, nullptr, _OnWpaInterfaceProxyReady,
                                                              nullptr);

        wpa_fi_w1_wpa_supplicant1_bss_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                        mWpaSupplicant.interfacePath, nullptr, _OnWpaBssProxyReady, nullptr);
    }
    else
    {
        GError * error  = nullptr;
        GVariant * args = nullptr;
        GVariantBuilder builder;

        ChipLogProgress(DeviceLayer, "wpa_supplicant: can't find interface %s: %s", sWiFiIfName,
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

            strncpy(sWiFiIfName, CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME, IFNAMSIZ);
            sWiFiIfName[IFNAMSIZ - 1] = '\0';

            wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                                  kWpaSupplicantServiceName, mWpaSupplicant.interfacePath, nullptr,
                                                                  _OnWpaInterfaceProxyReady, nullptr);

            wpa_fi_w1_wpa_supplicant1_bss_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                            mWpaSupplicant.interfacePath, nullptr, _OnWpaBssProxyReady, nullptr);
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
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    if (mWpaSupplicant.interfacePath)
    {
        return;
    }

    mWpaSupplicant.interfacePath = const_cast<gchar *>(path);
    if (mWpaSupplicant.interfacePath)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_GOT_INTERFACE_PATH;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: WiFi interface added: %s", mWpaSupplicant.interfacePath);

        wpa_fi_w1_wpa_supplicant1_interface_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                              mWpaSupplicant.interfacePath, nullptr, _OnWpaInterfaceProxyReady,
                                                              nullptr);

        wpa_fi_w1_wpa_supplicant1_bss_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                        mWpaSupplicant.interfacePath, nullptr, _OnWpaBssProxyReady, nullptr);
    }
}

void ConnectivityManagerImpl::_OnWpaInterfaceRemoved(WpaFiW1Wpa_supplicant1 * proxy, const gchar * path, GVariant * properties,
                                                     gpointer user_data)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    if (mWpaSupplicant.interfacePath == nullptr)
    {
        return;
    }

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

        if (mWpaSupplicant.bss)
        {
            g_object_unref(mWpaSupplicant.bss);
            mWpaSupplicant.bss = nullptr;
        }

        mWpaSupplicant.scanState = GDBusWpaSupplicant::WIFI_SCANNING_IDLE;
    }
}

void ConnectivityManagerImpl::_OnWpaProxyReady(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    GError * err = nullptr;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    mWpaSupplicant.proxy = wpa_fi_w1_wpa_supplicant1_proxy_new_for_bus_finish(res, &err);
    if (mWpaSupplicant.proxy != nullptr && err == nullptr)
    {
        mWpaSupplicant.state = GDBusWpaSupplicant::WPA_CONNECTED;
        ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to wpa_supplicant proxy");

        g_signal_connect(mWpaSupplicant.proxy, "interface-added", G_CALLBACK(_OnWpaInterfaceAdded), NULL);

        g_signal_connect(mWpaSupplicant.proxy, "interface-removed", G_CALLBACK(_OnWpaInterfaceRemoved), NULL);

        wpa_fi_w1_wpa_supplicant1_call_get_interface(mWpaSupplicant.proxy, sWiFiIfName, nullptr, _OnWpaInterfaceReady, nullptr);
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
    mWpaSupplicant.bss           = nullptr;
    mWpaSupplicant.interfacePath = nullptr;
    mWpaSupplicant.networkPath   = nullptr;

    ChipLogProgress(DeviceLayer, "wpa_supplicant: Start WiFi management");

    wpa_fi_w1_wpa_supplicant1_proxy_new_for_bus(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE, kWpaSupplicantServiceName,
                                                kWpaSupplicantObjectPath, nullptr, _OnWpaProxyReady, nullptr);
}

bool ConnectivityManagerImpl::IsWiFiManagementStarted()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    bool ret = mWpaSupplicant.state == GDBusWpaSupplicant::WPA_INTERFACE_CONNECTED;

    return ret;
}

void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;

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
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

            if (mLastAPDemandTime != System::Clock::kZero && now < (mLastAPDemandTime + mWiFiAPIdleTimeout))
            {
                targetState = kWiFiAPState_Active;

                // Compute the amount of idle time before the AP should be deactivated and
                // arm a timer to fire at that time.
                System::Clock::Timeout apTimeout = (mLastAPDemandTime + mWiFiAPIdleTimeout) - now;
                err                              = DeviceLayer::SystemLayer().StartTimer(apTimeout, DriveAPState, NULL);
                SuccessOrExit(err);
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " s",
                                std::chrono::duration_cast<System::Clock::Seconds32>(apTimeout).count());
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

    channel = ConnectivityUtils::MapChannelToFrequency(kWiFi_BAND_2_4_GHZ, CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL);

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

void ConnectivityManagerImpl::DriveAPState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveAPState();
}

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
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
                char ifName[chip::Inet::InterfaceId::kMaxIfNameLength];
                if (it.IsUp() && CHIP_NO_ERROR == it.GetInterfaceName(ifName, sizeof(ifName)) &&
                    strncmp(ifName, sWiFiIfName, sizeof(ifName)) == 0)
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

                        PlatformMgr().PostEventOrDie(&event);
                    }
                }
            }

            // Run dhclient for IP on WiFi.
            // TODO: The wifi can be managed by networkmanager on linux so we don't have to care about this.
            char cmdBuffer[128];
            sprintf(cmdBuffer, CHIP_DEVICE_CONFIG_LINUX_DHCPC_CMD, sWiFiIfName);
            int dhclientSystemRet = system(cmdBuffer);
            if (dhclientSystemRet != 0)
            {
                ChipLogError(DeviceLayer, "Failed to run dhclient, system() returns %d", dhclientSystemRet);
            }
            else
            {
                ChipLogProgress(DeviceLayer, "dhclient is running on the %s interface.", sWiFiIfName);
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
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

void ConnectivityManagerImpl::_ReleaseNetworkInterfaces(NetworkInterface * netifp)
{
    while (netifp)
    {
        NetworkInterface * del = netifp;
        netifp                 = netifp->Next;
        delete del;
    }
}

CHIP_ERROR ConnectivityManagerImpl::_GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        NetworkInterface * head = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET)
            {
                NetworkInterface * ifp = new NetworkInterface();

                strncpy(ifp->Name, ifa->ifa_name, Inet::InterfaceId::kMaxIfNameLength);
                ifp->Name[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';

                ifp->name                            = CharSpan(ifp->Name, strlen(ifp->Name));
                ifp->fabricConnected                 = ifa->ifa_flags & IFF_RUNNING;
                ifp->type                            = ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name);
                ifp->offPremiseServicesReachableIPv4 = false;
                ifp->offPremiseServicesReachableIPv6 = false;

                if (ConnectivityUtils::GetInterfaceHardwareAddrs(ifa->ifa_name, ifp->MacAddress, kMaxHardwareAddrSize) !=
                    CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Failed to get network hardware address");
                }
                else
                {
                    // Set 48-bit IEEE MAC Address
                    ifp->hardwareAddress = ByteSpan(ifp->MacAddress, 6);
                }

                ifp->Next = head;
                head      = ifp;
            }
        }

        *netifpp = head;
        err      = CHIP_NO_ERROR;

        freeifaddrs(ifaddr);
    }

    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthPHYRate(uint8_t & pHYRate)
{
    if (mEthIfName[0] == '\0')
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetEthPHYRate(mEthIfName, pHYRate);
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthFullDuplex(bool & fullDuplex)
{
    if (mEthIfName[0] == '\0')
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetEthFullDuplex(mEthIfName, fullDuplex);
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthTimeSinceReset(uint64_t & timeSinceReset)
{
    return GetDiagnosticDataProvider().GetUpTime(timeSinceReset);
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthPacketRxCount(uint64_t & packetRxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthPacketRxCount, count));
    VerifyOrReturnError(count >= mEthPacketRxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetRxCount = count - mEthPacketRxCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthPacketTxCount(uint64_t & packetTxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthPacketTxCount, count));
    VerifyOrReturnError(count >= mEthPacketTxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetTxCount = count - mEthPacketTxCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthTxErrCount(uint64_t & txErrCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthTxErrCount, count));
    VerifyOrReturnError(count >= mEthTxErrCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    txErrCount = count - mEthTxErrCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthCollisionCount(uint64_t & collisionCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthCollisionCount, count));
    VerifyOrReturnError(count >= mEthCollisionCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    collisionCount = count - mEthCollisionCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetEthOverrunCount(uint64_t & overrunCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthOverrunCount, count));
    VerifyOrReturnError(count >= mEthOverrunCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    overrunCount = count - mEthOverrunCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_ResetEthNetworkDiagnosticsCounts()
{
    return ResetEthernetStatsCount();
}

CHIP_ERROR ConnectivityManagerImpl::ResetEthernetStatsCount()
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET)
            {
                ChipLogProgress(DeviceLayer, "Found the primary Ethernet interface:%s", ifa->ifa_name);
                break;
            }
        }

        if (ifa != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
            {
                struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;

                mEthPacketRxCount  = stats->rx_packets;
                mEthPacketTxCount  = stats->tx_packets;
                mEthTxErrCount     = stats->tx_errors;
                mEthCollisionCount = stats->collisions;
                mEthOverrunCount   = stats->rx_over_errors;
                err                = CHIP_NO_ERROR;
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR ConnectivityManagerImpl::_GetWiFiChannelNumber(uint16_t & channelNumber)
{
    if (sWiFiIfName[0] == '\0')
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetWiFiChannelNumber(sWiFiIfName, channelNumber);
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiRssi(int8_t & rssi)
{
    if (sWiFiIfName[0] == '\0')
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetWiFiRssi(sWiFiIfName, rssi);
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    uint32_t count;

    if (sWiFiIfName[0] == '\0')
    {
        return CHIP_ERROR_READ_FAILED;
    }

    ReturnErrorOnFailure(ConnectivityUtils::GetWiFiBeaconLostCount(sWiFiIfName, count));
    VerifyOrReturnError(count >= mBeaconLostCount, CHIP_ERROR_INVALID_INTEGER_VALUE);
    beaconLostCount = count - mBeaconLostCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    if (sWiFiIfName[0] == '\0')
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetWiFiCurrentMaxRate(sWiFiIfName, currentMaxRate);
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiMulticastPacketRxCount, count));
    VerifyOrReturnError(count >= mPacketMulticastRxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketMulticastRxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetMulticastRxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiMulticastPacketTxCount, count));
    VerifyOrReturnError(count >= mPacketMulticastTxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketMulticastTxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetMulticastTxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiUnicastPacketRxCount, count));
    VerifyOrReturnError(count >= mPacketUnicastRxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketUnicastRxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetUnicastRxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiUnicastPacketTxCount, count));
    VerifyOrReturnError(count >= mPacketUnicastTxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketUnicastTxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetUnicastTxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetWiFiOverrunCount(uint64_t & overrunCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiOverrunCount, count));
    VerifyOrReturnError(count >= mOverrunCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    overrunCount = count - mOverrunCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_ResetWiFiNetworkDiagnosticsCounts()
{
    return ResetWiFiStatsCount();
}

CHIP_ERROR ConnectivityManagerImpl::ResetWiFiStatsCount()
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    ReturnErrorOnFailure(_GetWiFiBeaconLostCount(mBeaconLostCount));

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI)
            {
                ChipLogProgress(DeviceLayer, "Found the primary WiFi interface:%s", ifa->ifa_name);
                break;
            }
        }

        if (ifa != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
            {
                struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;

                mPacketMulticastRxCount = stats->multicast;
                mPacketMulticastTxCount = 0;
                mPacketUnicastRxCount   = stats->rx_packets;
                mPacketUnicastTxCount   = stats->tx_packets;
                mOverrunCount           = stats->rx_over_errors;

                err = CHIP_NO_ERROR;
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
