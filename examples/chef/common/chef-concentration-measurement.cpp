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
#include "chef-concentration-measurement.h"
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementDelegate.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/util/config.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;

#if defined(MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                  \
    defined(MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                   \
    defined(MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                 \
    defined(MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                            \
    defined(MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER) ||                                                           \
    defined(MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                     \
    defined(MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER) ||                                                              \
    defined(MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER) ||                                                             \
    defined(MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER) ||                                 \
    defined(MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER)

static constexpr BitFlags<Feature> kAllFeatures{
    Feature::kNumericMeasurement, Feature::kLevelIndication, Feature::kMediumLevel,
    Feature::kCriticalLevel,      Feature::kPeakMeasurement, Feature::kAverageMeasurement,
};

// One heap-allocated struct per endpoint keeps delegate, cluster, and registration alive together.
struct ChefConcentrationSensor
{
    DefaultDelegate delegate;
    ConcentrationMeasurementCluster cluster;
    ServerClusterRegistration registration;

    ChefConcentrationSensor(EndpointId ep, ClusterId clusterId, MeasurementMediumEnum medium, MeasurementUnitEnum unit) :
        delegate(medium, unit, MakeNullable(0.0f), MakeNullable(1000.0f), 0.0f), cluster(ep, clusterId, kAllFeatures, delegate),
        registration(cluster)
    {}
};

static std::map<EndpointId, ChefConcentrationSensor *> gCarbonMonoxideInstances{};
static std::map<EndpointId, ChefConcentrationSensor *> gCarbonDioxideInstances{};
static std::map<EndpointId, ChefConcentrationSensor *> gNitrogenDioxideInstances{};
static std::map<EndpointId, ChefConcentrationSensor *> gOzoneInstances{};
static std::map<EndpointId, ChefConcentrationSensor *> gPm25Instances{};
static std::map<EndpointId, ChefConcentrationSensor *> gFormaldehydeInstances{};
static std::map<EndpointId, ChefConcentrationSensor *> gPm1Instances{};
static std::map<EndpointId, ChefConcentrationSensor *> gPm10Instances{};
static std::map<EndpointId, ChefConcentrationSensor *> gRadonInstances{};
static std::map<EndpointId, ChefConcentrationSensor *> gTotalVolatileOrganicCompoundsInstances{};

static void RegisterSensor(ChefConcentrationSensor * sensor)
{
    LogErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(sensor->registration));
}

#endif // any concentration plugin

#ifdef MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                MeasurementUnitEnum::kPpm);
    gCarbonMonoxideInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(10.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(12.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(10.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                MeasurementUnitEnum::kPpm);
    gCarbonDioxideInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(426.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(523.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(421.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                MeasurementUnitEnum::kPpb);
    gNitrogenDioxideInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(70.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(138.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(97.5f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                MeasurementUnitEnum::kPpm);
    gOzoneInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(60.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(99.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(72.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor            = new ChefConcentrationSensor(endpoint, Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                           MeasurementUnitEnum::kUgm3);
    gPm25Instances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(35.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(50.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(43.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                MeasurementUnitEnum::kMgm3);
    gFormaldehydeInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(40.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(88.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(7200);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(40.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(7200);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor           = new ChefConcentrationSensor(endpoint, Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                          MeasurementUnitEnum::kPpm);
    gPm1Instances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(200.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(430.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(270.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor            = new ChefConcentrationSensor(endpoint, Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                           MeasurementUnitEnum::kUgm3);
    gPm10Instances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(50.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(81.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(67.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER
void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir,
                                                MeasurementUnitEnum::kPpm);
    gRadonInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(100.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(150.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(120.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER
void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    auto * sensor = new ChefConcentrationSensor(endpoint, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    gTotalVolatileOrganicCompoundsInstances[endpoint] = sensor;
    RegisterSensor(sensor);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetMeasuredValue(MakeNullable(5.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValue(MakeNullable(8.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED sensor->cluster.SetLevelValue(LevelValueEnum::kLow);
}
#endif

#if defined(MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                  \
    defined(MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                   \
    defined(MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                 \
    defined(MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                            \
    defined(MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER) ||                                                           \
    defined(MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER) ||                                                     \
    defined(MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER) ||                                                              \
    defined(MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER) ||                                                             \
    defined(MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER) ||                                 \
    defined(MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER)

chip::Protocols::InteractionModel::Status
chefConcentrationMeasurementReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                         const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                         uint16_t maxReadLength)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status
chefConcentrationMeasurementWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                          const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    return chip::Protocols::InteractionModel::Status::Success;
}

#endif // any concentration plugin
