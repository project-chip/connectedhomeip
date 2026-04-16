/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/concentration-measurement-server/CodegenIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

// All 10 aliased clusters share the same feature set in this example app.
static constexpr BitFlags<Feature> kAllFeatures(Feature::kNumericMeasurement, Feature::kLevelIndication, Feature::kMediumLevel,
                                                Feature::kCriticalLevel, Feature::kPeakMeasurement, Feature::kAverageMeasurement);

static Instance gCarbonDioxideConcentrationMeasurementInstance(EndpointId(1), CarbonDioxideConcentrationMeasurement::Id,
                                                               kAllFeatures, MeasurementMediumEnum::kAir,
                                                               MeasurementUnitEnum::kPpm);

static Instance gCarbonMonoxideConcentrationMeasurementInstance(EndpointId(1), CarbonMonoxideConcentrationMeasurement::Id,
                                                                kAllFeatures, MeasurementMediumEnum::kAir,
                                                                MeasurementUnitEnum::kPpm);

static Instance gNitrogenDioxideConcentrationMeasurementInstance(EndpointId(1), NitrogenDioxideConcentrationMeasurement::Id,
                                                                 kAllFeatures, MeasurementMediumEnum::kAir,
                                                                 MeasurementUnitEnum::kPpm);

static Instance gPm1ConcentrationMeasurementInstance(EndpointId(1), Pm1ConcentrationMeasurement::Id, kAllFeatures,
                                                     MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm10ConcentrationMeasurementInstance(EndpointId(1), Pm10ConcentrationMeasurement::Id, kAllFeatures,
                                                      MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm25ConcentrationMeasurementInstance(EndpointId(1), Pm25ConcentrationMeasurement::Id, kAllFeatures,
                                                      MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gRadonConcentrationMeasurementInstance(EndpointId(1), RadonConcentrationMeasurement::Id, kAllFeatures,
                                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance(
    EndpointId(1), TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, kAllFeatures, MeasurementMediumEnum::kAir,
    MeasurementUnitEnum::kPpm);

static Instance gOzoneConcentrationMeasurementInstance(EndpointId(1), OzoneConcentrationMeasurement::Id, kAllFeatures,
                                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gFormaldehydeConcentrationMeasurementInstance(EndpointId(1), FormaldehydeConcentrationMeasurement::Id, kAllFeatures,
                                                              MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gCarbonDioxideConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(2.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gCarbonMonoxideConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gNitrogenDioxideConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gPm1ConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gPm10ConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gPm25ConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gRadonConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gOzoneConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto & d = gFormaldehydeConcentrationMeasurementInstance.GetDelegate();
    d.HandleNewMinMeasuredValue(MakeNullable(0.0f));
    d.HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    d.HandleNewMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValue(MakeNullable(1.0f));
    d.HandleNewPeakMeasuredValueWindow(320);
    d.HandleNewAverageMeasuredValue(MakeNullable(1.0f));
    d.HandleNewAverageMeasuredValueWindow(320);
    d.HandleNewUncertainty(0.0f);
    d.HandleNewLevelValue(LevelValueEnum::kLow);
}

void emberAfCarbonDioxideConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfCarbonMonoxideConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfNitrogenDioxideConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfPm1ConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfPm10ConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfPm25ConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfRadonConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfOzoneConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
void emberAfFormaldehydeConcentrationMeasurementClusterShutdownCallback(EndpointId endpoint) {}
