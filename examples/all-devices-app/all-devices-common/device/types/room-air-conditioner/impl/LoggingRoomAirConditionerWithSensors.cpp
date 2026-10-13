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

#include <device/types/room-air-conditioner/impl/LoggingRoomAirConditionerWithSensors.h>

namespace chip::app {

namespace {

const EndpointComposition::SemanticTag kIndoorTag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = CommonNamespace::kLocationId,
    .tag         = static_cast<uint8_t>(Clusters::Globals::LocationTag::kIndoor),
};
const EndpointComposition::SemanticTag kOutdoorTag = {
    .mfgCode     = DataModel::NullNullable,
    .namespaceID = CommonNamespace::kLocationId,
    .tag         = static_cast<uint8_t>(Clusters::Globals::LocationTag::kOutdoor),
};

} // namespace

LoggingRoomAirConditionerWithSensors::LoggingRoomAirConditionerWithSensors(TimerDelegate & timerDelegate, FabricTable & fabricTable,
                                                                           std::optional<EndpointComposition::SemanticTag> tag) :
    LoggingRoomAirConditioner(timerDelegate, fabricTable, tag),
    mIndoorSensor(timerDelegate,
                  { .minMeasuredValue = DataModel::MakeNullable<int16_t>(-4000),
                    .maxMeasuredValue = DataModel::MakeNullable<int16_t>(12500) }),
    mOutdoorSensor(timerDelegate,
                   { .minMeasuredValue = DataModel::MakeNullable<int16_t>(-4000),
                     .maxMeasuredValue = DataModel::MakeNullable<int16_t>(12500) })
{}

CHIP_ERROR LoggingRoomAirConditionerWithSensors::RegisterAdditionalEndpoints(EndpointIdAllocator & allocator,
                                                                             CodeDrivenDataModelProvider & provider)
{
    ReturnErrorOnFailure(mIndoorSensor.Register(
        allocator.Allocate(), provider,
        EndpointComposition(GetEndpointId(), DataModel::EndpointCompositionPattern::kTree, Span(&kIndoorTag, 1))));
    ReturnErrorOnFailure(mOutdoorSensor.Register(
        allocator.Allocate(), provider,
        EndpointComposition(GetEndpointId(), DataModel::EndpointCompositionPattern::kTree, Span(&kOutdoorTag, 1))));

    // Fixed simulated readings in hundredths of a degree Celsius. These sensors
    // remain available independently of the parent's OnOff state.
    ReturnErrorOnFailure(mIndoorSensor.TemperatureMeasurementCluster().SetMeasuredValue(DataModel::MakeNullable<int16_t>(2500)));
    return mOutdoorSensor.TemperatureMeasurementCluster().SetMeasuredValue(DataModel::MakeNullable<int16_t>(1500));
}

void LoggingRoomAirConditionerWithSensors::UnregisterAdditionalEndpoints(CodeDrivenDataModelProvider & provider)
{
    // Registration may have failed after only one child was added.
    if (mOutdoorSensor.GetEndpointId() != kInvalidEndpointId)
    {
        mOutdoorSensor.Unregister(provider);
    }
    if (mIndoorSensor.GetEndpointId() != kInvalidEndpointId)
    {
        mIndoorSensor.Unregister(provider);
    }
}

} // namespace chip::app
