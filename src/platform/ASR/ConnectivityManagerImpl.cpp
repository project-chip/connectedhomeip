/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>
#include <platform/ASR/ASRUtils.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/BLEManager.h>

#include "lega_wlan_api.h"
#include "lwip/opt.h"
#include <platform/ASR/NetworkCommissioningDriver.h>
#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for ASR"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
#ifdef __cplusplus
extern "C" {
#endif
extern struct netif * lwip_get_netif(void);
#ifdef __cplusplus
}
#endif
namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    mWiFiStationMode = kWiFiStationMode_Enabled;
    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (val != kWiFiStationMode_ApplicationControlled)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
        mWiFiStationMode = val;
        /* Schedule work for disabled case causes station mode not getting enabled */
        if (mWiFiStationMode != kWiFiStationMode_Disabled)
        {
            DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
        }
        else
        {
            /* Call Drive Station directly to disable directly instead of scheduling */
            DriveStationState();
        }
    }

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return Internal::ASRUtils::IsStationProvisioned();
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        lega_wlan_wifi_conf stationConfig;

        memset(&stationConfig, 0, sizeof(stationConfig));
        Internal::ASRUtils::asr_wifi_set_config(&stationConfig);

        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
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

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl::_GetAndLogWifiStatsCounters");
    return CHIP_NO_ERROR;
}

// ==================== ConnectivityManager Platform Internal Methods ====================
CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl::_Init");
    CHIP_ERROR err                = CHIP_NO_ERROR;
    mLastStationConnectFailTime   = System::Clock::kZero;
    mLastAPDemandTime             = System::Clock::kZero;
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mWiFiAPMode                   = kWiFiAPMode_Disabled;
    mWiFiAPState                  = kWiFiAPState_NotActive;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mWiFiAPIdleTimeout            = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);
    mFlags.SetRaw(0);

    // Ensure that ASR station mode is enabled.
    err = Internal::ASRUtils::EnableStationMode();
    SuccessOrExit(err);
    err = Internal::ASRUtils::SetStationConnected(false);
    SuccessOrExit(err);

    // If there is no persistent station provision...
    if (!IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
    }
    else
    {
        // Enable WiFi station mode.
        ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
    }
    // Queue work items to bootstrap the station state machines once the Chip event loop is running.
    err = DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::DriveAPState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveAPState();
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::ASRWiFiDriver::GetInstance().OnNetworkStatusChange(); });
    }
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    if (mWiFiAPState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", WiFiAPStateToStr(mWiFiAPState), WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl::ConfigureWiFiAP");
    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::DriveAPState() {}

