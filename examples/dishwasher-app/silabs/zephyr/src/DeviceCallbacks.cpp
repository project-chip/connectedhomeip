/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "DeviceCallbacks.h"

#include <lib/support/logging/CHIPLogging.h>

void DishwasherApp::DeviceCallbacks::PostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                 chip::AttributeId attributeId, uint8_t type, uint16_t size,
                                                                 uint8_t * value)
{
    // Skeleton: dishwasher-specific attribute handling added in a follow-up commit.
    (void) endpoint;
    (void) clusterId;
    (void) attributeId;
    (void) type;
    (void) size;
    (void) value;
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::Zephyr::App::GetDeviceCallbacks()
{
    static DishwasherApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}
