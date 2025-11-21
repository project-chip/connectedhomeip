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

static std::map<int, Instance<true, true, true, true, true, true> *> gCarbonMonoxideConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gCarbonDioxideConcentrationMeasurementInstance{};

static std::map<int, Instance<true, true, true, true, true, true> *> gNitrogenDioxideConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gOzoneConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gPm25ConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gFormaldehydeConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gPm1ConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gPm10ConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *> gRadonConcentrationMeasurementInstance{};
static std::map<int, Instance<true, true, true, true, true, true> *>
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance{};

template <bool NumericMeasurementEnabled, bool LevelIndicationEnabled, bool MediumLevelEnabled, bool CriticalLevelEnabled,
          bool PeakMeasurementEnabled, bool AverageMeasurementEnabled>
Protocols::InteractionModel::Status chefConcentrationMeasurementWriteCallback(
    std::map<int,
             Instance<NumericMeasurementEnabled, LevelIndicationEnabled, MediumLevelEnabled, CriticalLevelEnabled,
                      PeakMeasurementEnabled, LevelIndicationEnabled> *> & map,
    AttributeId measuredValueId, chip::EndpointId endpoint, chip::ClusterId clusterId,
    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Success;

    if (map.find(endpoint) == map.end())
    {
        ChipLogError(DeviceLayer, "Invalid Endpoind ID: %d", endpoint);
        return Protocols::InteractionModel::Status::UnsupportedEndpoint;
    }

    Instance<NumericMeasurementEnabled, LevelIndicationEnabled, MediumLevelEnabled, CriticalLevelEnabled, PeakMeasurementEnabled,
             LevelIndicationEnabled> * clusterInstance = map[endpoint];
    AttributeId attributeId                            = attributeMetadata->attributeId;

    if (attributeId == measuredValueId)
    {
        float newValue;
        std::memcpy(&newValue, buffer, sizeof(float)); // Copy buffer content to float

        CHIP_ERROR err = clusterInstance->SetMeasuredValue(MakeNullable(newValue));
        if (CHIP_NO_ERROR == err)
        {
            ChipLogDetail(DeviceLayer, "Updated EP:%d, Cluster: 0x%04x MeasuredValue", endpoint, clusterId);
        }
        else
        {
            ret = Protocols::InteractionModel::Status::UnsupportedWrite;
            ChipLogError(DeviceLayer, "Invalid Attribute Update status: %" CHIP_ERROR_FORMAT, err.Format());
        }
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
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gCarbonMonoxideConcentrationMeasurementInstance, CarbonMonoxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
            endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::CarbonDioxideConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gCarbonDioxideConcentrationMeasurementInstance, CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id,
            endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gNitrogenDioxideConcentrationMeasurementInstance,
            NitrogenDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::OzoneConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gOzoneConcentrationMeasurementInstance, OzoneConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
            clusterId, attributeMetadata, buffer);
        break;
    case Clusters::Pm25ConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gPm25ConcentrationMeasurementInstance, Pm25ConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId,
            attributeMetadata, buffer);
        break;
    case Clusters::FormaldehydeConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gFormaldehydeConcentrationMeasurementInstance, FormaldehydeConcentrationMeasurement::Attributes::MeasuredValue::Id,
            endpoint, clusterId, attributeMetadata, buffer);
        break;
    case Clusters::Pm1ConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gPm1ConcentrationMeasurementInstance, Pm1ConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId,
            attributeMetadata, buffer);
        break;
    case Clusters::Pm10ConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gPm10ConcentrationMeasurementInstance, Pm10ConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint, clusterId,
            attributeMetadata, buffer);
        break;
    case Clusters::RadonConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
            gRadonConcentrationMeasurementInstance, RadonConcentrationMeasurement::Attributes::MeasuredValue::Id, endpoint,
            clusterId, attributeMetadata, buffer);
        break;
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        ret = chefConcentrationMeasurementWriteCallback<true, true, true, true, true, true>(
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
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(
        MakeNullable(10.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(
        MakeNullable(12.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(
        3600);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(10.0f));
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(
        3600);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(1.0f);
    TEMPORARY_RETURN_IGNORED gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(
        LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(
        MakeNullable(426.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(
        MakeNullable(523.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(421.0f));
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(
        3600);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    TEMPORARY_RETURN_IGNORED gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(
        LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(
        MakeNullable(70.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(
        MakeNullable(138.0f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(
        3600);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(97.5f));
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(
        3600);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    TEMPORARY_RETURN_IGNORED gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(
        LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(60.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(
        MakeNullable(99.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(72.0f));
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    TEMPORARY_RETURN_IGNORED gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(35.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(50.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(43.0f));
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(1.0f);
    TEMPORARY_RETURN_IGNORED gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kMgm3);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(
        MakeNullable(40.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(
        MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(
        MakeNullable(88.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(7200);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(40.0f));
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(
        7200);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(1.0f);
    TEMPORARY_RETURN_IGNORED gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(
        LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(200.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(430.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(270.0f));
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    TEMPORARY_RETURN_IGNORED gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(50.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(81.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(67.0f));
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    TEMPORARY_RETURN_IGNORED gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER
void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(100.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(
        MakeNullable(1000.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(
        MakeNullable(150.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(120.0f));
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(1.0f);
    TEMPORARY_RETURN_IGNORED gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER
void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)] =
        new Instance<true, true, true, true, true, true>(EndpointId(endpoint),
                                                         TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                         MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(
        MakeNullable(5.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]
        ->SetMinMeasuredValue(MakeNullable(1.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]
        ->SetMaxMeasuredValue(MakeNullable(100.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]
        ->SetPeakMeasuredValue(MakeNullable(8.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]
        ->SetPeakMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]
        ->SetAverageMeasuredValue(MakeNullable(2.0f));
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]
        ->SetAverageMeasuredValueWindow(3600);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(
        1.0f);
    TEMPORARY_RETURN_IGNORED gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(
        LevelValueEnum::kLow);
}
#endif
