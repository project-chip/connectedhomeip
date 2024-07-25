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
#include <platform/DeviceInstanceInfoProvider.h>

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
#include <platform/Infineon/PSOC6/PSOC6Utils.h>
#include <platform/internal/BLEManager.h>

#include "cy_network_mw_core.h"
#include "cy_nw_helper.h"

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include "lwip/opt.h"
#include <platform/Infineon/PSOC6/NetworkCommissioningDriver.h>
#include <type_traits>

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for PSoC6"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    uint32_t curWiFiMode;
    mWiFiStationMode = (Internal::PSOC6Utils::wifi_get_mode(curWiFiMode) == CHIP_NO_ERROR &&
                        (curWiFiMode == WIFI_MODE_APSTA || curWiFiMode == WIFI_MODE_STA))
        ? kWiFiStationMode_Enabled
        : kWiFiStationMode_Disabled;
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
    return Internal::PSOC6Utils::IsStationProvisioned();
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wifi_config_t stationConfig;

        memset(&stationConfig, 0, sizeof(stationConfig));
        Internal::PSOC6Utils::p6_wifi_set_config(WIFI_IF_STA, &stationConfig);

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

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWiFiStatsCounters(void)
{
    cy_wcm_associated_ap_info_t ap_info;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    CHIP_ERROR err   = CHIP_NO_ERROR;

    result = cy_wcm_get_associated_ap_info(&ap_info);
    if (result != CY_RSLT_SUCCESS)
    {
        ChipLogError(DeviceLayer, "cy_wcm_get_associated_ap_info failed: %d", (int) result);
        SuccessOrExit(err = CHIP_ERROR_INTERNAL);
    }

    ChipLogProgress(DeviceLayer,
                    "WiFi-Telemetry\n"
                    "BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n"
                    "RSSI: %d\n"
                    "Channel: %d\n"
                    "Channel Width: %d Mhz\n",
                    ap_info.BSSID[0], ap_info.BSSID[1], ap_info.BSSID[2], ap_info.BSSID[3], ap_info.BSSID[4], ap_info.BSSID[5],
                    ap_info.signal_strength, ap_info.channel, ap_info.channel_width);
exit:
    return err;
}

// ==================== ConnectivityManager Platform Internal Methods ====================
CHIP_ERROR ConnectivityManagerImpl::_Init()
{
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

    // Ensure that PSOC6 station mode is enabled.
    err = Internal::PSOC6Utils::EnableStationMode();
    SuccessOrExit(err);
    // If there is no persistent station provision...
    if (!IsWiFiStationProvisioned())
    {
        // If the code has been compiled with a default WiFi station provision, configure that now.
        if (CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID[0] != 0)
        {
            ChipLogProgress(DeviceLayer, "Setting default WiFi station configuration (SSID: %s)",
                            CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID);

            // Set a default station configuration.
            wifi_config_t wifiConfig;
            memset(&wifiConfig, 0, sizeof(wifiConfig));
            memcpy(wifiConfig.sta.ssid, CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID,
                   min(strlen(CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID), sizeof(wifiConfig.sta.ssid)));
            memcpy(wifiConfig.sta.password, CHIP_DEVICE_CONFIG_DEFAULT_STA_PASSWORD,
                   min(strlen(CHIP_DEVICE_CONFIG_DEFAULT_STA_PASSWORD), sizeof(wifiConfig.sta.password)));
            wifiConfig.sta.security = CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY;
            err                     = Internal::PSOC6Utils::p6_wifi_set_config(WIFI_IF_STA, &wifiConfig);
            SuccessOrExit(err);

            // Enable WiFi station mode.
            ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
        }
        else
        {
            ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Disabled));
        }
    }
    else
    {
        // Enable WiFi station mode.
        ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
    }
    // Force AP mode off for now.
    err = Internal::PSOC6Utils::SetAPMode(false);
    SuccessOrExit(err);

    // Queue work items to bootstrap the station state machines once the Chip event loop is running.
    err = DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

