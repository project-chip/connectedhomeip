/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#include "CommonDeviceCallbacks.h"

namespace ThermostatApp {
class DeviceCallbacks : public chip::NXP::App::CommonDeviceCallbacks
{
public:
    // This returns an instance of this class.
    static DeviceCallbacks & GetDefaultInstance();

    void PostAttributeChangeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                     uint8_t type, uint16_t size, uint8_t * value);

private:
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
};
} // namespace ThermostatApp

/**
 * Returns the application-specific implementation of the CommonDeviceCallbacks object.
 *
 * Applications can use this to gain access to features of the CommonDeviceCallbacks
 * that are specific to the selected application.
 */
chip::DeviceManager::CHIPDeviceManagerCallbacks & GetDeviceCallbacks();
