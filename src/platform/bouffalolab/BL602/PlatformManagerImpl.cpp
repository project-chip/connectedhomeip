/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/bouffalolab/BL602/NetworkCommissioningDriver.h>

#include <lwip/tcpip.h>

#include <bl_sec.h>

#include <hal_wifi.h>
#include <wifi_mgmr_ext.h>

namespace chip {
namespace DeviceLayer {

static wifi_conf_t conf = {
    .country_code = "CN",
};

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    bl_rand_stream(reinterpret_cast<uint8_t *>(output), static_cast<int>(len));
    *olen = len;

    return 0;
}

typedef void (*aos_event_cb)(input_event_t * event, void * private_data);

void OnWiFiPlatformEvent(input_event_t * event, void * private_data)
{
    switch (event->code)
    {
    case CODE_WIFI_ON_INIT_DONE: {
        wifi_mgmr_start_background(&conf);
    }
    break;
    case CODE_WIFI_ON_MGMR_DONE: {
    }
    break;
    case CODE_WIFI_ON_CONNECTED: {
        ChipLogProgress(DeviceLayer, "WiFi station connected.");
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE: {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        NetworkCommissioning::BLWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    case CODE_WIFI_ON_CONNECTING: {
        ChipLogProgress(DeviceLayer, "WiFi station starts connecting.");
    }
    break;
    case CODE_WIFI_ON_DISCONNECT: {
        ChipLogProgress(DeviceLayer, "WiFi station disconnect, reason %s.", wifi_mgmr_status_code_str(event->value));
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        if (ConnectivityManager::kWiFiStationState_Connecting == ConnectivityMgrImpl().GetWiFiStationState())
        {
            ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManager::kWiFiStationState_Connecting_Failed);
        }
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    case CODE_WIFI_CMD_RECONNECT: {
        ChipLogProgress(DeviceLayer, "WiFi station reconnect.");
    }
    break;
    case CODE_WIFI_ON_GOT_IP: {
        ChipLogProgress(DeviceLayer, "WiFi station gets IPv4 address.");
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        ConnectivityMgrImpl().ChangeWiFiStationState(ConnectivityManagerImpl::kWiFiStationState_Connected);
        ConnectivityMgrImpl().OnConnectivityChanged(wifi_mgmr_sta_netif_get());
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    case CODE_WIFI_ON_GOT_IP6: {
        ChipLogProgress(DeviceLayer, "WiFi station gets IPv6 address.");
        chip::DeviceLayer::PlatformMgr().LockChipStack();
        ConnectivityMgrImpl().OnConnectivityChanged(wifi_mgmr_sta_netif_get());
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    break;
    default: {
        ChipLogProgress(DeviceLayer, "WiFi station gets unknow code %u.", event->code);
        /*nothing*/
    }
    }
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TaskHandle_t backup_eventLoopTask;

    // Initialize LwIP.
    tcpip_init(NULL, NULL);
    aos_register_event_filter(EV_WIFI, OnWiFiPlatformEvent, NULL);

    hal_wifi_start_firmware_task();
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

} // namespace DeviceLayer
} // namespace chip
