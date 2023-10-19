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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/air-quality-server/air-quality-server.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>

#pragma once

namespace chip {
namespace app {
namespace Clusters {

class AirQualitySensorManager
{
public:
    AirQualitySensorManager(EndpointId aEndpointId) :
        mEndpointId(aEndpointId),
        airQualityInstance(mEndpointId,
                           BitMask<AirQuality::Feature, uint32_t>(AirQuality::Feature::kModerate, AirQuality::Feature::kFair,
                                                                  AirQuality::Feature::kVeryPoor,
                                                                  AirQuality::Feature::kExtremelyPoor)),
        carbonDioxideConcentrationMeasurementInstance(mEndpointId, CarbonDioxideConcentrationMeasurement::Id,
                                                      ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                      ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        carbonMonoxideConcentrationMeasurementInstance(mEndpointId, CarbonMonoxideConcentrationMeasurement::Id,
                                                       ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                       ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        nitrogenDioxideConcentrationMeasurementInstance(mEndpointId, NitrogenDioxideConcentrationMeasurement::Id,
                                                        ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                        ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm1ConcentrationMeasurementInstance(mEndpointId, Pm1ConcentrationMeasurement::Id,
                                            ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                            ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm10ConcentrationMeasurementInstance(mEndpointId, Pm10ConcentrationMeasurement::Id,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm25ConcentrationMeasurementInstance(mEndpointId, Pm25ConcentrationMeasurement::Id,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        radonConcentrationMeasurementInstance(mEndpointId, RadonConcentrationMeasurement::Id,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        totalVolatileOrganicCompoundsConcentrationMeasurementInstance(
            mEndpointId, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
            ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        ozoneConcentrationMeasurementInstance(mEndpointId, OzoneConcentrationMeasurement::Id,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        formaldehydeConcentrationMeasurementInstance(mEndpointId, FormaldehydeConcentrationMeasurement::Id,
                                                     ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                     ConcentrationMeasurement::MeasurementUnitEnum::kPpm){};

    void Init();

private:
    EndpointId mEndpointId;
    AirQuality::Instance airQualityInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> carbonDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> carbonMonoxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> nitrogenDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> pm1ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> pm10ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> pm25ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> radonConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true>
        totalVolatileOrganicCompoundsConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> ozoneConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance<true, true, true, true, true, true> formaldehydeConcentrationMeasurementInstance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
