/*
 *
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

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/

#include "DeviceCallbacks.h"
#include "common/Esp32ThreadInit.h"
#include "platform/ESP32/OpenthreadLauncher.h"
#include "platform/ThreadStackManager.h"
#include <esp_log.h>
#include <esp_openthread_border_router.h>
#include <esp_openthread_lock.h>
#include <lib/support/logging/CHIPLogging.h>

static const char TAG[] = "thread-br-app-callbacks";

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;
using namespace chip::app::Clusters;

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%" PRIx32 "', EndPoint ID: '0x%x', Attribute ID: '0x%" PRIx32 "'",
             clusterId, endpointId, attributeId);

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void AppDeviceCallbacksDelegate::OnIPv6ConnectivityEstablished(void)
{
    static bool sThreadBRInitialized = false;
    if (!sThreadBRInitialized)
    {
        esp_openthread_lock_acquire(portMAX_DELAY);
        esp_openthread_border_router_init();
        esp_openthread_lock_release();
        sThreadBRInitialized = true;
    }
}
