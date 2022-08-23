/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/internal/BLEManager.h>

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using chip::DeviceLayer::Internal::ESP32Utils;

namespace chip {
namespace DeviceLayer {

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

        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
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
        esp_wifi_set_config(WIFI_IF_STA, &stationConfig);

        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;

    DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);

exit:
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::Clock::kZero;
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        if (mWiFiAPState == kWiFiAPState_Activating || mWiFiAPState == kWiFiAPState_Active)
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
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

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

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWiFiStatsCounters(void)
{
    esp_err_t err;
    wifi_config_t wifiConfig;
    uint8_t primaryChannel;
    wifi_second_chan_t secondChannel;
    uint16_t freq;
    uint16_t bssid;

    IgnoreUnusedVariable(freq);
    IgnoreUnusedVariable(bssid);
    err = esp_wifi_get_config(WIFI_IF_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_config() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    err = esp_wifi_get_channel(&primaryChannel, &secondChannel);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_channel() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    freq = MapFrequency(WIFI_BAND_2_4GHZ, primaryChannel);
    static_assert(std::is_same<std::remove_reference<decltype(wifiConfig.sta.bssid[5])>::type, uint8_t>::value,
                  "Our bits are going to start overlapping");
    bssid = static_cast<uint16_t>((wifiConfig.sta.bssid[4] << 8) | wifiConfig.sta.bssid[5]);
    ChipLogProgress(DeviceLayer,
                    "WiFi-Telemetry\n"
                    "BSSID: %x\n"
                    "freq: %d\n",
                    bssid, freq);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::InitWiFi()
{
    mLastStationConnectFailTime   = System::Clock::kZero;
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    mLastAPDemandTime  = System::Clock::kZero;
    mWiFiAPMode        = kWiFiAPMode_Disabled;
    mWiFiAPState       = kWiFiAPState_NotActive;
    mWiFiAPIdleTimeout = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

    mFlags.SetRaw(0);

    // TODO Initialize the Chip Addressing and Routing Module.

    // Ensure that ESP station mode is enabled.
    ReturnErrorOnFailure(Internal::ESP32Utils::EnableStationMode());

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
            memcpy(wifiConfig.sta.ssid, CONFIG_DEFAULT_WIFI_SSID,
                   std::min(sizeof(wifiConfig.sta.ssid), strlen(CONFIG_DEFAULT_WIFI_SSID)));
            memcpy(wifiConfig.sta.password, CONFIG_DEFAULT_WIFI_PASSWORD,
                   std::min(sizeof(wifiConfig.sta.password), strlen(CONFIG_DEFAULT_WIFI_PASSWORD)));
            wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
            wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
            esp_err_t err              = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
            if (err != ESP_OK)
            {
                ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", esp_err_to_name(err));
            }

            // Enable WiFi station mode.
            ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
        }

        // Otherwise, ensure WiFi station mode is disabled.
        else
        {
            ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Disabled));
        }
    }

    // Force AP mode off for now.
    ReturnErrorOnFailure(Internal::ESP32Utils::SetAPMode(false));

    // Queue work items to bootstrap the AP and station state machines once the Chip event loop is running.
    ReturnErrorOnFailure(DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL));

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    ReturnErrorOnFailure(DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL));
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::OnWiFiPlatformEvent(const ChipDeviceEvent * event)
{
    // Handle ESP system events...
    if (event->Type == DeviceEventType::kESPSystemEvent)
    {
        if (event->Platform.ESPSystemEvent.Base == WIFI_EVENT)
        {
            switch (event->Platform.ESPSystemEvent.Id)
            {
            case WIFI_EVENT_SCAN_DONE:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_SCAN_DONE");
                NetworkCommissioning::ESPWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
                break;
            case WIFI_EVENT_STA_START:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_START");
                DriveStationState();
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_CONNECTED");
                if (mWiFiStationState == kWiFiStationState_Connecting)
                {
                    ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
                }
                DriveStationState();
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_DISCONNECTED");
                NetworkCommissioning::ESPWiFiDriver::GetInstance().SetLastDisconnectReason(event);
                if (mWiFiStationState == kWiFiStationState_Connecting)
                {
                    ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
                }
                DriveStationState();
                break;
            case WIFI_EVENT_STA_STOP:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_STOP");
                DriveStationState();
                break;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
            case WIFI_EVENT_AP_START:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_AP_START");
                ChangeWiFiAPState(kWiFiAPState_Active);
                DriveAPState();
                break;
            case WIFI_EVENT_AP_STOP:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_AP_STOP");
                ChangeWiFiAPState(kWiFiAPState_NotActive);
                DriveAPState();
                break;
            case WIFI_EVENT_AP_STACONNECTED:
                ChipLogProgress(DeviceLayer, "WIFI_EVENT_AP_STACONNECTED");
                MaintainOnDemandWiFiAP();
                break;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
            default:
                break;
            }
        }

        if (event->Platform.ESPSystemEvent.Base == IP_EVENT)
        {
            switch (event->Platform.ESPSystemEvent.Id)
            {
            case IP_EVENT_STA_GOT_IP:
                ChipLogProgress(DeviceLayer, "IP_EVENT_STA_GOT_IP");
                OnStationIPv4AddressAvailable(event->Platform.ESPSystemEvent.Data.IpGotIp);
                break;
            case IP_EVENT_STA_LOST_IP:
                ChipLogProgress(DeviceLayer, "IP_EVENT_STA_LOST_IP");
                OnStationIPv4AddressLost();
                break;
            case IP_EVENT_GOT_IP6:
                ChipLogProgress(DeviceLayer, "IP_EVENT_GOT_IP6");
                OnIPv6AddressAvailable(event->Platform.ESPSystemEvent.Data.IpGotIp6);
                break;
            default:
                break;
            }
        }
    }
}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    // Schedule a call to DriveStationState method in case a station connect attempt was
    // deferred because the scan was in progress.
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    // Schedule a call to the DriveStationState method to adjust the station state as needed.
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::DriveStationState()
{
    bool stationConnected;

    // Refresh the current station mode.  Specifically, this reads the ESP auto_connect flag,
    // which determine whether the WiFi station mode is kWiFiStationMode_Enabled or
    // kWiFiStationMode_Disabled.
    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        // Ensure that the ESP WiFi layer is started.
        ReturnOnFailure(Internal::ESP32Utils::StartWiFiLayer());

        // Ensure that station mode is enabled in the ESP WiFi layer.
        ReturnOnFailure(Internal::ESP32Utils::EnableStationMode());
    }

    // Determine if the ESP WiFi layer thinks the station interface is currently connected.
    ReturnOnFailure(Internal::ESP32Utils::IsStationConnected(stationConnected));

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
            esp_err_t err = esp_wifi_disconnect();
            if (err != ESP_OK)
            {
                ChipLogError(DeviceLayer, "esp_wifi_disconnect() failed: %s", esp_err_to_name(err));
                return;
            }

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
                ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
                esp_err_t err = esp_wifi_connect();
                if (err != ESP_OK)
                {
                    ChipLogError(DeviceLayer, "esp_wifi_connect() failed: %s", esp_err_to_name(err));
                    return;
                }

                ChangeWiFiStationState(kWiFiStationState_Connecting);
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                System::Clock::Timeout timeToNextConnect = (mLastStationConnectFailTime + mWiFiStationReconnectInterval) - now;

                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms",
                                System::Clock::Milliseconds32(timeToNextConnect).count());

                ReturnOnFailure(DeviceLayer::SystemLayer().StartTimer(timeToNextConnect, DriveStationState, NULL));
            }
        }
    }

    ChipLogProgress(DeviceLayer, "Done driving station state, nothing else to do...");
    // Kick-off any pending network scan that might have been deferred due to the activity
    // of the WiFi station.
}

