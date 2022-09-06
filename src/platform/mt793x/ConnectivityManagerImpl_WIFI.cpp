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

#include <platform/ConnectivityManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>
#include <platform/mt793x/NetworkCommissioningWiFiDriver.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#include "mt7933_pos.h"
#include "wifi_api_ex.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR ConnectivityManagerImpl::WiFiInit(void)
{
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    ChipLogProgress(DeviceLayer, "ConnectivityManager Wi-Fi init");

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    // Queue work items to bootstrap the AP and station state machines once
    // the Chip event loop is running.
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mLastStationConnectFailTime   = System::Clock::kZero;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    mWiFiAPMode        = kWiFiAPMode_Disabled;
    mWiFiAPState       = kWiFiAPState_NotActive;
    mWiFiAPIdleTimeout = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);
    mLastAPDemandTime  = System::Clock::kZero;
#endif
    mFlags.ClearAll();

    mFilogicCtx = PlatformMgrImpl().mFilogicCtx;

    if (!IsWiFiStationProvisioned())
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
        mWiFiAPMode  = kWiFiAPMode_Enabled;
        mWiFiAPState = kWiFiAPState_NotActive;
        filogic_wifi_init_async(mFilogicCtx, FILOGIC_WIFI_OPMODE_AP);
        err = CHIP_NO_ERROR;
        SuccessOrExit(err);
#endif
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        mWiFiStationMode  = kWiFiStationMode_Enabled;
        mWiFiStationState = kWiFiStationState_NotConnected;
        filogic_wifi_init_async(mFilogicCtx, FILOGIC_WIFI_OPMODE_STA);
        err = CHIP_NO_ERROR;
        SuccessOrExit(err);
#endif
    }

    ChipLogProgress(DeviceLayer, "ConnectivityManager Wi-Fi init done");

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

