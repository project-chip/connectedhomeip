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
#include <app/reporting/reporting.h>
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

/**
 * @brief Sets all setpoints to Max if state is On else NULL.
 */
void updateSetPointsOnOff(EndpointId endpointId, bool onOff)
{
    uint16_t epIndex;

    // #ifdef MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
    epIndex = getIndexTemperatureMeasurement(endpointId);
    if (epIndex < kTemperatureMeasurementCount)
    {
        auto updatedTemperature = onOff ? TemperatureRangeMax[epIndex] : DataModel::NullNullable;
        TemperatureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedTemperature);
        MatterReportingAttributeChangeCallback(endpointId, TemperatureMeasurement::Id,
                                               TemperatureMeasurement::Attributes::MeasuredValue::Id);
    }
    // #endif // MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER

    // #ifdef MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER
    epIndex = getIndexPressureMeasurement(endpointId);
    if (epIndex < kPressureMeasurementCount)
    {
        auto updatedPressure = onOff ? PressureRangeMax[epIndex] : DataModel::NullNullable;
        PressureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedPressure);
        MatterReportingAttributeChangeCallback(endpointId, PressureMeasurement::Id,
                                               PressureMeasurement::Attributes::MeasuredValue::Id);
    }
    // #endif // MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER

    // #ifdef MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
    epIndex = getIndexFlowMeasurement(endpointId);
    if (epIndex < kFlowMeasurementCount)
    {
        auto updatedFlow = onOff ? FlowRangeMax[epIndex] : DataModel::NullNullable;
        FlowMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedFlow);
        MatterReportingAttributeChangeCallback(endpointId, FlowMeasurement::Id, FlowMeasurement::Attributes::MeasuredValue::Id);
    }
    // #endif // MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
}

// #ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
constexpr size_t kLevelControlCount = MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;
std::unique_ptr<DataModel::Nullable<uint8_t>> gLevel[kLevelControlCount];
uint16_t getIndexLevelControl(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, LevelControl::Id, kLevelControlCount);
}

/**
 * @brief Maps the level (from LevelControl) to a setpoint value within range.
 */
template <typename T>
DataModel::Nullable<T> LevelToSetpoint(DataModel::Nullable<uint8_t> level, DataModel::Nullable<T> RangeMin,
                                       DataModel::Nullable<T> RangeMax)
{
    if (level.IsNull() || !level.Value() || RangeMin.IsNull() || RangeMax.IsNull())
        return DataModel::NullNullable;

    return RangeMin.Value() + (RangeMax.Value() - RangeMin.Value()) * std::min(level.ValueOr(0), (uint8_t) 200) / 200;
}

/**
 * Updates setpoints based on the current level.
 */
void updateSetPointsLevel(EndpointId endpointId, DataModel::Nullable<uint8_t> level)
{
    uint16_t epIndex;

    // #ifdef MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER
    epIndex = getIndexTemperatureMeasurement(endpointId);
    if (epIndex < kTemperatureMeasurementCount)
    {
        DataModel::Nullable<int16_t> updatedTemperature =
            LevelToSetpoint(level, TemperatureRangeMin[epIndex], TemperatureRangeMax[epIndex]);
        TemperatureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedTemperature);
        MatterReportingAttributeChangeCallback(endpointId, TemperatureMeasurement::Id,
                                               TemperatureMeasurement::Attributes::MeasuredValue::Id);
    }
    // #endif // MATTER_DM_PLUGIN_TEMPERATURE_MEASUREMENT_SERVER

    // #ifdef MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER
    epIndex = getIndexPressureMeasurement(endpointId);
    if (epIndex < kPressureMeasurementCount)
    {
        DataModel::Nullable<int16_t> updatedPressure = LevelToSetpoint(level, PressureRangeMin[epIndex], PressureRangeMax[epIndex]);
        PressureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedPressure);
        MatterReportingAttributeChangeCallback(endpointId, PressureMeasurement::Id,
                                               PressureMeasurement::Attributes::MeasuredValue::Id);
    }
    // #endif // MATTER_DM_PLUGIN_PRESSURE_MEASUREMENT_SERVER

    // #ifdef MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
    epIndex = getIndexFlowMeasurement(endpointId);
    if (epIndex < kFlowMeasurementCount)
    {
        DataModel::Nullable<uint16_t> updatedFlow = LevelToSetpoint(level, FlowRangeMin[epIndex], FlowRangeMax[epIndex]);
        FlowMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedFlow);
        MatterReportingAttributeChangeCallback(endpointId, FlowMeasurement::Id, FlowMeasurement::Attributes::MeasuredValue::Id);
    }
    // #endif // MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER
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

    uint16_t epIndex = getIndexLevelControl(endpoint);
    if (epIndex >= kLevelControlCount)
    {
        ChipLogError(DeviceLayer, "Level control: No valid index found for endpoint %d", endpoint);
        return;
    }

    VerifyOrDieWithMsg(bool(gLevel[epIndex]), DeviceLayer, "Storage for current level on endpoint %d isn't initialized.", endpoint);

    if (level)
        (*gLevel[epIndex]).SetNonNull(level);
    else
        (*gLevel[epIndex]).SetNull();

    updateSetPointsLevel(endpoint, *gLevel[epIndex]);
}
// #endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER

