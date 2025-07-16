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
#include <platform/senscomm/scm1612s/NetworkCommissioningWiFiDriver.h>
#include <platform/senscomm/scm1612s/CHIPDevicePlatformConfig.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#include "wise_event_loop.h"
#include "wise_wifi_types.h"
#include "wise_err.h"
#include "scm_wifi.h"
#include "wise_event.h"

#define IP2STR(ipaddr) ip4_addr1_16(ipaddr), \
	ip4_addr2_16(ipaddr), \
	ip4_addr3_16(ipaddr), \
	ip4_addr4_16(ipaddr)

#define IPSTR "%d.%d.%d.%d"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR ConnectivityManagerImpl::InitWiFi(void)
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

    if (!IsWiFiStationProvisioned())
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
        ConfigureWiFiAP();
        ChangeWiFiAPState(kWiFiAPState_Activating);

        mWiFiAPMode  = kWiFiAPMode_Enabled;
        mWiFiAPState = kWiFiAPState_NotActive;

        err = CHIP_NO_ERROR;
        SuccessOrExit(err);
#endif
    }
#if 0 /* Temporary fix!! */
    else
#endif
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        mWiFiStationMode  = kWiFiStationMode_Enabled;
        mWiFiStationState = kWiFiStationState_NotConnected;
        char ifname[WIFI_IFNAME_MAX_SIZE + 1] = {0};
        int len = sizeof(ifname);
        int ret = WISE_OK;
        ChipLogProgress(DeviceLayer, "WiFi station start");
        // set station mode, if sta already started, it will return OK and do nothing
        ret = scm_wifi_sta_start(ifname, &len);
        if (ret != WISE_OK)
        {
            ChipLogProgress(DeviceLayer, "WiFi start fail");
        }
        err = CHIP_NO_ERROR;
        SuccessOrExit(err);
#endif
    }

    ChipLogProgress(DeviceLayer, "ConnectivityManager Wi-Fi init done");