void ConnectivityManagerImpl::_OnWiFiPlatformEvent(const ChipDeviceEvent * event)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (event->Type != DeviceEventType::kMtkWiFiEvent)
        return;

    ChipLogProgress(DeviceLayer, "%s WiFi event %s", __func__, filogic_event_to_name(event->Platform.FilogicEvent.event));

    const filogic_async_event_data * event_data;
    bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);

    event_data = &event->Platform.MtkWiFiEvent.event_data;

    if (event_data->event_id == FILOGIC_WIFI_INIT_OK)
    {
        if (FILOGIC_WIFI_PORT_STA == event_data->u.wifi_init.port)
        {
            mWiFiStationState = kWiFiStationState_NotConnected;
            DriveStationState();
        }
        else if (FILOGIC_WIFI_PORT_AP == event_data->u.wifi_init.port)
        {
            ConfigureWiFiAP();
            ChangeWiFiAPState(kWiFiAPState_Activating);
        }
        else
            assert(0);
    }
    else if (event_data->event_id == FILOGIC_AP_START_OK)
    {
        ChangeWiFiAPState(kWiFiAPState_Active);
    }
    else if (event_data->event_id == FILOGIC_SET_OPMODE_OK)
    {
        if (event_data->u.wifi_opmode.opmode == WIFI_MODE_STA_ONLY)
            ChangeWiFiStationState(kWiFiStationState_NotConnected);
        else if (event_data->u.wifi_opmode.opmode == WIFI_MODE_AP_ONLY)
            ChangeWiFiAPState(kWiFiAPState_Active);
        else
            assert(0);
    }
    else if (event_data->event_id == FILOGIC_AP_STATION_CONNECTED)
    {
        MaintainOnDemandWiFiAP();
    }
    else if (event_data->event_id == FILOGIC_AP_STATION_DISCONNECTED)
    {
    }
    else if (event_data->event_id == FILOGIC_STA_DISCONNECTED_FROM_AP)
    {
        if (mWiFiStationState == kWiFiStationState_Connecting)
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting_Failed);
        }
        DriveStationState();
    }
    else if (!hadIPv4Conn && event_data->event_id == FILOGIC_STA_IPV4_ADDR_READY)
    {
        if (mWiFiStationState == kWiFiStationState_Connecting)
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        }

        ChipLogProgress(DeviceLayer, "ip addr: %s", event_data->u.ipv4_str.addr);
        DriveStationState();
        UpdateInternetConnectivityState(TRUE, FALSE, event_data->u.ipv4_str.addr);
    }
    if (!hadIPv6Conn && event_data->event_id == FILOGIC_STA_IPV6_ADDR_READY)
    {
        if (mWiFiStationState == kWiFiStationState_Connecting)
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        }

        ChipLogProgress(DeviceLayer, "ipv6 addr: %s", event_data->u.ipv6_str.addr);
        DriveStationState();
        UpdateInternetConnectivityState(FALSE, TRUE, event_data->u.ipv6_str.addr);
    }
    else if (event->Platform.FilogicEvent.event == FILOGIC_STA_CONNECTED_TO_AP)
    {
        ChipLogProgress(DeviceLayer, "WIFI_EVENT_STA_CONNECTED");
        if (mWiFiStationState == kWiFiStationState_Connecting)
        {
            ChangeWiFiStationState(kWiFiStationState_Connecting_Succeeded);
        }
        DriveStationState();
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::GetFilogicStationMode(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    filogic_wifi_opmode_t opmode;
    int32_t ret;

    filogic_wifi_opmode_get_sync(mFilogicCtx, &opmode);

    if (opmode == FILOGIC_WIFI_OPMODE_STA || opmode == FILOGIC_WIFI_OPMODE_DUAL)
        return kWiFiStationMode_Enabled;
#endif
    return kWiFiStationMode_Disabled;
}

ConnectivityManager::WiFiAPMode ConnectivityManagerImpl::GetFilogicAPMode(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    filogic_wifi_opmode_t opmode;
    int32_t ret;

    filogic_wifi_opmode_get_sync(mFilogicCtx, &opmode);

    if (opmode == FILOGIC_WIFI_OPMODE_AP || opmode == FILOGIC_WIFI_OPMODE_DUAL)
        return kWiFiAPMode_Enabled;
#endif
    return kWiFiAPMode_Disabled;
}

filogic_wifi_opmode_t ConnectivityManagerImpl::GetFilogicNextOpMode(WiFiStationMode staMode, WiFiAPMode apMode)
{
    bool sta, ap;
    filogic_wifi_opmode_t opmode;

    ChipLogProgress(DeviceLayer, "%s %d %d", __func__, staMode, apMode);

    sta = staMode == kWiFiStationMode_Enabled;
    ap  = apMode == kWiFiAPMode_Enabled;

    if (sta && ap)
        opmode = FILOGIC_WIFI_OPMODE_DUAL;
    else if (ap)
        opmode = FILOGIC_WIFI_OPMODE_AP;
    else if (sta)
        opmode = FILOGIC_WIFI_OPMODE_STA;
    else
        opmode = FILOGIC_WIFI_OPMODE_NONE;

    return opmode;
}

void ConnectivityManagerImpl::SetFlogicNextMode(filogic_wifi_opmode_t nextMode)
{
    ChipLogProgress(DeviceLayer, "WiFi driver mode set %s", filogic_opmode_to_name(nextMode));
    filogic_wifi_opmode_set_async(mFilogicCtx, nextMode);
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        filogic_wifi_opmode_t opmode;

        filogic_wifi_opmode_get_sync(mFilogicCtx, &opmode);

        if (opmode == FILOGIC_WIFI_OPMODE_AP)
            mWiFiStationMode = kWiFiStationMode_Disabled;
        else
            mWiFiStationMode = kWiFiStationMode_Enabled;
    }
    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
        mWiFiStationMode = val;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    mWiFiStationReconnectInterval = val;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWifiStatsCounters(void)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return GetWiFiStationMode() == kWiFiStationMode_Enabled;
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    filogic_wifi_sta_prov_t prov = {};

    /* See if we have SSID */
    if (filogic_wifi_sta_prov_get_sync(mFilogicCtx, &prov))
    {
        return prov.ssid[0] != '\0';
    }

    return false;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
#ifdef MT793X_PORTING
        wfx_clear_wifi_provision();
#endif /* MT793X_PORTING */

        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
    }
}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    // CHIP_ERROR_NOT_IMPLEMENTED
}

