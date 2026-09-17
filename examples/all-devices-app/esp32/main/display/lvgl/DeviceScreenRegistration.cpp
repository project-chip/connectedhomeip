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

#include "DeviceScreenRegistration.h"
#include "devices/DimmableLightScreen.h"
#include "devices/OnOffLightScreen.h"

namespace chip::app {

void RegisterDeviceScreen(OnOffLoad & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "On/Off Light",
        .endpointId = device.GetEndpointId(),
        .deviceType = "on-off-light",
        .renderFn   = [&device](lv_obj_t * parent) { ShowOnOffLightScreen(parent, device); },
    });
}

void RegisterDeviceScreen(DimmableLoad & device, DeviceScreenRegistry & registry)
{
    registry.Register({
        .title      = "Dimmable Light",
        .endpointId = device.GetEndpointId(),
        .deviceType = "dimmable-light",
        .renderFn   = [&device](lv_obj_t * parent) { ShowDimmableLightScreen(parent, device); },
    });
}

} // namespace chip::app
