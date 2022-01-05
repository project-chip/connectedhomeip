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
#include <platform/Linux/DiagnosticDataProviderImpl.h>
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/Linux/WirelessDefs.h>
#include <platform/internal/BLEManager.h>

#include <cstdlib>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include <ifaddrs.h>
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
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;

using namespace ::chip::DeviceLayer::NetworkCommissioning;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
char ConnectivityManagerImpl::sWiFiIfName[];
#endif

WiFiDriver::ScanCallback * ConnectivityManagerImpl::mpScanCallback;
NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * ConnectivityManagerImpl::mpConnectCallback;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
#endif
    mpConnectCallback = nullptr;
    mpScanCallback    = nullptr;

    if (ConnectivityUtils::GetEthInterfaceName(mEthIfName, IFNAMSIZ) == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Got Ethernet interface: %s", mEthIfName);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get Ethernet interface");
        mEthIfName[0] = '\0';
    }

    if (GetDiagnosticDataProvider().ResetEthNetworkDiagnosticsCounts() != CHIP_NO_ERROR)
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

    if (GetDiagnosticDataProvider().ResetWiFiNetworkDiagnosticsCounts() != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to reset WiFi statistic counts");
    }
#endif

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
bool ConnectivityManagerImpl::mAssociattionStarted = false;
BitFlags<Internal::GenericConnectivityManagerImpl_WiFi<ConnectivityManage rImpl>::ConnectivityFlags>
    ConnectivityManagerImpl::mConnectivityFlag;
struct GDBusWpaSupplicant ConnectivityManagerImpl::mWpaSupplicant;
std::mutex ConnectivityManagerImpl::mWpaSupplicantMutex;

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    mWiFiStationReconnectInterval = val;

    return CHIP_NO_ERROR;
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

