/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <ElectricalPowerMeasurementDelegate.h>
#include <app/reporting/reporting.h>

#include <app/clusters/electrical-power-measurement-server/electrical-power-measurement-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;

CHIP_ERROR ElectricalPowerMeasurementInstance::Init()
{
    return Instance::Init();
}

void ElectricalPowerMeasurementInstance::Shutdown()
{
    Instance::Shutdown();
}

// --------------- Internal Attribute Set APIs
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetPowerMode(PowerModeEnum newValue)
{
    PowerModeEnum oldValue = mPowerMode;

    if (newValue >= PowerModeEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mPowerMode = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mPowerMode updated to %d", static_cast<int>(mPowerMode));
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, PowerMode::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetNumberOfMeasurementTypes(uint8_t newValue)
{
    uint8_t oldValue = mNumberOfMeasurementTypes;

    if (newValue < 1 || newValue > kMaxNumberOfMeasurementTypes)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mNumberOfMeasurementTypes = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mNumberOfMeasurementTypes  updated to %d", static_cast<int>(mNumberOfMeasurementTypes));
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, NumberOfMeasurementTypes::Id);
    }

    return CHIP_NO_ERROR;
}

// CHIP_ERROR ElectricalPowerMeasurementDelegate::SetAccuracy(); // TODO
// CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRanges();   // TODO
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetVoltage(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mVoltage;

    mVoltage = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, Voltage::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetActiveCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mActiveCurrent;

    mActiveCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ActiveCurrent::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetReactiveCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mReactiveCurrent;

    mReactiveCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ReactiveCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetApparentCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mApparentCurrent;

    mApparentCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ApparentCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetActivePower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mActivePower;

    mActivePower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ActivePower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetReactivePower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mReactivePower;

    mReactivePower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ReactivePower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetApparentPower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mApparentPower;

    mApparentPower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, ApparentPower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRMSVoltage(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mRMSVoltage;

    mRMSVoltage = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, RMSVoltage::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRMSCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mRMSCurrent;

    mRMSCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, RMSCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetRMSPower(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mRMSPower;

    mRMSPower = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, RMSPower::Id);
    }

    return CHIP_NO_ERROR;
}
CHIP_ERROR ElectricalPowerMeasurementDelegate::SetFrequency(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mFrequency;

    mFrequency = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, Frequency::Id);
    }

    return CHIP_NO_ERROR;
}
// DataModel::Nullable<DataModel::List<Structs::HarmonicMeasurementStruct::Type>>;// TODO
// DataModel::Nullable<DataModel::List<Structs::HarmonicMeasurementStruct::Type>>;// TODO

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetPowerFactor(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mPowerFactor;

    mPowerFactor = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, PowerFactor::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ElectricalPowerMeasurementDelegate::SetNeutralCurrent(DataModel::Nullable<int64_t> newValue)
{
    DataModel::Nullable<int64_t> oldValue = mNeutralCurrent;

    mNeutralCurrent = newValue;
    if (oldValue != newValue)
    {
        // We won't log raw values since these could change frequently
        MatterReportingAttributeChangeCallback(mEndpointId, ElectricalPowerMeasurement::Id, NeutralCurrent::Id);
    }

    return CHIP_NO_ERROR;
}
