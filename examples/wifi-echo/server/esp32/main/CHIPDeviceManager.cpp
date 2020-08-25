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
#include <lib/support/CodeUtils.h>
#include <setup_payload/SetupPayload.h>
#include <support/ErrorStr.h>

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
    CHIP_ERROR err;
    mCB = cb;

    // Initialize the CHIP stack.
    err = PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    switch (static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE))
    {
    case RendezvousInformationFlags::kBLE:
        ConnectivityMgr().SetBLEAdvertisingEnabled(ConnectivityManager::kCHIPoBLEServiceMode_Enabled);
        break;

    case RendezvousInformationFlags::kWiFi:
        ConnectivityMgr().SetBLEAdvertisingEnabled(ConnectivityManager::kCHIPoBLEServiceMode_Disabled);
        ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled);
        break;

    case RendezvousInformationFlags::kNone:
        // If rendezvous is bypassed, enable SoftAP so that the device can still
        // be communicated with via its SoftAP as needed.
        ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled);
        break;

    default:
        break;
    }

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(CHIPDeviceManager::CommonDeviceEventHandler, reinterpret_cast<intptr_t>(cb));

    // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(err);

exit:
    return err;
}

extern "C" {
void emberAfPostAttributeChangeCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    CHIPDeviceManagerCallbacks * cb = CHIPDeviceManager::GetInstance().GetCHIPDeviceManagerCallbacks();
    if (cb != nullptr)
    {
        cb->PostAttributeChangeCallback(endpoint, clusterId, attributeId, mask, manufacturerCode, type, size, value);
    }
}
} // extern "C"

} // namespace DeviceManager
} // namespace chip