void ConnectivityManagerImpl::OnStationConnected()
{
    // Assign an IPv6 link local address to the station interface.
    esp_err_t err = esp_netif_create_ip6_linklocal(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_netif_create_ip6_linklocal() failed for WIFI_STA_DEF interface: %s", esp_err_to_name(err));
    }
    NetworkCommissioning::ESPWiFiDriver::GetInstance().OnConnectWiFiNetwork();
    // TODO Invoke WARM to perform actions that occur when the WiFi station interface comes up.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    PlatformMgr().PostEventOrDie(&event);
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();

    if (delegate)
    {
        delegate->OnConnectionStatusChanged(
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus::kConnected));
    }

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationDisconnected()
{
    // TODO Invoke WARM to perform actions that occur when the WiFi station interface goes down.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    PlatformMgr().PostEventOrDie(&event);
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();
    uint16_t reason                    = NetworkCommissioning::ESPWiFiDriver::GetInstance().GetLastDisconnectReason();
    uint8_t associationFailureCause =
        chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kUnknown);

    switch (reason)
    {
    case WIFI_REASON_ASSOC_TOOMANY:
    case WIFI_REASON_NOT_ASSOCED:
    case WIFI_REASON_ASSOC_NOT_AUTHED:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_GROUP_CIPHER_INVALID:
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
    case WIFI_REASON_AKMP_INVALID:
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
        associationFailureCause =
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kAssociationFailed);
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
        break;
    case WIFI_REASON_NOT_AUTHED:
    case WIFI_REASON_MIC_FAILURE:
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
    case WIFI_REASON_INVALID_RSN_IE_CAP:
    case WIFI_REASON_INVALID_PMKID:
    case WIFI_REASON_802_1X_AUTH_FAILED:
        associationFailureCause =
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kAuthenticationFailed);
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
        break;
    case WIFI_REASON_NO_AP_FOUND:
        associationFailureCause =
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kSsidNotFound);
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
    case WIFI_REASON_BEACON_TIMEOUT:
    case WIFI_REASON_AUTH_EXPIRE:
    case WIFI_REASON_AUTH_LEAVE:
    case WIFI_REASON_ASSOC_LEAVE:
    case WIFI_REASON_ASSOC_EXPIRE:
        break;

    default:
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
        break;
    }

    if (delegate)
    {
        delegate->OnDisconnectionDetected(reason);
        delegate->OnConnectionStatusChanged(
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus::kNotConnected));
    }

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::ESPWiFiDriver::GetInstance().OnNetworkStatusChange(); });
    }
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveStationState();
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;
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
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

            if (mLastAPDemandTime != System::Clock::kZero && now < (mLastAPDemandTime + mWiFiAPIdleTimeout))
            {
                targetState = kWiFiAPState_Active;

                // Compute the amount of idle time before the AP should be deactivated and
                // arm a timer to fire at that time.
                System::Clock::Timeout apTimeout = (mLastAPDemandTime + mWiFiAPIdleTimeout) - now;
                err                              = DeviceLayer::SystemLayer().StartTimer(apTimeout, DriveAPState, NULL);
                SuccessOrExit(err);
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " ms",
                                System::Clock::Milliseconds32(apTimeout).count());
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
    if (mWiFiAPState == kWiFiAPState_Active && Internal::ESP32Utils::IsInterfaceUp("WIFI_AP_DEF") &&
        !Internal::ESP32Utils::HasIPv6LinkLocalAddress("WIFI_AP_DEF"))
    {
        esp_err_t error = esp_netif_create_ip6_linklocal(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"));
        if (error != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_netif_create_ip6_linklocal() failed for WIFI_AP_DEF interface: %s",
                         esp_err_to_name(error));
            goto exit;
        }
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
    wifi_config_t wifiConfig;

    memset(&wifiConfig, 0, sizeof(wifiConfig));

    uint16_t discriminator;
    ReturnErrorOnFailure(GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

    uint16_t vendorId;
    uint16_t productId;
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetVendorId(vendorId));
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetProductId(productId));

    snprintf((char *) wifiConfig.ap.ssid, sizeof(wifiConfig.ap.ssid), "%s%03X-%04X-%04X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX,
             discriminator, vendorId, productId);
    wifiConfig.ap.channel         = CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL;
    wifiConfig.ap.authmode        = WIFI_AUTH_OPEN;
    wifiConfig.ap.max_connection  = CHIP_DEVICE_CONFIG_WIFI_AP_MAX_STATIONS;
    wifiConfig.ap.beacon_interval = CHIP_DEVICE_CONFIG_WIFI_AP_BEACON_INTERVAL;
    ChipLogProgress(DeviceLayer, "Configuring WiFi AP: SSID %s, channel %u", wifiConfig.ap.ssid, wifiConfig.ap.channel);
    esp_err_t err = esp_wifi_set_config(WIFI_IF_AP, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config(WIFI_IF_AP) failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    return CHIP_NO_ERROR;
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
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn      = false;
    bool haveIPv6Conn      = false;
    const bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    const bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    IPAddress addr;

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

                    esp_netif_ip_info_t ipInfo;
                    if (esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ipInfo) == ESP_OK)
                    {
                        char addrStr[INET_ADDRSTRLEN];
                        // ToDo: change the code to using IPv6 address
                        esp_ip4addr_ntoa(&ipInfo.ip, addrStr, sizeof(addrStr));
                        IPAddress::FromString(addrStr, addr);
                    }
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
        mFlags.Set(ConnectivityFlags::kHaveIPv4InternetConnectivity, haveIPv4Conn)
            .Set(ConnectivityFlags::kHaveIPv6InternetConnectivity, haveIPv6Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
        event.InternetConnectivityChange.IPv6      = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);
        event.InternetConnectivityChange.ipAddress = addr;

        PlatformMgr().PostEventOrDie(&event);

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

