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
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif
#include <platform/ConnectivityManager.h>

#include <platform/ESP32/ESP32Utils.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "esp_event.h"
#include "esp_wifi.h"

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for ESP32"
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
#error "WiFi AP support must be enabled when building for ESP32"
#endif

using namespace ::chip;
using namespace ::chip::TLV;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wifi_mode_t curWiFiMode;
        mWiFiStationMode =
            (esp_wifi_get_mode(&curWiFiMode) == ESP_OK && (curWiFiMode == WIFI_MODE_APSTA || curWiFiMode == WIFI_MODE_STA))
            ? kWiFiStationMode_Enabled
            : kWiFiStationMode_Disabled;
    }
    return mWiFiStationMode;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (val != kWiFiStationMode_ApplicationControlled)
    {
        bool autoConnect = (val == kWiFiStationMode_Enabled);
        err              = Internal::ESP32Utils::SetAPMode(autoConnect);
        SuccessOrExit(err);

        SystemLayer.ScheduleWork(DriveStationState, NULL);
    }

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return Internal::ESP32Utils::IsStationProvisioned();
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wifi_config_t stationConfig;

        memset(&stationConfig, 0, sizeof(stationConfig));
        esp_wifi_set_config(ESP_IF_WIFI_STA, &stationConfig);

        SystemLayer.ScheduleWork(DriveStationState, NULL);
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
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

    SystemLayer.ScheduleWork(DriveAPState, NULL);

exit:
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::Layer::GetClock_MonotonicMS();
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = 0;
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        if (mWiFiAPState == kWiFiAPState_Activating || mWiFiAPState == kWiFiAPState_Active)
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

#define WIFI_BAND_2_4GHZ 2400
#define WIFI_BAND_5_0GHZ 5000