void ConnectivityManagerImpl::DriveStationState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool stationConnected;

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        err = WiFi_init();
        SuccessOrExit(err);
        // Ensure that station mode is enabled in the ASR WiFi layer.
        err = Internal::ASRUtils::EnableStationMode();
        SuccessOrExit(err);
    }

    // Determine if the ASR WiFi layer thinks the station interface is currently connected.
    err = Internal::ASRUtils::IsStationConnected(stationConnected);
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
            err = Internal::ASRUtils::asr_wifi_disconnect();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "asr_wifi_disconnect() failed: %s", chip::ErrorStr(err));
            }
            SuccessOrExit(err);

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
                    ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
                    err = Internal::ASRUtils::asr_wifi_connect();
                    if (err != CHIP_NO_ERROR)
                    {
                        ChipLogError(DeviceLayer, "asr_wifi_connect() failed: %s", chip::ErrorStr(err));
                    }
                    SuccessOrExit(err);
                    ChangeWiFiStationState(kWiFiStationState_Connecting);
                }
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                System::Clock::Timeout timeToNextConnect = (mLastStationConnectFailTime + mWiFiStationReconnectInterval) - now;
                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms ",
                                System::Clock::Milliseconds32(timeToNextConnect).count());

                err = DeviceLayer::SystemLayer().StartTimer(timeToNextConnect, DriveStationState, NULL);
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
    NetworkCommissioning::ASRWiFiDriver::GetInstance().OnConnectWiFiNetwork();
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
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::ConnectionStatusEnum::kConnected));
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
    uint16_t reason                    = NetworkCommissioning::ASRWiFiDriver::GetInstance().GetLastDisconnectReason();
    uint8_t associationFailureCause =
        chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kUnknown);

    if (delegate)
    {
        switch (reason)
        {
        case WLAN_STA_MODE_BEACON_LOSS:
        case WLAN_STA_MODE_NO_AP_FOUND:
            associationFailureCause =
                chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kSsidNotFound);
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
        case WLAN_STA_MODE_ASSOC_FAIL:
            associationFailureCause =
                chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kAssociationFailed);
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
        case WLAN_STA_MODE_AUTH_FAIL:
        case WLAN_STA_MODE_PASSWORD_ERR:
            associationFailureCause = chip::to_underlying(
                chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCauseEnum::kAuthenticationFailed);
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
            break;
        case WLAN_STA_MODE_DHCP_FAIL:
        case WLAN_STA_MODE_CONN_RETRY_MAX:
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

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn            = false;
    bool haveIPv6Conn            = false;
    const bool hadIPv4Conn       = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    const bool hadIPv6Conn       = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    struct netif * net_interface = NULL;
    IPAddress addr;

    ChipLogProgress(DeviceLayer, "UpdateInternetConnectivityState");
    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        net_interface = lwip_get_netif();
        if (net_interface != NULL && netif_is_up(net_interface) && netif_is_link_up(net_interface))
        {
            // Check if a DNS server is currently configured.  If so...
            ip_addr_t dnsServerAddr = *dns_getserver(0);
            if (!ip_addr_isany_val(dnsServerAddr))
            {
                if (!ip4_addr_isany_val(*netif_ip4_addr(net_interface)) && !ip4_addr_isany_val(*netif_ip4_gw(net_interface)))
                {
                    haveIPv4Conn = true;
                    char addrStr[INET_ADDRSTRLEN];
                    ip4addr_ntoa_r(netif_ip4_addr(net_interface), addrStr, sizeof(addrStr));

                    // ChipLogProgress(DeviceLayer, "IPv4 Address Assigned : %s", ip4addr_ntoa(netif_ip4_addr(net_interface)));

                    IPAddress::FromString(addrStr, addr);
                }
                // Search among the IPv6 addresses assigned to the interface for a Global Unicast
                // address (2000::/3) that is in the valid state.  If such an address is found...
                for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
                {
                    if (ip6_addr_isglobal(netif_ip6_addr(net_interface, i)) &&
                        ip6_addr_isvalid(netif_ip6_addr_state(net_interface, i)))
                    {
                        // Determine if there is a default IPv6 router that is currently reachable
                        // via the station interface.  If so, presume for now that the device has
                        // IPv6 connectivity.
                        struct netif * found_if = nd6_find_route(IP6_ADDR_ANY6);
                        if (found_if && net_interface->num == found_if->num)
                        {
                            haveIPv6Conn = true;
                            // ChipLogProgress(DeviceLayer, "IPv6 Address Assigned : %s", ip6addr_ntoa(netif_ip6_addr(net_interface,
                            // i)));
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

void ConnectivityManagerImpl::lega_wifi_connect_state(lega_wifi_event_e stat)
{
    switch (stat)
    {
    case EVENT_STATION_UP:
        ChipLogProgress(DeviceLayer, "wifi_connect_done");
        Internal::ASRUtils::SetStationConnected(true);
        if (ConnectivityMgrImpl().mWiFiStationState == kWiFiStationState_Connecting)
        {
            ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        }
        ConnectivityMgrImpl().DriveStationState();
        break;
    case EVENT_STATION_DOWN:
        ChipLogProgress(DeviceLayer, "wifi_disconnect_done");
        Internal::ASRUtils::SetStationConnected(false);
        if (ConnectivityMgrImpl().mWiFiStationState == kWiFiStationState_Connecting)
        {
            ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
        }
        ConnectivityMgrImpl().DriveStationState();
        break;
    case EVENT_AP_UP:
        ChipLogProgress(DeviceLayer, "wifi_softap_open_done");
        break;
    case EVENT_AP_DOWN:
        ChipLogProgress(DeviceLayer, "wifi_softap_close_done");
        break;
    case EVENT_STA_CLOSE:
        ChipLogProgress(DeviceLayer, "wifi_sta_close_done");
        Internal::ASRUtils::SetStationConnected(false);
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_NotConnected);
        NetworkCommissioning::ASRWiFiDriver::GetInstance().SetLastDisconnectReason(0);
        ConnectivityMgrImpl().DriveStationState();
        break;
    default:
        ChipLogProgress(DeviceLayer, "unsupport wifi_event_e");
        break;
    }
}

void ConnectivityManagerImpl::lega_wifi_get_ip_ind(lega_wlan_ip_stat_t * pnet)
{
    ChipLogProgress(DeviceLayer, "Got ip : %s, gw : %s, mask : %s, mac : %s", pnet->ip, pnet->gate, pnet->mask, pnet->macaddr);

    ConnectivityMgrImpl().UpdateInternetConnectivityState();

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::lega_wifi_get_ip6_ind(lega_wlan_ip_stat_t * pnet)
{
    int i             = 0;
    int preferred_ip6 = 0;
    for (i = 0; i < 3; i++)
    {
        if (pnet->ip6[i].state == 0x30) /* IP6_ADDR_PREFERRED */
        {
            ChipLogProgress(DeviceLayer, "Got ip v6 #%d : %s", i, pnet->ip6[i].addr);
            preferred_ip6++;
        }
    }
    if (preferred_ip6)
    {
        ConnectivityMgrImpl().UpdateInternetConnectivityState();

        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
        event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV6_Assigned;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void ConnectivityManagerImpl::lega_wifi_ap_peer_change(lega_wlan_client_addr_info_t * peer_info, uint8_t connect) {}

void ConnectivityManagerImpl::lega_wlan_err_stat_handler(lega_wlan_err_status_e err_info)
{
    ChipLogProgress(DeviceLayer, "lega wlan err stat:%d", err_info);
    if (ConnectivityMgrImpl().mWiFiStationState == kWiFiStationState_Connecting)
    {
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
    }
    NetworkCommissioning::ASRWiFiDriver::GetInstance().SetLastDisconnectReason(err_info);
    ConnectivityMgrImpl().DriveStationState();
}

CHIP_ERROR ConnectivityManagerImpl::WiFi_init(void)
{
    lega_wlan_stat_chg_cb_register(ConnectivityManagerImpl::lega_wifi_connect_state);
    lega_wlan_ip_got_cb_register(ConnectivityManagerImpl::lega_wifi_get_ip_ind);
    lega_wlan_ip6_got_cb_register(ConnectivityManagerImpl::lega_wifi_get_ip6_ind);
    lega_wlan_ap_peer_change_cb_register(ConnectivityManagerImpl::lega_wifi_ap_peer_change);
    lega_wlan_err_stat_cb_register(ConnectivityManagerImpl::lega_wlan_err_stat_handler);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::ping_thread()
{
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
