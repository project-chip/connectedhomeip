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
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mWiFiStationMode                = kWiFiStationMode_Disabled;
    mWiFiStationState               = kWiFiStationState_NotConnected;
    mIsProvisioned                  = false;
    mIp4Address                     = IPAddress::Any;
    mIp6Address                     = IPAddress::Any;
    mWiFiAPMode                     = kWiFiAPMode_NotSupported;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;
    mWiFiAPIdleTimeoutMS            = CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT;

    // TODO Initialize the Chip Addressing and Routing Module.
    _interface = WiFiInterface::get_default_instance();
    _security  = NSAPI_SECURITY_WPA_WPA2;
    if (_interface)
    {
        // TODO: Add to user documentation that add_event_listener must be used
        // To add more listener to the interface
        _interface->add_event_listener([this](nsapi_event_t event, intptr_t data) {
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

void ConnectivityManagerImpl::_ProcessInterfaceChange(nsapi_connection_status_t new_status)
{
    switch (new_status)
    {
    case NSAPI_STATUS_LOCAL_UP:
        ChipLogDetail(DeviceLayer, "Status - LOCAL_UP");
        OnStationConnected();
        break;
    case NSAPI_STATUS_GLOBAL_UP:
        ChipLogDetail(DeviceLayer, "Status - GLOBAL_UP");
        OnStationConnected();
        break;
    case NSAPI_STATUS_DISCONNECTED:
        ChipLogDetail(DeviceLayer, "Status - DISCONNECTED");
        OnStationDisconnected();
        break;
    case NSAPI_STATUS_CONNECTING:
        ChipLogDetail(DeviceLayer, "Status - CONNECTING");
        OnStationConnecting();
        break;
    default:
        ChipLogDetail(DeviceLayer, "Unprocessed WiFi status: 0x%08X", new_status);
        break;
    }
}

void ConnectivityManagerImpl::OnInterfaceEvent(nsapi_event_t event, intptr_t data)
{
    if (event == NSAPI_EVENT_CONNECTION_STATUS_CHANGE)
    {
        _ProcessInterfaceChange((nsapi_connection_status_t) data);
    }
}

CHIP_ERROR ConnectivityManagerImpl::ProvisionWiFiNetwork(const char * ssid, const char * key)
{
#if defined(CHIP_DEVICE_CONFIG_WIFI_SECURITY_SCAN)
#error Wifi security scan Not implemented yet
#else
    // Validate the interface is available
    if (!_interface)
    {
        ChipLogDetail(DeviceLayer, "No WiFiInterface found ");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Connect the interface with the credentials provided
    auto error = _interface->connect(ssid, key, _security);
    if (error)
    {
        ChipLogError(DeviceLayer, "Connection result %d", error);
        return CHIP_ERROR_INTERNAL;
    }

    mIsProvisioned = true;
    auto status    = _interface->get_connection_status();
    ChipLogError(DeviceLayer, "Connection result %d status: %s", error, status2str(status));
    _ProcessInterfaceChange(status);
    return CHIP_NO_ERROR;
#endif
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (!_interface)
    {
        ChipLogDetail(DeviceLayer, "No WiFiInterface found ");
        return;
    }

    // Reset credentials
    _security = NSAPI_SECURITY_WPA_WPA2;
    auto err  = _interface->set_credentials(NULL, NULL, _security);
    if (err)
    {
        ChipLogError(DeviceLayer, "Failed to reset WiFi credentials: error = %d", err);
    }
    else
    {
        mIsProvisioned = false;
    }

    // Disconnect from the WiFi station
    err = _interface->disconnect();
    if (err)
    {
        ChipLogError(DeviceLayer, "Failed to disconnect WiFi interface: error = %d", err);
    }
    else
    {
        mWiFiStationMode = kWiFiStationMode_Disabled;
    }

    _ProcessInterfaceChange(_interface->get_connection_status());
}

CHIP_ERROR ConnectivityManagerImpl::OnStationConnected()
{
    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState != kWiFiStationState_Connected)
    {
        mWiFiStationState = kWiFiStationState_Connected;
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Established;
        PlatformMgr().PostEvent(&event);
        ChipLogProgress(DeviceLayer, "Event - StationConnected");
    }

    // Update IPv4 address
    SocketAddress address;
    auto error = _interface->get_ip_address(&address);
    if (error)
    {
        if (mIp4Address != IPAddress::Any)
        {
            // Unnexpected change, forward to the application
            mIp4Address = IPAddress::Any;
            ChipDeviceEvent event;
            event.Type                            = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4 = kConnectivity_Lost;
            event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
            PlatformMgr().PostEvent(&event);
            ChipLogError(DeviceLayer, "Unnexpected loss of Ip4 address");
        }
    }
    else
    {
        IPAddress addr;
        if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp4Address)
        {
            mIp4Address = addr;
            ChipDeviceEvent event;
            event.Type                            = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4 = kConnectivity_Established;
            event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
            PlatformMgr().PostEvent(&event);
            ChipLogProgress(DeviceLayer, "New Ip4 address set");
        }
    }

    // Update IPv6 address
    error = _interface->get_ipv6_link_local_address(&address);
    if (error)
    {
        if (mIp6Address != IPAddress::Any)
        {
            // Unnexpected change, forward to the application
            mIp6Address = IPAddress::Any;
            ChipDeviceEvent event;
            event.Type                            = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
            event.InternetConnectivityChange.IPv6 = kConnectivity_Lost;
            PlatformMgr().PostEvent(&event);
            ChipLogError(DeviceLayer, "Unnexpected loss of Ip6 address");
        }
    }
    else
    {
        IPAddress addr;
        if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp6Address)
        {
            mIp6Address = addr;
            ChipDeviceEvent event;
            event.Type                            = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
            event.InternetConnectivityChange.IPv6 = kConnectivity_Established;
            PlatformMgr().PostEvent(&event);
            ChipLogProgress(DeviceLayer, "New Ip6 address set");
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationDisconnected()
{
    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState != kWiFiStationState_NotConnected)
    {
        mWiFiStationState = kWiFiStationState_NotConnected;
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
        ChipLogProgress(DeviceLayer, "Event - StationDisconnected");
    }

    // Update IPv4 address
    if (mIp4Address != IPAddress::Any)
    {
        // Unnexpected change, forward to the application
        mIp4Address = IPAddress::Any;
        ChipDeviceEvent event;
        event.Type                            = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = kConnectivity_Lost;
        event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
        PlatformMgr().PostEvent(&event);
        ChipLogError(DeviceLayer, "Loss of Ip4 address");
    }

    if (mIp6Address != IPAddress::Any)
    {
        // Unnexpected change, forward to the application
        mIp6Address = IPAddress::Any;
        ChipDeviceEvent event;
        event.Type                            = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
        event.InternetConnectivityChange.IPv6 = kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
        ChipLogError(DeviceLayer, "Loss of Ip6 address");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationConnecting()
{
    ChipLogProgress(DeviceLayer, "Event - StationConnecting");

    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
    }

    mWiFiStationState = kWiFiStationState_Connecting;

    // Update IPv4 address
    if (mIp4Address != IPAddress::Any)
    {
        // Unnexpected change, forward to the application
        mIp4Address = IPAddress::Any;
        ChipDeviceEvent event;
        event.Type                            = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = kConnectivity_Lost;
        event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
        PlatformMgr().PostEvent(&event);
        ChipLogError(DeviceLayer, "Loss of Ip4 address");
    }

    if (mIp6Address != IPAddress::Any)
    {
        // Unnexpected change, forward to the application
        mIp6Address = IPAddress::Any;
        ChipDeviceEvent event;
        event.Type                            = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
        event.InternetConnectivityChange.IPv6 = kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
        ChipLogError(DeviceLayer, "Loss of Ip6 address");
    }

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

} // namespace DeviceLayer
} // namespace chip
