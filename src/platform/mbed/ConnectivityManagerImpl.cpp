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
using namespace ::chip::DeviceLayer::Internal;
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

void ConnectivityManagerImpl::GetWifiStatus(::chip::DeviceLayer::Internal::NetworkStatus * WifiStatus)
{
    // TODO Update with snprintf or memcpy + strlen
    if (!_interface)
    {
        ChipLogDetail(DeviceLayer, "No WiFiInterface found ");
        return;
    }
    sprintf(WifiStatus->Status, "%s", status2str(_interface->get_connection_status()));
    sprintf(WifiStatus->MAC, "%s", _interface->get_mac_address());
    SocketAddress a;
    _interface->get_ip_address(&a);
    sprintf(WifiStatus->IP, "%s", a.get_ip_address());
    _interface->get_netmask(&a);
    sprintf(WifiStatus->Netmask, "%s", a.get_ip_address());
    _interface->get_gateway(&a);
    sprintf(WifiStatus->Gateway, "%s", a.get_ip_address());
    WifiStatus->RSSI = _interface->get_rssi();
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
    _interface = WiFiInterface::get_default_instance();
    _security  = NSAPI_SECURITY_WPA_WPA2;
    if (_interface)
    {
        _interface->attach([this](nsapi_event_t event, intptr_t data) {
            PlatformMgrImpl().mQueue.call([this, event, data] {
                PlatformMgr().LockChipStack();
                OnInterfaceEvent(event, data);
                PlatformMgr().UnlockChipStack();
            });
        });
    }
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // This is for internal use, state change is handled by platform middleware
}

void ConnectivityManagerImpl::OnInterfaceEvent(nsapi_event_t event, intptr_t data)
{
    if (event == NSAPI_EVENT_CONNECTION_STATUS_CHANGE)
    {
        switch (data)
        {
        case NSAPI_STATUS_LOCAL_UP:
            ChipLogDetail(DeviceLayer, "Status - LOCAL_UP");
            break;
        case NSAPI_STATUS_GLOBAL_UP:
            OnStationConnected();
            break;
        case NSAPI_STATUS_DISCONNECTED:
            OnStationDisconnected();
            break;
        case NSAPI_STATUS_CONNECTING:
            ChipLogDetail(DeviceLayer, "Status - CONNECTING");

            break;
        default:

            break;
        }
    }
}

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
#if defined(CHIP_DEVICE_CONFIG_WIFI_SECURITY_SCAN)
#error Wifi security scan Not implemented yet
#else
    if (!_interface)
    {
        ChipLogDetail(DeviceLayer, "No WiFiInterface found ");
        return -1;
    }
    auto result = _interface->connect(ssid, key, _security);
    auto status = _interface->get_connection_status();
    ChipLogDetail(DeviceLayer, "Connection result %d status: %s", result, status2str(status));
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
#endif
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    mWiFiStationMode  = kWiFiStationMode_Disabled;
    mWiFiStationState = kWiFiStationState_NotConnected;
    mIsProvisioned    = false;
    _security         = NSAPI_SECURITY_WPA_WPA2;

    if (!_interface)
    {
        ChipLogDetail(DeviceLayer, "No WiFiInterface found ");
        return;
    }
    _interface->set_credentials(NULL, NULL, _security);
    _interface->disconnect();
}
int ConnectivityManagerImpl::ScanWiFi(int APlimit, ::chip::DeviceLayer::Internal::NetworkInfo * wifiInfo)
{

    if (!_interface)
    {
        ChipLogDetail(DeviceLayer, "No WiFiInterface found ");
        return -1;
    }
    auto status = _interface->get_connection_status();
    if (status != NSAPI_STATUS_GLOBAL_UP)
    {
        ChipLogDetail(DeviceLayer, "Currently device not connected to any WIFI  AP");
    }
    WiFiAccessPoint * ap;

    int count = _interface->scan(NULL, 0);

    if (count <= 0)
    {
        ChipLogDetail(DeviceLayer, "scan() failed with return value: %d", count);
        return 0;
    }

    count = count < APlimit ? count : APlimit;

    ap    = new WiFiAccessPoint[count];
    count = _interface->scan(ap, count);

    if (count <= 0)
    {
        return 0;
    }
    // use snprintf
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
    ChipLogDetail(DeviceLayer, "Event - StationConnected");
    return CHIP_NO_ERROR;
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
    ChipLogDetail(DeviceLayer, "Event - StationDisconnected");
    return CHIP_NO_ERROR;
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
void ConnectivityManagerImpl::SetWifiSecurity(::chip::DeviceLayer::Internal::WiFiAuthSecurityType security)
{
    switch (security)
    {
    case kWiFiSecurityType_None:
        _security = NSAPI_SECURITY_NONE;
    case kWiFiSecurityType_WEP:
        _security = NSAPI_SECURITY_WEP;
    case kWiFiSecurityType_WPAPersonal:
        _security = NSAPI_SECURITY_WPA;
    case kWiFiSecurityType_WPA2Personal:
        _security = NSAPI_SECURITY_WPA_WPA2;
    case kWiFiSecurityType_WPA3Personal:
        _security = NSAPI_SECURITY_WPA3;
    default:
        _security = NSAPI_SECURITY_WPA_WPA2;
    }
}

} // namespace DeviceLayer
} // namespace chip
