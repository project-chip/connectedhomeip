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

#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementDelegate.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

static constexpr BitFlags<Feature> kAllFeatures{
    Feature::kNumericMeasurement, Feature::kLevelIndication, Feature::kMediumLevel,
    Feature::kCriticalLevel,      Feature::kPeakMeasurement, Feature::kAverageMeasurement,
};

struct ConcentrationSensor
{
    DefaultDelegate delegate;
    ConcentrationMeasurementCluster cluster;
    ServerClusterRegistration registration;

    ConcentrationSensor(EndpointId ep, ClusterId clusterId, MeasurementMediumEnum medium, MeasurementUnitEnum unit) :
        delegate(medium, unit, MakeNullable(0.0f), MakeNullable(1000.0f), 0.0f), cluster(ep, clusterId, kAllFeatures, delegate),
        registration(cluster)
    {}
};

static ConcentrationSensor * gCarbonDioxideInstance                 = nullptr;
static ConcentrationSensor * gCarbonMonoxideInstance                = nullptr;
static ConcentrationSensor * gNitrogenDioxideInstance               = nullptr;
static ConcentrationSensor * gPm1Instance                           = nullptr;
static ConcentrationSensor * gPm10Instance                          = nullptr;
static ConcentrationSensor * gPm25Instance                          = nullptr;
static ConcentrationSensor * gRadonInstance                         = nullptr;
static ConcentrationSensor * gTotalVolatileOrganicCompoundsInstance = nullptr;
static ConcentrationSensor * gOzoneInstance                         = nullptr;
static ConcentrationSensor * gFormaldehydeInstance                  = nullptr;

static void RegisterSensor(ConcentrationSensor * sensor)
{
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(sensor->registration));
}

void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonDioxideInstance = new ConcentrationSensor(endpoint, CarbonDioxideConcentrationMeasurement::Id,
                                                     MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    RegisterSensor(gCarbonDioxideInstance);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideInstance->cluster.SetMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonMonoxideInstance = new ConcentrationSensor(endpoint, CarbonMonoxideConcentrationMeasurement::Id,
                                                      MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    RegisterSensor(gCarbonMonoxideInstance);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideInstance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gNitrogenDioxideInstance = new ConcentrationSensor(endpoint, NitrogenDioxideConcentrationMeasurement::Id,
                                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    RegisterSensor(gNitrogenDioxideInstance);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideInstance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm1Instance =
        new ConcentrationSensor(endpoint, Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    RegisterSensor(gPm1Instance);
    TEMPORARY_RETURN_IGNORED gPm1Instance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1Instance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1Instance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm1Instance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1Instance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm1Instance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm10Instance =
        new ConcentrationSensor(endpoint, Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    RegisterSensor(gPm10Instance);
    TEMPORARY_RETURN_IGNORED gPm10Instance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10Instance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10Instance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm10Instance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10Instance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm10Instance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm25Instance =
        new ConcentrationSensor(endpoint, Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    RegisterSensor(gPm25Instance);
    TEMPORARY_RETURN_IGNORED gPm25Instance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25Instance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25Instance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm25Instance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25Instance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gPm25Instance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gRadonInstance = new ConcentrationSensor(endpoint, RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                             MeasurementUnitEnum::kPpm);
    RegisterSensor(gRadonInstance);
    TEMPORARY_RETURN_IGNORED gRadonInstance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gRadonInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gRadonInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gTotalVolatileOrganicCompoundsInstance =
        new ConcentrationSensor(endpoint, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                MeasurementUnitEnum::kPpm);
    RegisterSensor(gTotalVolatileOrganicCompoundsInstance);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsInstance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gOzoneInstance = new ConcentrationSensor(endpoint, OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                             MeasurementUnitEnum::kPpm);
    RegisterSensor(gOzoneInstance);
    TEMPORARY_RETURN_IGNORED gOzoneInstance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gOzoneInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gOzoneInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
}

void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gFormaldehydeInstance = new ConcentrationSensor(endpoint, FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                    MeasurementUnitEnum::kPpm);
    RegisterSensor(gFormaldehydeInstance);
    TEMPORARY_RETURN_IGNORED gFormaldehydeInstance->cluster.SetMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeInstance->cluster.SetPeakMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeInstance->cluster.SetPeakMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gFormaldehydeInstance->cluster.SetAverageMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeInstance->cluster.SetAverageMeasuredValueWindow(320);
    TEMPORARY_RETURN_IGNORED gFormaldehydeInstance->cluster.SetLevelValue(LevelValueEnum::kLow);
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
