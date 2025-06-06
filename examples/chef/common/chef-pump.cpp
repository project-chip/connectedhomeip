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

namespace chef {
namespace pump {

namespace {

constexpr size_t kTemperatureMeasurementCount = MATTER_DM_TEMPERATURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
DataModel::Nullable<int16_t> TemperatureRangeMin[kTemperatureMeasurementCount];
DataModel::Nullable<int16_t> TemperatureRangeMax[kTemperatureMeasurementCount];
uint16_t getIndexTemperatureMeasurement(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, TemperatureMeasurement::Id, kTemperatureMeasurementCount);
}
int16_t kDefaultMinTemperature = 100;
int16_t kDefaultMaxTemperature = 9000;

constexpr size_t kPressureMeasurementCount = MATTER_DM_PRESSURE_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
DataModel::Nullable<int16_t> PressureRangeMin[kPressureMeasurementCount];
DataModel::Nullable<int16_t> PressureRangeMax[kPressureMeasurementCount];
uint16_t getIndexPressureMeasurement(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, PressureMeasurement::Id, kPressureMeasurementCount);
}
int16_t kDefaultMinPressure = 30;
int16_t kDefaultMaxPressure = 10000;

constexpr size_t kFlowMeasurementCount = MATTER_DM_FLOW_MEASUREMENT_CLUSTER_SERVER_ENDPOINT_COUNT;
DataModel::Nullable<uint16_t> FlowRangeMin[kFlowMeasurementCount];
DataModel::Nullable<uint16_t> FlowRangeMax[kFlowMeasurementCount];
uint16_t getIndexFlowMeasurement(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, FlowMeasurement::Id, kFlowMeasurementCount);
}
uint16_t kDefaultMinFlow = 1;
uint16_t kDefaultMaxFlow = 10;

int16_t kMinCapacity = 0;
int16_t kMaxCapacity = 100;

/**
 * @brief Sets all setpoints to Max if state is On else NULL.
 */
void updateSetPointsOnOff(EndpointId endpointId, bool onOff)
{
    uint16_t epIndex;

    epIndex = getIndexTemperatureMeasurement(endpointId);
    if (epIndex < kTemperatureMeasurementCount)
    {
        auto updatedTemperature = onOff ? TemperatureRangeMax[epIndex] : chip::app::DataModel::Nullable<int16_t>(0);
        TemperatureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedTemperature);
        MatterReportingAttributeChangeCallback(endpointId, TemperatureMeasurement::Id,
                                               TemperatureMeasurement::Attributes::MeasuredValue::Id);
    }

    epIndex = getIndexPressureMeasurement(endpointId);
    if (epIndex < kPressureMeasurementCount)
    {
        auto updatedPressure = onOff ? PressureRangeMax[epIndex] : chip::app::DataModel::Nullable<int16_t>(0);
        PressureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedPressure);
        MatterReportingAttributeChangeCallback(endpointId, PressureMeasurement::Id,
                                               PressureMeasurement::Attributes::MeasuredValue::Id);
    }

    epIndex = getIndexFlowMeasurement(endpointId);
    if (epIndex < kFlowMeasurementCount)
    {
        auto updatedFlow = onOff ? FlowRangeMax[epIndex] : chip::app::DataModel::Nullable<uint16_t>(0);
        FlowMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedFlow);
        MatterReportingAttributeChangeCallback(endpointId, FlowMeasurement::Id, FlowMeasurement::Attributes::MeasuredValue::Id);
    }

    DataModel::Nullable<int16_t> capacity = onOff ? DataModel::Nullable<int16_t>(kMaxCapacity) : DataModel::Nullable<int16_t>(0);
    PumpConfigurationAndControl::Attributes::Capacity::Set(endpointId, capacity);
    MatterReportingAttributeChangeCallback(endpointId, PumpConfigurationAndControl::Id,
                                           PumpConfigurationAndControl::Attributes::Capacity::Id);
}

constexpr size_t kLevelControlCount = MATTER_DM_LEVEL_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;

uint16_t getIndexLevelControl(EndpointId endpointId)
{
    return emberAfGetClusterServerEndpointIndex(endpointId, LevelControl::Id, kLevelControlCount);
}
constexpr uint8_t kMinLevel  = 1;
constexpr uint8_t kMaxLevel  = 254;
constexpr uint8_t kNullLevel = 255;

/**
 * @brief Maps the level (from LevelControl) to a setpoint value within range.
 */
