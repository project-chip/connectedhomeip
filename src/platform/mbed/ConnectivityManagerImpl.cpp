/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/* this file behaves like a config.h, comes first */

#include "netsocket/WiFiInterface.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.cpp>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.cpp>

#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <type_traits>

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
//#error "WiFi Station support must be enabled when building for mbed"
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
//#error "WiFi AP support must be enabled when building for mbed"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    return mWiFiStationMode;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationApplicationControlled(void)
{
    return mWiFiStationMode == kWiFiStationMode_ApplicationControlled;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;

    return err;
}
bool ConnectivityManagerImpl::_IsWiFiStationConnected(void)
{
    return mWiFiStationState == kWiFiStationState_Connected;
}
bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return mIsProvisioned;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mWiFiAPMode    = val;
    return err;
}

#define WIFI_BAND_2_4GHZ 2400
#define WIFI_BAND_5_0GHZ 5000

static uint16_t Map2400MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inChannel >= 1 && inChannel <= 13)
    {
        // Cast is OK because we definitely fit in 16 bits.
        frequency = static_cast<uint16_t>(2412 + ((inChannel - 1) * 5));
    }
    else if (inChannel == 14)
    {
        frequency = 2484;
    }

    return frequency;
}

static uint16_t Map5000MHz(const uint8_t inChannel)
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

    if (inBand == WIFI_BAND_2_4GHZ)
    {
        frequency = Map2400MHz(inChannel);
    }
    else if (inBand == WIFI_BAND_5_0GHZ)
    {
        frequency = Map5000MHz(inChannel);
    }

    return frequency;
}

CHIP_ERROR ConnectivityManagerImpl::GetWifiStatus(void)
{
    _interface  = WiFiInterface::get_default_instance();
    auto status = _interface->get_connection_status();
    ChipLogDetail(DeviceLayer, "Connection status: %s", status2str(status));
    printf("MAC: %s\n", _interface->get_mac_address());
    SocketAddress a;
    _interface->get_ip_address(&a);
    printf("IP: %s\n", a.get_ip_address());
    _interface->get_netmask(&a);
    printf("Netmask: %s\n", a.get_ip_address());
    _interface->get_gateway(&a);
    printf("Gateway: %s\n", a.get_ip_address());
    printf("RSSI: %d\n\n", _interface->get_rssi());
    return CHIP_NO_ERROR;
}

// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mWiFiStationMode                = kWiFiStationMode_Disabled;
    mWiFiStationState               = kWiFiStationState_NotConnected;
    mIsProvisioned                  = false;
    mWiFiAPMode                     = kWiFiAPMode_NotSupported;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;
    mWiFiAPIdleTimeoutMS            = CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT;

    // TODO Initialize the Chip Addressing and Routing Module.

    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    _interface = WiFiInterface::get_default_instance();

    ChipLogDetail(DeviceLayer, "connect to %s with %s password", ssid, key);
    _interface->attach(
        [](nsapi_event_t event, intptr_t data) { ChipLogDetail(DeviceLayer, "WiFi event: event = %d, data = %p", event, data); });

    auto result = _interface->connect(ssid, key, NSAPI_SECURITY_WPA_WPA2);
    ChipLogDetail(DeviceLayer, "Connection result: %d", result);
    auto status = _interface->get_connection_status();
    ChipLogDetail(DeviceLayer, "Connection status: %s", status2str(status));
    mWiFiStationMode = kWiFiStationMode_Enabled;
    if (status == NSAPI_STATUS_GLOBAL_UP)
    {
        mWiFiStationState = kWiFiStationState_Connected;
        mIsProvisioned    = true;
    }
    else
    {
        mWiFiStationState = kWiFiStationState_NotConnected;
    }

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    _interface = WiFiInterface::get_default_instance();
    _interface->set_credentials(NULL, NULL, NSAPI_SECURITY_NONE);
    mWiFiStationMode  = kWiFiStationMode_Disabled;
    mWiFiStationState = kWiFiStationState_NotConnected;
    mIsProvisioned    = false;
}
int ConnectivityManagerImpl::ScanWiFi(int APlimit, NetworkInfo * wifiInfo)
{

    _interface = WiFiInterface::get_default_instance();
    if (!_interface)
    {
        printf("ERROR: No WiFiInterface found.\n");
        return -1;
    }
    auto status = _interface->get_connection_status();
    if (status != NSAPI_STATUS_GLOBAL_UP)
    {
        printf(" Currently device not connected to any WIFI  AP\n");
    }
    WiFiAccessPoint * ap;

    int count = _interface->scan(NULL, 0);

    if (count <= 0)
    {
        printf("scan() failed with return value: %d\n", count);
        return 0;
    }

    count = count < APlimit ? count : APlimit;

    ap    = new WiFiAccessPoint[count];
    count = _interface->scan(ap, count);

    if (count <= 0)
    {
        return 0;
    }

    for (int i = 0; i < count; i++)
    {
        sprintf(wifiInfo[i].WiFiSSID, "%s", ap[i].get_ssid());
        wifiInfo[i].security = NsapiToNetworkSecurity(ap[i].get_security());
        wifiInfo[i].BSSID[0] = ap[i].get_bssid()[0];
        wifiInfo[i].BSSID[1] = ap[i].get_bssid()[1];
        wifiInfo[i].BSSID[2] = ap[i].get_bssid()[2];
        wifiInfo[i].BSSID[3] = ap[i].get_bssid()[3];
        wifiInfo[i].BSSID[4] = ap[i].get_bssid()[4];
        wifiInfo[i].BSSID[5] = ap[i].get_bssid()[5];
        wifiInfo[i].RSSI     = ap[i].get_rssi();
        wifiInfo[i].channel  = ap[i].get_channel();
    }

    delete[] ap;

    return count;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationConnected()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Assign an IPv6 link local address to the station interface.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    PlatformMgr().PostEvent(&event);
    mWiFiStationState = kWiFiStationState_Connected;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationDisconnected()
{
    // TODO Invoke WARM to perform actions that occur when the WiFi station interface goes down.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    PlatformMgr().PostEvent(&event);
    mWiFiStationState = kWiFiStationState_NotConnected;
}

const char * ConnectivityManagerImpl::status2str(nsapi_connection_status_t status)
{
    switch (status)
    {
    case NSAPI_STATUS_LOCAL_UP:
        return "Network local UP";
    case NSAPI_STATUS_GLOBAL_UP:
        return "Network global UP";
    case NSAPI_STATUS_DISCONNECTED:
        return "Network disconnected";
    case NSAPI_STATUS_CONNECTING:
        return "Network connecting";
    default:
        return "Unknown";
    }
}

WiFiAuthSecurityType ConnectivityManagerImpl::NsapiToNetworkSecurity(nsapi_security_t nsapi_security)
{
    switch (nsapi_security)
    {
    case NSAPI_SECURITY_NONE:
        return kWiFiSecurityType_None;
    case NSAPI_SECURITY_WEP:
        return kWiFiSecurityType_WEP;
    case NSAPI_SECURITY_WPA:
        return kWiFiSecurityType_WPAPersonal;
    case NSAPI_SECURITY_WPA2:
        return kWiFiSecurityType_WPA2Personal;
    case NSAPI_SECURITY_WPA_WPA2:
        return kWiFiSecurityType_WPAEnterprise;
    case NSAPI_SECURITY_PAP:
        return kWiFiSecurityType_NotSpecified;
    case NSAPI_SECURITY_CHAP:
        return kWiFiSecurityType_NotSpecified;
    case NSAPI_SECURITY_EAP_TLS:
        return kWiFiSecurityType_NotSpecified;
    case NSAPI_SECURITY_PEAP:
        return kWiFiSecurityType_NotSpecified;
    case NSAPI_SECURITY_WPA2_ENT:
        return kWiFiSecurityType_WPA2Enterprise;
    case NSAPI_SECURITY_WPA3:
        return kWiFiSecurityType_WPA3Personal;
    case NSAPI_SECURITY_WPA3_WPA2:
        return kWiFiSecurityType_WPA3Enterprise;
    default:
        return kWiFiSecurityType_NotSpecified;
    }
}

} // namespace DeviceLayer
} // namespace chip