void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    // Schedule a call to the DriveStationState method to adjust the station state as needed.
    ChipLogProgress(DeviceLayer, "_ON WIFI PROVISION CHANGE");
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
void ConnectivityManagerImpl::DriveAPState(void)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    WiFiAPMode driverAPMode = GetFilogicAPMode();
    filogic_wifi_opmode_t nextMode;

    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiAPMode != driverAPMode)
    {
        nextMode = GetFilogicNextOpMode(mWiFiStationMode, driverAPMode);
        ChipLogProgress(DeviceLayer, "WiFi Driver AP mode set: %d", nextMode);
        SetFlogicNextMode(nextMode);
        if (driverAPMode == kWiFiAPMode_Enabled)
        {
        }
        // TODO wait driver event
    }
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP(void)
{
    char ssid[32];
    int ssid_len;

    ChipLogProgress(DeviceLayer, "%s", __func__);

    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO, generate
    uint16_t discriminator = 0x8888;

    ssid_len       = snprintf(ssid, sizeof(ssid), "%s%03X-%04X-%04X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX, discriminator,
                        CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);
    int8_t channel = CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL;

    filogic_wifi_ap_config_async(mFilogicCtx, channel, ssid, ssid_len);

    return err;
}

void ConnectivityManagerImpl::DriveAPState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveAPState();
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }

    mWiFiAPMode = val;

exit:
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::Clock::kZero;
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

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
    ChipLogProgress(DeviceLayer, "%s", __func__);

    mWiFiAPIdleTimeout = val;
    DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP */

/****************************************************************************
 * ConnectivityManager Private Methods
 ****************************************************************************/

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
void ConnectivityManagerImpl::DriveStationState()
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    CHIP_ERROR err = CHIP_NO_ERROR;
    int32_t status;
    bool stationConnected;

    // Refresh the current station mode.
    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        if (mWiFiStationMode != kWiFiStationMode_Enabled)
        {
            ChipLogProgress(DeviceLayer, "WiFi station mode set");
            mWiFiStationState = kWiFiStationState_NotConnected;
            SetFlogicNextMode(FILOGIC_WIFI_OPMODE_STA);
            return;
        }
    }

    stationConnected = filogic_wifi_sta_get_link_status_sync(mFilogicCtx);

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

            status = wifi_connection_disconnect_ap();
            if (status < 0)
            {
                ChipLogError(DeviceLayer, "WiFi disconnect : FAIL: %ld", status);
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
                if (mWiFiStationState != kWiFiStationState_Connecting)
                {
                    ChipLogProgress(DeviceLayer, "Attempting to connect WiFi");

                    status = wifi_config_reload_setting();

                    if (status < 0)
                    {
                        ChipLogError(DeviceLayer, "WiFi start connect : FAIL %ld", status);
                        goto exit;
                    }

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
}

void ConnectivityManagerImpl::OnStationConnected()
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    ChipDeviceEvent event;

    NetworkCommissioning::GenioWiFiDriver::GetInstance().OnConnectWiFiNetwork();

    // Alert other components of the new state.
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    (void) PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState(FALSE, FALSE, NULL);
}

void ConnectivityManagerImpl::OnStationDisconnected()
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    // TODO Invoke WARM to perform actions that occur when the WiFi station interface goes down.

    // Alert other components of the new state.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    (void) PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState(FALSE, FALSE, NULL);
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
    }
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(bool haveIPv4Conn, bool haveIPv6Conn, const uint8_t * ipAddr)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    IPAddress addr;

    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        IPAddress::FromString((char *) ipAddr, addr);
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
#endif

} // namespace DeviceLayer
} // namespace chip
