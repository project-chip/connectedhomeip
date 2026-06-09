/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "FakeAttributeAccess.h"

#include <optional>

#include "pigweed/rpc_services/internal/StatusUtils.h"
#include <app/ConcreteAttributePath.h>
#include <app/data-model/Nullable.h>
#include <app/util/config.h>
#include <pigweed/rpc_services/AccessInterceptor.h>
#include <pigweed/rpc_services/AccessInterceptorRegistry.h>

#if MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/temperature-measurement-server/CodegenIntegration.h>
#endif

#if MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/valve-configuration-and-control-server/CodegenIntegration.h>
#endif

#if MATTER_DM_ILLUMINANCE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/illuminance-measurement-server/CodegenIntegration.h>
#endif

#if MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/occupancy-sensor-server/CodegenIntegration.h>
#endif

#if MATTER_DM_SWITCH_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/switch-server/CodegenIntegration.h>
#endif

#if MATTER_DM_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/relative-humidity-measurement-server/CodegenIntegration.h>
#endif

#if MATTER_DM_FLOW_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/flow-measurement-server/CodegenIntegration.h>
#endif

#if MATTER_DM_PRESSURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/pressure-measurement-server/CodegenIntegration.h>
#endif

#if MATTER_DM_BOOLEAN_STATE_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#include <app/clusters/boolean-state-configuration-server/CodegenIntegration.h>
#endif

