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

static Instance gCarbonDioxideConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gCarbonMonoxideConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gNitrogenDioxideConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm1ConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm10ConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gPm25ConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gRadonConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gTotalVolatileOrganicCompoundsConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
        MeasurementUnitEnum::kPpm);

static Instance gOzoneConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

static Instance gFormaldehydeConcentrationMeasurementServer =
    CreateNumericMeasurementAndLevelIndicationConcentrationCluster<true, true, true, true>(
        EndpointId(1), FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);

void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonDioxideConcentrationMeasurementServer.Init();

    gCarbonDioxideConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gCarbonDioxideConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gCarbonDioxideConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(2.0f));
    gCarbonDioxideConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gCarbonDioxideConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gCarbonDioxideConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gCarbonDioxideConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gCarbonDioxideConcentrationMeasurementServer.SetUncertainty(0.0f);
    gCarbonDioxideConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonMonoxideConcentrationMeasurementServer.Init();

    gCarbonMonoxideConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gCarbonMonoxideConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gCarbonMonoxideConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gCarbonMonoxideConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gCarbonMonoxideConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gCarbonMonoxideConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gCarbonMonoxideConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gCarbonMonoxideConcentrationMeasurementServer.SetUncertainty(0.0f);
    gCarbonMonoxideConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gNitrogenDioxideConcentrationMeasurementServer.Init();

    gNitrogenDioxideConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gNitrogenDioxideConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gNitrogenDioxideConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gNitrogenDioxideConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gNitrogenDioxideConcentrationMeasurementServer.SetUncertainty(0.0f);
    gNitrogenDioxideConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm1ConcentrationMeasurementServer.Init();

    gPm1ConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gPm1ConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gPm1ConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gPm1ConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gPm1ConcentrationMeasurementServer.SetUncertainty(0.0f);
    gPm1ConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm10ConcentrationMeasurementServer.Init();

    gPm10ConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gPm10ConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gPm10ConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gPm10ConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gPm10ConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gPm10ConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gPm10ConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gPm10ConcentrationMeasurementServer.SetUncertainty(0.0f);
    gPm10ConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm25ConcentrationMeasurementServer.Init();

    gPm25ConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gPm25ConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gPm25ConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gPm25ConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gPm25ConcentrationMeasurementServer.SetUncertainty(0.0f);
    gPm25ConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gRadonConcentrationMeasurementServer.Init();

    gRadonConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gRadonConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gRadonConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gRadonConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gRadonConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gRadonConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gRadonConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gRadonConcentrationMeasurementServer.SetUncertainty(0.0f);
    gRadonConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.Init();

    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetUncertainty(0.0f);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gOzoneConcentrationMeasurementServer.Init();

    gOzoneConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gOzoneConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gOzoneConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gOzoneConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gOzoneConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gOzoneConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gOzoneConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gOzoneConcentrationMeasurementServer.SetUncertainty(0.0f);
    gOzoneConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gFormaldehydeConcentrationMeasurementServer.Init();

    gFormaldehydeConcentrationMeasurementServer.SetMinMeasuredValue(MakeNullable(0.0f));
    gFormaldehydeConcentrationMeasurementServer.SetMaxMeasuredValue(MakeNullable(1000.0f));
    gFormaldehydeConcentrationMeasurementServer.SetMeasuredValue(MakeNullable(1.0f));
    gFormaldehydeConcentrationMeasurementServer.SetPeakMeasuredValue(MakeNullable(1.0f));
    gFormaldehydeConcentrationMeasurementServer.SetPeakMeasuredValueWindow(320);
    gFormaldehydeConcentrationMeasurementServer.SetAverageMeasuredValue(MakeNullable(1.0f));
    gFormaldehydeConcentrationMeasurementServer.SetAverageMeasuredValueWindow(320);
    gFormaldehydeConcentrationMeasurementServer.SetUncertainty(0.0f);
    gFormaldehydeConcentrationMeasurementServer.SetLevelValue(LevelValueEnum::kLow);
}
