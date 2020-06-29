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
 * @file EchoDeviceCallbacks.h
 *
 * Implementations for the Device callbacks for this application
 *
 **/

#ifndef ECHO_DEVICE_CALLBACKS_H
#define ECHO_DEVICE_CALLBACKS_H

#include <platform/CHIPDeviceLayer.h>
#include <core/CHIPDevice.h>

using namespace chip::DeviceLayer;
class EchoDeviceCallbacks : public DeviceCallbacks
{
public:
    virtual void DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg);
    virtual void PostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId,
                                             ChipZclAttributeId attributeId, uint8_t mask,
                                             uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                             uint8_t * value);
};

#endif // ECHO_DEVICE_CALLBACKS_H