/**
 * @brief Handler when OnOff value has changed. Updates setpoints if LevelControl is disabled for endpoint. If LevelControl is
 * enabled, only level is updated, handleMoveToLevel should be called to update setponts.
 */
void handleOnOff(EndpointId endpoint, bool value)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside handleOnOff");
    // #ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
    uint16_t epIndex = getIndexLevelControl(endpoint);
    if (epIndex < kLevelControlCount)
    {
        if (value)
        {
            VerifyOrDieWithMsg(bool(gLevel[epIndex]), DeviceLayer, "Storage for current level on endpoint %d isn't initialized.",
                               endpoint);
            if ((*gLevel[epIndex]).IsNull())
                (*gLevel[epIndex]).SetNonNull(255);
            LevelControl::Attributes::CurrentLevel::Set(endpoint, *gLevel[epIndex]);
            MatterReportingAttributeChangeCallback(endpoint, LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);
        }
        else
        {
            LevelControl::Attributes::CurrentLevel::SetNull(endpoint);
        }
        MatterReportingAttributeChangeCallback(endpoint, OnOff::Id, OnOff::Attributes::OnOff::Id);
    }
    else
    {
        updateSetPointsOnOff(endpoint, value);
    }
    // #else
    // updateSetPointsOnOff(endpoint, value);
    // #endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
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
            if (FlowMeasurement::Attributes::MinMeasuredValue::Get(endpointId, FlowRangeMin[epIndex]) != Status::Success ||
                FlowRangeMin[epIndex].IsNull())
            {
                FlowRangeMin[epIndex].SetNonNull(30);
            }
            if (FlowMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, FlowRangeMax[epIndex]) != Status::Success ||
                FlowRangeMax[epIndex].IsNull())
            {
                FlowRangeMax[epIndex].SetNonNull(10000);
            }
        }
        // #endif // MATTER_DM_PLUGIN_FLOW_MEASUREMENT_SERVER

        // #ifdef MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER
        epIndex = getIndexLevelControl(endpointId);
        if (epIndex < kLevelControlCount)
        {
            VerifyOrDieWithMsg(LevelControl::Attributes::CurrentLevel::SetNull(endpointId) == Status::Success, DeviceLayer,
                               "Failed to initialize Current Level to NULL for Endpoint: %d", endpointId);
            gLevel[epIndex] = std::make_unique<DataModel::Nullable<uint8_t>>(DataModel::NullNullable);
        }
        // #endif // MATTER_DM_PLUGIN_LEVEL_CONTROL_SERVER

        VerifyOrDieWithMsg(OnOff::Attributes::OnOff::Set(endpointId, false) == Status::Success, DeviceLayer,
                           "Failed to initialize OnOff to false for Endpoint: %d", endpointId);
    }
}

} // namespace pump
} // namespace chef

// #endif // MATTER_DM_PLUGIN_ON_OFF_SERVER
