/*
 *
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

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ConnectivityManager.h>
#include <Weave/DeviceLayer/internal/NetworkProvisioningServer.h>
#include <Weave/DeviceLayer/internal/DeviceNetworkInfo.h>
#include <Weave/DeviceLayer/internal/ServiceTunnelAgent.h>
#include <Weave/DeviceLayer/internal/BLEManager.h>
#include <Weave/Profiles/WeaveProfiles.h>
#include <Weave/Profiles/common/CommonProfile.h>
#include <Warm/Warm.h>

#include <Weave/DeviceLayer/ESP32/ESP32Utils.h>

#include "esp_event.h"
#include "esp_wifi.h"

#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/nd6.h>
#include <lwip/dns.h>

#include <new>

#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if !WEAVE_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for ESP32"
#endif

#if !WEAVE_DEVICE_CONFIG_ENABLE_WIFI_AP
#error "WiFi AP support must be enabled when building for ESP32"
#endif

#if WEAVE_DEVICE_CONFIG_ENABLE_WIFI_TELEMETRY
#include <Weave/Support/TraitEventUtils.h>
#include <nest/trait/network/TelemetryNetworkTrait.h>
#include <nest/trait/network/TelemetryNetworkWifiTrait.h>
#endif

using namespace ::nl;
using namespace ::nl::Weave;
using namespace ::nl::Weave::TLV;
using namespace ::nl::Weave::Profiles::Common;
using namespace ::nl::Weave::Profiles::NetworkProvisioning;
using namespace ::nl::Weave::Profiles::WeaveTunnel;
using namespace ::nl::Weave::DeviceLayer::Internal;

using Profiles::kWeaveProfile_Common;
using Profiles::kWeaveProfile_NetworkProvisioning;

namespace nl {
namespace Weave {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        bool autoConnect;
        mWiFiStationMode = (esp_wifi_get_auto_connect(&autoConnect) == ESP_OK && autoConnect)
                ? kWiFiStationMode_Enabled
                : kWiFiStationMode_Disabled;
    }
    return mWiFiStationMode;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

WEAVE_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = WEAVE_ERROR_INVALID_ARGUMENT);

    if (val != kWiFiStationMode_ApplicationControlled)
    {
        bool autoConnect = (val == kWiFiStationMode_Enabled);
        err = esp_wifi_set_auto_connect(autoConnect);
        SuccessOrExit(err);

        SystemLayer.ScheduleWork(DriveStationState, NULL);
    }

    if (mWiFiStationMode != val)
    {
        WeaveLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode), WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return ESP32Utils::IsStationProvisioned();
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

WEAVE_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = WEAVE_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        WeaveLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;

    SystemLayer.ScheduleWork(DriveAPState, NULL);

exit:
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand ||
        mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::Layer::GetClock_MonotonicMS();
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand ||
        mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = 0;
        SystemLayer.ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand ||
        mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
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

    if (inChannel >= 1 && inChannel <= 13) {
        frequency = 2412 + ((inChannel - 1) * 5);

    } else if (inChannel == 14) {
        frequency = 2484;

    }

    return frequency;
}

static uint16_t Map5000MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    switch (inChannel) {

    case 183: frequency = 4915; break;
    case 184: frequency = 4920; break;
    case 185: frequency = 4925; break;
    case 187: frequency = 4935; break;
    case 188: frequency = 4940; break;
    case 189: frequency = 4945; break;
    case 192: frequency = 4960; break;
    case 196: frequency = 4980; break;
    case 7:   frequency = 5035; break;
    case 8:   frequency = 5040; break;
    case 9:   frequency = 5045; break;
    case 11:  frequency = 5055; break;
    case 12:  frequency = 5060; break;
    case 16:  frequency = 5080; break;
    case 34:  frequency = 5170; break;
    case 36:  frequency = 5180; break;
    case 38:  frequency = 5190; break;
    case 40:  frequency = 5200; break;
    case 42:  frequency = 5210; break;
    case 44:  frequency = 5220; break;
    case 46:  frequency = 5230; break;
    case 48:  frequency = 5240; break;
    case 52:  frequency = 5260; break;
    case 56:  frequency = 5280; break;
    case 60:  frequency = 5300; break;
    case 64:  frequency = 5320; break;
    case 100: frequency = 5500; break;
    case 104: frequency = 5520; break;
    case 108: frequency = 5540; break;
    case 112: frequency = 5560; break;
    case 116: frequency = 5580; break;
    case 120: frequency = 5600; break;
    case 124: frequency = 5620; break;
    case 128: frequency = 5640; break;
    case 132: frequency = 5660; break;
    case 136: frequency = 5680; break;
    case 140: frequency = 5700; break;
    case 149: frequency = 5745; break;
    case 153: frequency = 5765; break;
    case 157: frequency = 5785; break;
    case 161: frequency = 5805; break;
    case 165: frequency = 5825; break;

    }

    return frequency;
}

static uint16_t MapFrequency(const uint16_t inBand, const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inBand == WIFI_BAND_2_4GHZ) {
        frequency = Map2400MHz(inChannel);

    } else if (inBand == WIFI_BAND_5_0GHZ) {
        frequency = Map5000MHz(inChannel);

    }

    return frequency;
}

WEAVE_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    WEAVE_ERROR err;
    nl::Weave::Profiles::DataManagement_Current::event_id_t eventId;
    Schema::Nest::Trait::Network::TelemetryNetworkWifiTrait::NetworkWiFiStatsEvent statsEvent;
    wifi_config_t wifiConfig;
    uint8_t primaryChannel;
    wifi_second_chan_t secondChannel;

    VerifyOrExit(_IsWiFiStationConnected() && _IsWiFiStationConnected(), err = WEAVE_NO_ERROR);

    err = esp_wifi_get_config(ESP_IF_WIFI_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        WeaveLogError(DeviceLayer, "esp_wifi_get_config() failed: %s", nl::ErrorStr(err));
    }
    SuccessOrExit(err);

    err = esp_wifi_get_channel(&primaryChannel, &secondChannel);
    if (err != ESP_OK)
    {
        WeaveLogError(DeviceLayer, "esp_wifi_get_channel() failed: %s", nl::ErrorStr(err));
    }
    SuccessOrExit(err);

    statsEvent.bssid            = (wifiConfig.sta.bssid[4] << 8) | wifiConfig.sta.bssid[5];
    statsEvent.freq             = MapFrequency(WIFI_BAND_2_4GHZ, primaryChannel);
    statsEvent.rssi             = 0;
    statsEvent.bcnRecvd         = 0;
    statsEvent.bcnLost          = 0;
    statsEvent.pktMcastRx       = 0;
    statsEvent.pktUcastRx       = 0;
    statsEvent.currTxRate       = 0;
    statsEvent.currRxRate       = 0;
    statsEvent.sleepTimePercent = 0;
    statsEvent.numOfAp          = 0;

    WeaveLogProgress(DeviceLayer,
                     "WiFi-Telemtry\n"
                     "BSSID:         %x\n"
                     "freq:          %d\n"
                     "rssi:          %d\n"
                     "bcn recvd:     %d\n"
                     "bcn lost:      %d\n"
                     "mcast:         %d\n"
                     "ucast:         %d\n"
                     "rx rate:       %d\n"
                     "tx rate:       %d\n"
                     "sleep percent: %d\n"
                     "Num of AP:     %d\n",
                     statsEvent.bssid, statsEvent.freq, statsEvent.rssi, statsEvent.bcnRecvd, statsEvent.bcnLost,
                     statsEvent.pktMcastRx, statsEvent.pktUcastRx, statsEvent.currRxRate, statsEvent.currTxRate,
                     statsEvent.sleepTimePercent, statsEvent.numOfAp);

    eventId = nl::LogEvent(&statsEvent);
    WeaveLogProgress(DeviceLayer, "WiFi Telemetry Stats Event Id: %u\n", eventId);

exit:
    return err;
}

WEAVE_ERROR ConnectivityManagerImpl::_SetServiceTunnelMode(ServiceTunnelMode val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(val != kServiceTunnelMode_NotSupported, err = WEAVE_ERROR_INVALID_ARGUMENT);

    mServiceTunnelMode = val;

    SystemLayer.ScheduleWork(DriveServiceTunnelState, NULL);

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsServiceTunnelConnected(void)
{
    WeaveTunnelAgent::AgentState tunnelState = ServiceTunnelAgent.GetWeaveTunnelAgentState();
    return (tunnelState == WeaveTunnelAgent::kState_PrimaryTunModeEstablished ||
            tunnelState == WeaveTunnelAgent::kState_PrimaryAndBkupTunModeEstablished ||
            tunnelState == WeaveTunnelAgent::kState_BkupOnlyTunModeEstablished);
}

bool ConnectivityManagerImpl::_IsServiceTunnelRestricted(void)
{
    return ServiceTunnelAgent.IsTunnelRoutingRestricted();
}

bool ConnectivityManagerImpl::_HaveServiceConnectivityViaTunnel(void)
{
    return IsServiceTunnelConnected() && !IsServiceTunnelRestricted();
}


// ==================== ConnectivityManager Platform Internal Methods ====================

WEAVE_ERROR ConnectivityManagerImpl::_Init()
{
    WEAVE_ERROR err;

    mLastStationConnectFailTime = 0;
    mLastAPDemandTime = 0;
    mWiFiStationMode = kWiFiStationMode_Disabled;
    mWiFiStationState = kWiFiStationState_NotConnected;
    mWiFiAPMode = kWiFiAPMode_Disabled;
    mWiFiAPState = kWiFiAPState_NotActive;
    mServiceTunnelMode = kServiceTunnelMode_Enabled;
    mWiFiStationReconnectIntervalMS = WEAVE_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL;
    mWiFiAPIdleTimeoutMS = WEAVE_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT;
    mFlags = 0;

    // Initialize the Weave Addressing and Routing Module.
    err = Warm::Init(FabricState);
    SuccessOrExit(err);

    // Initialize the service tunnel agent.
    err = InitServiceTunnelAgent();
    SuccessOrExit(err);
    ServiceTunnelAgent.OnServiceTunStatusNotify = HandleServiceTunnelNotification;

    // Ensure that ESP station mode is enabled.
    err = ESP32Utils::EnableStationMode();
    SuccessOrExit(err);

    // If there is no persistent station provision...
    if (!IsWiFiStationProvisioned())
    {
        // If the code has been compiled with a default WiFi station provision, configure that now.
        if (CONFIG_DEFAULT_WIFI_SSID[0] != 0)
        {
            WeaveLogProgress(DeviceLayer, "Setting default WiFi station configuration (SSID: %s)", CONFIG_DEFAULT_WIFI_SSID);

            // Set a default station configuration.
            wifi_config_t wifiConfig;
            memset(&wifiConfig, 0, sizeof(wifiConfig));
            memcpy(wifiConfig.sta.ssid, CONFIG_DEFAULT_WIFI_SSID, strlen(CONFIG_DEFAULT_WIFI_SSID) + 1);
            memcpy(wifiConfig.sta.password, CONFIG_DEFAULT_WIFI_PASSWORD, strlen(CONFIG_DEFAULT_WIFI_PASSWORD) + 1);
            wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
            wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
            err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig);
            if (err != ESP_OK)
            {
                WeaveLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", nl::ErrorStr(err));
            }
            err = WEAVE_NO_ERROR;

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
    err = ESP32Utils::SetAPMode(false);
    SuccessOrExit(err);

    // Queue work items to bootstrap the AP and station state machines once the Weave event loop is running.
    err = SystemLayer.ScheduleWork(DriveStationState, NULL);
    SuccessOrExit(err);
    err = SystemLayer.ScheduleWork(DriveAPState, NULL);
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const WeaveDeviceEvent * event)
{
    // Handle ESP system events...
    if (event->Type == DeviceEventType::kESPSystemEvent)
    {
        switch(event->Platform.ESPSystemEvent.event_id) {
        case SYSTEM_EVENT_STA_START:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_START");
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_CONNECTED");
            if (mWiFiStationState == kWiFiStationState_Connecting)
            {
                ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
            }
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_DISCONNECTED");
            if (mWiFiStationState == kWiFiStationState_Connecting)
            {
                ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
            }
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_STOP:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_STOP");
            DriveStationState();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_GOT_IP");
            OnStationIPv4AddressAvailable(event->Platform.ESPSystemEvent.event_info.got_ip);
            break;
        case SYSTEM_EVENT_STA_LOST_IP:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_LOST_IP");
            OnStationIPv4AddressLost();
            break;
        case SYSTEM_EVENT_GOT_IP6:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_GOT_IP6");
            OnIPv6AddressAvailable(event->Platform.ESPSystemEvent.event_info.got_ip6);
            break;
        case SYSTEM_EVENT_AP_START:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_AP_START");
            ChangeWiFiAPState(kWiFiAPState_Active);
            DriveAPState();
            break;
        case SYSTEM_EVENT_AP_STOP:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_AP_STOP");
            ChangeWiFiAPState(kWiFiAPState_NotActive);
            DriveAPState();
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            WeaveLogProgress(DeviceLayer, "SYSTEM_EVENT_AP_STACONNECTED");
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

#if !WEAVE_DEVICE_CONFIG_DISABLE_ACCOUNT_PAIRING

    // Handle account pairing changes.
    else if (event->Type == DeviceEventType::kAccountPairingChange)
    {
        // When account pairing successfully completes, if the tunnel to the
        // service is subject to routing restrictions (imposed because at the time
        // the tunnel was established the device was not paired to an account)
        // then force the tunnel to close.  This will result in the tunnel being
        // re-established, which should lift the service-side restrictions.
        if (event->AccountPairingChange.IsPairedToAccount &&
            GetFlag(mFlags, kFlag_ServiceTunnelStarted) &&
            ServiceTunnelAgent.IsTunnelRoutingRestricted())
        {
            WeaveLogProgress(DeviceLayer, "Restarting service tunnel to lift routing restrictions");
            ClearFlag(mFlags, kFlag_ServiceTunnelStarted);
            ServiceTunnelAgent.StopServiceTunnel(WEAVE_ERROR_TUNNEL_FORCE_ABORT);
            DriveServiceTunnelState();
        }
    }

#endif // !WEAVE_DEVICE_CONFIG_DISABLE_ACCOUNT_PAIRING
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
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    bool stationConnected;

    // Refresh the current station mode.  Specifically, this reads the ESP auto_connect flag,
    // which determine whether the WiFi station mode is kWiFiStationMode_Enabled or
    // kWiFiStationMode_Disabled.
    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        // Ensure that the ESP WiFi layer is started.
        err = ESP32Utils::StartWiFiLayer();
        SuccessOrExit(err);

        // Ensure that station mode is enabled in the ESP WiFi layer.
        err = ESP32Utils::EnableStationMode();
        SuccessOrExit(err);
    }

    // Determine if the ESP WiFi layer thinks the station interface is currently connected.
    err = ESP32Utils::IsStationConnected(stationConnected);
    SuccessOrExit(err);

    // If the station interface is currently connected ...
    if (stationConnected)
    {
        // Advance the station state to Connected if it was previously NotConnected or
        // a previously initiated connect attempt succeeded.
        if (mWiFiStationState == kWiFiStationState_NotConnected ||
            mWiFiStationState == kWiFiStationState_Connecting_Succeeded)
        {
            ChangeWiFiStationState(kWiFiStationState_Connected);
            WeaveLogProgress(DeviceLayer, "WiFi station interface connected");
            mLastStationConnectFailTime = 0;
            OnStationConnected();
        }

        // If the WiFi station interface is no longer enabled, or no longer provisioned,
        // disconnect the station from the AP, unless the WiFi station mode is currently
        // under application control.
        if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled &&
            (mWiFiStationMode != kWiFiStationMode_Enabled || !IsWiFiStationProvisioned()))
        {
            WeaveLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
            err = esp_wifi_disconnect();
            if (err != ESP_OK)
            {
                WeaveLogError(DeviceLayer, "esp_wifi_disconnect() failed: %s", nl::ErrorStr(err));
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
        if (mWiFiStationState == kWiFiStationState_Connected ||
            mWiFiStationState == kWiFiStationState_Disconnecting ||
            mWiFiStationState == kWiFiStationState_Connecting_Failed)
        {
            WiFiStationState prevState = mWiFiStationState;
            ChangeWiFiStationState(kWiFiStationState_NotConnected);
            if (prevState != kWiFiStationState_Connecting_Failed)
            {
                WeaveLogProgress(DeviceLayer, "WiFi station interface disconnected");
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
        if (mWiFiStationMode == kWiFiStationMode_Enabled && IsWiFiStationProvisioned() && !NetworkProvisioningSvr().ScanInProgress())
        {
            // Initiate a connection to the AP if we haven't done so before, or if enough
            // time has passed since the last attempt.
            if (mLastStationConnectFailTime == 0 || now >= mLastStationConnectFailTime + mWiFiStationReconnectIntervalMS)
            {
                WeaveLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
                err = esp_wifi_connect();
                if (err != ESP_OK)
                {
                    WeaveLogError(DeviceLayer, "esp_wifi_connect() failed: %s", nl::ErrorStr(err));
                }
                SuccessOrExit(err);

                ChangeWiFiStationState(kWiFiStationState_Connecting);
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                uint32_t timeToNextConnect = (uint32_t)((mLastStationConnectFailTime + mWiFiStationReconnectIntervalMS) - now);

                WeaveLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms", timeToNextConnect);

                err = SystemLayer.StartTimer(timeToNextConnect, DriveStationState, NULL);
                SuccessOrExit(err);
            }
        }
    }

exit:

    // Kick-off any pending network scan that might have been deferred due to the activity
    // of the WiFi station.
    NetworkProvisioningSvr().StartPendingScan();
}

void ConnectivityManagerImpl::OnStationConnected()
{
    WEAVE_ERROR err;

    // Assign an IPv6 link local address to the station interface.
    err = tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
    if (err != ESP_OK)
    {
        WeaveLogError(DeviceLayer, "tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA) failed: %s", nl::ErrorStr(err));
    }

    // Invoke WARM to perform actions that occur when the WiFi station interface comes up.
    Warm::WiFiInterfaceStateChange(Warm::kInterfaceStateUp);

    // Alert other components of the new state.
    WeaveDeviceEvent event;
    event.Type = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationDisconnected()
{
    // Invoke WARM to perform actions that occur when the WiFi station interface goes down.
    Warm::WiFiInterfaceStateChange(Warm::kInterfaceStateDown);

    // Alert other components of the new state.
    WeaveDeviceEvent event;
    event.Type = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        WeaveLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState), WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
    }
}

void ConnectivityManagerImpl::DriveStationState(nl::Weave::System::Layer * aLayer, void * aAppState, nl::Weave::System::Error aError)
{
    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::DriveAPState()
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    WiFiAPState targetState;
    uint64_t now;
    uint32_t apTimeout;
    bool espAPModeEnabled;

    // Determine if AP mode is currently enabled in the ESP WiFi layer.
    err = ESP32Utils::IsAPEnabled(espAPModeEnabled);
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
        err = ESP32Utils::StartWiFiLayer();
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
        else if (mWiFiAPMode == kWiFiAPMode_OnDemand ||
                 mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
        {
            now = System::Layer::GetClock_MonotonicMS();

            if (mLastAPDemandTime != 0 && now < (mLastAPDemandTime + mWiFiAPIdleTimeoutMS))
            {
                targetState = kWiFiAPState_Active;

                // Compute the amount of idle time before the AP should be deactivated and
                // arm a timer to fire at that time.
                apTimeout = (uint32_t)((mLastAPDemandTime + mWiFiAPIdleTimeoutMS) - now);
                err = SystemLayer.StartTimer(apTimeout, DriveAPState, NULL);
                SuccessOrExit(err);
                WeaveLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " ms", apTimeout);
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
                    err = ESP32Utils::SetAPMode(true);
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
                    err = ESP32Utils::SetAPMode(false);
                    SuccessOrExit(err);

                    ChangeWiFiAPState(kWiFiAPState_Deactivating);
                }
            }
        }
    }

    // If AP is active, but the interface doesn't have an IPv6 link-local
    // address, assign one now.
    if (mWiFiAPState == kWiFiAPState_Active &&
        ESP32Utils::IsInterfaceUp(TCPIP_ADAPTER_IF_AP) &&
        !ESP32Utils::HasIPv6LinkLocalAddress(TCPIP_ADAPTER_IF_AP))
    {
        err = tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_AP);
        if (err != ESP_OK)
        {
            WeaveLogError(DeviceLayer, "tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_AP) failed: %s", nl::ErrorStr(err));
        }
        SuccessOrExit(err);
    }

exit:
    if (err != WEAVE_NO_ERROR && mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        SetWiFiAPMode(kWiFiAPMode_Disabled);
        ESP32Utils::SetAPMode(false);
    }
}

WEAVE_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    wifi_config_t wifiConfig;

    memset(&wifiConfig, 0, sizeof(wifiConfig));
    err = ConfigurationMgr().GetWiFiAPSSID((char *)wifiConfig.ap.ssid, sizeof(wifiConfig.ap.ssid));
    SuccessOrExit(err);
    wifiConfig.ap.channel = WEAVE_DEVICE_CONFIG_WIFI_AP_CHANNEL;
    wifiConfig.ap.authmode = WIFI_AUTH_OPEN;
    wifiConfig.ap.max_connection = WEAVE_DEVICE_CONFIG_WIFI_AP_MAX_STATIONS;
    wifiConfig.ap.beacon_interval = WEAVE_DEVICE_CONFIG_WIFI_AP_BEACON_INTERVAL;
    WeaveLogProgress(DeviceLayer, "Configuring WiFi AP: SSID %s, channel %u", wifiConfig.ap.ssid, wifiConfig.ap.channel);
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifiConfig);
    if (err != ESP_OK)
    {
        WeaveLogError(DeviceLayer, "esp_wifi_set_config(ESP_IF_WIFI_AP) failed: %s", nl::ErrorStr(err));
    }
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    if (mWiFiAPState != newState)
    {
        WeaveLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", WiFiAPStateToStr(mWiFiAPState), WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}

void ConnectivityManagerImpl::DriveAPState(nl::Weave::System::Layer * aLayer, void * aAppState, nl::Weave::System::Error aError)
{
    sInstance.DriveAPState();
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn = false;
    bool haveIPv6Conn = false;
    bool hadIPv4Conn = GetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity);
    bool hadIPv6Conn = GetFlag(mFlags, kFlag_HaveIPv6InternetConnectivity);

    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        // Get the LwIP netif for the WiFi station interface.
        struct netif * netif = ESP32Utils::GetStationNetif();

        // If the WiFi station interface is up...
        if (netif != NULL && netif_is_up(netif) && netif_is_link_up(netif))
        {
            // Check if a DNS server is currently configured.  If so...
            ip_addr_t dnsServerAddr = dns_getserver(0);
            if (!ip_addr_isany_val(dnsServerAddr))
            {
                // If the station interface has been assigned an IPv4 address, and has
                // an IPv4 gateway, then presume that the device has IPv4 Internet
                // connectivity.
                if (!ip4_addr_isany_val(*netif_ip4_addr(netif)) &&
                    !ip4_addr_isany_val(*netif_ip4_gw(netif)))
                {
                    haveIPv4Conn = true;
                }

                // Search among the IPv6 addresses assigned to the interface for a Global Unicast
                // address (2000::/3) that is in the valid state.  If such an address is found...
                for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
                {
                    if (ip6_addr_isglobal(netif_ip6_addr(netif, i)) &&
                        ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
                    {
                        // Determine if there is a default IPv6 router that is currently reachable
                        // via the station interface.  If so, presume for now that the device has
                        // IPv6 connectivity.
                        if (nd6_select_router(IP6_ADDR_ANY6, netif) >= 0)
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
        WeaveDeviceEvent event;
        event.Type = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4 = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
        event.InternetConnectivityChange.IPv6 = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);
        PlatformMgr().PostEvent(&event);

        if (haveIPv4Conn != hadIPv4Conn)
        {
            WeaveLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv4", (haveIPv4Conn) ? "ESTABLISHED" : "LOST");
        }

        if (haveIPv6Conn != hadIPv6Conn)
        {
            WeaveLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv6", (haveIPv6Conn) ? "ESTABLISHED" : "LOST");
        }

        DriveServiceTunnelState();
    }
}

void ConnectivityManagerImpl::OnStationIPv4AddressAvailable(const system_event_sta_got_ip_t & got_ip)
{
#if WEAVE_PROGRESS_LOGGING
    {
        char ipAddrStr[INET_ADDRSTRLEN], netMaskStr[INET_ADDRSTRLEN], gatewayStr[INET_ADDRSTRLEN];
        IPAddress::FromIPv4(got_ip.ip_info.ip).ToString(ipAddrStr, sizeof(ipAddrStr));
        IPAddress::FromIPv4(got_ip.ip_info.netmask).ToString(netMaskStr, sizeof(netMaskStr));
        IPAddress::FromIPv4(got_ip.ip_info.gw).ToString(gatewayStr, sizeof(gatewayStr));
        WeaveLogProgress(DeviceLayer, "IPv4 address %s on WiFi station interface: %s/%s gateway %s",
                 (got_ip.ip_changed) ? "changed" : "ready",
                 ipAddrStr, netMaskStr, gatewayStr);
    }
#endif // WEAVE_PROGRESS_LOGGING

    RefreshMessageLayer();

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationIPv4AddressLost(void)
{
    WeaveLogProgress(DeviceLayer, "IPv4 address lost on WiFi station interface");

    RefreshMessageLayer();

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnIPv6AddressAvailable(const system_event_got_ip6_t & got_ip)
{
#if WEAVE_PROGRESS_LOGGING
    {
        IPAddress ipAddr = IPAddress::FromIPv6(got_ip.ip6_info.ip);
        char ipAddrStr[INET6_ADDRSTRLEN];
        ipAddr.ToString(ipAddrStr, sizeof(ipAddrStr));
        WeaveLogProgress(DeviceLayer, "%s ready on %s interface: %s",
                 CharacterizeIPv6Address(ipAddr),
                 ESP32Utils::InterfaceIdToName(got_ip.if_index),
                 ipAddrStr);
    }
#endif // WEAVE_PROGRESS_LOGGING

    RefreshMessageLayer();

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::DriveServiceTunnelState(void)
{
    WEAVE_ERROR err;
    bool startServiceTunnel;

    // Determine if the tunnel to the service should be started.
    startServiceTunnel = (mServiceTunnelMode == kServiceTunnelMode_Enabled
                          && GetFlag(mFlags, kFlag_HaveIPv4InternetConnectivity)
                          && ConfigurationMgr().IsMemberOfFabric()
#if !WEAVE_DEVICE_CONFIG_ENABLE_FIXED_TUNNEL_SERVER
                          && ConfigurationMgr().IsServiceProvisioned()
#endif
                         );

    // If the tunnel should be started but isn't, or vice versa, ...
    if (startServiceTunnel != GetFlag(mFlags, kFlag_ServiceTunnelStarted))
    {
        // Update the tunnel started state.
        SetFlag(mFlags, kFlag_ServiceTunnelStarted, startServiceTunnel);

        // Start or stop the tunnel as necessary.
        if (startServiceTunnel)
        {
            WeaveLogProgress(DeviceLayer, "Starting service tunnel");

            err = ServiceTunnelAgent.StartServiceTunnel();
            if (err != WEAVE_NO_ERROR)
            {
                WeaveLogError(DeviceLayer, "StartServiceTunnel() failed: %s", nl::ErrorStr(err));
                ClearFlag(mFlags, kFlag_ServiceTunnelStarted);
            }
        }

        else
        {
            WeaveLogProgress(DeviceLayer, "Stopping service tunnel");
            ServiceTunnelAgent.StopServiceTunnel();
        }
    }
}

void ConnectivityManagerImpl::DriveServiceTunnelState(nl::Weave::System::Layer * aLayer, void * aAppState, nl::Weave::System::Error aError)
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

void ConnectivityManagerImpl::RefreshMessageLayer(void)
{
    WEAVE_ERROR err = MessageLayer.RefreshEndpoints();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "MessageLayer.RefreshEndpoints() failed: %s", nl::ErrorStr(err));
    }
}

void ConnectivityManagerImpl::HandleServiceTunnelNotification(WeaveTunnelConnectionMgr::TunnelConnNotifyReasons reason,
            WEAVE_ERROR err, void *appCtxt)
{
    bool newTunnelState = false;
    bool prevTunnelState = GetFlag(sInstance.mFlags, kFlag_ServiceTunnelUp);
    bool isRestricted = false;

    switch (reason)
    {
    case WeaveTunnelConnectionMgr::kStatus_TunDown:
        WeaveLogProgress(DeviceLayer, "ConnectivityManager: Service tunnel down");
        break;
    case WeaveTunnelConnectionMgr::kStatus_TunPrimaryConnError:
        WeaveLogProgress(DeviceLayer, "ConnectivityManager: Service tunnel connection error: %s", ::nl::ErrorStr(err));
        break;
    case WeaveTunnelConnectionMgr::kStatus_TunPrimaryUp:
        newTunnelState = true;
        isRestricted = (err == WEAVE_ERROR_TUNNEL_ROUTING_RESTRICTED);
        WeaveLogProgress(DeviceLayer, "ConnectivityManager: %service tunnel established", (isRestricted) ? "RESTRICTED s" : "S");
        break;
    default:
        break;
    }

    // If the tunnel state has changed...
    if (newTunnelState != prevTunnelState)
    {
        // Update the cached copy of the state.
        SetFlag(sInstance.mFlags, kFlag_ServiceTunnelUp, newTunnelState);

        // Alert other components of the change to the tunnel state.
        WeaveDeviceEvent event;
        event.Clear();
        event.Type = DeviceEventType::kServiceTunnelStateChange;
        event.ServiceTunnelStateChange.Result = GetConnectivityChange(prevTunnelState, newTunnelState);
        event.ServiceTunnelStateChange.IsRestricted = isRestricted;
        PlatformMgr().PostEvent(&event);

        // If the new tunnel state represents a logical change in connectivity to the service, as it
        // relates to the application, post a ServiceConnectivityChange event.
        // (Note that the establishment of a restricted tunnel to the service does not constitute a
        // logical change in service connectivity from the application's standpoint, as such a tunnel
        // cannot be used for general application interactions, only pairing).
        if (!newTunnelState || !isRestricted)
        {
            event.Clear();
            event.Type = DeviceEventType::kServiceConnectivityChange;
            event.ServiceConnectivityChange.ViaTunnel.Result = (newTunnelState) ? kConnectivity_Established : kConnectivity_Lost;
            event.ServiceConnectivityChange.ViaThread.Result = kConnectivity_NoChange;
            event.ServiceConnectivityChange.Overall.Result = sInstance.HaveServiceConnectivityViaThread()
                    ? kConnectivity_NoChange
                    : event.ServiceConnectivityChange.ViaTunnel.Result;
            PlatformMgr().PostEvent(&event);
        }
    }
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
