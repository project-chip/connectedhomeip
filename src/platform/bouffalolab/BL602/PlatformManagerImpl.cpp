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

#include "AppConfig.h"

#include <aos/kernel.h>
#include <aos/yloop.h>
#include <bl_sec.h>
#include <event_device.h>
#include <hal_wifi.h>
#include <tcpip.h>
#include <wifi_mgmr_ext.h>

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

void event_cb_wifi_event(input_event_t * event, void * private_data)
{
    static char * ssid;
    static char * password;

    switch (event->code)
    {
    case CODE_WIFI_ON_INIT_DONE: {
        wifi_mgmr_start_background(&conf);
        log_info("CODE_WIFI_ON_INIT_DONE DONE.\r\n");
    }
    break;
    case CODE_WIFI_ON_MGMR_DONE: {
        log_info("[APP] [EVT] MGMR DONE %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_SCAN_DONE: {
        log_info("[APP] [EVT] SCAN Done %lld, SCAN Result: %s\r\n", aos_now_ms(),
                 WIFI_SCAN_DONE_EVENT_OK == event->value ? "OK" : "Busy now");

        // wifi_mgmr_cli_scanlist();
        NetworkCommissioning::BLWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
    }
    break;
    case CODE_WIFI_ON_CONNECTING: {
        log_info("[APP] [EVT] Connecting %lld\r\n", aos_now_ms());
        ConnectivityManagerImpl::mWiFiStationState = ConnectivityManager::kWiFiStationState_Connecting;
    }
    break;
    case CODE_WIFI_CMD_RECONNECT: {
        log_info("[APP] [EVT] Reconnect %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_CONNECTED: {
        log_info("[APP] [EVT] connected %lld\r\n", aos_now_ms());
        ConnectivityManagerImpl::mWiFiStationState = ConnectivityManager::kWiFiStationState_Connecting_Succeeded;
    }
    break;
    case CODE_WIFI_ON_PRE_GOT_IP: {
        log_info("[APP] [EVT] connected %lld\r\n", aos_now_ms());
    }
    break;
    case CODE_WIFI_ON_GOT_IP: {
        log_info("[APP] [EVT] GOT IP %lld\r\n", aos_now_ms());
        log_info("[SYS] Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());

        ConnectivityManagerImpl::mWiFiStationState = ConnectivityManager::kWiFiStationState_Connected;
        ConnectivityMgrImpl().WifiStationStateChange();
        ConnectivityMgrImpl().OnStationConnected();
    }
    break;
    case CODE_WIFI_ON_PROV_SSID: {
        log_info("[APP] [EVT] [PROV] [SSID] %lld: %s\r\n", aos_now_ms(), event->value ? (const char *) event->value : "UNKNOWN");
        if (ssid)
        {
            vPortFree(ssid);
            ssid = NULL;
        }
        ssid = (char *) event->value;
    }
    break;
    case CODE_WIFI_ON_PROV_BSSID: {
        log_info("[APP] [EVT] [PROV] [BSSID] %lld: %s\r\n", aos_now_ms(), event->value ? (const char *) event->value : "UNKNOWN");
        if (event->value)
        {
            vPortFree((void *) event->value);
        }
    }
    break;
    case CODE_WIFI_ON_PROV_PASSWD: {
        log_info("[APP] [EVT] [PROV] [PASSWD] %lld: %s\r\n", aos_now_ms(), event->value ? (const char *) event->value : "UNKNOWN");
        if (password)
        {
            vPortFree(password);
            password = NULL;
        }
        password = (char *) event->value;
    }
    break;
    case CODE_WIFI_ON_PROV_CONNECT: {
        log_info("[APP] [EVT] [PROV] [CONNECT] %lld\r\n", aos_now_ms());
#if defined(CONFIG_BT_MESH_SYNC)
        if (event->value)
        {
            struct _wifi_conn * conn_info = (struct _wifi_conn *) event->value;
            break;
        }
#endif
        log_info("connecting to %s:%s...\r\n", ssid, password);
    }
    break;
    case CODE_WIFI_ON_PROV_DISCONNECT: {
        log_info("[APP] [EVT] [PROV] [DISCONNECT] %lld\r\n", aos_now_ms());
#if defined(CONFIG_BT_MESH_SYNC)
        // wifi_mgmr_sta_disconnect();
        vTaskDelay(1000);
// wifi_mgmr_sta_disable(NULL);
#endif
        ConnectivityManagerImpl::mWiFiStationState = ConnectivityManager::kWiFiStationState_NotConnected;
    }
    break;
#if defined(CONFIG_BT_MESH_SYNC)
    case CODE_WIFI_ON_PROV_SCAN_START: {
        log_info("[APP] [EVT] [PROV] [SCAN] %lld\r\n", aos_now_ms());
        // wifiprov_scan((void *)event->value);
    }
    break;
    case CODE_WIFI_ON_PROV_STATE_GET: {
        log_info("[APP] [EVT] [PROV] [STATE] %lld\r\n", aos_now_ms());
        // wifiprov_wifi_state_get((void *)event->value);
    }
    break;
#endif /*CONFIG_BT_MESH_SYNC*/
    default: {
        log_info("[APP] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
        /*nothing*/
    }
    }
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    // Initialize the configuration system.
    err = Internal::BL602Config::Init();
    log_error("err: %d\r\n", err);
    SuccessOrExit(err);

    // Initialize LwIP.
    tcpip_init(NULL, NULL);
    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);

    /*wifi fw stack and thread stuff*/
    static uint8_t stack_wifi_init = 0;

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
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
