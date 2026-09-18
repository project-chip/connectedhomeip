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

#pragma once

#include "DeviceScreenRegistry.h"
#include <device/types/aggregator/Aggregator.h>
#include <device/types/air-purifier/AirPurifier.h>
#include <device/types/boolean-state-sensor/BooleanStateSensor.h>
#include <device/types/bridged-node/BridgedNode.h>
#include <device/types/chime/Chime.h>
#include <device/types/color-temperature-light/ColorTemperatureLight.h>
#include <device/types/dimmable-light/DimmableLight.h>
#include <device/types/dimmable-plug-in-unit/DimmablePlugInUnit.h>
#include <device/types/extended-color-light/ExtendedColorLight.h>
#include <device/types/extractor-hood/ExtractorHood.h>
#include <device/types/fan/Fan.h>
#include <device/types/mounted-dimmable-load-control/MountedDimmableLoadControl.h>
#include <device/types/mounted-on-off-control/MountedOnOffControl.h>
#include <device/types/occupancy-sensor/OccupancySensor.h>
#include <device/types/on-off-light/OnOffLight.h>
#include <device/types/on-off-plug-in-unit/OnOffPlugInUnit.h>
#include <device/types/temperature-sensor/TemperatureSensor.h>

namespace chip::app {

void RegisterDeviceScreen(OnOffLight & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(DimmableLight & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(ColorTemperatureLight & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(ExtendedColorLight & device, DeviceScreenRegistry & registry);

void RegisterDeviceScreen(OnOffPlugInUnit & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(DimmablePlugInUnit & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(MountedOnOffControl & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(MountedDimmableLoadControl & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(BooleanStateSensor & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(OccupancySensor & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(TemperatureSensor & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(Fan & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(AirPurifier & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(ExtractorHood & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(Chime & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(BridgedNode & device, DeviceScreenRegistry & registry);
void RegisterDeviceScreen(Aggregator & device, DeviceScreenRegistry & registry);

// Fallback for device types that have no screen. Registers an entry without a render
// function so the device still shows up in the list, marked as having no UI. The endpoint
// is passed in because multi-endpoint devices do not expose a single endpoint id.
void RegisterMissingDeviceScreen(DeviceInterface & device, EndpointId endpointId, DeviceScreenRegistry & registry);

} // namespace chip::app
