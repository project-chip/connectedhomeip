/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "chef-pump.h"
#include "DeviceTypes.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

// #ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER

namespace chef {
namespace pump {

// #ifdef MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
constexpr size_t kTemperatureMeasurementCount = MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
DataModel::Nullable<int16_t> TemperatureRangeMin[kTemperatureMeasurementCount];
DataModel::Nullable<int16_t> TemperatureRangeMax[kTemperatureMeasurementCount];
uint16_t getIndexTemperatureMeasurement(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, TemperatureMeasurement::Id, kTemperatureMeasurementCount);
}
// #endif // MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER

// #ifdef MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER
constexpr size_t kPressureMeasurementCount = MATTER_DM_PRESSURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
DataModel::Nullable<int16_t> PressureRangeMin[kPressureMeasurementCount];
DataModel::Nullable<int16_t> PressureRangeMax[kPressureMeasurementCount];
uint16_t getIndexPressureMeasurement(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, PressureMeasurement::Id, kPressureMeasurementCount);
}
// #endif // MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER

// #ifdef MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
constexpr size_t kFlowMeasurementCount = MATTER_DM_FLOW_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
DataModel::Nullable<uint16_t> FlowRangeMin[kFlowMeasurementCount];
DataModel::Nullable<uint16_t> FlowRangeMax[kFlowMeasurementCount];
uint16_t getIndexFlowMeasurement(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, FlowMeasurement::Id, kFlowMeasurementCount);
}
// #endif // MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER

#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
constexpr size_t kLevelControlCount = MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;
std::unique_ptr<DataModel::Nullable<uint8_t>> gLevel[kLevelControlCount];
uint16_t getIndexLevelControl(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, LevelControl::Id, kLevelControlCount);
}

void handleMoveToLevel(EndpointId endpoint, uint8_t level)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside handleMoveToLevel");

    bool onOff = false;
    if (OnOff::Attributes::OnOff::Get(endpoint, &onOff) != Status::Success)
    {
        ChipLogError(DeviceLayer, "Failed to read onOff for Endpoint %d", endpoint);
        return;
    }

    if (!onOff)
    {
        ChipLogDetail(DeviceLayer, "Device is Off. Returning.");
        return;
    }
}
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER

/**
 * @brief Handler when OnOff value has changed.
 */
void handleOnOff(EndpointId endpoint, bool value)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside handleOnOff");
    if (value) // Pump has been turned On.
    {
#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
    }
}

void Init()
{
    const uint16_t endpointCount = emberAfEndpointCount();

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {

        EndpointId endpointId = emberAfEndpointFromIndex(endpointIndex);
        if (endpointId == kInvalidEndpointId)
        {
            continue;
        }

        if (!chef::DeviceTypes::EndpointHasDeviceType(endpointId, chef::DeviceTypes::kPumpDeviceId))
        {
            continue;
        }

        uint16_t epIndex;

        // #ifdef MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
        epIndex = getIndexTemperatureMeasurement(endpointId);
        if (epIndex < kTemperatureMeasurementCount)
        {
            VerifyOrDieWithMsg(TemperatureMeasurement::Attributes::MeasuredValue::SetNull(endpointId) == Status::Success,
                               DeviceLayer, "Failed to initialize Temperature Measured Value to NULL for Endpoint: %d", endpointId);
            if (TemperatureMeasurement::Attributes::MinMeasuredValue::Get(endpointId, TemperatureRangeMin[epIndex]) !=
                    Status::Success ||
                TemperatureRangeMin[epIndex].IsNull())
            {
                TemperatureRangeMin[epIndex].SetNonNull(-2500);
            }
            if (TemperatureMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, TemperatureRangeMax[epIndex]) !=
                    Status::Success ||
                TemperatureRangeMax[epIndex].IsNull())
            {
                TemperatureRangeMax[epIndex].SetNonNull(40000);
            }
        }
        // #endif // MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER

        // #ifdef MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER
        epIndex = getIndexPressureMeasurement(endpointId);
        if (epIndex < kPressureMeasurementCount)
        {
            VerifyOrDieWithMsg(PressureMeasurement::Attributes::MeasuredValue::SetNull(endpointId) == Status::Success, DeviceLayer,
                               "Failed to initialize Pressure Measured Value to NULL for Endpoint: %d", endpointId);
            if (PressureMeasurement::Attributes::MinMeasuredValue::Get(endpointId, PressureRangeMin[epIndex]) != Status::Success ||
                PressureRangeMin[epIndex].IsNull())
            {
                PressureRangeMin[epIndex].SetNonNull(30);
            }
            if (PressureMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, PressureRangeMax[epIndex]) != Status::Success ||
                PressureRangeMax[epIndex].IsNull())
            {
                PressureRangeMax[epIndex].SetNonNull(10000);
            }
        }
        // #endif // MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER

        // #ifdef MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
        epIndex = getIndexFlowMeasurement(endpointId);
        if (epIndex < kFlowMeasurementCount)
        {
            VerifyOrDieWithMsg(FlowMeasurement::Attributes::MeasuredValue::SetNull(endpointId) == Status::Success, DeviceLayer,
                               "Failed to initialize Flow Measured Value to NULL for Endpoint: %d", endpointId);
            if (FlowMeasurement::Attributes::MinMeasuredValue::Get(endpointId, PressureRangeMin[epIndex]) != Status::Success ||
                FlowRangeMin[epIndex].IsNull())
            {
                FlowRangeMin[epIndex].SetNonNull(30);
            }
            if (FlowMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, PressureRangeMax[epIndex]) != Status::Success ||
                FlowRangeMax[epIndex].IsNull())
            {
                FlowRangeMax[epIndex].SetNonNull(10000);
            }
        }
        // #endif // MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER

#ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
        epIndex = getIndexLevelControl(endpointId);
        if (epIndex < kLevelControlCount)
        {
            VerifyOrDieWithMsg(LevelControl::Attributes::CurrentLevel::SetNull(endpointId) == Status::Success, DeviceLayer,
                               "Failed to initialize Current Level to NULL for Endpoint: %d", endpointId);
            gLevel[epIndex] = std::make_unique<DataModel::Nullable<uint8_t>>(DataModel::NullNullable);
        }
#endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER

        VerifyOrDieWithMsg(OnOff::Attributes::OnOff::Set(endpointId, false) == Status::Success, DeviceLayer,
                           "Failed to initialize OnOff to false for Endpoint: %d", endpointId);
    }
}

} // namespace pump
} // namespace chef

// #endif // MATTER_DM_PLUGIN_ON_OFF_SERVER