void ConnectivityManagerImpl::OnStationIPv4AddressAvailable(const ip_event_got_ip_t & got_ip)
{
#if CHIP_PROGRESS_LOGGING
    {
        ChipLogProgress(DeviceLayer, "IPv4 address %s on WiFi station interface: " IPSTR "/" IPSTR " gateway " IPSTR,
                        (got_ip.ip_changed) ? "changed" : "ready", IP2STR(&got_ip.ip_info.ip), IP2STR(&got_ip.ip_info.netmask),
                        IP2STR(&got_ip.ip_info.gw));
    }
#endif // CHIP_PROGRESS_LOGGING

    UpdateInternetConnectivityState();

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnStationIPv4AddressLost(void)
{
    ChipLogProgress(DeviceLayer, "IPv4 address lost on WiFi station interface");

    UpdateInternetConnectivityState();

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Lost;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnIPv6AddressAvailable(const ip_event_got_ip6_t & got_ip)
{
#if CHIP_PROGRESS_LOGGING
    {
        ChipLogProgress(DeviceLayer, "IPv6 addr available. Ready on %s interface: " IPV6STR, esp_netif_get_ifkey(got_ip.esp_netif),
                        IPV62STR(got_ip.ip6_info.ip));
    }
#endif // CHIP_PROGRESS_LOGGING

    UpdateInternetConnectivityState();

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV6_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
