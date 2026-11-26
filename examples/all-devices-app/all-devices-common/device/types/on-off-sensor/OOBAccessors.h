/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/on-off-sensor/OnOffSensor.h>
#include <oob-accessors/OOBAccessorRegistry.h>

namespace chip::app {

/**
 * Registers SensorOn, SensorOff, SensorToggle, and SensorIdentify actions for this endpoint.
 * Each action takes an anonymous TLV structure with tag 1 = EndpointId (uint16).
 * SensorIdentify also requires tag 2 = IdentifyTime (uint16, seconds).
 */
void RegisterOOBAccessors(OnOffSensor & device, OOBAccessorRegistry & registry);

} // namespace chip::app
