/*
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

#include "OnOffPlugWithSensors.h"

#include <devices/Types.h>

namespace chip::app {

namespace {
constexpr DataModel::DeviceTypeEntry kOnOffPlugWithSensorsDeviceTypes[] = {
    Device::Type::kOnOffPlugInUnit,
    Device::Type::kElectricalSensor,
};
} // namespace

OnOffPlugWithSensors::OnOffPlugWithSensors(const Context & context, TestEventTriggerDelegate & testEventTriggerDelegate) :
    OnOffPlugInUnit(Span<const DataModel::DeviceTypeEntry>(kOnOffPlugWithSensorsDeviceTypes), context),
    mElectricalSensor(context.timerDelegate, testEventTriggerDelegate)
{}

CHIP_ERROR OnOffPlugWithSensors::RegisterAdditionalClusters(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    return mElectricalSensor.RegisterClusters(endpoint, provider);
}

void OnOffPlugWithSensors::UnregisterAdditionalClusters(CodeDrivenDataModelProvider & provider)
{
    mElectricalSensor.UnregisterClusters(provider);
}

} // namespace chip::app
