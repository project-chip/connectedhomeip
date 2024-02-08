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
EmberAfStatus chefConcentrationMeasurementWriteCallback(
    std::map<int,
             Instance<NumericMeasurementEnabled, LevelIndicationEnabled, MediumLevelEnabled, CriticalLevelEnabled,
                      PeakMeasurementEnabled, LevelIndicationEnabled> *> & map,
    AttributeId measuredValueId, chip::EndpointId endpoint, chip::ClusterId clusterId,
    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    EmberAfStatus ret = EMBER_ZCL_STATUS_SUCCESS;

    if (map.find(endpoint) == map.end())
    {
        ChipLogError(DeviceLayer, "Invalid Endpoind ID: %d", endpoint);
        return EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT;
    }

    Instance<NumericMeasurementEnabled, LevelIndicationEnabled, MediumLevelEnabled, CriticalLevelEnabled, PeakMeasurementEnabled,
             LevelIndicationEnabled> * clusterInstance = map[endpoint];
    AttributeId attributeId                            = attributeMetadata->attributeId;

    if (attributeId == measuredValueId)
    {
        float newValue  = 0;
        uint16_t tlvLen = *(uint16_t *) buffer;
        chip::TLV::TLVReader reader;
        reader.Init(buffer + sizeof(uint16_t), tlvLen);
        reader.Next();
        reader.Get(newValue);

        ChipLogDetail(DeviceLayer, "TLV Type %d, Length %d \n", static_cast<int>(reader.GetType()), tlvLen);
        // 2 bytes buf length + 5 bytes TLV for float
        ChipLogDetail(DeviceLayer, "buffer: %02x%02x%02x%02x%02x%02x%02x \n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4],
                      buffer[5], buffer[6]);

        CHIP_ERROR err = clusterInstance->SetMeasuredValue(MakeNullable(newValue));
        if (CHIP_NO_ERROR == err)
        {
            ChipLogDetail(DeviceLayer, "Updated EP:%d, Cluster: 0x%04x to MeasuredValue: %f", endpoint, clusterId, newValue);
        }
        else
        {
            ret = EMBER_ZCL_STATUS_UNSUPPORTED_WRITE;
            ChipLogError(DeviceLayer, "Invalid Attribute Update status: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else
    {
        ret = EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
        ChipLogError(DeviceLayer, "Unsupported Attribute ID: %d", static_cast<int>(attributeId));
    }

    return ret;
}

EmberAfStatus chefConcentrationMeasurementWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                        const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    EmberAfStatus ret = EMBER_ZCL_STATUS_SUCCESS;

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

EmberAfStatus chefConcentrationMeasurementReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                       const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                       uint16_t maxReadLength)
{
    EmberAfStatus ret = EMBER_ZCL_STATUS_SUCCESS;

    return ret;
}
#endif

#ifdef MATTER_DM_PLUGIN_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonMonoxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), CarbonMonoxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(401.0f));
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(50.0f));
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(1500.0f));
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(511.0f));
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(213.0f));
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    gCarbonMonoxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kHigh);
}
#endif

#ifdef MATTER_DM_PLUGIN_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfCarbonDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), CarbonDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(458.0f));
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(300.0f));
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(2000.0f));
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(523.0f));
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(421.0f));
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(5.0f);
    gCarbonDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kLow);
}
#endif

#ifdef MATTER_DM_PLUGIN_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfNitrogenDioxideConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), NitrogenDioxideConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(3.0f));
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(150.0f));
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(3.0f));
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(120);
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(3.0f));
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(120);
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(1.0f);
    gNitrogenDioxideConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kLow);
}
#endif

#ifdef MATTER_DM_PLUGIN_OZONE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfOzoneConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), OzoneConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(10.0f));
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(3.0f));
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(300.0f));
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(50.0f));
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(20.0f));
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(0.0f);
    gOzoneConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kLow);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM2__5_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm25ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), Pm25ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(42.0f));
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(400.0f));
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(90.0f));
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(35.0f));
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(4.0f);
    gPm25ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_SERVER
void emberAfFormaldehydeConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), FormaldehydeConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kMgm3);
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(10.0f));
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(0.0f));
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(200.0f));
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(10.0f));
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(7200);
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(2.0f));
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(7200);
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(0.0f);
    gFormaldehydeConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kMedium);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM1_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm1ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), Pm1ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kUgm3);
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(39.0f));
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(400.0f));
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(70.0f));
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(41.0f));
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(4.0f);
    gPm1ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kLow);
}
#endif

#ifdef MATTER_DM_PLUGIN_PM10_CONCENTRATION_MEASUREMENT_SERVER
void emberAfPm10ConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), Pm10ConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpt);
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(7.0f));
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(2.0f));
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(400.0f));
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(49.0f));
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(43.0f));
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(10.0f);
    gPm10ConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kLow);
}
#endif

#ifdef MATTER_DM_PLUGIN_RADON_CONCENTRATION_MEASUREMENT_SERVER
void emberAfRadonConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)] = new Instance<true, true, true, true, true, true>(
        EndpointId(endpoint), RadonConcentrationMeasurement::Id, MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(10.0f));
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(5.0f));
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(100.0f));
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(36.0f));
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(MakeNullable(20.0f));
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(0.0f);
    gRadonConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kHigh);
}
#endif

#ifdef MATTER_DM_PLUGIN_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT_SERVER
void emberAfTotalVolatileOrganicCompoundsConcentrationMeasurementClusterInitCallback(EndpointId endpoint)
{
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)] =
        new Instance<true, true, true, true, true, true>(EndpointId(endpoint),
                                                         TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                         MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->Init();
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMeasuredValue(MakeNullable(5.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMinMeasuredValue(MakeNullable(1.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetMaxMeasuredValue(MakeNullable(100.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValue(MakeNullable(8.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetPeakMeasuredValueWindow(3600);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValue(
        MakeNullable(2.0f));
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetAverageMeasuredValueWindow(3600);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetUncertainty(1.0f);
    gTotalVolatileOrganicCompoundsConcentrationMeasurementInstance[EndpointId(endpoint)]->SetLevelValue(LevelValueEnum::kLow);
}
#endif
