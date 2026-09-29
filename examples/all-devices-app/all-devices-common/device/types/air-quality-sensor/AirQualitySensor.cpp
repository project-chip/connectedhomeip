/*
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

#include <device/types/air-quality-sensor/AirQualitySensor.h>

namespace chip {
namespace app {
namespace AirQualitySensorInternal {

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

ConcentrationMeasurementCluster::Config DefaultConcentrationConfig(ClusterId clusterId)
{
    MeasurementUnitEnum unit = MeasurementUnitEnum::kPpm;
    float maxMeasured        = 1000.0f;

    switch (clusterId)
    {
    case CarbonDioxideConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kPpm;
        maxMeasured = 5000.0f;
        break;
    case Pm25ConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kUgm3;
        maxMeasured = 1000.0f;
        break;
    case TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kPpm;
        maxMeasured = 10000.0f;
        break;
    case CarbonMonoxideConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kPpm;
        maxMeasured = 1000.0f;
        break;
    case NitrogenDioxideConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kPpm;
        maxMeasured = 1000.0f;
        break;
    case OzoneConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kPpm;
        maxMeasured = 1000.0f;
        break;
    case FormaldehydeConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kPpm;
        maxMeasured = 1000.0f;
        break;
    case Pm1ConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kUgm3;
        maxMeasured = 1000.0f;
        break;
    case Pm10ConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kUgm3;
        maxMeasured = 1000.0f;
        break;
    case RadonConcentrationMeasurement::Id:
        unit        = MeasurementUnitEnum::kBqm3;
        maxMeasured = 10000.0f;
        break;
    default:
        break;
    }

    return ConcentrationMeasurementCluster::Config{
        .clusterId   = clusterId,
        .features    = BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement, Feature::kAverageMeasurement,
                                      Feature::kLevelIndication),
        .medium      = MeasurementMediumEnum::kAir,
        .unit        = unit,
        .minMeasured = DataModel::MakeNullable(0.0f),
        .maxMeasured = DataModel::MakeNullable(maxMeasured),
        .uncertainty = 0.0f,
    };
}

} // namespace AirQualitySensorInternal
} // namespace app
} // namespace chip