static uint16_t Map2400MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inChannel >= 1 && inChannel <= 13)
    {
        frequency = 2412 + ((inChannel - 1) * 5);
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

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    CHIP_ERROR err;
    wifi_config_t wifiConfig;
    uint8_t primaryChannel;
    wifi_second_chan_t secondChannel;
    uint16_t freq;
    uint16_t bssid;

    err = esp_wifi_get_config(ESP_IF_WIFI_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_config() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    err = esp_wifi_get_channel(&primaryChannel, &secondChannel);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_channel() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    freq  = MapFrequency(WIFI_BAND_2_4GHZ, primaryChannel);
    bssid = (wifiConfig.sta.bssid[4] << 8) | wifiConfig.sta.bssid[5];
    ChipLogProgress(DeviceLayer,
                    "Wifi-Telemetry\n"
                    "BSSID: %x\n"
                    "freq: %d\n",
                    bssid, freq);
exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_SetServiceTunnelMode(ServiceTunnelMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kServiceTunnelMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    mServiceTunnelMode = val;

    SystemLayer.ScheduleWork(DriveServiceTunnelState, NULL);

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsServiceTunnelConnected(void)
{
    return false;
}

bool ConnectivityManagerImpl::_IsServiceTunnelRestricted(void)
{
    return false;
}

bool ConnectivityManagerImpl::_HaveServiceConnectivityViaTunnel(void)
{
    return IsServiceTunnelConnected() && !IsServiceTunnelRestricted();
}

// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err;

    mLastStationConnectFailTime     = 0;
    mLastAPDemandTime               = 0;
    mWiFiStationMode                = kWiFiStationMode_Disabled;
    mWiFiStationState               = kWiFiStationState_NotConnected;
    mWiFiAPMode                     = kWiFiAPMode_Disabled;
    mWiFiAPState                    = kWiFiAPState_NotActive;
    mServiceTunnelMode              = kServiceTunnelMode_Enabled;
    mWiFiStationReconnectIntervalMS = CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;
    mWiFiAPIdleTimeoutMS            = CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT;
    mFlags                          = 0;

    // TODO Initialize the Chip Addressing and Routing Module.

    // TODO Initialize the service tunnel agent.

    // Ensure that ESP station mode is enabled.
    err = Internal::ESP32Utils::EnableStationMode();
    SuccessOrExit(err);

    // If there is no persistent station provision...
    if (!IsWiFiStationProvisioned())
    {
        // If the code has been compiled with a default WiFi station provision, configure that now.
        if (CONFIG_DEFAULT_WIFI_SSID[0] != 0)
        {
            ChipLogProgress(DeviceLayer, "Setting default WiFi station configuration (SSID: %s)", CONFIG_DEFAULT_WIFI_SSID);

            // Set a default station configuration.
            wifi_config_t wifiConfig;
            memset(&wifiConfig, 0, sizeof(wifiConfig));
            memcpy(wifiConfig.sta.ssid, CONFIG_DEFAULT_WIFI_SSID, strlen(CONFIG_DEFAULT_WIFI_SSID) + 1);
            memcpy(wifiConfig.sta.password, CONFIG_DEFAULT_WIFI_PASSWORD, strlen(CONFIG_DEFAULT_WIFI_PASSWORD) + 1);
            wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
            wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
            err                        = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig);
            if (err != ESP_OK)
            {
                ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", chip::ErrorStr(err));
            }
            err = CHIP_NO_ERROR;

            // Enable WiFi station mode.
            err = SetWiFiStationMode(kWiFiStationMode_Enabled);
            SuccessOrExit(err);
        }

        // Otherwise, ensure WiFi station mode is disabled.
        else
        {
            err = SetWiFiStationMode(kWiFiStationMode_Disabled);
            SuccessOrExit(err);
        }
    }

    // Force AP mode off for now.
    err = Internal::ESP32Utils::SetAPMode(false);
    SuccessOrExit(err);

    // Queue work items to bootstrap the AP and station state machines once the Chip event loop is running.
    err = SystemLayer.ScheduleWork(DriveStationState, NULL);
    SuccessOrExit(err);
    err = SystemLayer.ScheduleWork(DriveAPState, NULL);
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Handle ESP system events...
    if (event->Type == DeviceEventType::kESPSystemEvent)
    {
        switch (event->Platform.ESPSystemEvent.event_id)
        {
        case SYSTEM_EVENT_STA_START:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_START");
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_CONNECTED");
            if (mWiFiStationState == kWiFiStationState_Connecting)
            {
                ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
            }
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_DISCONNECTED");
            if (mWiFiStationState == kWiFiStationState_Connecting)
            {
                ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
            }
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_STOP:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_STOP");
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_GOT_IP");
            OnStationIPv4AddressAvailable(event->Platform.ESPSystemEvent.event_info.got_ip);
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_LOST_IP");
            OnStationIPv4AddressLost();
            break;
        case SYSTEM_EVENT_GOT_IP6:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_GOT_IP6");
            OnIPv6AddressAvailable(event->Platform.ESPSystemEvent.event_info.got_ip6);
            break;
        case SYSTEM_EVENT_AP_START:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_AP_START");
            ChangeWiFiAPState(kWiFiAPState_Active);
            DriveAPState();
            break;
        case SYSTEM_EVENT_AP_STOP:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_AP_STOP");
            ChangeWiFiAPState(kWiFiAPState_NotActive);
            DriveAPState();
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_AP_STACONNECTED");
            MaintainOnDemandWiFiAP();
            break;
        default:
            break;
        }
    }

    // Handle fabric membership changes.
    else if (event->Type == DeviceEventType::kFabricMembershipChange)
    {
        DriveServiceTunnelState();
    }

    // Handle service provisioning changes.
    else if (event->Type == DeviceEventType::kServiceProvisioningChange)
    {
        DriveServiceTunnelState();
    }

#if !CHIP_DEVICE_CONFIG_DISABLE_ACCOUNT_PAIRING

    // Handle account pairing changes.
    else if (event->Type == DeviceEventType::kAccountPairingChange)
    {
        // When account pairing successfully completes, if the tunnel to the
        // service is subject to routing restrictions (imposed because at the time
        // the tunnel was established the device was not paired to an account)
        // then force the tunnel to close.  This will result in the tunnel being
        // re-established, which should lift the service-side restrictions.
        if (event->AccountPairingChange.IsPairedToAccount && GetFlag(mFlags, kFlag_ServiceTunnelStarted))
        {
            ChipLogProgress(DeviceLayer, "Restarting service tunnel to lift routing restrictions");
            ClearFlag(mFlags, kFlag_ServiceTunnelStarted);
            DriveServiceTunnelState();
        }
    }

#endif // !CHIP_DEVICE_CONFIG_DISABLE_ACCOUNT_PAIRING
}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    // Schedule a call to DriveStationState method in case a station connect attempt was
    // deferred because the scan was in progress.
    SystemLayer.ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    // Schedule a call to the DriveStationState method to adjust the station state as needed.
    SystemLayer.ScheduleWork(DriveStationState, NULL);
}

