/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements the CHIP Device Interface that is used by
 *      applications to interact with the CHIP stack
 *
 */

#include <stdlib.h>

#include "CHIPDeviceManager.h"
#include <app/ConcreteAttributePath.h>
#include <app/util/basic-types.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <setup_payload/SetupPayload.h>

#include "esp_log.h"

using namespace ::chip;

namespace chip {

namespace DeviceManager {

using namespace ::chip::DeviceLayer;

void CHIPDeviceManager::CommonDeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    CHIPDeviceManagerCallbacks * cb = reinterpret_cast<CHIPDeviceManagerCallbacks *>(arg);
    if (cb != nullptr)
    {
        cb->DeviceEventCallback(event, reinterpret_cast<intptr_t>(cb));
    }
}

/**
 *
 */
CHIP_ERROR CHIPDeviceManager::Init(CHIPDeviceManagerCallbacks * cb)
{
    mCB                              = cb;
    RendezvousInformationFlags flags = RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE);

    ReturnErrorOnFailure(Platform::MemoryInit());

    // Initialize the CHIP stack.
    ReturnErrorOnFailure(PlatformMgr().InitChipStack());

    if (flags.Has(RendezvousInformationFlag::kBLE))
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);
    }
    else if (flags.Has(RendezvousInformationFlag::kSoftAP))
    {
        // TODO(cecille): Fix for the case where BLE and SoftAP are both enabled.`
        ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled);
    }

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(CHIPDeviceManager::CommonDeviceEventHandler, reinterpret_cast<intptr_t>(cb));

    // Start a task to run the CHIP Device background event loop.
    ReturnErrorOnFailure(PlatformMgr().StartBackgroundEventLoopTask());

    // Start a task to run the CHIP Device event loop.
    return PlatformMgr().StartEventLoopTask();
}
} // namespace DeviceManager
} // namespace chip

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path, uint8_t type, uint16_t size, uint8_t * value)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    const char * name = pcTaskGetName(task);
    if (strcmp(name, "CHIP"))
    {
        ESP_LOGE("all-clusters-app", "Attribute changed on non-Matter task '%s'\n", name);
    }

    chip::DeviceManager::CHIPDeviceManagerCallbacks * cb =
        chip::DeviceManager::CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();
    if (cb != nullptr)
    {
        cb->PostAttributeChangeCallback(path.mEndpointId, path.mClusterId, path.mAttributeId, type, size, value);
    }
}