template <typename T>
DataModel::Nullable<T> LevelToSetpoint(DataModel::Nullable<uint8_t> level, DataModel::Nullable<T> RangeMin,
                                       DataModel::Nullable<T> RangeMax)
{
    if (level.IsNull() || level.Value() == kNullLevel || RangeMin.IsNull() || RangeMax.IsNull())
        return DataModel::NullNullable;

    if (!level.Value())
    {
        return DataModel::Nullable<T>(0);
    }

    return RangeMin.Value() + (RangeMax.Value() - RangeMin.Value()) * std::min(level.ValueOr(0), (uint8_t) 200) / 200;
}

/**
 * Updates setpoints based on the current level.
 */
void updateSetPointsLevel(EndpointId endpointId, DataModel::Nullable<uint8_t> level)
{
    uint16_t epIndex;

    epIndex = getIndexTemperatureMeasurement(endpointId);
    if (epIndex < kTemperatureMeasurementCount)
    {
        DataModel::Nullable<int16_t> updatedTemperature =
            LevelToSetpoint(level, TemperatureRangeMin[epIndex], TemperatureRangeMax[epIndex]);
        TemperatureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedTemperature);
        MatterReportingAttributeChangeCallback(endpointId, TemperatureMeasurement::Id,
                                               TemperatureMeasurement::Attributes::MeasuredValue::Id);
    }

    epIndex = getIndexPressureMeasurement(endpointId);
    if (epIndex < kPressureMeasurementCount)
    {
        DataModel::Nullable<int16_t> updatedPressure = LevelToSetpoint(level, PressureRangeMin[epIndex], PressureRangeMax[epIndex]);
        PressureMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedPressure);
        MatterReportingAttributeChangeCallback(endpointId, PressureMeasurement::Id,
                                               PressureMeasurement::Attributes::MeasuredValue::Id);
    }

    epIndex = getIndexFlowMeasurement(endpointId);
    if (epIndex < kFlowMeasurementCount)
    {
        DataModel::Nullable<uint16_t> updatedFlow = LevelToSetpoint(level, FlowRangeMin[epIndex], FlowRangeMax[epIndex]);
        FlowMeasurement::Attributes::MeasuredValue::Set(endpointId, updatedFlow);
        MatterReportingAttributeChangeCallback(endpointId, FlowMeasurement::Id, FlowMeasurement::Attributes::MeasuredValue::Id);
    }

    DataModel::Nullable<int16_t> capacity =
        LevelToSetpoint(level, DataModel::Nullable<int16_t>(kMinCapacity), DataModel::Nullable<int16_t>(kMaxCapacity));
    PumpConfigurationAndControl::Attributes::Capacity::Set(endpointId, capacity);
    MatterReportingAttributeChangeCallback(endpointId, PumpConfigurationAndControl::Id,
                                           PumpConfigurationAndControl::Attributes::Capacity::Id);
}

CHIP_ERROR setPumpStatus(EndpointId endpoint, uint16_t status)
{

    Status res = PumpConfigurationAndControl::Attributes::PumpStatus::Set(
        endpoint, BitMask<PumpConfigurationAndControl::PumpStatusBitmap>(status));
    VerifyOrReturnLogError(res == Status::Success, CHIP_ERROR_INTERNAL);
    MatterReportingAttributeChangeCallback(endpoint, PumpConfigurationAndControl::Id,
                                           PumpConfigurationAndControl::Attributes::PumpStatus::Id);
    return CHIP_NO_ERROR;
}
} // namespace

/**
 * @brief Post moveToLevel handler. Updates all setpoint values to match current level.
 */
void postMoveToLevel(EndpointId endpoint, uint8_t level)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside handleMoveToLevel. level = %d", level);

    bool pumpOn = false;
    OnOff::Attributes::OnOff::Get(endpoint, &pumpOn);

    if (!pumpOn)
    {
        ChipLogDetail(DeviceLayer, "[chef-pump] Pump is not on. Setpoints not updated.");
        return;
    }

    if (level && level != kNullLevel)
        updateSetPointsLevel(endpoint, DataModel::Nullable<uint8_t>(level));
    else
        updateSetPointsLevel(endpoint, DataModel::Nullable<uint8_t>(0));
}

/**
 * @brief Post onOff change handler. Matches setpoints to currentLevel when going from
 * off to on. Sets setpoints to NULL when going from On to Off.
 */