exit:
    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    if (mWiFiAPState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", WiFiAPStateToStr(mWiFiAPState), WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}
#endif

void ConnectivityManagerImpl::OnStationIPv4AddressAvailable(const system_event_sta_got_ip_t & got_ip)
{
    bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);

#if CHIP_PROGRESS_LOGGING
    {
        ChipLogProgress(DeviceLayer, "IPv4 address %s on WiFi station interface: " IPSTR "/" IPSTR " gateway " IPSTR,
                        (got_ip.ip_changed) ? "changed" : "ready", IP2STR(&got_ip.ip_info.ip), IP2STR(&got_ip.ip_info.netmask),
                        IP2STR(&got_ip.ip_info.gw));
    }
#endif // CHIP_PROGRESS_LOGGING
    char addrStr[INET_ADDRSTRLEN];
    ip4addr_ntoa_r(&got_ip.ip_info.ip, addrStr, sizeof(addrStr));

    UpdateInternetConnectivityState(true, hadIPv6Conn, reinterpret_cast<const uint8_t *>(addrStr));

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Assigned;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnStationIPv4AddressLost(void)
{
    ChipLogProgress(DeviceLayer, "IPv4 address lost on WiFi station interface");

    UpdateInternetConnectivityState(false, false, nullptr);

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Lost;
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::_OnWiFiPlatformEvent(const ChipDeviceEvent * event)
{
    if (event->Type != DeviceEventType::kSCMSystemEvent)
        return;

    bool hadIPv4Conn = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    bool hadIPv6Conn = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    ChipLogProgress(DeviceLayer, "%s WiFi event %d", __func__, event->Platform.SCMSystemEvent.event.event_id);

    switch (event->Platform.SCMSystemEvent.event.event_id)
    {
    case SYSTEM_EVENT_SCAN_DONE:
        ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_SCAN_DONE");
        NetworkCommissioning::WiseWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
        break;
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
        scm_wifi_dhcp_start();
        DriveStationState();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_DISCONNECTED");
        NetworkCommissioning::WiseWiFiDriver::GetInstance().SetLastDisconnectReason(event);
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
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
        ChipLogProgress(DeviceLayer, "WIFI_EVENT_AP_STACONNECTED");
        MaintainOnDemandWiFiAP();
        break;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    case SYSTEM_EVENT_STA_GOT_IP:
        ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_GOT_IP");
        OnStationIPv4AddressAvailable(event->Platform.SCMSystemEvent.event.event_info.got_ip);
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_STA_LOST_IP");
        OnStationIPv4AddressLost();
        break;
    case SYSTEM_EVENT_GOT_IP6:
    {
        char addrStr[INET_ADDRSTRLEN];
        system_event_got_ip6_t got_ip = event->Platform.SCMSystemEvent.event.event_info.got_ip6;
        ChipLogProgress(DeviceLayer, "SYSTEM_EVENT_GOT_IP6");
        DriveStationState();
        
        ip6addr_ntoa_r(&got_ip.ip6_info.ip, addrStr, sizeof(addrStr));

        UpdateInternetConnectivityState(hadIPv4Conn, true, reinterpret_cast<const uint8_t *>(addrStr));
    }
        break;
    default:
        break;
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wifi_mode_t curWiFiMode;
        mWiFiStationMode =
            (scm_wifi_get_mode(&curWiFiMode, WIFI_IF_STA) == WISE_OK && (curWiFiMode == WIFI_MODE_APSTA || curWiFiMode == WIFI_MODE_STA))
            ? kWiFiStationMode_Enabled
            : kWiFiStationMode_Disabled;
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
    char ssid[32];
    size_t ssid_len = 0;

    SCM1612SConfig::ReadConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, ssid, sizeof(ssid),
                                           ssid_len);

    return !!(ssid_len);
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    ChipLogProgress(DeviceLayer, "%s", __func__);

    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        scm_wifi_clear_config(WIFI_IF_STA);
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;
    bool IsAPModeEnabled;

    // Determine if AP mode is currently enabled in the ESP WiFi layer.
    scm_wifi_sap_get_state(&IsAPModeEnabled, NULL, NULL);


    // Adjust the Connectivity Manager's AP state to match the state in the WiFi layer.
    if (IsAPModeEnabled && (mWiFiAPState == kWiFiAPState_NotActive || mWiFiAPState == kWiFiAPState_Deactivating))
    {
        ChangeWiFiAPState(kWiFiAPState_Activating);
    }
    if (!IsAPModeEnabled && (mWiFiAPState == kWiFiAPState_Active || mWiFiAPState == kWiFiAPState_Activating))
    {
        ChangeWiFiAPState(kWiFiAPState_Deactivating);
    }

    // If the AP interface is not under application control...
    if (mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        // Ensure the WiFi layer is started.
        //StartWiFiLayer();
        //SuccessOrExit(err);

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
                    scm_wifi_sap_stop();
                    ChangeWiFiAPState(kWiFiAPState_Deactivating);
                }
            }
        }
    }

#if 0 //TODO
    // If AP is active, but the interface doesn't have an IPv6 link-local
    // address, assign one now.
    if (mWiFiAPState == kWiFiAPState_Active && scm_wifi_interface_up() && !scm_wifi_get_ipv6(&addr))
    {
        scm_err_t error = scm_wifi_set_ipv6_addr(addr);
        if (error != WISE_OK)
        {
            ChipLogError(DeviceLayer, "create_ip6_linklocal() failed for %s interface",
                         "wlan1");
            goto exit;
        }
    }
#endif

exit:
    if (err != CHIP_NO_ERROR && mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        SetWiFiAPMode(kWiFiAPMode_Disabled);
        scm_wifi_sap_stop();
    }
}

CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP(void)
{
    int ret = WISE_FAIL;
    char ifname[WIFI_IFNAME_MAX_SIZE + 1] = {0};
    int len = sizeof(ifname);

    char ssid[32];
    int ssid_len;
    char key[] = "12345678";

    ChipLogProgress(DeviceLayer, "%s", __func__);

    CHIP_ERROR err = CHIP_NO_ERROR;

    scm_wifi_softap_config sap = {0};

    //srand(time(NULL));
    //only support 12bit
    //uint16_t discriminator = (uint16_t)(rand() & 0x0FFF);
    uint16_t discriminator = 0x0F00;

    ssid_len       = snprintf(ssid, sizeof(ssid), "%s%03X-%04X-%04X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX, discriminator,
                              CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID);

    memcpy(sap.ssid, ssid, ssid_len);

    sap.channel_num = CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL;
    sap.authmode = SCM_WIFI_SECURITY_WPA2PSK;
    sap.pairwise = SCM_WIFI_PAIRWISE_AES;
    /* Encrypt&Valid: memcpy */
    memcpy(sap.key, key, strlen(key));

    scm_wifi_sap_set_config(&sap);

    ret = scm_wifi_sap_start(ifname, &len);

    if (ret == WISE_OK) {
        printf("ifname: %s\n", ifname);
        scm_wifi_set_ip("wlan1", "192.168.200.1", NULL, NULL);
        scm_wifi_dhcps_start();
    }

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
    bool stationConnected;
    int ret = WISE_OK;
    scm_wifi_status connect_status = {0};

    // Refresh the current station mode.
    GetWiFiStationMode();

    // If the station interface is NOT under application control...
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        if (mWiFiStationMode != kWiFiStationMode_Enabled)
        {
	        char ifname[WIFI_IFNAME_MAX_SIZE + 1] = {0};
	        int len = sizeof(ifname);
            ChipLogProgress(DeviceLayer, "WiFi station mode set");
            mWiFiStationState = kWiFiStationState_NotConnected;
            // set station mode, if sta already started, it will return OK and do nothing
            ret = scm_wifi_sta_start(ifname, &len);
            if (ret != WISE_OK) {
                ChipLogProgress(DeviceLayer, "WiFi station mode set failed");
            }
            return;
        }
    }

    ret = scm_wifi_get_options(SCM_WIFI_STA_GET_CONNECT, &connect_status);
    if (ret != WISE_OK) {
        ChipLogProgress(DeviceLayer, "WiFi station mode get failed");
        return;
    }
    stationConnected = (connect_status.status == SCM_WIFI_CONNECTED);

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

            ret = scm_wifi_sta_disconnect();
            if (ret != WISE_OK)
            {
                ChipLogError(DeviceLayer, "WiFi disconnect : FAIL");
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
                ret = scm_wifi_sta_connect();
                if (ret != WISE_OK)
                {
                    ChipLogError(DeviceLayer, "scm_wifi_connect() failed");
                    return;
                }
                ChangeWiFiStationState(kWiFiStationState_Connecting);
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

    NetworkCommissioning::WiseWiFiDriver::GetInstance().OnConnectWiFiNetwork();

    // Alert other components of the new state.
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    (void) PlatformMgr().PostEvent(&event);

    UpdateInternetConnectivityState(false, false, NULL);
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

    UpdateInternetConnectivityState(false, false, NULL);
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

    ConnectivityChange connV4Change = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
    ConnectivityChange connV6Change = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);

    IPAddress addr;

    // If the WiFi station is currently in the connected state...
    if (mWiFiStationState == kWiFiStationState_Connected)
    {
        IPAddress::FromString((char *) ipAddr, addr);
    }

    // If the internet connectivity state has changed...
    if (connV4Change != kConnectivity_NoChange)
    {
        // Update the current state.
        mFlags.Set(ConnectivityFlags::kHaveIPv4InternetConnectivity, haveIPv4Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = connV4Change;
        event.InternetConnectivityChange.IPv6      = kConnectivity_NoChange;
        event.InternetConnectivityChange.ipAddress = addr;

        (void) PlatformMgr().PostEvent(&event);

        ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv4", (haveIPv4Conn) ? "ESTABLISHED" : "LOST");
    }

    // If the internet connectivity state has changed...
    if (connV6Change != kConnectivity_NoChange)
    {
        // Update the current state.
        mFlags.Set(ConnectivityFlags::kHaveIPv6InternetConnectivity, haveIPv6Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = kConnectivity_NoChange;
        event.InternetConnectivityChange.IPv6      = connV6Change;
        event.InternetConnectivityChange.ipAddress = addr;

        (void) PlatformMgr().PostEvent(&event);

        ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv6", (haveIPv6Conn) ? "ESTABLISHED" : "LOST");
    }
}
#endif

} // namespace DeviceLayer
} // namespace chip
