/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/Optional.h>

#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

bool NotifyCumulativeEnergyMeasured(EndpointId endpointId, const Optional<Structs::EnergyMeasurementStruct::Type> & energyImported,
                                    const Optional<Structs::EnergyMeasurementStruct::Type> & energyExported);

bool NotifyPeriodicEnergyMeasured(EndpointId endpointId, const Optional<Structs::EnergyMeasurementStruct::Type> & energyImported,
                                  const Optional<Structs::EnergyMeasurementStruct::Type> & energyExported);

CHIP_ERROR SetMeasurementAccuracy(EndpointId endpointId, const Structs::MeasurementAccuracyStruct::Type & accuracy);

} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