void ConnectivityManagerImpl::wlan_event_cb(cy_wcm_event_t event, cy_wcm_event_data_t * event_data)
{
    switch (event)
    {
    case CY_WCM_EVENT_CONNECTING:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_CONNECTING");
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting);
        break;
    case CY_WCM_EVENT_CONNECTED:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_CONNECTED");
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        NetworkCommissioning::P6WiFiDriver::GetInstance().OnConnectWiFiNetwork();
        ConnectivityMgrImpl().DriveStationState();
        break;
    case CY_WCM_EVENT_CONNECT_FAILED:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_CONNECT_FAILED");
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
        ConnectivityMgrImpl().DriveStationState();
        break;
    case CY_WCM_EVENT_RECONNECTED:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_RECONNECTED");
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        NetworkCommissioning::P6WiFiDriver::GetInstance().OnConnectWiFiNetwork();
        ConnectivityMgrImpl().DriveStationState();
        break;
    case CY_WCM_EVENT_DISCONNECTED:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_DISCONNECTED");
        ConnectivityMgrImpl().ChangeWiFiStationState(kWiFiStationState_Disconnecting);
        NetworkCommissioning::P6WiFiDriver::GetInstance().SetLastDisconnectReason(WLC_E_SUP_DEAUTH);
        break;
    case CY_WCM_EVENT_IP_CHANGED:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_IP_CHANGED");
        ConnectivityMgrImpl().OnIPAddressAvailable();
        break;
    case CY_WCM_EVENT_STA_JOINED_SOFTAP:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_STA_JOINED_SOFTAP");
        break;
    case CY_WCM_EVENT_STA_LEFT_SOFTAP:
        ChipLogProgress(DeviceLayer, "CY_WCM_EVENT_STA_LEFT_SOFTAP");
        break;
    default:
        ChipLogProgress(DeviceLayer, "UnSupported Event");
        break;
    }
}
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
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::P6WiFiDriver::GetInstance().OnNetworkStatusChange(); });
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

#define INITIALISER_IPV4_ADDRESS1(addr_var, addr_val) addr_var = { CY_WCM_IP_VER_V4, { .v4 = (uint32_t) (addr_val) } }
#define MAKE_IPV4_ADDRESS1(a, b, c, d) ((((uint32_t) d) << 24) | (((uint32_t) c) << 16) | (((uint32_t) b) << 8) | ((uint32_t) a))
static const cy_wcm_ip_setting_t ap_mode_ip_settings = {
    INITIALISER_IPV4_ADDRESS1(.ip_address, MAKE_IPV4_ADDRESS1(192, 168, 0, 2)),
    INITIALISER_IPV4_ADDRESS1(.gateway, MAKE_IPV4_ADDRESS1(192, 168, 0, 2)),
    INITIALISER_IPV4_ADDRESS1(.netmask, MAKE_IPV4_ADDRESS1(255, 255, 255, 0)),
};

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t wifiConfig;

    memset(&wifiConfig.ap, 0, sizeof(wifi_config_ap_t));

    uint16_t vendorId;
    uint16_t productId;
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetVendorId(vendorId));
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetProductId(productId));

    snprintf((char *) wifiConfig.ap.ssid, sizeof(wifiConfig.ap.ssid), "%s-%04X-%04X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX,
             vendorId, productId);
    memcpy(wifiConfig.ap.password, CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD, strlen(CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD));
    wifiConfig.ap.channel                = CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL;
    wifiConfig.ap.security               = CHIP_DEVICE_CONFIG_WIFI_AP_SECURITY;
    wifiConfig.ap.ip_settings.ip_address = ap_mode_ip_settings.ip_address;
    wifiConfig.ap.ip_settings.netmask    = ap_mode_ip_settings.netmask;
    wifiConfig.ap.ip_settings.gateway    = ap_mode_ip_settings.gateway;

    ChipLogProgress(DeviceLayer, "Configuring WiFi AP: SSID %s, channel %u", wifiConfig.ap.ssid, wifiConfig.ap.channel);
    err = Internal::PSOC6Utils::p6_wifi_set_config(WIFI_IF_AP, &wifiConfig);
    SuccessOrExit(err);

    err = Internal::PSOC6Utils::p6_start_ap();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "p6_start_ap failed: %s", chip::ErrorStr(err));
    }

