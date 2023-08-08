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
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

static const BitMask<Feature, uint32_t> gConcentrationMeasurementAllFeatures(Feature::kNumericMeasurement,
                                                                             Feature::kLevelIndication, Feature::kMediumLevel,
                                                                             Feature::kCriticalLevel, Feature::kPeakMeasurement,
                                                                             Feature::kAverageMeasurement);

static Instance * gCarbonDioxideConcentrationMeasurementServer                 = nullptr;
static Instance * gCarbonMonoxideConcentrationMeasurementServer                = nullptr;
static Instance * gNitrogenDioxideConcentrationMeasurementServer               = nullptr;
static Instance * gPm1ConcentrationMeasurementServer                           = nullptr;
static Instance * gPm10ConcentrationMeasurementServer                          = nullptr;
static Instance * gPm25ConcentrationMeasurementServer                          = nullptr;
static Instance * gRadonConcentrationMeasurementServer                         = nullptr;
static Instance * gTotalVolatileOrganicCompoundsConcentrationMeasurementServer = nullptr;
static Instance * gOzoneConcentrationMeasurementServer                         = nullptr;
static Instance * gFormaldehydeConcentrationMeasurementServer                  = nullptr;

void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gCarbonDioxideConcentrationMeasurementServer == nullptr)
    {

        gCarbonDioxideConcentrationMeasurementServer =
            new Instance(endpoint, CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                         MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gCarbonDioxideConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gCarbonDioxideConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gCarbonDioxideConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gCarbonDioxideConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gCarbonDioxideConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gCarbonDioxideConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gCarbonDioxideConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gCarbonDioxideConcentrationMeasurementServer->SetUncertainty(0.0);
        gCarbonDioxideConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gCarbonMonoxideConcentrationMeasurementServer == nullptr)
    {
        gCarbonMonoxideConcentrationMeasurementServer =
            new Instance(endpoint, CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                         MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gCarbonMonoxideConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gCarbonMonoxideConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gCarbonMonoxideConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gCarbonMonoxideConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gCarbonMonoxideConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gCarbonMonoxideConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gCarbonMonoxideConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gCarbonMonoxideConcentrationMeasurementServer->SetUncertainty(0.0);
        gCarbonMonoxideConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gNitrogenDioxideConcentrationMeasurementServer == nullptr)
    {
        gNitrogenDioxideConcentrationMeasurementServer =
            new Instance(endpoint, NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                         MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gNitrogenDioxideConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gNitrogenDioxideConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gNitrogenDioxideConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gNitrogenDioxideConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gNitrogenDioxideConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gNitrogenDioxideConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gNitrogenDioxideConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gNitrogenDioxideConcentrationMeasurementServer->SetUncertainty(0.0);
        gNitrogenDioxideConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfPm1ConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gPm1ConcentrationMeasurementServer == nullptr)
    {
        gPm1ConcentrationMeasurementServer = new Instance(endpoint, Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                          MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gPm1ConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gPm1ConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gPm1ConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm1ConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm1ConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gPm1ConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm1ConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gPm1ConcentrationMeasurementServer->SetUncertainty(0.0);
        gPm1ConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfPm10ConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gPm10ConcentrationMeasurementServer == nullptr)
    {
        gPm10ConcentrationMeasurementServer = new Instance(endpoint, Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                           MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gPm10ConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gPm10ConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gPm10ConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm10ConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm10ConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gPm10ConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm10ConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gPm10ConcentrationMeasurementServer->SetUncertainty(0.0);
        gPm10ConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfPm25ConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gPm25ConcentrationMeasurementServer == nullptr)
    {
        gPm25ConcentrationMeasurementServer = new Instance(endpoint, Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                           MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gPm25ConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gPm25ConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gPm25ConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm25ConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm25ConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gPm25ConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gPm25ConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gPm25ConcentrationMeasurementServer->SetUncertainty(0.0);
        gPm25ConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfRadonConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gRadonConcentrationMeasurementServer == nullptr)
    {
        gRadonConcentrationMeasurementServer =
            new Instance(endpoint, RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm,
                         gConcentrationMeasurementAllFeatures.Raw());

        gRadonConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gRadonConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gRadonConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gRadonConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gRadonConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gRadonConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gRadonConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gRadonConcentrationMeasurementServer->SetUncertainty(0.0);
        gRadonConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gTotalVolatileOrganicCompoundsConcentrationMeasurementServer == nullptr)
    {
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer =
            new Instance(endpoint, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                         MeasurementUnitEnum::kPpm, gConcentrationMeasurementAllFeatures.Raw());

        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetUncertainty(0.0);
        gTotalVolatileOrganicCompoundsConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfOzoneConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gOzoneConcentrationMeasurementServer == nullptr)
    {
        gOzoneConcentrationMeasurementServer =
            new Instance(endpoint, OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm,
                         gConcentrationMeasurementAllFeatures.Raw());

        gOzoneConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gOzoneConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gOzoneConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gOzoneConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gOzoneConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gOzoneConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gOzoneConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gOzoneConcentrationMeasurementServer->SetUncertainty(0.0);
        gOzoneConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(chip::EndpointId endpoint)
{
    if (gFormaldehydeConcentrationMeasurementServer == nullptr)
    {
        gFormaldehydeConcentrationMeasurementServer =
            new Instance(endpoint, FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm,
                         gConcentrationMeasurementAllFeatures.Raw());

        gFormaldehydeConcentrationMeasurementServer->SetMinMeasuredValue(DataModel::Nullable<float>(0.0));
        gFormaldehydeConcentrationMeasurementServer->SetMaxMeasuredValue(DataModel::Nullable<float>(1000.0));
        gFormaldehydeConcentrationMeasurementServer->SetMeasuredValue(DataModel::Nullable<float>(1.0));
        gFormaldehydeConcentrationMeasurementServer->SetPeakMeasuredValue(DataModel::Nullable<float>(1.0));
        gFormaldehydeConcentrationMeasurementServer->SetPeakMeasuredValueWindow(320);
        gFormaldehydeConcentrationMeasurementServer->SetAverageMeasuredValue(DataModel::Nullable<float>(1.0));
        gFormaldehydeConcentrationMeasurementServer->SetAverageMeasuredValueWindow(320);
        gFormaldehydeConcentrationMeasurementServer->SetUncertainty(0.0);
        gFormaldehydeConcentrationMeasurementServer->SetLevel(LevelValueEnum::kLow);
    }
}
