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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/EFR32/NetworkCommissioningWiFiDriver.h>
#include <platform/internal/BLEManager.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#include "CHIPDevicePlatformConfig.h"
#include "wfx_host_events.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err;
    // Queue work items to bootstrap the AP and station state machines once the Chip event loop is running.
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mLastStationConnectFailTime   = System::Clock::kZero;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mFlags.ClearAll();

    // TODO Initialize the Chip Addressing and Routing Module.

    // Ensure that station mode is enabled.
    wfx_enable_sta_mode();

    err = DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);

    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
    // Handle Wfx wifi events...
    if (event->Type == DeviceEventType::kWFXSystemEvent)
    {
        if (event->Platform.WFXSystemEvent.eventBase == WIFI_EVENT)
        {
            switch (event->Platform.WFXSystemEvent.data.genericMsgEvent.header.id)
            {
            case SL_WFX_STARTUP_IND_ID:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_START");
                DriveStationState();
                break;
            case SL_WFX_CONNECT_IND_ID:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_CONNECTED");
                if (mWiFiStationState == kWiFiStationState_Connecting)
                {
                    if (event->Platform.WFXSystemEvent.data.connectEvent.body.status == 0)
                    {
                        ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
                    }
                    else
                    {
                        ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
                    }
                }
                DriveStationState();
                break;
            case SL_WFX_DISCONNECT_IND_ID:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_DISCONNECTED");
                if (mWiFiStationState == kWiFiStationState_Connecting)
                {
                    ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
                }
                DriveStationState();
                break;
            default:
                break;
            }
        }
        else if (event->Platform.WFXSystemEvent.eventBase == IP_EVENT)
        {
            switch (event->Platform.WFXSystemEvent.data.genericMsgEvent.header.id)
            {
            case IP_EVENT_STA_GOT_IP:
                ChipLogProgress(DeviceLayer, "IP_EVENT_STA_GOT_IP");
                UpdateInternetConnectivityState();
                break;
            case IP_EVENT_STA_LOST_IP:
                ChipLogProgress(DeviceLayer, "IP_EVENT_STA_LOST_IP");
                UpdateInternetConnectivityState();
                break;
            case IP_EVENT_GOT_IP6:
                ChipLogProgress(DeviceLayer, "IP_EVENT_GOT_IP6");
                UpdateInternetConnectivityState();
                break;
            default:
                break;
            }
        }
    }
}

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wifi_mode_t curWiFiMode = wfx_get_wifi_mode();
        if ((curWiFiMode == WIFI_MODE_STA) || (curWiFiMode == WIFI_MODE_APSTA))
        {
            mWiFiStationMode = kWiFiStationMode_Enabled;
        }
        else
        {
            mWiFiStationMode = kWiFiStationMode_Disabled;
        }
    }
    return mWiFiStationMode;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    wfx_wifi_provision_t wifiConfig;
    if (wfx_get_wifi_provision(&wifiConfig))
    {
        return (wifiConfig.ssid[0] != 0);
    }
    return false;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return wfx_is_sta_mode_enabled();
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    mWiFiStationReconnectInterval = val;
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wfx_clear_wifi_provision();

        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
    }
}

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    // CHIP_ERROR_NOT_IMPLEMENTED
}

void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    // Schedule a call to the DriveStationState method to adjust the station state as needed.
    ChipLogProgress(DeviceLayer, "_ON WIFI PROVISION CHANGE");
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

// == == == == == == == == == == ConnectivityManager Private Methods == == == == == == == == == ==