exit:
    return err;
}

void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;
    bool APModeEnabled;

    // Determine if AP mode is currently enabled in the PSOC6 WiFi layer.
    err = Internal::PSOC6Utils::IsAPEnabled(APModeEnabled);
    SuccessOrExit(err);

    // Adjust the Connectivity Manager's AP state to match the state in the WiFi layer.
    if (APModeEnabled && (mWiFiAPState == kWiFiAPState_NotActive || mWiFiAPState == kWiFiAPState_Deactivating))
    {
        ChangeWiFiAPState(kWiFiAPState_Activating);
    }
    if (!APModeEnabled && (mWiFiAPState == kWiFiAPState_Active || mWiFiAPState == kWiFiAPState_Activating))
    {
        ChangeWiFiAPState(kWiFiAPState_Deactivating);
    }
    // If the AP interface is not under application control...
    if (mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
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
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " ms",
                                System::Clock::Milliseconds32(apTimeout).count());
                SystemLayer().ScheduleLambda([apTimeout, this] {
                    CHIP_ERROR ret = CHIP_NO_ERROR;
                    ret            = DeviceLayer::SystemLayer().StartTimer(apTimeout, DriveAPState, this);
                    VerifyOrReturn(ret == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "StartTimer failed %s: ", chip::ErrorStr(ret)));
                });
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
            // a SYSTEM_EVENT_AP_START event will be received from the PSOC6 WiFi layer which will
            // cause the state to transition to 'Active'.
            if (targetState == kWiFiAPState_Active)
            {
                if (mWiFiAPState != kWiFiAPState_Active)
                {
                    err = Internal::PSOC6Utils::SetAPMode(true);
                    SuccessOrExit(err);
                    err = ConfigureWiFiAP();
                    SuccessOrExit(err);
                    ChangeWiFiAPState(kWiFiAPState_Active);
                }
            }

            // Otherwise, if the target state is 'NotActive' and the current state is not 'Deactivating',
            // disable the AP interface and enter the 'Deactivating' state.  Later a SYSTEM_EVENT_AP_STOP
            // event will move the AP state to 'NotActive'.
            else
            {
                if (mWiFiAPState != kWiFiAPState_Deactivating)
                {
                    err = Internal::PSOC6Utils::SetAPMode(false);
                    SuccessOrExit(err);
                    err = Internal::PSOC6Utils::p6_stop_ap();
                    SuccessOrExit(err);
                    ChangeWiFiAPState(kWiFiAPState_Deactivating);
                }
            }
        }
    }

exit:
    if (err != CHIP_NO_ERROR && mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        SetWiFiAPMode(kWiFiAPMode_Disabled);
        Internal::PSOC6Utils::SetAPMode(false);
        Internal::PSOC6Utils::p6_stop_ap();
    }
}

void ConnectivityManagerImpl::DriveStationState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool stationConnected;

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        // Ensure that the PSOC6 WiFi layer is started.
        err = WiFi_init();
        SuccessOrExit(err);

        // Ensure that station mode is enabled in the PSOC6 WiFi layer.
        err = Internal::PSOC6Utils::EnableStationMode();
        SuccessOrExit(err);
    }

    // Determine if the PSOC6 WiFi layer thinks the station interface is currently connected.
    err = Internal::PSOC6Utils::IsStationConnected(stationConnected);
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
        }

        // If the WiFi station interface is no longer enabled, or no longer provisioned,
        // disconnect the station from the AP, unless the WiFi station mode is currently
        // under application control.
        if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled &&
            (mWiFiStationMode != kWiFiStationMode_Enabled || !IsWiFiStationProvisioned()))
        {
            ChipLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
            err = Internal::PSOC6Utils::p6_wifi_disconnect();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "p6_wifi_disconnect() failed: %s", chip::ErrorStr(err));
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
                ChangeWiFiStationState(kWiFiStationState_Connecting);
                ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
                err = Internal::PSOC6Utils::p6_wifi_connect();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "p6_wifi_connect() failed: %s", chip::ErrorStr(err));
                }
                SuccessOrExit(err);
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                System::Clock::Timeout timeToNextConnect = (mLastStationConnectFailTime + mWiFiStationReconnectInterval) - now;
                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms ",
                                System::Clock::Milliseconds32(timeToNextConnect).count());
                SystemLayer().ScheduleLambda([timeToNextConnect, this] {
                    CHIP_ERROR ret = CHIP_NO_ERROR;
                    ret            = DeviceLayer::SystemLayer().StartTimer(timeToNextConnect, DriveStationState, this);
                    VerifyOrReturn(ret == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "StartTimer failed %s: ", chip::ErrorStr(ret)));
                });
            }
        }
    }

