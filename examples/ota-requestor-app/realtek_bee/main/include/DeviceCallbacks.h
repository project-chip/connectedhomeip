/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file DeviceCallbacks.h
 *
 * Implementations for the DeviceManager callbacks for this application
 *
 **/

#pragma once

#include "CHIPDeviceManager.h"
// #include <app/util/af-types.h>
// #include <app/util/basic-types.h>
#include <platform/CHIPDeviceLayer.h>

class DeviceCallbacks : public chip::DeviceManager::CHIPDeviceManagerCallbacks
{
public:
    void DeviceEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg) override;
    static void UpdateStatusLED();

private:
    void OnInternetConnectivityChange(const chip::DeviceLayer::ChipDeviceEvent * event);
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
};