#if (MATTER_DM_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                     \
    (MATTER_DM_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                      \
    (MATTER_DM_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                    \
    (MATTER_DM_PM1_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                                 \
    (MATTER_DM_PM10_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                                \
    (MATTER_DM_PM2_5_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                               \
    (MATTER_DM_RADON_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                               \
    (MATTER_DM_TVOC_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                                \
    (MATTER_DM_OZONE_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0) ||                                               \
    (MATTER_DM_FORMALDEHYDE_CONCENTRATION_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0)
#define MATTER_DM_CONCENTRATION_MEASUREMENT_WANTED 1
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/concentration-measurement-server/concentration-measurement-cluster-objects.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#endif

namespace chip {
namespace app {
namespace Clusters {
namespace Chef {
/**
 * Once clusters are converted to code driven, fake attribute access through datamodel provider
 * will be blocked. Therefore we make this attribute accessor to allow fake attribute access for
 * code driven apps.
 */
class AttributeAccessor : public chip::rpc::PigweedDebugAccessInterceptor
{
public:
    std::optional<::pw::Status> Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override
    {
        ChipLogProgress(Zcl, "Inside AttributeAccessor::Write for Cluster: " ChipLogFormatMEI " , Attribute: " ChipLogFormatMEI,
                        ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
        switch (path.mClusterId)
        {
#if MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case TemperatureMeasurement::Id:
            switch (path.mAttributeId)
            {
            case TemperatureMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<int16_t> measuredValue;
                CHIP_ERROR err = decoder.Decode(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                err = TemperatureMeasurement::SetMeasuredValue(path.mEndpointId, measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                if (measuredValue.IsNull())
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to null.");
                }
                else
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to %d.", measuredValue.Value());
                }
                return ::pw::OkStatus();
            }
            case TemperatureMeasurement::Attributes::MinMeasuredValue::Id:
            case TemperatureMeasurement::Attributes::MaxMeasuredValue::Id: {
                auto temperatureMeasurement = TemperatureMeasurement::FindClusterOnEndpoint(path.mEndpointId);
                if (temperatureMeasurement == nullptr)
                {
                    return ::pw::Status::Internal();
                }

                DataModel::Nullable<int16_t> value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measured value: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                DataModel::Nullable<int16_t> min;
                DataModel::Nullable<int16_t> max;

                if (path.mAttributeId == TemperatureMeasurement::Attributes::MinMeasuredValue::Id)
                {
                    min = value;
                    max = temperatureMeasurement->GetMaxMeasuredValue();
                }
                else
                {
                    min = temperatureMeasurement->GetMinMeasuredValue();
                    max = value;
                }

                err = TemperatureMeasurement::SetMeasuredValueRange(path.mEndpointId, min, max);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measured value range: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case ValveConfigurationAndControl::Id:
            switch (path.mAttributeId)
            {
            case ValveConfigurationAndControl::Attributes::CurrentLevel::Id:
                Percent level;
                CHIP_ERROR err = decoder.Decode(level);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode currentLevel: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = ValveConfigurationAndControl::UpdateCurrentLevel(path.mEndpointId, level);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to update currentLevel: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                ChipLogProgress(Zcl, "[Pw] Successfully set current level to " ChipLogFormatMEI ".", ChipLogValueMEI(level));
                return ::pw::OkStatus();
            }
            break;
#endif // MATTER_DM_VALVE_CONFIGURATION_AND_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_ILLUMINANCE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case IlluminanceMeasurement::Id:
            switch (path.mAttributeId)
            {
            case IlluminanceMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<uint16_t> measuredValue;
                CHIP_ERROR err = decoder.Decode(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                err = IlluminanceMeasurement::SetMeasuredValue(path.mEndpointId, measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                if (measuredValue.IsNull())
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to null.");
                }
                else
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to %u.", measuredValue.Value());
                }
                return ::pw::OkStatus();
            }
            case IlluminanceMeasurement::Attributes::MinMeasuredValue::Id:
            case IlluminanceMeasurement::Attributes::MaxMeasuredValue::Id: {
                auto illuminanceMeasurement = IlluminanceMeasurement::FindClusterOnEndpoint(path.mEndpointId);
                if (illuminanceMeasurement == nullptr)
                {
                    return ::pw::Status::Internal();
                }

                DataModel::Nullable<uint16_t> value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measured value: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                DataModel::Nullable<uint16_t> min;
                DataModel::Nullable<uint16_t> max;

                if (path.mAttributeId == IlluminanceMeasurement::Attributes::MinMeasuredValue::Id)
                {
                    min = value;
                    max = illuminanceMeasurement->GetMaxMeasuredValue();
                }
                else
                {
                    min = illuminanceMeasurement->GetMinMeasuredValue();
                    max = value;
                }

                err = IlluminanceMeasurement::SetMeasuredValueRange(path.mEndpointId, min, max);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measured value range: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_ILLUMINANCE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case OccupancySensing::Id:
            switch (path.mAttributeId)
            {
            case OccupancySensing::Attributes::Occupancy::Id: {
                BitMask<OccupancySensing::OccupancyBitmap> occupancy;
                CHIP_ERROR err = decoder.Decode(occupancy);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode occupancy: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                auto occupancySensing = OccupancySensing::FindClusterOnEndpoint(path.mEndpointId);
                if (occupancySensing == nullptr)
                {
                    return ::pw::Status::Internal();
                }

                occupancySensing->SetOccupancy(occupancy.Has(OccupancySensing::OccupancyBitmap::kOccupied));
                ChipLogProgress(Zcl, "[Pw] Successfully set occupancy to %d.",
                                occupancy.Has(OccupancySensing::OccupancyBitmap::kOccupied));
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_OCCUPANCY_SENSING_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_SWITCH_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case Switch::Id:
            switch (path.mAttributeId)
            {
            case Switch::Attributes::CurrentPosition::Id: {
                uint8_t currentPosition;
                CHIP_ERROR err = decoder.Decode(currentPosition);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode switch current position: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                auto switchCluster = Switch::FindClusterOnEndpoint(path.mEndpointId);
                if (switchCluster == nullptr)
                {
                    ChipLogError(Zcl, "[Pw] Failed to find switch cluster on endpoint: %d", path.mEndpointId);
                    return ::pw::Status::Internal();
                }
                err = switchCluster->SetCurrentPosition(currentPosition);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set switch current position: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_SWITCH_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case RelativeHumidityMeasurement::Id:
            switch (path.mAttributeId)
            {
            case RelativeHumidityMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<uint16_t> measuredValue;
                CHIP_ERROR err = decoder.Decode(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode relative humidity measurement measured value: %" CHIP_ERROR_FORMAT,
                                 err.Format());
                    return ::pw::Status::Internal();
                }
                auto relativeHumidityMeasurementCluster = RelativeHumidityMeasurement::FindClusterOnEndpoint(path.mEndpointId);
                if (relativeHumidityMeasurementCluster == nullptr)
                {
                    ChipLogError(Zcl, "[Pw] Failed to find relative humidity measurement cluster on endpoint: %d",
                                 path.mEndpointId);
                    return ::pw::Status::Internal();
                }
                err = relativeHumidityMeasurementCluster->SetMeasuredValue(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set relative humidity measurement measured value: %" CHIP_ERROR_FORMAT,
                                 err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_RELATIVE_HUMIDITY_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_FLOW_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case FlowMeasurement::Id:
            switch (path.mAttributeId)
            {
            case FlowMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<uint16_t> measuredValue;
                CHIP_ERROR err = decoder.Decode(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                err = FlowMeasurement::SetMeasuredValue(path.mEndpointId, measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                if (measuredValue.IsNull())
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to null.");
                }
                else
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to %u.", measuredValue.Value());
                }
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_FLOW_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_PRESSURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case PressureMeasurement::Id:
            switch (path.mAttributeId)
            {
            case PressureMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<int16_t> measuredValue;
                CHIP_ERROR err = decoder.Decode(measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                err = PressureMeasurement::SetMeasuredValue(path.mEndpointId, measuredValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set measuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                if (measuredValue.IsNull())
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to null.");
                }
                else
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set measuredValue to %d.", measuredValue.Value());
                }
                return ::pw::OkStatus();
            }
            case PressureMeasurement::Attributes::ScaledValue::Id: {
                DataModel::Nullable<int16_t> scaledValue;
                CHIP_ERROR err = decoder.Decode(scaledValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode scaledValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                err = PressureMeasurement::SetScaledValue(path.mEndpointId, scaledValue);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set scaledValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }

                if (scaledValue.IsNull())
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set scaledValue to null.");
                }
                else
                {
                    ChipLogProgress(Zcl, "[Pw] Successfully set scaledValue to %d.", scaledValue.Value());
                }
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_PRESSURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if MATTER_DM_BOOLEAN_STATE_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT > 0
        case BooleanStateConfiguration::Id:
            switch (path.mAttributeId)
            {
            case BooleanStateConfiguration::Attributes::AlarmsActive::Id: {
                BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarms;
                CHIP_ERROR err = decoder.Decode(alarms);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode alarms active: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = BooleanStateConfiguration::SetAlarmsActive(path.mEndpointId, alarms);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set alarms active: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                ChipLogProgress(Zcl, "[Pw] Successfully set alarms active to 0x%02x.", alarms.Raw());
                return ::pw::OkStatus();
            }
            }
            break;
#endif // MATTER_DM_BOOLEAN_STATE_CONFIGURATION_CLUSTER_SERVER_ENDPOINT_COUNT > 0
#if defined(MATTER_DM_CONCENTRATION_MEASUREMENT_WANTED)
        case CarbonDioxideConcentrationMeasurement::Id:
        case CarbonMonoxideConcentrationMeasurement::Id:
        case NitrogenDioxideConcentrationMeasurement::Id:
        case Pm1ConcentrationMeasurement::Id:
        case Pm10ConcentrationMeasurement::Id:
        case Pm25ConcentrationMeasurement::Id:
        case RadonConcentrationMeasurement::Id:
        case TotalVolatileOrganicCompoundsConcentrationMeasurement::Id:
        case OzoneConcentrationMeasurement::Id:
        case FormaldehydeConcentrationMeasurement::Id: {
            auto & registry                           = CodegenDataModelProvider::Instance().Registry();
            ServerClusterInterface * clusterInterface = registry.Get(ConcreteClusterPath(path.mEndpointId, path.mClusterId));
            if (clusterInterface == nullptr)
            {
                ChipLogError(Zcl, "[Pw] Failed to find concentration measurement cluster " ChipLogFormatMEI " on endpoint %d",
                             ChipLogValueMEI(path.mClusterId), path.mEndpointId);
                return ::pw::Status::Internal();
            }
            auto * cluster = static_cast<ConcentrationMeasurement::ConcentrationMeasurementCluster *>(clusterInterface);

            // Note: The following attributes are read-only/static and cannot be written at runtime
            // because they lack setters in ConcentrationMeasurementCluster:
            // - MinMeasuredValue
            // - MaxMeasuredValue
            // - Uncertainty
            // - MeasurementUnit
            // - MeasurementMedium
            switch (path.mAttributeId)
            {
            case ConcentrationMeasurement::Attributes::MeasuredValue::Id: {
                DataModel::Nullable<float> value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode MeasuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = cluster->SetMeasuredValue(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set MeasuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            case ConcentrationMeasurement::Attributes::PeakMeasuredValue::Id: {
                DataModel::Nullable<float> value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode PeakMeasuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = cluster->SetPeakMeasuredValue(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set PeakMeasuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            case ConcentrationMeasurement::Attributes::PeakMeasuredValueWindow::Id: {
                uint32_t value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode PeakMeasuredValueWindow: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = cluster->SetPeakMeasuredValueWindow(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set PeakMeasuredValueWindow: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            case ConcentrationMeasurement::Attributes::AverageMeasuredValue::Id: {
                DataModel::Nullable<float> value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode AverageMeasuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = cluster->SetAverageMeasuredValue(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set AverageMeasuredValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            case ConcentrationMeasurement::Attributes::AverageMeasuredValueWindow::Id: {
                uint32_t value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode AverageMeasuredValueWindow: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = cluster->SetAverageMeasuredValueWindow(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set AverageMeasuredValueWindow: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            case ConcentrationMeasurement::Attributes::LevelValue::Id: {
                ConcentrationMeasurement::LevelValueEnum value;
                CHIP_ERROR err = decoder.Decode(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to decode LevelValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                err = cluster->SetLevelValue(value);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "[Pw] Failed to set LevelValue: %" CHIP_ERROR_FORMAT, err.Format());
                    return ::pw::Status::Internal();
                }
                return ::pw::OkStatus();
            }
            }
            break;
        }
#endif // MATTER_DM_CONCENTRATION_MEASUREMENT_WANTED
        }
        return std::nullopt;
    }
};

namespace {
static AttributeAccessor gAttributeAccessor;
} // namespace

void RegisterAttributeAccessor()
{
    chip::rpc::PigweedDebugAccessInterceptorRegistry::Instance().Register(&gAttributeAccessor);
}

} // namespace Chef
} // namespace Clusters
} // namespace app
} // namespace chip
