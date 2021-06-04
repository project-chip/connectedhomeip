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
#error "WiFi Station support must be enabled when building for mbed"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mWiFiStationMode                = kWiFiStationMode_NotSupported;
    mWiFiStationState               = kWiFiStationState_NotConnected;
    mIsProvisioned                  = false;
    mWiFiAPMode                     = kWiFiAPMode_NotSupported;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;
    mWiFiAPIdleTimeoutMS            = CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT;
    mSecurityType                   = NSAPI_SECURITY_WPA_WPA2;

    NetworkInterface * net_if = NetworkInterface::get_default_instance();
    if (net_if == nullptr)
    {
        ChipLogError(DeviceLayer, "No network interface available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (net_if->wifiInterface() != nullptr)
    {
        mWifiInterface   = net_if->wifiInterface();
        mWiFiStationMode = kWiFiStationMode_Enabled;

        // TODO: Add to user documentation that add_event_listener must be used
        // To add more listener to the interface
        mWifiInterface->add_event_listener([this](nsapi_event_t event, intptr_t data) {
            if (event == NSAPI_EVENT_CONNECTION_STATUS_CHANGE)
            {
                mConnectionStatus = (nsapi_connection_status_t) data;
                PlatformMgr().ScheduleWork(OnConnectionStatusChange, NULL);
            }
        });

        mWifiInterface->set_blocking(false);
    }

    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != val)
    {
        ChipLogDetail(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                      WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;

exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;

    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectIntervalMS(uint32_t val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mWiFiStationReconnectIntervalMS != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station reconnect interval MS change: %d -> %d", mWiFiStationReconnectIntervalMS, val);
    }

    mWiFiStationReconnectIntervalMS = val;

    return err;
}

void ConnectivityManagerImpl::GetWifiStatus(::chip::DeviceLayer::Internal::NetworkStatus * WifiStatus)
{
    SocketAddress address;

    // Validate the interface is available
    if (!mWifiInterface)
    {
        ChipLogError(DeviceLayer, "WiFi interface not supported");
        return;
    }

    strncpy(WifiStatus->Status, status2str(mWifiInterface->get_connection_status()), kMaxWiFiStatusLength);
    strncpy(WifiStatus->MAC, mWifiInterface->get_mac_address(), kMaxWiFiStatusLength);
    mWifiInterface->get_ip_address(&address);
    strncpy(WifiStatus->IP, address.get_ip_address(), kMaxWiFiStatusLength);
    mWifiInterface->get_netmask(&address);
    strncpy(WifiStatus->Netmask, address.get_ip_address(), kMaxWiFiStatusLength);
    mWifiInterface->get_gateway(&address);
    strncpy(WifiStatus->Gateway, address.get_ip_address(), kMaxWiFiStatusLength);
    WifiStatus->RSSI = (int8_t) mWifiInterface->get_rssi();
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // This is for internal use, state change is handled by platform middleware
}

void ConnectivityManagerImpl::ChangeConnectionStatus()
{
    switch (mConnectionStatus)
    {
    case NSAPI_STATUS_LOCAL_UP:
        ChipLogDetail(DeviceLayer, "Connection status - LOCAL_UP");
        OnStationConnected();
        break;
    case NSAPI_STATUS_GLOBAL_UP:
        ChipLogDetail(DeviceLayer, "Connection status - GLOBAL_UP");
        OnStationConnected();
        break;
    case NSAPI_STATUS_DISCONNECTED:
        ChipLogDetail(DeviceLayer, "Connection status - DISCONNECTED");
        OnStationDisconnected();
        break;
    case NSAPI_STATUS_CONNECTING:
        ChipLogDetail(DeviceLayer, "Connection status - CONNECTING");
        OnStationConnecting();
        break;
    default:
        ChipLogDetail(DeviceLayer, "Unknown connection status: 0x%08X", mConnectionStatus);
        break;
    }
}

void ConnectivityManagerImpl::OnConnectionStatusChange(intptr_t arg)
{
    sInstance.ChangeConnectionStatus();
}

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    // Validate the interface is available
    if (!mWifiInterface)
    {
        ChipLogError(DeviceLayer, "WiFi interface not supported");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Set WiFi credentials
    auto error = mWifiInterface->set_credentials(ssid, key, mSecurityType);
    if (error)
    {
        ChipLogError(DeviceLayer, "Set WiFi credentials failed %d", error);
        return CHIP_ERROR_INTERNAL;
    }

    mIsProvisioned = true;

    PlatformMgr().ScheduleWork(OnWifiStationChange, NULL);

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    // Validate the interface is available
    if (!mWifiInterface)
    {
        ChipLogError(DeviceLayer, "WiFi interface not supported");
        return;
    }

    // Reset credentials
    auto error = mWifiInterface->set_credentials("ssid", NULL, NSAPI_SECURITY_NONE);
    if (error)
    {
        ChipLogError(DeviceLayer, "Reset WiFi credentials failed %d", error);
        return;
    }

    mIsProvisioned = false;

    PlatformMgr().ScheduleWork(OnWifiStationChange, NULL);
}

int ConnectivityManagerImpl::ScanWiFi(int APlimit, ::chip::DeviceLayer::Internal::NetworkInfo * wifiInfo)
{
    WiFiAccessPoint * ap;
    int count;

    // Validate the interface is available
    if (!mWifiInterface)
    {
        ChipLogError(DeviceLayer, "WiFi interface not supported");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    count = mWifiInterface->scan(NULL, 0);
    if (count < 0)
    {
        ChipLogError(DeviceLayer, "WiFi scan failed with %d", count);
        return CHIP_ERROR_INTERNAL;
    }

    if (count == 0)
    {
        ChipLogDetail(DeviceLayer, "No available networks");
        return count;
    }

    count = count < APlimit ? count : APlimit;
    ap    = new WiFiAccessPoint[count];

    count = mWifiInterface->scan(ap, count);
    if (count < 0)
    {
        ChipLogError(DeviceLayer, "WiFi scan failed with %d", count);
        return CHIP_ERROR_INTERNAL;
    }

    // use snprintf
    for (int i = 0; i < count; i++)
    {
        strncpy(wifiInfo[i].WiFiSSID, ap[i].get_ssid(), kMaxWiFiSSIDLength);
        wifiInfo[i].security = NsapiToNetworkSecurity(ap[i].get_security());
        memcpy(wifiInfo[i].BSSID, ap[i].get_bssid(), sizeof(wifiInfo[i].BSSID));
        wifiInfo[i].RSSI    = (int8_t) ap[i].get_rssi();
        wifiInfo[i].channel = (int8_t) ap[i].get_channel();
    }

    delete[] ap;

    return count;
}

CHIP_ERROR ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn      = false;
    bool haveIPv6Conn      = false;
    const bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    const bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    IPAddress addr;
    SocketAddress address;
    nsapi_error_t ret;

    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        ret = mWifiInterface->get_ip_address(&address);
        if (ret == NSAPI_ERROR_OK)
        {
            IPAddress::FromString(address.get_ip_address(), addr);
            if (address.get_ip_version() == NSAPI_IPv4)
            {
                haveIPv4Conn = true;
            }
            else if (address.get_ip_version() == NSAPI_IPv6)
            {
                haveIPv6Conn = true;
            }

            ChipLogDetail(DeviceLayer, "%s Internet connectivity with address %s", addr.IsIPv4() ? "IPv4" : "IPv6",
                          address.get_ip_address());
        }
        else
        {
            ChipLogError(DeviceLayer, "Network interface get IP address failed [%d]", ret);
            return CHIP_ERROR_INTERNAL;
        }
    }

    // If the internet connectivity state has changed...
    if (haveIPv4Conn != hadIPv4Conn || haveIPv6Conn != hadIPv6Conn)
    {
        // Update the current state.
        mFlags.Set(ConnectivityFlags::kHaveIPv4InternetConnectivity, haveIPv4Conn)
            .Set(ConnectivityFlags::kHaveIPv6InternetConnectivity, haveIPv6Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                            = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
        event.InternetConnectivityChange.IPv6 = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);
        addr.ToString(event.InternetConnectivityChange.address);
        PlatformMgr().PostEvent(&event);

        if (haveIPv4Conn != hadIPv4Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv4", (haveIPv4Conn) ? "ESTABLISHED" : "LOST");
        }

        if (haveIPv6Conn != hadIPv6Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv6", (haveIPv6Conn) ? "ESTABLISHED" : "LOST");
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationConnected()
{
    ChipLogDetail(DeviceLayer, "OnStationConnected");

    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState != kWiFiStationState_Connected)
    {
        ChipLogProgress(DeviceLayer, "WiFi connectivity ESTABLISHED");
        mWiFiStationState = kWiFiStationState_Connected;
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Established;
        PlatformMgr().PostEvent(&event);
    }

    UpdateInternetConnectivityState();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationDisconnected()
{
    ChipLogDetail(DeviceLayer, "OnStationDisconnected");

    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState != kWiFiStationState_NotConnected)
    {
        mWiFiStationState = kWiFiStationState_NotConnected;
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
    }

    UpdateInternetConnectivityState();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationConnecting()
{
    ChipLogDetail(DeviceLayer, "OnStationConnecting");

    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        ChipLogProgress(DeviceLayer, "WiFi connectivity LOST");
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
    }
    else
    {
        mWiFiStationState = kWiFiStationState_Connecting;
    }

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::ExecuteStationChange()
{
    nsapi_error_t error;
    if ((mWiFiStationMode == kWiFiStationMode_Enabled) && IsWiFiStationProvisioned() &&
        mWiFiStationState != kWiFiStationState_Connected)
    {
        // Connect the interface with network
        error = mWifiInterface->connect();
        if (error)
        {
            ChipLogError(DeviceLayer, "Network connection failed %d", error);
        }
    }

    if ((mWiFiStationMode == kWiFiStationMode_Enabled) && !IsWiFiStationProvisioned() &&
        mWiFiStationState == kWiFiStationState_Connected)
    {
        // Connect the interface with network
        error = mWifiInterface->disconnect();
        if (error)
        {
            ChipLogError(DeviceLayer, "Network disconnect failed %d", error);
        }
    }
}

void ConnectivityManagerImpl::OnWifiStationChange(intptr_t arg)
{
    sInstance.ExecuteStationChange();
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
        mSecurityType = NSAPI_SECURITY_NONE;
    case kWiFiSecurityType_WEP:
        mSecurityType = NSAPI_SECURITY_WEP;
    case kWiFiSecurityType_WPAPersonal:
        mSecurityType = NSAPI_SECURITY_WPA;
    case kWiFiSecurityType_WPA2Personal:
        mSecurityType = NSAPI_SECURITY_WPA_WPA2;
    case kWiFiSecurityType_WPA3Personal:
        mSecurityType = NSAPI_SECURITY_WPA3;
    default:
        mSecurityType = NSAPI_SECURITY_WPA_WPA2;
    }
}

} // namespace DeviceLayer
} // namespace chip
