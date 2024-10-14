/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#include <platform/Ameba/AmebaUtils.h>
#include <platform/Ameba/NetworkCommissioningDriver.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include <chip_porting.h>
#include <lwip_netconf.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

// ==================== ConnectivityManager Platform Internal Methods ====================

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    mLastStationConnectFailTime   = System::Clock::kZero;
    mLastAPDemandTime             = System::Clock::kZero;
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mWiFiAPMode                   = kWiFiAPMode_Disabled;
    mWiFiAPState                  = kWiFiAPState_NotActive;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mWiFiAPIdleTimeout            = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);
    mFlags.SetRaw(0);

    // Set callback functions from chip_porting
    chip_connmgr_set_callback_func((chip_connmgr_callback) (conn_callback_dispatcher), this);

    // Register WiFi event handlers
    matter_wifi_reg_event_handler(MATTER_WIFI_EVENT_CONNECT, ConnectivityManagerImpl::RtkWiFiStationConnectedHandler, NULL);
    matter_wifi_reg_event_handler(MATTER_WIFI_EVENT_FOURWAY_HANDSHAKE_DONE, ConnectivityManagerImpl::RtkWiFiStationConnectedHandler,
                                  NULL);
    matter_wifi_reg_event_handler(MATTER_WIFI_EVENT_DISCONNECT, ConnectivityManagerImpl::RtkWiFiStationDisconnectedHandler, NULL);
    matter_wifi_reg_event_handler(MATTER_WIFI_EVENT_DHCP6_DONE, ConnectivityManagerImpl::RtkWiFiDHCPCompletedHandler, NULL);

    err = Internal::AmebaUtils::StartWiFi();
    SuccessOrExit(err);
    bool stationIPLinked;
    err = Internal::AmebaUtils::IsStationIPLinked(stationIPLinked);

    if (stationIPLinked)
    {
        ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
    }
    else
    {
        ChangeWiFiStationState(kWiFiStationState_NotConnected);
    }

    // Force AP mode off for now.

    // Queue work items to bootstrap the AP and station state machines once the Chip event loop is running.
    ReturnErrorOnFailure(DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL));
    ReturnErrorOnFailure(DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL));

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (event->Type == DeviceEventType::kRtkWiFiStationConnectedEvent)
    {
        ChipLogProgress(DeviceLayer, "WiFiStationConnected");
        // Only do DHCP when connecting to wifi via matter provisioning
        // External wifi provisioning will do DHCP on their own
        if (mWiFiStationState == kWiFiStationState_Connecting)
        {
            DHCPProcess();
        }
        // Allow external wifi provisioning methods by allowing NotConnected states to advance to Connecting_succeed
        if ((mWiFiStationState == kWiFiStationState_Connecting) || (mWiFiStationState == kWiFiStationState_NotConnected))
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        }
        DriveStationState();
    }
    if (event->Type == DeviceEventType::kRtkWiFiStationDisconnectedEvent)
    {
        ChipLogProgress(DeviceLayer, "WiFiStationDisconnected");
        NetworkCommissioning::AmebaWiFiDriver::GetInstance().SetLastDisconnectReason(event);
        if (mWiFiStationState == kWiFiStationState_Connecting)
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
        }
        if (mWiFiStationState == kWiFiStationState_Connected)
        {
            ChangeWiFiStationState(kWiFiStationState_Disconnecting);
        }
        DriveStationState();
    }
    if (event->Type == DeviceEventType::kRtkWiFiScanCompletedEvent)
    {
        ChipLogProgress(DeviceLayer, "WiFiScanCompleted");
        NetworkCommissioning::AmebaWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
    }
    if (event->Type == DeviceEventType::kRtkWiFiDHCPCompletedEvent)
    {
        ChipLogProgress(DeviceLayer, "WiFiDHCPCompleted");
        const bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
        const bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
        if (!(hadIPv4Conn || hadIPv6Conn))
        {
            ChipLogProgress(DeviceLayer, "WiFiDHCPCompleted: UpdateInternetConnectivityState\n");
            UpdateInternetConnectivityState();
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        mWiFiStationMode = (wifi_mode == RTW_MODE_STA) ? kWiFiStationMode_Enabled : kWiFiStationMode_Disabled;
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
    return Internal::AmebaUtils::IsStationProvisioned();
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    // Clear Ameba WiFi station config
    Internal::AmebaUtils::ClearWiFiConfig();
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
    return CHIP_NO_ERROR;
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

// ==================== ConnectivityManager Private Methods ====================

void ConnectivityManagerImpl::DriveStationState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool stationConnected;

    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        err = Internal::AmebaUtils::StartWiFi();
        SuccessOrExit(err);
        err = Internal::AmebaUtils::EnableStationMode();
        SuccessOrExit(err);
    }

    // Determine if the WiFi layer thinks the station interface is currently connected.
    err = Internal::AmebaUtils::IsStationConnected(stationConnected);

    // If the station interface is currently connected ...
    if (stationConnected)
    {
        // Advance the station state to Connected if
        // a previously initiated connect attempt succeeded.
        if (mWiFiStationState == kWiFiStationState_Connecting_Succeeded)
        {
            ChangeWiFiStationState(kWiFiStationState_Connected);
            ChipLogProgress(DeviceLayer, "WiFi station interface connected");
            mLastStationConnectFailTime = System::Clock::kZero;
            OnStationConnected();
        }
        else
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting);
        }
    }

    // Otherwise the station interface is NOT connected to an AP, so...
    else
    {
        System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

        // Advance the station state to NotConnected if it was previously Disconnecting,
        // or if a previous initiated connect attempt failed.
        if (mWiFiStationState == kWiFiStationState_Disconnecting || mWiFiStationState == kWiFiStationState_Connecting_Failed)
        {
            WiFiStationState prevState = mWiFiStationState;
            if (prevState == kWiFiStationState_Connecting_Failed)
            {
                ChipLogProgress(DeviceLayer, "WiFi station failed to connect");
                ChangeWiFiStationState(kWiFiStationState_Connecting);
                // TODO: check retry count if exceeded, then clearwificonfig
            }
            else
            {
                ChipLogProgress(DeviceLayer, "WiFi station disconnected");
                ChangeWiFiStationState(kWiFiStationState_NotConnected);
            }
            mLastStationConnectFailTime = now;
            OnStationDisconnected();
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
                ChangeWiFiStationState(kWiFiStationState_Connecting);
                err = Internal::AmebaUtils::WiFiConnectProvisionedNetwork();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "WiFiConnectProvisionedNetwork() failed: %s", chip::ErrorStr(err));
                }
                SuccessOrExit(err);
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                System::Clock::Timeout timeToNextConnect = (mLastStationConnectFailTime + mWiFiStationReconnectInterval) - now;

                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms",
                                System::Clock::Milliseconds32(timeToNextConnect).count());

                ReturnOnFailure(DeviceLayer::SystemLayer().StartTimer(timeToNextConnect, DriveStationState, nullptr));
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
    NetworkCommissioning::AmebaWiFiDriver::GetInstance().OnConnectWiFiNetwork();
    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    PlatformMgr().PostEventOrDie(&event);
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();

    if (delegate)
    {
        delegate->OnConnectionStatusChanged(
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::ConnectionStatusEnum::kConnected));
    }

    UpdateInternetConnectivityState();
}

