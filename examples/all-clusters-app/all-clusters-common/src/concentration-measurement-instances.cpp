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
    gCarbonDioxideConcentrationMeasurementInstance.Init();

    gCarbonDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gCarbonDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gCarbonDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(2.0f));
    gCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gCarbonDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gCarbonDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gCarbonDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gCarbonDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonMonoxideConcentrationMeasurementInstance.Init();

    gCarbonMonoxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gCarbonMonoxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gCarbonMonoxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gCarbonMonoxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gCarbonMonoxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gCarbonMonoxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gCarbonMonoxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gNitrogenDioxideConcentrationMeasurementInstance.Init();

    gNitrogenDioxideConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gNitrogenDioxideConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gNitrogenDioxideConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gNitrogenDioxideConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gNitrogenDioxideConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm1ConcentrationMeasurementInstance.Init();

    gPm1ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gPm1ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gPm1ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gPm1ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gPm1ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gPm1ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm10ConcentrationMeasurementInstance.Init();

    gPm10ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gPm10ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gPm10ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gPm10ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gPm10ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gPm10ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gPm10ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gPm10ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gPm10ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm25ConcentrationMeasurementInstance.Init();

    gPm25ConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gPm25ConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gPm25ConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gPm25ConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gPm25ConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gPm25ConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gRadonConcentrationMeasurementInstance.Init();

    gRadonConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gRadonConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gRadonConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gRadonConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gRadonConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gRadonConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gRadonConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gRadonConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gRadonConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.Init();

    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gOzoneConcentrationMeasurementInstance.Init();

    gOzoneConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gOzoneConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gOzoneConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gOzoneConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gOzoneConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gOzoneConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gOzoneConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gOzoneConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gOzoneConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gFormaldehydeConcentrationMeasurementInstance.Init();

    gFormaldehydeConcentrationMeasurementInstance.SetMinMeasuredValue(MakeNullable(0.0f));
    gFormaldehydeConcentrationMeasurementInstance.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gFormaldehydeConcentrationMeasurementInstance.SetMeasuredValue(MakeNullable(1.0f));
    gFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValue(MakeNullable(1.0f));
    gFormaldehydeConcentrationMeasurementInstance.SetPeakMeasuredValueWindow(320);
    gFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValue(MakeNullable(1.0f));
    gFormaldehydeConcentrationMeasurementInstance.SetAverageMeasuredValueWindow(320);
    gFormaldehydeConcentrationMeasurementInstance.SetUncertainty(0.0f);
    gFormaldehydeConcentrationMeasurementInstance.SetLevelValue(LevelValueEnum::kLow);
}
