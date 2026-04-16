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
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLVReader.h>
#include <map>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;

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
#include <app/clusters/concentration-measurement-server/concentration-measurement-server.h>
using namespace chip::app::Clusters::ConcentrationMeasurement;

// All six feature flags — matches the old Instance<true,true,true,true,true,true>.
static constexpr BitFlags<Feature> kAllFeatures{ Feature::kNumericMeasurement, Feature::kLevelIndication,
                                                 Feature::kMediumLevel,        Feature::kCriticalLevel,
                                                 Feature::kPeakMeasurement,    Feature::kAverageMeasurement };

static std::map<int, Instance *> gCarbonMonoxideConcentrationMeasurementInstance{};
static std::map<int, Instance *> gCarbonDioxideConcentrationMeasurementInstance{};
static std::map<int, Instance *> gNitrogenDioxideConcentrationMeasurementInstance{};
static std::map<int, Instance *> gOzoneConcentrationMeasurementInstance{};
static std::map<int, Instance *> gPm25ConcentrationMeasurementInstance{};
static std::map<int, Instance *> gFormaldehydeConcentrationMeasurementInstance{};
static std::map<int, Instance *> gPm1ConcentrationMeasurementInstance{};
static std::map<int, Instance *> gPm10ConcentrationMeasurementInstance{};
static std::map<int, Instance *> gRadonConcentrationMeasurementInstance{};
static std::map<int, Instance *> gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance{};

static Protocols::InteractionModel::Status
chefConcentrationMeasurementWriteCallbackHelper(std::map<int, Instance *> & map, AttributeId measuredValueId,
                                                chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    if (map.find(endpoint) == map.end())
    {
        ChipLogError(DeviceLayer, "Invalid Endpoind ID: %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    Instance * clusterInstance = map[endpoint];
    AttributeId attributeId    = attributeMetadata->attributeId;

    if (attributeId == measuredValueId)
    {
        float newValue;
        std::memcpy(&newValue, buffer, sizeof(float));

        clusterInstance->GetDelegate().HandleNewMeasuredValue(MakeNullable(newValue));
        ChipLogDetail(DeviceLayer, "Updated EP:%d, Cluster: 0x%04x MeasuredValue", endpoint, clusterId);
    }
    else
    {
        ret = Protocols::InteractionModel::Status::UnsupportedAttribute;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
    }

    return ret;
}

Protocols::InteractionModel::Status chefConcentrationMeasurementWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                              const EmberAfAttributeMetadata * attributeMetadata,
                                                                              uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    switch (clusterId)
    {
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gCarbonMonoxideConcentrationMeasurementInstance,
                                                              CarbonMonoxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
                                                              endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::CarbonDioxideConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gCarbonDioxideConcentrationMeasurementInstance,
                                                              CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
                                                              endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(
            gNitrogenDioxideConcentrationMeasurementInstance,
            NitrogenDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::OzoneConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gOzoneConcentrationMeasurementInstance,
                                                              OzoneConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
                                                              clusterId, attributeMetadata, buffer);
        break;
    case Clusters::Pm25ConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gPm25ConcentrationMeasurementInstance,
                                                              Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
                                                              clusterId, attributeMetadata, buffer);
        break;
    case Clusters::FormaldehydeConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(
            gFormaldehydeConcentrationMeasurementInstance,
            FormaldehydeConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::Pm1ConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gPm1ConcentrationMeasurementInstance,
                                                              Pm1ConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
                                                              clusterId, attributeMetadata, buffer);
        break;
    case Clusters::Pm10ConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gPm10ConcentrationMeasurementInstance,
                                                              Pm10ConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
                                                              clusterId, attributeMetadata, buffer);
        break;
    case Clusters::RadonConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(gRadonConcentrationMeasurementInstance,
                                                              RadonConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
                                                              clusterId, attributeMetadata, buffer);
        break;
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallbackHelper(
            gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance,
            TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId,
            attributeMetadata, buffer);
        break;

    default:
        break;
    }

    return ret;
}

Protocols::InteractionModel::Status chefConcentrationMeasurementReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                             const EmberAfAttributeMetadata * attributeMetadata,
                                                                             uint8_t * buffer, uint16_t maxReadLength)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    return ret;
}
#endif

#ifdef MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), CarbonMonoxideConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(10.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(12.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(10.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(1.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), CarbonDioxideConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(426.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(523.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(421.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(10.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), NitrogenDioxideConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(70.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(138.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(97.5f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(10.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), OzoneConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(60.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(99.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(72.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(10.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), Pm25ConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(35.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(50.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(43.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(1.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), FormaldehydeConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kMgm3);
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(40.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(88.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(7200);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(40.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(7200);
    instance->GetDelegate().HandleNewUncertainty(1.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), Pm1ConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(200.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(430.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(270.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(10.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), Pm10ConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(50.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(81.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(67.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(10.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER
void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), RadonConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(100.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(1000.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(150.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(120.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(1.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER
void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    Instance * instance = new Instance(EndpointId(endpoint), TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, kAllFeatures,
                                       MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)] = instance;
    instance->GetDelegate().HandleNewMeasuredValue(MakeNullable(5.0f));
    instance->GetDelegate().HandleNewMinMeasuredValue(MakeNullable(1.0f));
    instance->GetDelegate().HandleNewMaxMeasuredValue(MakeNullable(100.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValue(MakeNullable(8.0f));
    instance->GetDelegate().HandleNewPeakMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewAverageMeasuredValue(MakeNullable(2.0f));
    instance->GetDelegate().HandleNewAverageMeasuredValueWindow(3600);
    instance->GetDelegate().HandleNewUncertainty(1.0f);
    instance->GetDelegate().HandleNewLevelValue(LevelValueEnum::kLow);
}
#endif