// ==================== ConnectivityManager Private Methods ====================

void ConnectivityManagerImpl::DriveStationState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool stationConnected;

    // Refresh the current station mode.  Specifically, this reads the ESP auto_connect flag,
    // which determine whether the WiFi station mode is kWiFiStationMode_Enabled or
    // kWiFiStationMode_Disabled.
    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        // Ensure that the ESP WiFi layer is started.
        err = Internal::ESP32Utils::StartWiFiLayer();
        SuccessOrExit(err);

        // Ensure that station mode is enabled in the ESP WiFi layer.
        err = Internal::ESP32Utils::EnableStationMode();
        SuccessOrExit(err);
    }

    // Determine if the ESP WiFi layer thinks the station interface is currently connected.
    err = Internal::ESP32Utils::IsStationConnected(stationConnected);
    SuccessOrExit(err);

    // If the station interface is currently connected ...
    if (stationConnected)
    {
        // Advance the station state to Connected if it was previously NotConnected or
        // a previously initiated connect attempt succeeded.
        if (mWiFiStationState == kWiFiStationState_NotConnected || mWiFiStationState == kWiFiStationState_Connecting_Succeeded)
        {
            ChangeWiFiStationState(kWiFiStationState_Connected);
            ChipLogProgress(DeviceLayer, "WiFi station interface connected");
            mLastStationConnectFailTime = 0;
            OnStationConnected();
        }

        // If the WiFi station interface is no longer enabled, or no longer provisioned,
        // disconnect the station from the AP, unless the WiFi station mode is currently
        // under application control.
        if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled &&
            (mWiFiStationMode != kWiFiStationMode_Enabled || !IsWiFiStationProvisioned()))
        {
            ChipLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
            err = esp_wifi_disconnect();
            if (err != ESP_OK)
            {
                ChipLogError(DeviceLayer, "esp_wifi_disconnect() failed: %s", chip::ErrorStr(err));
            }
            SuccessOrExit(err);

            ChangeWiFiStationState(kWiFiStationState_Disconnecting);
        }
    }

    // Otherwise the station interface is NOT connected to an AP, so...
    else
    {
        uint64_t now = System::Layer::GetClock_MonotonicMS();

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
                mLastStationConnectFailTime = 0;
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
            if (mLastStationConnectFailTime == 0 || now >= mLastStationConnectFailTime + mWiFiStationReconnectIntervalMS)
            {
                ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
                err = esp_wifi_connect();
                if (err != ESP_OK)
                {
                    ChipLogError(DeviceLayer, "esp_wifi_connect() failed: %s", chip::ErrorStr(err));
                }
                SuccessOrExit(err);

                ChangeWiFiStationState(kWiFiStationState_Connecting);
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                uint32_t timeToNextConnect = (uint32_t)((mLastStationConnectFailTime + mWiFiStationReconnectIntervalMS) - now);

                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms", timeToNextConnect);

                err = SystemLayer.StartTimer(timeToNextConnect, DriveStationState, NULL);
                SuccessOrExit(err);
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
    CHIP_ERROR err;

    // Assign an IPv6 link local address to the station interface.
    err = tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA) failed: %s", chip::ErrorStr(err));
    }

    // TODO Invoke WARM to perform actions that occur when the WiFi station interface comes up.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationDisconnected()
{
    // TODO Invoke WARM to perform actions that occur when the WiFi station interface goes down.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState();
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

void ConnectivityManagerImpl::DriveStationState(chip::System::Layer * aLayer, void * aAppState, chip::System::Error aError)
{
    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;
    uint64_t now;
    uint32_t apTimeout;
    bool espAPModeEnabled;

    // Determine if AP mode is currently enabled in the ESP WiFi layer.
    err = Internal::ESP32Utils::IsAPEnabled(espAPModeEnabled);
    SuccessOrExit(err);

    // Adjust the Connectivity Manager's AP state to match the state in the WiFi layer.
    if (espAPModeEnabled && (mWiFiAPState == kWiFiAPState_NotActive || mWiFiAPState == kWiFiAPState_Deactivating))
    {
        ChangeWiFiAPState(kWiFiAPState_Activating);
    }
    if (!espAPModeEnabled && (mWiFiAPState == kWiFiAPState_Active || mWiFiAPState == kWiFiAPState_Activating))
    {
        ChangeWiFiAPState(kWiFiAPState_Deactivating);
    }

    // If the AP interface is not under application control...
    if (mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        // Ensure the ESP WiFi layer is started.
        err = Internal::ESP32Utils::StartWiFiLayer();
        SuccessOrExit(err);

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
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " ms", apTimeout);
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
            // If the target state is 'Active' and the current state is NOT 'Activating', enable
            // and configure the AP interface, and then enter the 'Activating' state.  Eventually
            // a SYSTEM_EVENT_AP_START event will be received from the ESP WiFi layer which will
            // cause the state to transition to 'Active'.
            if (targetState == kWiFiAPState_Active)
            {
                if (mWiFiAPState != kWiFiAPState_Activating)
                {
                    err = Internal::ESP32Utils::SetAPMode(true);
                    SuccessOrExit(err);

                    err = ConfigureWiFiAP();
                    SuccessOrExit(err);

                    ChangeWiFiAPState(kWiFiAPState_Activating);
                }
            }

            // Otherwise, if the target state is 'NotActive' and the current state is not 'Deactivating',
            // disable the AP interface and enter the 'Deactivating' state.  Later a SYSTEM_EVENT_AP_STOP
            // event will move the AP state to 'NotActive'.
            else
            {
                if (mWiFiAPState != kWiFiAPState_Deactivating)
                {
                    err = Internal::ESP32Utils::SetAPMode(false);
                    SuccessOrExit(err);

                    ChangeWiFiAPState(kWiFiAPState_Deactivating);
                }
            }
        }
    }

    // If AP is active, but the interface doesn't have an IPv6 link-local
    // address, assign one now.
    if (mWiFiAPState == kWiFiAPState_Active && Internal::ESP32Utils::IsInterfaceUp(TCPIP_ADAPTER_IF_AP) &&
        !Internal::ESP32Utils::HasIPv6LinkLocalAddress(TCPIP_ADAPTER_IF_AP))
    {
        err = tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_AP);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_AP) failed: %s", chip::ErrorStr(err));
        }
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR && mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        SetWiFiAPMode(kWiFiAPMode_Disabled);
        Internal::ESP32Utils::SetAPMode(false);
    }
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t wifiConfig;

    memset(&wifiConfig, 0, sizeof(wifiConfig));

    uint32_t discriminator;
    SuccessOrExit(err = ConfigurationMgr().GetSetupDiscriminator(discriminator));

    snprintf((char *) wifiConfig.ap.ssid, sizeof(wifiConfig.ap.ssid), "%s%04u", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX,
             discriminator);
    wifiConfig.ap.channel         = CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL;
    wifiConfig.ap.authmode        = WIFI_AUTH_OPEN;
    wifiConfig.ap.max_connection  = CHIP_DEVICE_CONFIG_WIFI_AP_MAX_STATIONS;
    wifiConfig.ap.beacon_interval = CHIP_DEVICE_CONFIG_WIFI_AP_BEACON_INTERVAL;
    ChipLogProgress(DeviceLayer, "Configuring WiFi AP: SSID %s, channel %u", wifiConfig.ap.ssid, wifiConfig.ap.channel);
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config(ESP_IF_WIFI_AP) failed: %s", chip::ErrorStr(err));
    }
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    if (mWiFiAPState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", WiFiAPStateToStr(mWiFiAPState), WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}

void ConnectivityManagerImpl::DriveAPState(chip::System::Layer * aLayer, void * aAppState, chip::System::Error aError)
{
    sInstance.DriveAPState();
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn = false;
    bool haveIPv6Conn = false;
    bool hadIPv4Conn  = GetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity);
    bool hadIPv6Conn  = GetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity);

    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        // Get the LwIP netif for the WiFi station interface.
        struct netif * netif = Internal::ESP32Utils::GetStationNetif();

        // If the WiFi station interface is up...
        if (netif != NULL && netif_is_up(netif) && netif_is_link_up(netif))
        {
            // Check if a DNS server is currently configured.  If so...
            ip_addr_t dnsServerAddr = *dns_getserver(0);
            if (!ip_addr_isany_val(dnsServerAddr))
            {
                // If the station interface has been assigned an IPv4 address, and has
                // an IPv4 gateway, then presume that the device has IPv4 Internet
                // connectivity.
                if (!ip4_addr_isany_val(*netif_ip4_addr(netif)) && !ip4_addr_isany_val(*netif_ip4_gw(netif)))
                {
                    haveIPv4Conn = true;
                }

                // Search among the IPv6 addresses assigned to the interface for a Global Unicast
                // address (2000::/3) that is in the valid state.  If such an address is found...
                for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
                {
                    if (ip6_addr_isglobal(netif_ip6_addr(netif, i)) && ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
                    {
                        // Determine if there is a default IPv6 router that is currently reachable
                        // via the station interface.  If so, presume for now that the device has
                        // IPv6 connectivity.
                        struct netif * found_if = nd6_find_route(IP6_ADDR_ANY6);
                        if (found_if && netif->num == found_if->num)
                        {
                            haveIPv6Conn = true;
                        }
                    }
                }
            }
        }
    }

    // If the internet connectivity state has changed...
    if (haveIPv4Conn != hadIPv4Conn || haveIPv6Conn != hadIPv6Conn)
    {
        // Update the current state.
        SetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity, haveIPv4Conn);
        SetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity, haveIPv6Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                            = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
        event.InternetConnectivityChange.IPv6 = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);
        PlatformMgr().PostEvent(&event);

        if (haveIPv4Conn != hadIPv4Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv4", (haveIPv4Conn) ? "ESTABLISHED" : "LOST");
        }

        if (haveIPv6Conn != hadIPv6Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv6", (haveIPv6Conn) ? "ESTABLISHED" : "LOST");
        }

        DriveServiceTunnelState();
    }
}

