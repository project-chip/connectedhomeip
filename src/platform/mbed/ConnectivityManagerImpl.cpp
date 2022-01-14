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

#include <net_common.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.cpp>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.cpp>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/BLEManager.h>

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

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogDetail(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;

exit:
    return err;
}

// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mWiFiStationMode              = kWiFiStationMode_NotSupported;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mIsProvisioned                = false;
    mIp4Address                   = IPAddress::Any;
    mIp6Address                   = IPAddress::Any;
    mWiFiAPMode                   = kWiFiAPMode_NotSupported;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mWiFiAPIdleTimeout            = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);
    mSecurityType                 = NSAPI_SECURITY_WPA_WPA2;

    auto net_if = get_mbed_net_if();
    if (net_if == nullptr)
    {
        ChipLogError(DeviceLayer, "No network interface available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (net_if->wifiInterface() != nullptr)
    {
        mWiFiInterface   = net_if->wifiInterface();
        mWiFiStationMode = kWiFiStationMode_Enabled;

        // TODO: Add to user documentation that add_event_listener must be used
        // To add more listener to the interface
        mWiFiInterface->add_event_listener([this](nsapi_event_t event, intptr_t data) {
            PlatformMgrImpl().mQueue.call([this, event, data] {
                PlatformMgr().LockChipStack();
                OnInterfaceEvent(event, data);
                PlatformMgr().UnlockChipStack();
            });
        });

        mWiFiInterface->set_blocking(false);
    }
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mWiFiStationReconnectInterval != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station reconnect interval change: %lu ms -> %lu ms",
                        System::Clock::Milliseconds32(mWiFiStationReconnectInterval).count(),
                        System::Clock::Milliseconds32(val).count());
    }

    mWiFiStationReconnectInterval = val;

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
        ChipLogDetail(DeviceLayer, "Unknown connection status: 0x%08X", new_status);
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
    // Validate the interface is available
    if (!mWiFiInterface)
    {
        ChipLogError(DeviceLayer, "WiFi interface not supported");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Set WiFi credentials
    auto error = mWiFiInterface->set_credentials(ssid, key, mSecurityType);
    if (error)
    {
        ChipLogError(DeviceLayer, "Set WiFi credentials failed %d", error);
        return CHIP_ERROR_INTERNAL;
    }

    mIsProvisioned = true;

    PlatformMgr().ScheduleWork(OnWiFiStationChange, 0);

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    // Validate the interface is available
    if (!mWiFiInterface)
    {
        ChipLogError(DeviceLayer, "WiFi interface not supported");
        return;
    }

    // Reset credentials
    auto error = mWiFiInterface->set_credentials("ssid", NULL, NSAPI_SECURITY_NONE);
    if (error)
    {
        ChipLogError(DeviceLayer, "Reset WiFi credentials failed %d", error);
        return;
    }

    mIsProvisioned = false;

    PlatformMgr().ScheduleWork(OnWiFiStationChange, 0);
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
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
        ChipLogProgress(DeviceLayer, "Event - StationConnected");
    }

    // Update IP address
    SocketAddress address;
    auto error = mWiFiInterface->get_ip_address(&address);
    if (error != NSAPI_ERROR_OK)
    {
        if (mIp4Address != IPAddress::Any)
        {
            // Unnexpected change, forward to the application
            mIp4Address = IPAddress::Any;
            ChipDeviceEvent event;
            event.Type                            = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4 = kConnectivity_Lost;
            event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
            ChipLogError(DeviceLayer, "Unnexpected loss of Ip4 address");
        }

        if (mIp6Address != IPAddress::Any)
        {
            // Unnexpected change, forward to the application
            mIp6Address = IPAddress::Any;
            ChipDeviceEvent event;
            event.Type                            = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
            event.InternetConnectivityChange.IPv6 = kConnectivity_Lost;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
            ChipLogError(DeviceLayer, "Unnexpected loss of Ip6 address");
        }
    }
    else
    {
        IPAddress addr;
        if (address.get_ip_version() == NSAPI_IPv4)
        {
            if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp4Address)
            {
                mIp4Address = addr;
                ChipDeviceEvent event;
                event.Type                            = DeviceEventType::kInternetConnectivityChange;
                event.InternetConnectivityChange.IPv4 = kConnectivity_Established;
                event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
                ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
                ChipLogProgress(DeviceLayer, "New Ip4 address set: %s", address.get_ip_address());
            }

            error = mWiFiInterface->get_ipv6_link_local_address(&address);
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
                    ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
                    ChipLogError(DeviceLayer, "Unnexpected loss of Ip6 address");
                }
            }
            else
            {
                if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp6Address)
                {
                    mIp6Address = addr;
                    ChipDeviceEvent event;
                    event.Type                            = DeviceEventType::kInternetConnectivityChange;
                    event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
                    event.InternetConnectivityChange.IPv6 = kConnectivity_Established;
                    ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
                    ChipLogProgress(DeviceLayer, "New Ip6 address set %s", address.get_ip_address());
                }
            }
        }
        else
        {
            if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp6Address)
            {
                mIp6Address = addr;
                ChipDeviceEvent event;
                event.Type                            = DeviceEventType::kInternetConnectivityChange;
                event.InternetConnectivityChange.IPv4 = kConnectivity_NoChange;
                event.InternetConnectivityChange.IPv6 = kConnectivity_Established;
                ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
                ChipLogProgress(DeviceLayer, "New Ip6 address set %s", address.get_ip_address());
            }
        }
    }

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
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
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
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
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
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
        ChipLogError(DeviceLayer, "Loss of Ip6 address");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::OnStationConnecting()
{
    ChipLogDetail(DeviceLayer, "OnStationConnecting");

    // Update WiFi station state and propagate it if necessary
    if (mWiFiStationState != kWiFiStationState_Connected)
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
        error = mWiFiInterface->connect();
        if (error)
        {
            ChipLogError(DeviceLayer, "Network connection failed %d", error);
        }
    }

    if ((mWiFiStationMode == kWiFiStationMode_Enabled) && !IsWiFiStationProvisioned() &&
        mWiFiStationState == kWiFiStationState_Connected)
    {
        // Connect the interface with network
        error = mWiFiInterface->disconnect();
        if (error)
        {
            ChipLogError(DeviceLayer, "Network disconnect failed %d", error);
        }
    }
}

void ConnectivityManagerImpl::OnWiFiStationChange(intptr_t arg)
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

} // namespace DeviceLayer
} // namespace chip
