/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/PlatformManagerImpl.h>

#include "esp_event.h"
#include "esp_netif.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR InitPlatformNetworkStack()
{
    // Initialize TCP/IP network interface before PacketBufferHandle::New(), because LwIP pool allocation needs it.
    esp_err_t err = esp_netif_init();
    VerifyOrReturnError(err == ESP_OK, ESP32Utils::MapError(err));

    err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, nullptr);
    VerifyOrReturnError(err == ESP_OK, ESP32Utils::MapError(err));

    return CHIP_NO_ERROR;
}

void DeinitPlatformNetworkStack()
{
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent);
    esp_netif_deinit();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
