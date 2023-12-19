/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "DeviceEnergyManagementDelegateImpl.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/EventLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;

using chip::Optional;
using namespace chip::app;
using CostsList = DataModel::List<const Structs::CostStruct::Type>;

Status DeviceEnergyManagementDelegate::PowerAdjustRequest(const int64_t power, const uint32_t duration)
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement
    mEsaState = ESAStateEnum::kPowerAdjustActive;

    // TODO:  Generate a PowerAdjustStart Event, then begins to adjust its power
    // When done, raise PowerAdjustEnd & ESAState set to kOnline.

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);

    return status;
}

Status DeviceEnergyManagementDelegate::CancelPowerAdjustRequest()
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement
    /* TODO:  If the command is accepted, the ESA SHALL generate an PowerAdjustEnd Event.    */
    mEsaState = ESAStateEnum::kOnline;
    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);

    return status;
}

Status DeviceEnergyManagementDelegate::StartTimeAdjustRequest(const uint32_t requestedStartTime)
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = GetForecast();

    if (!forecast.IsNull())
    {
        uint32_t duration = forecast.Value().endTime - forecast.Value().startTime; // the current entire forecast duration

        /* Modify start time and end time */
        forecast.Value().startTime = requestedStartTime;
        forecast.Value().endTime   = requestedStartTime + duration;

        SetForecast(forecast);
    }
    return status;
}

Status DeviceEnergyManagementDelegate::PauseRequest(const uint32_t duration)
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement
    return status;
}

Status DeviceEnergyManagementDelegate::ResumeRequest()
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement
    mEsaState = ESAStateEnum::kOnline;
    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);

    return status;
}

Status DeviceEnergyManagementDelegate::ModifyForecastRequest(
    const uint32_t forecastId, const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments)
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement
    return status;
}

Status DeviceEnergyManagementDelegate::RequestConstraintBasedForecast(
    const DataModel::DecodableList<Structs::ConstraintsStruct::DecodableType> & constraints)
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement
    return status;
}

// ------------------------------------------------------------------
// Get attribute methods
ESATypeEnum DeviceEnergyManagementDelegate::GetESAType()
{
    return mEsaType;
}

bool DeviceEnergyManagementDelegate::GetESACanGenerate()
{
    return mEsaCanGenerate;
}

ESAStateEnum DeviceEnergyManagementDelegate::GetESAState()
{
    return mEsaState;
}

int64_t DeviceEnergyManagementDelegate::GetAbsMinPower()
{
    return mAbsMinPower;
}

int64_t DeviceEnergyManagementDelegate::GetAbsMaxPower()
{
    return mAbsMaxPower;
}

PowerAdjustmentCapability::TypeInfo::Type DeviceEnergyManagementDelegate::GetPowerAdjustmentCapability()
{
    return mPowerAdjustmentCapability;
}

DataModel::Nullable<Structs::ForecastStruct::Type> DeviceEnergyManagementDelegate::GetForecast()
{
    return mForecast;
}

// ------------------------------------------------------------------
// Set attribute methods

CHIP_ERROR DeviceEnergyManagementDelegate::SetESAType(ESATypeEnum newValue)
{
    ESATypeEnum oldValue = mEsaType;

    if (newValue >= ESATypeEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mEsaType = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mEsaType updated to %d", static_cast<int>(mEsaType));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAType::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetESACanGenerate(bool newValue)
{
    bool oldValue = mEsaCanGenerate;

    mEsaCanGenerate = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mEsaCanGenerate updated to %d", static_cast<int>(mEsaCanGenerate));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESACanGenerate::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetESAState(ESAStateEnum newValue)
{
    ESAStateEnum oldValue = mEsaState;

    if (newValue >= ESAStateEnum::kUnknownEnumValue)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mEsaState = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mEsaState updated to %d", static_cast<int>(mEsaState));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetAbsMinPower(int64_t newValue)
{
    int64_t oldValue = mAbsMinPower;

    mAbsMinPower = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mAbsMinPower updated to %d", static_cast<int>(mAbsMinPower));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, AbsMinPower::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetAbsMaxPower(int64_t newValue)
{
    int64_t oldValue = mAbsMaxPower;

    mAbsMaxPower = newValue;
    if (oldValue != newValue)
    {
        ChipLogDetail(AppServer, "mAbsMaxPower updated to %d", static_cast<int>(mAbsMaxPower));
        MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, AbsMaxPower::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DeviceEnergyManagementDelegate::SetPowerAdjustmentCapability(PowerAdjustmentCapability::TypeInfo::Type powerAdjustmentCapability)
{
    if (powerAdjustmentCapability.IsNull())
    {
        mPowerAdjustmentCapability.SetNull();
        ChipLogDetail(AppServer, "null src - %s", __FUNCTION__);
    }
    else
    {
        auto src    = powerAdjustmentCapability.Value();
        auto target = mPowerAdjustmentCapability.Value();
        auto size   = src.size();

        if (size > 8)
        {
            return CHIP_ERROR_BAD_REQUEST;
        }

        target.reduce_size(0);

        for (size_t i = 0; i < size; i++)
        {
            const_cast<Structs::PowerAdjustStruct::Type &>(target[i]) = src[i];
        }
    }

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, PowerAdjustmentCapability::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetForecast(DataModel::Nullable<Structs::ForecastStruct::Type> forecast)
{
    if (forecast.IsNull())
    {
        mForecast.SetNull();
        ChipLogDetail(AppServer, "null src - %s", __FUNCTION__);
    }
    else
    {
        auto src    = forecast.Value();
        auto target = mForecast.Value();

        target.forecastId++;
        target.activeSlotNumber  = src.activeSlotNumber;
        target.startTime         = src.startTime;
        target.endTime           = src.endTime;
        target.earliestStartTime = src.earliestStartTime;
        target.latestEndTime     = src.latestEndTime;
        target.isPauseable       = src.isPauseable;

        auto slots = src.slots;
        auto size  = slots.size();
        target.slots.reduce_size(0); // clear them
        ChipLogDetail(AppServer, "src.slots.size() %d - %s", (int) size, __FUNCTION__);

        for (size_t i = 0; i < size; i++)
        {
            const_cast<Structs::SlotStruct::Type &>(target.slots[i]) =
                slots[i]; // TODO: test if this works for optional/nullable stuff
        }
    }

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, Forecast::Id);

    return CHIP_NO_ERROR;
}
