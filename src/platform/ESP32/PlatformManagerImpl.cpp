/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app-common/zap-generated/enums.h>
#include <crypto/CHIPCryptoPAL.h>
#include <platform/ESP32/DiagnosticDataProviderImpl.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/SystemTimeSupport.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>

#include "esp_event.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "spi_flash_mmap.h"
#else
#include "esp_spi_flash.h"
#endif
#include "esp_system.h"
#include "esp_wifi.h"

namespace chip {
namespace DeviceLayer {

namespace Internal {
extern CHIP_ERROR InitLwIPCoreLock(void);
}

PlatformManagerImpl PlatformManagerImpl::sInstance;

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    esp_fill_random(output, len);
    *olen = len;
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    esp_err_t err;
    // Arrange for CHIP-encapsulated ESP32 errors to be translated to text
    Internal::ESP32Utils::RegisterESP32ErrorFormatter();

    // Make sure the LwIP core lock has been initialized
    ReturnErrorOnFailure(Internal::InitLwIPCoreLock());

    err = esp_netif_init();
    if (err != ESP_OK)
    {
        goto exit;
    }

    // Arrange for the ESP event loop to deliver events into the CHIP Device layer.
    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        goto exit;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    {
        wifi_init_config_t cfg;
        uint8_t ap_mac[6];
        wifi_mode_t mode;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
        esp_netif_create_default_wifi_ap();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
        esp_netif_create_default_wifi_sta();

        esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, NULL);
        esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, NULL);
        mStartTime = System::SystemClock().GetMonotonicTimestamp();

        // Initialize the ESP WiFi layer.
        cfg = WIFI_INIT_CONFIG_DEFAULT();
        err = esp_wifi_init(&cfg);
        if (err != ESP_OK)
        {
            goto exit;
        }

        esp_wifi_get_mode(&mode);
        if ((mode == WIFI_MODE_AP) || (mode == WIFI_MODE_APSTA))
        {
            esp_fill_random(ap_mac, sizeof(ap_mac));
            /* Bit 0 of the first octet of MAC Address should always be 0 */
            ap_mac[0] &= (uint8_t) ~0x01;
            err = esp_wifi_set_mac(WIFI_IF_AP, ap_mac);
            if (err != ESP_OK)
            {
                goto exit;
            }
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    ReturnErrorOnFailure(chip::Crypto::add_entropy_source(app_entropy_source, NULL, 16));

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack());

    ReturnErrorOnFailure(System::Clock::InitClock_RealTime());
exit:
    return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
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

    esp_event_loop_delete_default();
}

void PlatformManagerImpl::HandleESPSystemEvent(void * arg, esp_event_base_t eventBase, int32_t eventId, void * eventData)
{
    ChipDeviceEvent event;
    memset(&event, 0, sizeof(event));
    event.Type                         = DeviceEventType::kESPSystemEvent;
    event.Platform.ESPSystemEvent.Base = eventBase;
    event.Platform.ESPSystemEvent.Id   = eventId;
    if (eventBase == IP_EVENT)
    {
        switch (eventId)
        {
        case IP_EVENT_STA_GOT_IP:
            memcpy(&event.Platform.ESPSystemEvent.Data.IpGotIp, eventData, sizeof(event.Platform.ESPSystemEvent.Data.IpGotIp));
            break;
        case IP_EVENT_GOT_IP6:
            memcpy(&event.Platform.ESPSystemEvent.Data.IpGotIp6, eventData, sizeof(event.Platform.ESPSystemEvent.Data.IpGotIp6));
            break;
        case IP_EVENT_AP_STAIPASSIGNED:
            memcpy(&event.Platform.ESPSystemEvent.Data.IpApStaIpAssigned, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.IpApStaIpAssigned));
            break;
        default:
            break;
        }
    }
    else if (eventBase == WIFI_EVENT)
    {
        switch (eventId)
        {
        case WIFI_EVENT_SCAN_DONE:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiStaScanDone, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiStaScanDone));
            break;
        case WIFI_EVENT_STA_CONNECTED:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiStaConnected, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiStaConnected));
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiStaDisconnected, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiStaDisconnected));
            break;
        case WIFI_EVENT_STA_AUTHMODE_CHANGE:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiStaAuthModeChange, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiStaAuthModeChange));
            break;
        case WIFI_EVENT_STA_WPS_ER_PIN:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiStaWpsErPin, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiStaWpsErPin));
            break;
        case WIFI_EVENT_STA_WPS_ER_FAILED:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiStaWpsErFailed, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiStaWpsErFailed));
            break;
        case WIFI_EVENT_AP_STACONNECTED:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiApStaConnected, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiApStaConnected));
            break;
        case WIFI_EVENT_AP_STADISCONNECTED:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiApStaDisconnected, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiApStaDisconnected));
            break;
        case WIFI_EVENT_AP_PROBEREQRECVED:
            memcpy(&event.Platform.ESPSystemEvent.Data.WiFiApProbeReqRecved, eventData,
                   sizeof(event.Platform.ESPSystemEvent.Data.WiFiApProbeReqRecved));
            break;
        default:
            break;
        }
    }

    sInstance.PostEventOrDie(&event);
}

} // namespace DeviceLayer
} // namespace chip
