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
        carbonDioxideConcentrationMeasurementInstance(mEndpointId, CarbonDioxideConcentrationMeasurement::Id, kAllFeatures,
                                                      ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                      ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        carbonMonoxideConcentrationMeasurementInstance(mEndpointId, CarbonMonoxideConcentrationMeasurement::Id, kAllFeatures,
                                                       ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                       ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        nitrogenDioxideConcentrationMeasurementInstance(mEndpointId, NitrogenDioxideConcentrationMeasurement::Id, kAllFeatures,
                                                        ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                        ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm1ConcentrationMeasurementInstance(mEndpointId, Pm1ConcentrationMeasurement::Id, kAllFeatures,
                                            ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                            ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm10ConcentrationMeasurementInstance(mEndpointId, Pm10ConcentrationMeasurement::Id, kAllFeatures,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        pm25ConcentrationMeasurementInstance(mEndpointId, Pm25ConcentrationMeasurement::Id, kAllFeatures,
                                             ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                             ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        radonConcentrationMeasurementInstance(mEndpointId, RadonConcentrationMeasurement::Id, kAllFeatures,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        totalVolatileOrganicCompoundsConcentrationMeasurementInstance(
            mEndpointId, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, kAllFeatures,
            ConcentrationMeasurement::MeasurementMediumEnum::kAir, ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        ozoneConcentrationMeasurementInstance(mEndpointId, OzoneConcentrationMeasurement::Id, kAllFeatures,
                                              ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                              ConcentrationMeasurement::MeasurementUnitEnum::kPpm),
        formaldehydeConcentrationMeasurementInstance(mEndpointId, FormaldehydeConcentrationMeasurement::Id, kAllFeatures,
                                                     ConcentrationMeasurement::MeasurementMediumEnum::kAir,
                                                     ConcentrationMeasurement::MeasurementUnitEnum::kPpm){};

    void Init();

private:
    // Matches the old Instance<true,true,true,true,true,true>: all six features enabled.
    static constexpr BitFlags<ConcentrationMeasurement::Feature> kAllFeatures{
        ConcentrationMeasurement::Feature::kNumericMeasurement, ConcentrationMeasurement::Feature::kLevelIndication,
        ConcentrationMeasurement::Feature::kMediumLevel,        ConcentrationMeasurement::Feature::kCriticalLevel,
        ConcentrationMeasurement::Feature::kPeakMeasurement,    ConcentrationMeasurement::Feature::kAverageMeasurement,
    };

    EndpointId mEndpointId;
    AirQuality::Instance airQualityInstance;
    ConcentrationMeasurement::Instance carbonDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance carbonMonoxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance nitrogenDioxideConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance pm1ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance pm10ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance pm25ConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance radonConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance totalVolatileOrganicCompoundsConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance ozoneConcentrationMeasurementInstance;
    ConcentrationMeasurement::Instance formaldehydeConcentrationMeasurementInstance;
};

} // namespace Clusters
} // namespace app
} // namespace chip
