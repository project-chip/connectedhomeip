/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
/* this file behaves like a config.h, comes first */
#include <iomanip>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <sstream>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>
#include <platform/nxp/mw320/ConnectivityManagerImpl.h>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/nxp/mw320/NetworkCommissioningDriver.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

extern "C" {
#include "wlan.h"
void test_wlan_scan(int argc, char ** argv);
void test_wlan_add(int argc, char ** argv);
static struct wlan_network sta_network;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif
// NetworkCommission++
#include <platform/nxp/mw320/NetworkCommissioningDriver.h>
// NetworkCommission--

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::GeneralDiagnostics;
using namespace ::chip::app::Clusters::WiFiNetworkDiagnostics;
using namespace ::chip::app::Clusters::NetworkCommissioning;
using namespace ::chip::DeviceLayer::NetworkCommissioning;

// NetworkCommission++
namespace {
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

#if (USE_ETHERNET_COMMISSION == 1)
DeviceLayer::NetworkCommissioning::Mw320EthernetDriver sEthernetDriver;
app::Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain,
                                                                                &sEthernetDriver);
#else
Mw320WiFiDriver sWiFiDriver;
Instance sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain, &sWiFiDriver);
#endif // USE_ETHERNET_COMMISSION
} // namespace
// NetworkCommission--

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

uint8_t ConnectivityManagerImpl::sInterestedSSID[Internal::kMaxWiFiSSIDLength];
uint8_t ConnectivityManagerImpl::sInterestedSSIDLen;
// Configured SSID
uint8_t ConnectivityManagerImpl::sCfgSSID[Internal::kMaxWiFiSSIDLength];
uint8_t ConnectivityManagerImpl::sCfgSSIDLen;

NetworkCommissioning::WiFiDriver::ScanCallback * ConnectivityManagerImpl::mpScanCallback;
NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * ConnectivityManagerImpl::mpConnectCallback;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mpConnectCallback = nullptr;
    mpScanCallback    = nullptr;

    mWiFiStationMode                = kWiFiStationMode_Disabled;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;

    sWiFiNetworkCommissioningInstance.Init();
    // Initialize the generic base classes that require it.

    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        mWiFiStationMode =
            kWiFiStationMode_Enabled; //(mWpaSupplicant.iface != nullptr) ? kWiFiStationMode_Enabled : kWiFiStationMode_Disabled;
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
    test_wlan_scan(0, NULL);
exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;
exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled()
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationConnected()
{
    // ToDo: Change the status to response the WiFi status honestly
    bool ret = true;

    return ret;
}

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled()
{
    return mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

void ConnectivityManagerImpl::StartWiFiManagement() {}

CHIP_ERROR ConnectivityManagerImpl::CommitConfig()
{
    ChipLogProgress(DeviceLayer, "[mw320] save config, connected network (ToDo)");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiBssId(ByteSpan & value)
{
    int ret = wlan_get_current_network(&sta_network);
    uint8_t macAddress[6];

    if (ret == WM_SUCCESS)
    {
        memcpy(macAddress, sta_network.bssid, 6);
    }
    else
    {
        memset(macAddress, 0, 6);
    }
    ChipLogProgress(DeviceLayer, "GetWiFiBssId: %02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2],
                    macAddress[3], macAddress[4], macAddress[5]);
    value = ByteSpan(macAddress, 6);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiSecurityType(uint8_t & securityType)
{
    int ret = wlan_get_current_network(&sta_network);
    if (ret != WM_SUCCESS)
    {
        // Set as no security by default
        securityType = EMBER_ZCL_SECURITY_TYPE_NONE;
        return CHIP_NO_ERROR;
    }
    switch (sta_network.security.type)
    {
    case WLAN_SECURITY_WEP_OPEN:
    case WLAN_SECURITY_WEP_SHARED:
        securityType = EMBER_ZCL_SECURITY_TYPE_WEP;
        break;
    case WLAN_SECURITY_WPA:
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA;
        break;
    case WLAN_SECURITY_WPA2:
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA2;
        break;
    case WLAN_SECURITY_WPA3_SAE:
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA3;
        break;
    case WLAN_SECURITY_NONE:
    default: // Default: No_security
        securityType = EMBER_ZCL_SECURITY_TYPE_NONE;
    }

    ChipLogProgress(DeviceLayer, "GetWiFiSecurityType: %u", securityType);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetWiFiVersion(uint8_t & wiFiVersion)
{
    wiFiVersion = EMBER_ZCL_WI_FI_VERSION_TYPE_802__11N;
    ChipLogProgress(DeviceLayer, "GetWiFiVersion: %u", wiFiVersion);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::GetConfiguredNetwork(NetworkCommissioning::Network & network)
{
    network.connected = true;
    memcpy(network.networkID, sCfgSSID, sCfgSSIDLen);
    network.networkIDLen = sCfgSSIDLen;

    ChipLogDetail(DeviceLayer, "[mw320] Current connected network: (ToDo)");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::StartWiFiScan(ByteSpan ssid, NetworkCommissioning::WiFiDriver::ScanCallback * callback)
{
    // There is another ongoing scan request, reject the new one.
    // ====> Do it after scan is implemented (ToDo)
    VerifyOrReturnError(mpScanCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(ssid.size() <= sizeof(sInterestedSSID), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR ret = CHIP_NO_ERROR;
    memset(sInterestedSSID, 0, sizeof(sInterestedSSID));
    memcpy(sInterestedSSID, ssid.data(), ssid.size());
    sInterestedSSIDLen = ssid.size();

    ChipLogProgress(DeviceLayer, "[mw320] initialized network scan. %u, [%s]", sInterestedSSIDLen, sInterestedSSID);
    mpScanCallback = callback;

    // Do Scan
    if (wlan_scan(_OnWpaInterfaceScanDone))
    {
        ChipLogProgress(DeviceLayer, "Error: scan request failed");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Scan scheduled now...");
    }

    ChipLogProgress(DeviceLayer, "[mw320]: initialized network scan. ");
    return ret;
}

void ConnectivityManagerImpl::UpdateNetworkStatus()
{
    Network configuredNetwork;

    VerifyOrReturn(IsWiFiStationEnabled() && mpStatusChangeCallback != nullptr);

    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }

    // If we have already connected to the WiFi AP, then return null to indicate a success state.
    if (IsWiFiStationConnected())
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
        return;
    }

    mpStatusChangeCallback->OnNetworkingStatusChange(
        Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

CHIP_ERROR
ConnectivityManagerImpl::ConnectWiFiNetworkAsync(ByteSpan ssid, ByteSpan credentials,
                                                 NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * apCallback)
{
    CHIP_ERROR ret                        = CHIP_NO_ERROR;
    char ssidStr[kMaxWiFiSSIDLength + 1u] = { 0 };
    char keyStr[kMaxWiFiKeyLength + 1u]   = { 0 };

    VerifyOrReturnError(ssid.size() <= kMaxWiFiSSIDLength, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(credentials.size() <= kMaxWiFiKeyLength, CHIP_ERROR_INVALID_ARGUMENT);

    // There is another ongoing connect request, reject the new one.
    VerifyOrReturnError(mpConnectCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);
    memcpy(ssidStr, ssid.data(), ssid.size());
    memcpy(keyStr, credentials.data(), credentials.size());
    //
    memcpy(sCfgSSID, ssidStr, ssid.size());
    sCfgSSIDLen = ssid.size();
    //
    mpConnectCallback = apCallback;
#if (MW320_CONNECT_SCAN_SYNC == 1)
    if (mpConnectCallback != nullptr)
    {
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
#endif // MW320_CONNECT_SCAN_SYNC

    return ret;
}

bool ConnectivityManagerImpl::_GetBssInfo(const uint8_t sid, NetworkCommissioning::WiFiScanResponse & result)
{
    struct wlan_scan_result res;
    int err;
    err = wlan_get_scan_result(sid, &res);
    if (err)
    {
        ChipLogProgress(DeviceLayer, "Error: can't get scan res %d", sid);
        return false;
    }
    // => ssid
    memset(result.ssid, 0, sizeof(result.ssid));
    result.ssidLen = strlen(res.ssid);
    memcpy(result.ssid, res.ssid, result.ssidLen);
    // => bssid
    memcpy(result.bssid, res.bssid, kWiFiBSSIDLength);
    // => rssi
    result.rssi = static_cast<int8_t>(0 - res.rssi);
    // => band, mw320 only works in 2.4G
    result.wiFiBand = app::Clusters::NetworkCommissioning::WiFiBand::k2g4;
    // => channel
    result.channel = res.channel;
    // => security
    if (res.wep)
    {
        result.security.SetRaw(EMBER_ZCL_SECURITY_TYPE_WEP);
    }
    else if (res.wpa)
    {
        result.security.SetRaw(EMBER_ZCL_SECURITY_TYPE_WPA);
    }
    else if ((res.wpa2) || (res.wpa2_entp))
    {
        result.security.SetRaw(EMBER_ZCL_SECURITY_TYPE_WPA2);
    }
    else if (res.wpa3_sae)
    {
        result.security.SetRaw(EMBER_ZCL_SECURITY_TYPE_WPA3);
    }
    else
    {
        result.security.SetRaw(EMBER_ZCL_SECURITY_TYPE_NONE);
    }

    return true;
}

/*
    Convert the ascii string to hex string with upper case
*/
std::string ConnectivityManagerImpl::to_hex_string(const std::string & input)
{
    std::stringstream hex_stream;
    std::string hex_upstr;
    hex_stream << std::hex << std::internal << std::setfill('0');
    for (auto & byte : input)
        hex_stream << std::setw(2) << static_cast<int>(static_cast<unsigned char>(byte));
    hex_upstr = hex_stream.str();
    transform(hex_upstr.begin(), hex_upstr.end(), hex_upstr.begin(), [](unsigned char c) { return toupper(c); });
    return hex_upstr;
}

int ConnectivityManagerImpl::_OnWpaInterfaceScanDone(unsigned int count)
{
    ChipLogProgress(DeviceLayer, "network scan done (%d)", count);
    // No ap reported
    if (count == 0)
    {
        ChipLogProgress(DeviceLayer, "=> no network found");
        DeviceLayer::SystemLayer().ScheduleLambda([]() {
            if (mpScanCallback != nullptr)
            {
                mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
                mpScanCallback = nullptr;
            }
        });
        return 0;
    }

    // Get the scan result from SDK and push to the list
    std::vector<WiFiScanResponse> * networkScanned = new std::vector<WiFiScanResponse>();
    for (uint8_t id = 0; id < count; id++)
    {
        WiFiScanResponse network;
        if (_GetBssInfo(id, network))
        {
            std::string ascii_ssid((char *) (network.ssid));
            std::string hex_ssid = to_hex_string(ascii_ssid);
            if (sInterestedSSIDLen == 0)
            {
                networkScanned->push_back(network);
            }
            else if (network.ssidLen == sInterestedSSIDLen && memcmp(network.ssid, sInterestedSSID, sInterestedSSIDLen) == 0)
            // else if ((network.ssidLen<<1) == sInterestedSSIDLen && memcmp(hex_ssid.c_str(), sInterestedSSID, sInterestedSSIDLen)
            // == 0)
            {
                networkScanned->push_back(network);
            }
        }
    }

    DeviceLayer::SystemLayer().ScheduleLambda([networkScanned]() {
        // Note: We cannot post a event in ScheduleLambda since std::vector is not trivial copiable. This results in the use of
        // const_cast but should be fine for almost all cases, since we actually handled the ownership of this element to this
        // lambda.
        if (mpScanCallback != nullptr)
        {
            Mw320ScanResponseIterator<WiFiScanResponse> iter(const_cast<std::vector<WiFiScanResponse> *>(networkScanned));
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
            mpScanCallback = nullptr;
        }

        delete const_cast<std::vector<WiFiScanResponse> *>(networkScanned);
    });
    return 0;
}

} // namespace DeviceLayer
} // namespace chip