void ConnectivityManagerImpl::DriveStationState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    sl_status_t serr;
    bool stationConnected;

    // Refresh the current station mode.
    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        // Ensure that the WFX is started.
        if ((serr = wfx_wifi_start()) != SL_STATUS_OK)
        {
            ChipLogError(DeviceLayer, "wfx_wifi_start() failed: %s", chip::ErrorStr(err));
            return;
        }
        // Ensure that station mode is enabled in the WFX WiFi layer.
        wfx_enable_sta_mode();
    }

    stationConnected = wfx_is_sta_connected();

    // If the station interface is currently connected ...
    if (stationConnected)
    {
        // Advance the station state to Connected if it was previously NotConnected or
        // a previously initiated connect attempt succeeded.
        if (mWiFiStationState == kWiFiStationState_NotConnected || mWiFiStationState == kWiFiStationState_Connecting_Succeeded)
        {
            ChangeWiFiStationState(kWiFiStationState_Connected);
            ChipLogProgress(DeviceLayer, "WiFi station interface connected");
            mLastStationConnectFailTime = System::Clock::kZero;
            OnStationConnected();
        }

        // If the WiFi station interface is no longer enabled, or no longer provisioned,
        // disconnect the station from the AP, unless the WiFi station mode is currently
        // under application control.
        if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled &&
            (mWiFiStationMode != kWiFiStationMode_Enabled || !IsWiFiStationProvisioned()))
        {
            ChipLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
            serr = wfx_sta_discon();
            if (serr != SL_STATUS_OK)
            {
                ChipLogError(DeviceLayer, "wfx_wifi_disconnect() failed: %s", chip::ErrorStr(err));
            }
            SuccessOrExit(serr);

            ChangeWiFiStationState(kWiFiStationState_Disconnecting);
        }
    }
    // Otherwise the station interface is NOT connected to an AP, so...
    else
    {
        System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

        // Advance the station state to NotConnected if it was previously Connected or Disconnecting,
        // or if a previous initiated connect attempt failed.
        if (mWiFiStationState == kWiFiStationState_Connected || mWiFiStationState == kWiFiStationState_Disconnecting ||
            mWiFiStationState == kWiFiStationState_Connecting_Failed)
        {
            WiFiStationState prevState = mWiFiStationState;
            ChangeWiFiStationState(kWiFiStationState_NotConnected);
            if (prevState != kWiFiStationState_Connecting_Failed)
            {
                ChipLogProgress(DeviceLayer, "WiFi station interface disconnected");
                mLastStationConnectFailTime = System::Clock::kZero;
                OnStationDisconnected();
            }
            else
            {
                mLastStationConnectFailTime = now;
            }
        }

        // If the WiFi station interface is now enabled and provisioned (and by implication,
        // not presently under application control), AND the system is not in the process of
        // scanning, then...
        if (mWiFiStationMode == kWiFiStationMode_Enabled && IsWiFiStationProvisioned())
        {
            // Initiate a connection to the AP if we haven't done so before, or if enough
            // time has passed since the last attempt.
            if (mLastStationConnectFailTime == System::Clock::kZero ||
                now >= mLastStationConnectFailTime + mWiFiStationReconnectInterval)
            {
                if (mWiFiStationState != kWiFiStationState_Connecting)
                {
                    ChipLogProgress(DeviceLayer, "Attempting to connect WiFi");
                    if ((serr = wfx_connect_to_ap()) != SL_STATUS_OK)
                    {
                        ChipLogError(DeviceLayer, "wfx_connect_to_ap() failed.");
                    }
                    SuccessOrExit(serr);

                    ChangeWiFiStationState(kWiFiStationState_Connecting);
                }
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                System::Clock::Timestamp timeToNextConnect = (mLastStationConnectFailTime + mWiFiStationReconnectInterval) - now;

                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms",
                                System::Clock::Milliseconds32(timeToNextConnect).count());

                ReturnOnFailure(DeviceLayer::SystemLayer().StartTimer(timeToNextConnect, DriveStationState, NULL));
            }
        }
    }

exit:

    ChipLogProgress(DeviceLayer, "Done driving station state, nothing else to do...");
    // Kick-off any pending network scan that might have been deferred due to the activity
    // of the WiFi station.
}

void ConnectivityManagerImpl::OnStationConnected()
{
    ChipDeviceEvent event;
    wfx_setup_ip6_link_local(SL_WFX_STA_INTERFACE);

    NetworkCommissioning::SlWiFiDriver::GetInstance().OnConnectWiFiNetwork();
    // Alert other components of the new state.
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    (void) PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationDisconnected()
{
    // TODO Invoke WARM to perform actions that occur when the WiFi station interface goes down.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    (void) PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
    }
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn = false;
    bool haveIPv6Conn = false;
    bool hadIPv4Conn  = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    bool hadIPv6Conn  = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    IPAddress addr;

    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
        haveIPv4Conn = wfx_have_ipv4_addr(SL_WFX_STA_INTERFACE);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        haveIPv6Conn = wfx_have_ipv6_addr(SL_WFX_STA_INTERFACE);
    }

    // If the internet connectivity state has changed...
    if (haveIPv4Conn != hadIPv4Conn || haveIPv6Conn != hadIPv6Conn)
    {
        // Update the current state.
        mFlags.Set(ConnectivityFlags::kHaveIPv4InternetConnectivity, haveIPv4Conn)
            .Set(ConnectivityFlags::kHaveIPv6InternetConnectivity, haveIPv6Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
        event.InternetConnectivityChange.IPv6      = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);
        event.InternetConnectivityChange.ipAddress = addr;

        (void) PlatformMgr().PostEvent(&event);

        if (haveIPv4Conn != hadIPv4Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv4", (haveIPv4Conn) ? "ESTABLISHED" : "LOST");
        }

        if (haveIPv6Conn != hadIPv6Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv6", (haveIPv6Conn) ? "ESTABLISHED" : "LOST");
        }
    }
}

} // namespace DeviceLayer
} // namespace chip