exit:

    ChipLogProgress(DeviceLayer, "Done driving station state, nothing else to do...");
    // Kick-off any pending network scan that might have been deferred due to the activity
    // of the WiFi station.
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn            = false;
    bool haveIPv6Conn            = false;
    const bool hadIPv4Conn       = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    const bool hadIPv6Conn       = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    struct netif * net_interface = NULL;
    IPAddress addr;
    bool stationConnected;
    Internal::PSOC6Utils::IsStationConnected(stationConnected);

    ChipLogProgress(DeviceLayer, "UpdateInternetConnectivityState");
    // If the WiFi station is currently in the connected state...
    if ((mWiFiStationState == kWiFiStationState_Connected) || stationConnected)
    {
        net_interface = (netif *) cy_network_get_nw_interface(CY_NETWORK_WIFI_STA_INTERFACE, 0);
        if (net_interface != NULL && netif_is_up(net_interface) && netif_is_link_up(net_interface))
        {
            if (!ip4_addr_isany(netif_ip4_addr(net_interface)) && !ip4_addr_isany(netif_ip4_gw(net_interface)))
            {
                haveIPv4Conn = true;
                ChipDeviceEvent event;
                event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
                event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Assigned;
                PlatformMgr().PostEventOrDie(&event);
                ChipLogProgress(DeviceLayer, "IPv4 Address Assigned : %s", ip4addr_ntoa(netif_ip4_addr(net_interface)));
            }
            // Search among the IPv6 addresses assigned to the interface for a Global Unicast
            // address (2000::/3) that is in the valid state.  If such an address is found...
            for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
            {
                if (ip6_addr_islinklocal(netif_ip6_addr(net_interface, i)) &&
                    ip6_addr_isvalid(netif_ip6_addr_state(net_interface, i)))
                {
                    haveIPv6Conn = true;
                    ChipDeviceEvent event;
                    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
                    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV6_Assigned;
                    PlatformMgr().PostEventOrDie(&event);
                    ChipLogProgress(DeviceLayer, "IPv6 Address Assigned : %s", ip6addr_ntoa(netif_ip6_addr(net_interface, i)));
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

CHIP_ERROR ConnectivityManagerImpl::OnIPAddressAvailable(void)
{
    ChipLogProgress(DeviceLayer, "IP address available on WiFi station interface");
    UpdateInternetConnectivityState();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::WiFi_init(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    err              = Internal::PSOC6Utils::StartWiFiLayer();
    SuccessOrExit(err);
    /* Register event callback */
    if (eventcallback == false)
    {
        result = cy_wcm_register_event_callback(ConnectivityManagerImpl::wlan_event_cb);
        if (result != CY_RSLT_SUCCESS)
        {
            ChipLogError(DeviceLayer, "cy_wcm_register_event_callback failed....! \r\n");
            err = CHIP_ERROR_INTERNAL;
            SuccessOrExit(err);
        }
        eventcallback = true;
    }
exit:
    return err;
}

CHIP_ERROR ConnectivityManagerImpl::ping_thread()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = Internal::PSOC6Utils::ping_init();
    return err;
}

} // namespace DeviceLayer
} // namespace chip
