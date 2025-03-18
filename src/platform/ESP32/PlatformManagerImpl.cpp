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
extern CHIP_ERROR InitLwIPCoreLock();
}

PlatformManagerImpl PlatformManagerImpl::sInstance;

static int app_entropy_source(void * data, unsigned char * output, size_t len, size_t * olen)
{
    esp_fill_random(output, len);
    *olen = len;
    return 0;
}

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    // Arrange for CHIP-encapsulated ESP32 errors to be translated to text
    Internal::ESP32Utils::RegisterESP32ErrorFormatter();
    // Make sure the LwIP core lock has been initialized
    ReturnErrorOnFailure(Internal::InitLwIPCoreLock());

    // Initialize TCP/IP network interface, which internally initializes LwIP stack. We have to
    // call this before the usage of PacketBufferHandle::New() because in case of LwIP-based pool
    // allocator, the LwIP pool allocator uses the LwIP stack.
    esp_err_t err = esp_netif_init();
    VerifyOrReturnError(err == ESP_OK, Internal::ESP32Utils::MapError(err));

    // Arrange for the ESP event loop to deliver events into the CHIP Device layer.
    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, nullptr);
    VerifyOrReturnError(err == ESP_OK, Internal::ESP32Utils::MapError(err));

    mStartTime = System::SystemClock().GetMonotonicTimestamp();
    ReturnErrorOnFailure(chip::Crypto::add_entropy_source(app_entropy_source, nullptr, 16));

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack());

    ReturnErrorOnFailure(System::Clock::InitClock_RealTime());
    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown()
{
    uint32_t totalOperationalHours = 0;

    if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
    {
        ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
    }

    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();

    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent);
    esp_netif_deinit();
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
        ChipLogProgress(DeviceLayer, "Posting ESPSystemEvent: IP Event with eventId : %ld", eventId);
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
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    else if (eventBase == WIFI_EVENT)
    {
        ChipLogProgress(DeviceLayer, "Posting ESPSystemEvent: Wifi Event with eventId : %ld", eventId);
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
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
    else
    {
        ChipLogProgress(DeviceLayer, "Posting ESPSystemEvent with eventId : %ld", eventId);
    }
    sInstance.PostEventOrDie(&event);
}

} // namespace DeviceLayer
} // namespace chip