void postOnOff(EndpointId endpoint, bool value)
{
    ChipLogDetail(DeviceLayer, "[chef-pump] Inside postOnOff. value: %d", value);
    uint16_t epIndex = getIndexLevelControl(endpoint);
    if (epIndex < kLevelControlCount)
    {
        if (value) // Off to On
        {
            auto level = DataModel::Nullable<uint8_t>(kMaxLevel);
            LevelControl::Attributes::CurrentLevel::Get(endpoint, level);
            updateSetPointsLevel(endpoint, level);
        }
        else // On to Off
        {
            updateSetPointsLevel(endpoint, DataModel::Nullable<uint8_t>(0));
        }
    }
    else
    {
        updateSetPointsOnOff(endpoint, value);
    }

    BitMask<PumpConfigurationAndControl::PumpStatusBitmap> currentStatus;
    Status res = PumpConfigurationAndControl::Attributes::PumpStatus::Get(endpoint, &currentStatus);
    VerifyOrReturn(res == Status::Success, ChipLogError(DeviceLayer, "Failed to read pump status."));
    if (value)
    {
        if (!currentStatus.GetField(PumpConfigurationAndControl::PumpStatusBitmap::kRunning))
        {
            currentStatus.SetField(PumpConfigurationAndControl::PumpStatusBitmap::kRunning, 1);
            setPumpStatus(endpoint, currentStatus.Raw());
        }
    }
    else
    {
        if (currentStatus.GetField(PumpConfigurationAndControl::PumpStatusBitmap::kRunning))
        {
            currentStatus.SetField(PumpConfigurationAndControl::PumpStatusBitmap::kRunning, 0);
            setPumpStatus(endpoint, currentStatus.Raw());
        }
    }
}

void init()
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

        epIndex = getIndexTemperatureMeasurement(endpointId);
        if (epIndex < kTemperatureMeasurementCount)
        {
            VerifyOrDieWithMsg(TemperatureMeasurement::Attributes::MeasuredValue::SetNull(endpointId) == Status::Success,
                               DeviceLayer, "Failed to initialize Temperature Measured Value to NULL for Endpoint: %d", endpointId);
            if (TemperatureMeasurement::Attributes::MinMeasuredValue::Get(endpointId, TemperatureRangeMin[epIndex]) !=
                    Status::Success ||
                TemperatureRangeMin[epIndex].IsNull())
            {
                TemperatureRangeMin[epIndex].SetNonNull(kDefaultMinTemperature);
            }
            if (TemperatureMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, TemperatureRangeMax[epIndex]) !=
                    Status::Success ||
                TemperatureRangeMax[epIndex].IsNull())
            {
                TemperatureRangeMax[epIndex].SetNonNull(kDefaultMaxTemperature);
            }
        }

        epIndex = getIndexPressureMeasurement(endpointId);
        if (epIndex < kPressureMeasurementCount)
        {
            VerifyOrDieWithMsg(PressureMeasurement::Attributes::MeasuredValue::SetNull(endpointId) == Status::Success, DeviceLayer,
                               "Failed to initialize Pressure Measured Value to NULL for Endpoint: %d", endpointId);
            if (PressureMeasurement::Attributes::MinMeasuredValue::Get(endpointId, PressureRangeMin[epIndex]) != Status::Success ||
                PressureRangeMin[epIndex].IsNull())
            {
                PressureRangeMin[epIndex].SetNonNull(kDefaultMinPressure);
            }
            if (PressureMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, PressureRangeMax[epIndex]) != Status::Success ||
                PressureRangeMax[epIndex].IsNull())
            {
                PressureRangeMax[epIndex].SetNonNull(kDefaultMaxPressure);
            }
        }

        epIndex = getIndexFlowMeasurement(endpointId);
        if (epIndex < kFlowMeasurementCount)
        {
            VerifyOrDieWithMsg(FlowMeasurement::Attributes::MeasuredValue::SetNull(endpointId) == Status::Success, DeviceLayer,
                               "Failed to initialize Flow Measured Value to NULL for Endpoint: %d", endpointId);
            if (FlowMeasurement::Attributes::MinMeasuredValue::Get(endpointId, FlowRangeMin[epIndex]) != Status::Success ||
                FlowRangeMin[epIndex].IsNull())
            {
                FlowRangeMin[epIndex].SetNonNull(kDefaultMinFlow);
            }
            if (FlowMeasurement::Attributes::MaxMeasuredValue::Get(endpointId, FlowRangeMax[epIndex]) != Status::Success ||
                FlowRangeMax[epIndex].IsNull())
            {
                FlowRangeMax[epIndex].SetNonNull(kDefaultMaxFlow);
            }
        }

        epIndex = getIndexLevelControl(endpointId);
        if (epIndex < kLevelControlCount)
        {
            VerifyOrDieWithMsg(LevelControl::Attributes::CurrentLevel::Set(endpointId, kMinLevel) == Status::Success, DeviceLayer,
                               "Failed to initialize Current Level to %d for Endpoint: %d", kMinLevel, endpointId);
        }

        VerifyOrDieWithMsg(OnOff::Attributes::OnOff::Set(endpointId, false) == Status::Success, DeviceLayer,
                           "Failed to initialize OnOff to false for Endpoint: %d", endpointId);
        updateSetPointsOnOff(endpointId, false);

        setPumpStatus(endpointId, 0);
    }
}

} // namespace pump
} // namespace chef