CHIP_ERROR
ConnectivityManagerImpl::ConnectWiFiNetworkAsync(ByteSpan ssid, ByteSpan credentials,
                                                 NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback)
{
    CHIP_ERROR ret  = CHIP_NO_ERROR;
    GError * err    = nullptr;
    GVariant * args = nullptr;
    GVariantBuilder builder;
    gboolean result;
    char ssidStr[kMaxWiFiSSIDLength] = { 0 };
    char keyStr[kMaxWiFiKeyLength]   = { 0 };
    // There is another ongoing connect request, reject the new one.
    VerifyOrReturnError(mpConnectCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);

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
    memcpy(ssidStr, ssid.data(), ssid.size());
    memcpy(keyStr, credentials.data(), credentials.size());
    g_variant_builder_add(&builder, "{sv}", "ssid", g_variant_new_string(ssidStr));
    g_variant_builder_add(&builder, "{sv}", "psk", g_variant_new_string(keyStr));
    g_variant_builder_add(&builder, "{sv}", "key_mgmt", g_variant_new_string("WPA-PSK"));
    args = g_variant_builder_end(&builder);

    result = wpa_fi_w1_wpa_supplicant1_interface_call_add_network_sync(mWpaSupplicant.iface, args, &mWpaSupplicant.networkPath,
                                                                       nullptr, &err);

    if (result)
    {
        // Note: wpa_supplicant will return immediately if the network is already connected, but it will still try reconnect in the
        // background. The client still need to wait for a few seconds for this reconnect operation. So we always disconnect from
        // the network we are connected and ignore any errors.
        wpa_fi_w1_wpa_supplicant1_interface_call_disconnect_sync(mWpaSupplicant.iface, nullptr, nullptr);
        ChipLogProgress(DeviceLayer, "wpa_supplicant: added network: %s", mWpaSupplicant.networkPath);

        wpa_fi_w1_wpa_supplicant1_interface_call_select_network(mWpaSupplicant.iface, mWpaSupplicant.networkPath, nullptr,
                                                                _ConnectWiFiNetworkAsyncCallback, this);
        mpConnectCallback = apCallback;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to add network: %s", err ? err->message : "unknown error");

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

            PostNetworkConnect();

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

CHIP_ERROR ConnectivityManagerImpl::CommitConfig()
{
    gboolean result;
    std::unique_ptr<GError, GErrorDeleter> err;

    ChipLogProgress(DeviceLayer, "wpa_supplicant: connected to network");

    result = wpa_fi_w1_wpa_supplicant1_interface_call_save_config_sync(mWpaSupplicant.iface, nullptr,
                                                                       &MakeUniquePointerReceiver(err).Get());

    if (!result)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to save config: %s", err ? err->message : "unknown error");
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, "wpa_supplicant: save config succeeded!");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiBssId(ByteSpan & value)
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
        uint8_t macAddress[kMaxHardwareAddrSize];

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI)
            {
                if (ConnectivityUtils::GetInterfaceHardwareAddrs(ifa->ifa_name, macAddress, kMaxHardwareAddrSize) != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Failed to get WiFi network hardware address");
                }
                else
                {
                    // Set 48-bit IEEE MAC Address
                    value = ByteSpan(macAddress, 6);
                    err   = CHIP_NO_ERROR;
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiSecurityType(uint8_t & securityType)
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

CHIP_ERROR ConnectivityManagerImpl::GetWiFiVersion(uint8_t & wiFiVersion)
{
    // We don't have driect API to get the WiFi version yet, retrun 802.11n on Linux simulation.
    wiFiVersion = EMBER_ZCL_WI_FI_VERSION_TYPE_802__11N;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetConnectedNetwork(NetworkCommissioning::Network & network)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    std::unique_ptr<GError, GErrorDeleter> err;

    const gchar * networkPath = wpa_fi_w1_wpa_supplicant1_interface_get_current_network(mWpaSupplicant.iface);

    std::unique_ptr<WpaFiW1Wpa_supplicant1Network, GObjectDeleter> networkInfo(
        wpa_fi_w1_wpa_supplicant1_network_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                                 kWpaSupplicantServiceName, networkPath, nullptr,
                                                                 &MakeUniquePointerReceiver(err).Get()));
    if (networkInfo == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    network.connected     = wpa_fi_w1_wpa_supplicant1_network_get_enabled(networkInfo.get());
    GVariant * properties = wpa_fi_w1_wpa_supplicant1_network_get_properties(networkInfo.get());
    GVariant * ssid       = g_variant_lookup_value(properties, "ssid", nullptr);
    gsize length;
    const gchar * ssidStr = g_variant_get_string(ssid, &length);
    if (length > sizeof(network.networkID))
    {
        return CHIP_ERROR_INTERNAL;
    }
    // TODO: wpa_supplicant will return ssid with quotes! We should have a better way to get the actual ssid in bytes.
    ChipLogDetail(DeviceLayer, "Current connected network: %s", ssidStr);
    memcpy(network.networkID, ssidStr + 1, length - 2);
    network.networkIDLen = length - 2;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::StopAutoScan()
{
    std::unique_ptr<GError, GErrorDeleter> err;

    gboolean result;
    result = wpa_fi_w1_wpa_supplicant1_interface_call_auto_scan_sync(
        mWpaSupplicant.iface, "" /* empty string means disabling auto scan */, nullptr, &MakeUniquePointerReceiver(err).Get());
    if (!result)
    {
        ChipLogError(DeviceLayer, "wpa_supplicant: Failed to stop auto network scan: %s", err ? err->message : "unknown");
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::StartWiFiScan(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    VerifyOrReturnError(mWpaSupplicant.iface != nullptr, CHIP_ERROR_INCORRECT_STATE);
    // There is another ongoing scan request, reject the new one.
    VerifyOrReturnError(mpScanCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR ret  = CHIP_NO_ERROR;
    GError * err    = nullptr;
    GVariant * args = nullptr;
    GVariantBuilder builder;
    gboolean result;

    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "Type", g_variant_new_string("active"));
    args = g_variant_builder_end(&builder);

    result = wpa_fi_w1_wpa_supplicant1_interface_call_scan_sync(mWpaSupplicant.iface, args, nullptr, &err);

    if (result)
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: initialized network scan.");
        mpScanCallback = callback;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "wpa_supplicant: failed to start network scan: %s", err ? err->message : "unknown error");
        ret = CHIP_ERROR_INTERNAL;
    }

    if (err != nullptr)
    {
        g_error_free(err);
    }
    return ret;
}

namespace {
// wpa_supplicant's scan results don't contains the channel information, so we need this lookup table for resolving the band and
// channel information.
std::pair<WiFiBand, uint16_t> GetBandAndChannelFromFrequency(uint32_t freq)
{
    std::pair<WiFiBand, uint16_t> ret = std::make_pair(WiFiBand::k2g4, 0);
    if (freq <= 2472)
    {
        ret.second = static_cast<uint16_t>((freq - 2412) / 5 + 1);
    }
    else if (freq == 2484)
    {
        ret.second = 14;
    }
    else if (freq >= 3600 && freq <= 3700)
    {
        // Note: There are not many devices supports this band, and this band contains rational frequency in MHz, need to figure out
        // the behavior of wpa_supplicant in this case.
        ret.first = WiFiBand::k3g65;
    }
    else if (freq >= 5035 && freq <= 5945)
    {
        ret.first  = WiFiBand::k5g;
        ret.second = static_cast<uint16_t>((freq - 5000) / 5);
    }
    else if (freq == 5960 || freq == 5980)
    {
        ret.first  = WiFiBand::k5g;
        ret.second = static_cast<uint16_t>((freq - 5000) / 5);
    }
    else if (freq >= 5955)
    {
        ret.first  = WiFiBand::k6g;
        ret.second = static_cast<uint16_t>((freq - 5950) / 5);
    }
    else if (freq >= 58000)
    {
        ret.first = WiFiBand::k60g;
        // Note: Some channel has the same center frequency but different bandwidth. Should figure out wpa_supplicant's behavior in
        // this case. Also, wpa_supplicant's frequency property is uint16 infact.
        switch (freq)
        {
        case 58'320:
            ret.second = 1;
            break;
        case 60'480:
            ret.second = 2;
            break;
        case 62'640:
            ret.second = 3;
            break;
        case 64'800:
            ret.second = 4;
            break;
        case 66'960:
            ret.second = 5;
            break;
        case 69'120:
            ret.second = 6;
            break;
        case 59'400:
            ret.second = 9;
            break;
        case 61'560:
            ret.second = 10;
            break;
        case 63'720:
            ret.second = 11;
            break;
        case 65'880:
            ret.second = 12;
            break;
        case 68'040:
            ret.second = 13;
            break;
        }
    }
    return ret;
}
} // namespace

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

} // namespace DeviceLayer
} // namespace chip
