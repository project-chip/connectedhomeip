/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for BL602 platforms using the Bouffalolab BL602 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <crypto/CHIPCryptoPAL.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/bouffalolab/BL602/DiagnosticDataProviderImpl.h>
#include <platform/bouffalolab/BL602/NetworkCommissioningDriver.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

#include <lwip/tcpip.h>

#include <aos/kernel.h>
#include <aos/yloop.h>
#include <bl60x_fw_api.h>
#include <bl_sec.h>
#include <event_device.h>
#include <hal_wifi.h>
#include <tcpip.h>
#include <wifi_mgmr_ext.h>

extern "C" {
#include <bl_sec.h>
}

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

static wifi_conf_t conf = {
    .country_code = "CN",
};

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{

    bl_rand_stream(reinterpret_cast<uint8_t *>(output), static_cast<int>(len));
    *olen = len;

    return 0;
}

static void WifiStaDisconect(void)
{
    NetworkCommissioning::BLWiFiDriver::GetInstance().SetLastDisconnectReason(NULL);

    uint16_t reason = NetworkCommissioning::BLWiFiDriver::GetInstance().GetLastDisconnectReason();
    uint8_t associationFailureCause =
        chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kUnknown);
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();

    if (ConnectivityManagerImpl::mWiFiStationState == ConnectivityManager::kWiFiStationState_Disconnecting)
    {
        return;
    }

    switch (reason)
    {
    case WLAN_FW_TX_ASSOC_FRAME_ALLOCATE_FAIILURE:
    case WLAN_FW_ASSOCIATE_FAIILURE:
    case WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE:
        associationFailureCause =
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kAssociationFailed);
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
        break;
    case WLAN_FW_TX_AUTH_FRAME_ALLOCATE_FAIILURE:
    case WLAN_FW_AUTHENTICATION_FAIILURE:
    case WLAN_FW_AUTH_ALGO_FAIILURE:
    case WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION:
    case WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION:
    case WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE:
    case WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE:
    case WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE:
    case WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH:
    case WLAN_FW_DISCONNECT_BY_USER_NO_DEAUTH:
        associationFailureCause =
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kAuthenticationFailed);
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
        break;
    case WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL:
        associationFailureCause =
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::AssociationFailureCause::kSsidNotFound);
        if (delegate)
        {
            delegate->OnAssociationFailureDetected(associationFailureCause, reason);
        }
        break;
    case WLAN_FW_BEACON_LOSS:
    case WLAN_FW_JOIN_NETWORK_SECURITY_NOMATCH:
    case WLAN_FW_JOIN_NETWORK_WEPLEN_ERROR:
    case WLAN_FW_DISCONNECT_BY_FW_PS_TX_NULLFRAME_FAILURE:
    case WLAN_FW_CREATE_CHANNEL_CTX_FAILURE_WHEN_JOIN_NETWORK:
    case WLAN_FW_ADD_STA_FAILURE:
    case WLAN_FW_JOIN_NETWORK_FAILURE:
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

    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManagerImpl::kWiFiStationState_Disconnecting);
}

static void WifiStaConnected(void)
{
    char ap_ssid[64];
    WiFiDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetWiFiDiagnosticsDelegate();

    if (ConnectivityManagerImpl::mWiFiStationState == ConnectivityManager::kWiFiStationState_Connected)
    {
        return;
    }

    memset(ap_ssid, 0, sizeof(ap_ssid));
    // wifi_mgmr_sta_ssid_get(ap_ssid);
    // wifi_mgmr_ap_item_t * ap_info = mgmr_get_ap_info_handle();
    // wifi_mgmr_get_scan_result_filter(ap_info, ap_ssid);

    ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManagerImpl::kWiFiStationState_Connected);
    ConnectivityMgrImpl().WifiStationStateChange();
    ConnectivityMgrImpl().OnStationConnected();
    if (delegate)
    {
        delegate->OnConnectionStatusChanged(
            chip::to_underlying(chip::app::Clusters::WiFiNetworkDiagnostics::WiFiConnectionStatus::kConnected));
    }
}

void OnWiFiPlatformEvent(input_event_t * event, void * private_data)
{
    static char * ssid;
    static char * password;
    int ret;

    switch (event->code)
    {
    case CODE_WIFI_ON_INIT_DONE: {
        wifi_mgmr_start_background(&conf);
    }
    break;
    case CODE_WIFI_ON_MGMR_DONE: {
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE: {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        NetworkCommissioning::BLWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    case CODE_WIFI_ON_DISCONNECT: {
        log_info("[APP] [EVT] disconnect %lld, Reason: %s\r\n", aos_now_ms(), wifi_mgmr_status_code_str(event->value));

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        WifiStaDisconect();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    case CODE_WIFI_CMD_RECONNECT: {
        log_info("[APP] [EVT] Reconnect %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_GOT_IP: {
        log_info("[APP] [EVT] GOT IP %lld\r\n", aos_now_ms());

        chip::DeviceLayer::PlatformMgr().LockChipStack();
        WifiStaConnected();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    case CODE_WIFI_ON_GOT_IP6: {
        log_info("[APP] [EVT] GOT IP6 %lld\r\n", aos_now_ms());
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        ConnectivityMgrImpl().OnIPv6AddressAvailable();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    default: {
        log_info("[APP] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
        /*nothing*/
    }
    }
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;
    static uint8_t stack_wifi_init = 0;
    TaskHandle_t backup_eventLoopTask;

    // Initialize the configuration system.
    err = Internal::BL602Config::Init();
    SuccessOrExit(err);

    // Initialize LwIP.
    tcpip_init(NULL, NULL);
    aos_register_event_filter(EV_WIFI, OnWiFiPlatformEvent, NULL);

    if (1 == stack_wifi_init)
    {
        log_error("Wi-Fi already initialized!\r\n");
        return;
    }

    hal_wifi_start_firmware_task();
    stack_wifi_init = 1;
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);

    err = chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16);
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    /** weiyin, backup mEventLoopTask which is reset in _InitChipStack */
    backup_eventLoopTask = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::mEventLoopTask;
    err                  = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::mEventLoopTask = backup_eventLoopTask;
exit:
    return err;
}

void PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }

    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
