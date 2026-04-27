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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

static Instance gCarbonDioxideConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gCarbonMonoxideConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gNitrogenDioxideConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm1ConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm10ConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm25ConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gRadonConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
        MeasurementUnitEnum::kPpm);

static Instance gOzoneConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gFormaldehydeConcentrationMeasurementInstance =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.Init();

    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetUncertainty(0.0f);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
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
