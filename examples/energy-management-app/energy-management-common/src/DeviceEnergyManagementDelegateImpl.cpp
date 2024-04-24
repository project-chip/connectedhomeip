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

#include <app/EventLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;

using chip::Optional;
using namespace chip::app;
using CostsList = DataModel::List<const Structs::CostStruct::Type>;

/**
 * @brief Delegate handler for PowerAdjustRequest
 *
 * Note: checking of the validity of the PowerAdjustRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that it should apply a new power setting.
 * It should:
 *   1) notify the appliance - if the appliance hardware cannot be adjusted, then return Failure
 *   2) start a timer (or restart the existing PowerAdjust timer) for duration seconds
 *   3) generate a PowerAdjustStart event (if there is not an existing PowerAdjustRequest running)
 *   4) if appropriate, update the forecast with the new expected end time
 *
 *  and when the timer expires:
 *   5) notify the appliance's that it can resume its intended power setting (or go idle)
 *   6) generate a PowerAdjustEnd event with cause NormalCompletion
 *   7) if necessary, update the forecast with new expected end time
 */
Status DeviceEnergyManagementDelegate::PowerAdjustRequest(const int64_t power, const uint32_t duration, AdjustmentCauseEnum cause)
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement
    mEsaState = ESAStateEnum::kPowerAdjustActive;

    // TODO:  Generate a PowerAdjustStart Event, then begins to adjust its power
    // When done, raise PowerAdjustEnd & ESAState set to kOnline.

    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);

    return status;
}
/**
 * @brief Delegate handler for CancelPowerAdjustRequest
 *
 * Note: checking of the validity of the CancelPowerAdjustRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that it should resume its intended power setting (or go idle).

 * It should:
 *   1) notify the appliance's that it can resume its intended power setting (or go idle)
 *   2) generate a PowerAdjustEnd event with cause code Cancelled
 *   3) if necessary, update the forecast with new expected end time
 */
Status DeviceEnergyManagementDelegate::CancelPowerAdjustRequest()
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement
    /* TODO:  If the command is accepted, the ESA SHALL generate an PowerAdjustEnd Event.    */
    mEsaState = ESAStateEnum::kOnline;
    MatterReportingAttributeChangeCallback(mEndpointId, DeviceEnergyManagement::Id, ESAState::Id);

    return status;
}

/**
 * @brief Delegate handler for StartTimeAdjustRequest
 *
 * Note: checking of the validity of the StartTimeAdjustRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that the forecast has been updated by a client.
 *
 * It should:
 *      1) update the forecast attribute with the revised start time
 *      2) send a callback notification to the appliance so it can refresh its internal schedule
 */
Status DeviceEnergyManagementDelegate::StartTimeAdjustRequest(const uint32_t requestedStartTime, AdjustmentCauseEnum cause)
{
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = GetForecast();

    if (forecast.IsNull())
    {
        return Status::Failure;
    }

    uint32_t duration = forecast.Value().endTime - forecast.Value().startTime; // the current entire forecast duration

    /* Modify start time and end time */
    forecast.Value().startTime = requestedStartTime;
    forecast.Value().endTime   = requestedStartTime + duration;

    SetForecast(forecast); // This will increment forecast ID

    // TODO: callback to the appliance to notify it of a new start time

    return Status::Success;
}
/**
 * @brief Delegate handler for Pause Request
 *
 * Note: checking of the validity of the Pause Request has been done by the lower layer
 *
 * This function needs to notify the appliance that it should now pause.
 * It should:
 *   1) pause the appliance - if the appliance hardware cannot be paused, then return Failure
 *   2) start a timer for duration seconds
 *   3) generate a Paused event
 *   4) update the forecast with the new expected end time
 *
 *  and when the timer expires:
 *   5) restore the appliance's operational state
 *   6) generate a Resumed event
 *   7) if necessary, update the forecast with new expected end time
 */
Status DeviceEnergyManagementDelegate::PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause)
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement the behaviour above
    return status;
}

/**
 * @brief Delegate handler for ResumeRequest
 *
 * Note: checking of the validity of the ResumeRequest has been done by the lower layer
 *
 * This function needs to notify the appliance that it should now resume operation
 *
 * It should:
 *   1) restore the appliance's operational state
 *   2) generate a Resumed event
 *   3) update the forecast with new expected end time (given that the pause duration was shorter than originally requested)
 *
 */
Status DeviceEnergyManagementDelegate::ResumeRequest()
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement the behaviour above
    SetESAState(ESAStateEnum::kOnline);

    return status;
}

/**
 * @brief Delegate handler for ModifyForecastRequest
 *
 * Note: Only basic checking of the validity of the ModifyForecastRequest has been
 * done by the lower layer. This is a more complex use-case and requires higher-level
 * work by the delegate.
 *
 * It should:
 *      1) determine if the new forecast adjustments are acceptable to the appliance
 *       - if not return Failure. For example, if it may cause the home to be too hot
 *         or too cold, or a battery to be insufficiently charged
 *      2) if the slot adjustments are acceptable, then update the forecast
 *      3) notify the appliance to follow the revised schedule
 */
Status DeviceEnergyManagementDelegate::ModifyForecastRequest(
    const uint32_t forecastId, const DataModel::DecodableList<Structs::SlotAdjustmentStruct::DecodableType> & slotAdjustments,
    AdjustmentCauseEnum cause)
{
    Status status = Status::UnsupportedCommand; // Status::Success;

    // TODO: implement the behaviour above
    return status;
}

/**
 * @brief Delegate handler for RequestConstraintBasedForecast
 *
 * Note: Only basic checking of the validity of the RequestConstraintBasedForecast has been
 * done by the lower layer. This is a more complex use-case and requires higher-level
 * work by the delegate.
 *
 * It should:
 *      1) perform a higher level optimization (e.g. using tariff information, and user preferences)
 *      2) if a solution can be found, then update the forecast, else return Failure
 *      3) notify the appliance to follow the revised schedule
 */
Status DeviceEnergyManagementDelegate::RequestConstraintBasedForecast(
    const DataModel::DecodableList<Structs::ConstraintsStruct::DecodableType> & constraints, AdjustmentCauseEnum cause)
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement the behaviour above
    return status;
}

/**
 * @brief Delegate handler for CancelRequest
 *
 * Note: This is a more complex use-case and requires higher-level work by the delegate.
 *
 * It SHALL:
 *      1) Check if the forecastUpdateReason was already InternalOptimization (and reject the command)
 *      2) Update its forecast (based on its optimization strategy) ignoring previous requests
 *      3) Update its Forecast attribute to match its new intended operation, and update the
 *         ForecastStruct.ForecastUpdateReason to `Internal Optimization`.
 */
Status DeviceEnergyManagementDelegate::CancelRequest()
{
    Status status = Status::UnsupportedCommand; // Status::Success;
    // TODO: implement the behaviour above
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

OptOutStateEnum DeviceEnergyManagementDelegate::GetOptOutState()
{
    return mOptOutState;
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
    // TODO see Issue #31147
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceEnergyManagementDelegate::SetForecast(DataModel::Nullable<Structs::ForecastStruct::Type> forecast)
{
    // TODO see Issue #31147

    return CHIP_NO_ERROR;
}