void ConnectivityManagerImpl::OnStationDisconnected()
{
    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    PlatformMgr().PostEventOrDie(&event);
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();
    uint16_t reason                    = NetworkCommissioning::AmebaWiFiDriver::GetInstance().GetLastDisconnectReason();
    uint8_t associationFailureCause =
        chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kUnknown);

    if (delegate)
    {
        switch (reason)
        {
        case RTW_NO_ERROR:
        case RTW_NONE_NETWORK:
            associationFailureCause =
                chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kSsidNotFound);
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
        case RTW_CONNECT_FAIL:
            associationFailureCause =
                chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kAssociationFailed);
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
        case RTW_WRONG_PASSWORD:
            associationFailureCause = chip::to_underlying(
                chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kAuthenticationFailed);
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
#if defined(CONFIG_PLATFORM_8710C)
        case RTW_4WAY_HANDSHAKE_TIMEOUT:
#endif
        case RTW_DHCP_FAIL:
        case RTW_UNKNOWN:
            break;

        default:
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
        }
        delegate->OnDisconnectionDetected(reason);
        delegate->OnConnectionStatusChanged(
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::ConnectionStatusEnum::kNotConnected));
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
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::AmebaWiFiDriver::GetInstance().OnNetworkStatusChange(); });
    }
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
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
        struct netif * netif = &xnetif[0];

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
                    char addrStr[INET_ADDRSTRLEN];
                    ip4addr_ntoa_r((const ip4_addr_t *) matter_LwIP_GetIP(0), addrStr, sizeof(addrStr));
                    IPAddress::FromString(addrStr, addr);
                }

                // Search among the IPv6 addresses assigned to the interface for a Global Unicast
                // address (2000::/3) that is in the valid state.  If such an address is found...
                for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
                {
                    if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
                    {
                        haveIPv6Conn = true;
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

void ConnectivityManagerImpl::OnStationIPv4v6AddressAvailable(void)
{
    uint8_t * ip  = matter_LwIP_GetIP(0);
    uint8_t * gw  = matter_LwIP_GetGW(0);
    uint8_t * msk = matter_LwIP_GetMASK(0);
#if LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
#if LWIP_IPV6
    uint8_t * ipv6_0 = matter_LwIP_GetIPv6_linklocal(0);
    uint8_t * ipv6_1 = matter_LwIP_GetIPv6_global(0);
#endif
#endif // LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0

#if CHIP_PROGRESS_LOGGING
    {
        ChipLogProgress(DeviceLayer, "\n\r\tIP              => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
        ChipLogProgress(DeviceLayer, "\n\r\tGW              => %d.%d.%d.%d\n\r", gw[0], gw[1], gw[2], gw[3]);
        ChipLogProgress(DeviceLayer, "\n\r\tmsk             => %d.%d.%d.%d\n\r", msk[0], msk[1], msk[2], msk[3]);
#if LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
#if LWIP_IPV6
        ChipLogProgress(DeviceLayer,
                        "\n\r\tLink-local IPV6 => %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        ipv6_0[0], ipv6_0[1], ipv6_0[2], ipv6_0[3], ipv6_0[4], ipv6_0[5], ipv6_0[6], ipv6_0[7], ipv6_0[8],
                        ipv6_0[9], ipv6_0[10], ipv6_0[11], ipv6_0[12], ipv6_0[13], ipv6_0[14], ipv6_0[15]);
        ChipLogProgress(DeviceLayer,
                        "\n\r\tIPV6            => %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        ipv6_1[0], ipv6_1[1], ipv6_1[2], ipv6_1[3], ipv6_1[4], ipv6_1[5], ipv6_1[6], ipv6_1[7], ipv6_1[8],
                        ipv6_1[9], ipv6_1[10], ipv6_1[11], ipv6_1[12], ipv6_1[13], ipv6_1[14], ipv6_1[15]);
#endif
#endif // LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
    }
#endif // CHIP_PROGRESS_LOGGING

    RefreshMessageLayer();

    UpdateInternetConnectivityState();

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV6_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnStationIPv4v6AddressLost(void)
{
    ChipLogProgress(DeviceLayer, "IPv4 address lost on WiFi station interface");

    RefreshMessageLayer();

    UpdateInternetConnectivityState();

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Lost;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::RefreshMessageLayer(void) {}

void ConnectivityManagerImpl::RtkWiFiStationConnectedHandler(char * buf, int buf_len, int flags, void * userdata)
{
    bool stationConnected;
    Internal::AmebaUtils::IsStationConnected(stationConnected);
    if (Internal::AmebaUtils::IsStationOpenSecurity())
    {
        // continue
    }
    else if (!stationConnected)
    {
        return;
    }

    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type = DeviceEventType::kRtkWiFiStationConnectedEvent;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::RtkWiFiStationDisconnectedHandler(char * buf, int buf_len, int flags, void * userdata)
{
    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type = DeviceEventType::kRtkWiFiStationDisconnectedEvent;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::RtkWiFiScanCompletedHandler(void)
{
    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type = DeviceEventType::kRtkWiFiScanCompletedEvent;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::RtkWiFiDHCPCompletedHandler(char * buf, int buf_len, int flags, void * userdata)
{
    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type = DeviceEventType::kRtkWiFiDHCPCompletedEvent;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::DHCPProcessThread(void * param)
{
    matter_lwip_dhcp();
#if LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
#if LWIP_IPV6
    matter_lwip_dhcp6();
#endif
#endif // LWIP_VERSION_MAJOR > 2 || LWIP_VERSION_MINOR > 0
    PlatformMgr().LockChipStack();
    sInstance.OnStationIPv4v6AddressAvailable();
    PlatformMgr().UnlockChipStack();
    vTaskDelete(NULL);
}

void ConnectivityManagerImpl::DHCPProcess(void)
{
    xTaskCreate(DHCPProcessThread, "DHCPProcess", 4096 / sizeof(StackType_t), this, 1, NULL);
}

int ConnectivityManagerImpl::conn_callback_dispatcher(void * object)
{
    ConnectivityManagerImpl * connmgr = static_cast<ConnectivityManagerImpl *>(object);
    connmgr->RtkWiFiScanCompletedHandler();
    return 0;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace DeviceLayer
} // namespace chip
