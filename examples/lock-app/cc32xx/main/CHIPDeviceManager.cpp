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

using namespace ::chip;

namespace chip {

namespace DeviceManager {

using namespace ::chip::DeviceLayer;

void CHIPDeviceManager::DeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    CHIPDeviceManagerCallbacks * cb = reinterpret_cast<CHIPDeviceManagerCallbacks *>(arg);
    if (cb != nullptr)
    {
        cb->DeviceEventCallback(event, reinterpret_cast<intptr_t>(cb));
    }
}

CHIP_ERROR CHIPDeviceManager::Init(CHIPDeviceManagerCallbacks * cb)
{
    mCB = cb;

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(CHIPDeviceManager::DeviceEventHandler, reinterpret_cast<intptr_t>(cb));

    // Start a task to run the CHIP Device event loop.
    return PlatformMgr().StartEventLoopTask();
}

void CHIPDeviceManagerCallbacks::DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg) {}

CHIPDeviceManagerCallbacks::~CHIPDeviceManagerCallbacks() {}

} // namespace DeviceManager
} // namespace chip