void ConnectivityManagerImpl::OnStationIPv4AddressAvailable(const system_event_sta_got_ip_t & got_ip)
{
#if CHIP_PROGRESS_LOGGING
    {
        char ipAddrStr[INET_ADDRSTRLEN], netMaskStr[INET_ADDRSTRLEN], gatewayStr[INET_ADDRSTRLEN];
        IPAddress::FromIPv4(got_ip.ip_info.ip).ToString(ipAddrStr, sizeof(ipAddrStr));
        IPAddress::FromIPv4(got_ip.ip_info.netmask).ToString(netMaskStr, sizeof(netMaskStr));
        IPAddress::FromIPv4(got_ip.ip_info.gw).ToString(gatewayStr, sizeof(gatewayStr));
        ChipLogProgress(DeviceLayer, "IPv4 address %s on WiFi station interface: %s/%s gateway %s",
                        (got_ip.ip_changed) ? "changed" : "ready", ipAddrStr, netMaskStr, gatewayStr);
    }
#endif // CHIP_PROGRESS_LOGGING

    RefreshMessageLayer();

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationIPv4AddressLost(void)
{
    ChipLogProgress(DeviceLayer, "IPv4 address lost on WiFi station interface");

    RefreshMessageLayer();

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnIPv6AddressAvailable(const system_event_got_ip6_t & got_ip)
{
#if CHIP_PROGRESS_LOGGING
    {
        IPAddress ipAddr = IPAddress::FromIPv6(got_ip.ip6_info.ip);
        char ipAddrStr[INET6_ADDRSTRLEN];
        ipAddr.ToString(ipAddrStr, sizeof(ipAddrStr));
        ChipLogProgress(DeviceLayer, "IPv6 addr available. Ready on %s interface: %s",
                        Internal::ESP32Utils::InterfaceIdToName(got_ip.if_index), ipAddrStr);
    }
#endif // CHIP_PROGRESS_LOGGING

    RefreshMessageLayer();

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::DriveServiceTunnelState(void)
{
    bool startServiceTunnel;

    // Determine if the tunnel to the service should be started.
    startServiceTunnel = (mServiceTunnelMode == kServiceTunnelMode_Enabled && GetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity));

    // If the tunnel should be started but isn't, or vice versa, ...
    if (startServiceTunnel != GetFlag(mFlags, kFlag_ServiceTunnelStarted))
    {
        // Update the tunnel started state.
        SetFlag(mFlags, kFlag_ServiceTunnelStarted, startServiceTunnel);

        // Start or stop the tunnel as necessary.
        if (startServiceTunnel)
        {
            ChipLogProgress(DeviceLayer, "Starting service tunnel");

            ChipLogError(DeviceLayer, "Service tunnel unsupported");
            ClearFlag(mFlags, kFlag_ServiceTunnelStarted);
        }

        else
        {
            ChipLogProgress(DeviceLayer, "Stopping service tunnel");
        }
    }
}

void ConnectivityManagerImpl::DriveServiceTunnelState(chip::System::Layer * aLayer, void * aAppState, chip::System::Error aError)
{
    sInstance.DriveServiceTunnelState();
}

const char * ConnectivityManagerImpl::_WiFiStationModeToStr(WiFiStationMode mode)
{
    switch (mode)
    {
    case kWiFiStationMode_NotSupported:
        return "NotSupported";
    case kWiFiStationMode_ApplicationControlled:
        return "AppControlled";
    case kWiFiStationMode_Enabled:
        return "Enabled";
    case kWiFiStationMode_Disabled:
        return "Disabled";
    default:
        return "(unknown)";
    }
}

const char * ConnectivityManagerImpl::_WiFiAPModeToStr(WiFiAPMode mode)
{
    switch (mode)
    {
    case kWiFiAPMode_NotSupported:
        return "NotSupported";
    case kWiFiAPMode_ApplicationControlled:
        return "AppControlled";
    case kWiFiAPMode_Disabled:
        return "Disabled";
    case kWiFiAPMode_Enabled:
        return "Enabled";
    case kWiFiAPMode_OnDemand:
        return "OnDemand";
    case kWiFiAPMode_OnDemand_NoStationProvision:
        return "OnDemand_NoStationProvision";
    default:
        return "(unknown)";
    }
}

const char * ConnectivityManagerImpl::_ServiceTunnelModeToStr(ServiceTunnelMode mode)
{
    switch (mode)
    {
    case kServiceTunnelMode_NotSupported:
        return "NotSupported";
    case kServiceTunnelMode_Disabled:
        return "Disabled";
    case kServiceTunnelMode_Enabled:
        return "Enabled";
    default:
        return "(unknown)";
    }
}

const char * ConnectivityManagerImpl::WiFiStationStateToStr(WiFiStationState state)
{
    switch (state)
    {
    case kWiFiStationState_NotConnected:
        return "NotConnected";
    case kWiFiStationState_Connecting:
        return "Connecting";
    case kWiFiStationState_Connecting_Succeeded:
        return "Connecting_Succeeded";
    case kWiFiStationState_Connecting_Failed:
        return "Connecting_Failed";
    case kWiFiStationState_Connected:
        return "Connected";
    case kWiFiStationState_Disconnecting:
        return "Disconnecting";
    default:
        return "(unknown)";
    }
}

const char * ConnectivityManagerImpl::WiFiAPStateToStr(WiFiAPState state)
{
    switch (state)
    {
    case kWiFiAPState_NotActive:
        return "NotActive";
    case kWiFiAPState_Activating:
        return "Activating";
    case kWiFiAPState_Active:
        return "Active";
    case kWiFiAPState_Deactivating:
        return "Deactivating";
    default:
        return "(unknown)";
    }
}

void ConnectivityManagerImpl::RefreshMessageLayer(void) {}

} // namespace DeviceLayer
} // namespace chip
