/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/device-energy-management-server/DeviceEnergyManagementCluster.h>
#include <app/clusters/device-energy-management-server/tests/DeviceEnergyManagementMockDelegate.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

DeviceEnergyManagementMockDelegate::DeviceEnergyManagementMockDelegate()
{
    static Structs::PowerAdjustStruct::Type sPowerAdjustArray[1];
    sPowerAdjustArray[0].minPower    = kAbsMinPower;
    sPowerAdjustArray[0].maxPower    = kAbsMaxPower;
    sPowerAdjustArray[0].minDuration = 0;
    sPowerAdjustArray[0].maxDuration = 3600;

    DataModel::List<const Structs::PowerAdjustStruct::Type> powerAdjustList(sPowerAdjustArray, 1);
    Structs::PowerAdjustCapabilityStruct::Type capabilityStruct;
    capabilityStruct.powerAdjustCapability.SetNonNull(powerAdjustList);
    capabilityStruct.cause = PowerAdjustReasonEnum::kLocalOptimizationAdjustment;
    mPowerAdjustmentCapability.SetNonNull(capabilityStruct);

    // Initialize a default Forecast to support StartTimeAdjustRequest and PauseRequest commands
    static Structs::SlotStruct::Type sSlotArray[1];
    sSlotArray[0].slotIsPausable.SetValue(true);
    sSlotArray[0].minPauseDuration.SetValue(0);
    sSlotArray[0].maxPauseDuration.SetValue(3600);

    DataModel::List<const Structs::SlotStruct::Type> slotList(sSlotArray, 1);
    Structs::ForecastStruct::Type forecast;
    forecast.forecastID = 1;
    forecast.startTime  = 1000;
    forecast.endTime    = 2000;
    forecast.isPausable = true;
    forecast.activeSlotNumber.SetNonNull(0);
    forecast.forecastUpdateReason = ForecastUpdateReasonEnum::kLocalOptimization;
    forecast.slots                = slotList;
    forecast.earliestStartTime.SetValue(DataModel::Nullable<uint32_t>(900));
    forecast.latestEndTime.SetValue(2100);

    mForecast.SetNonNull(forecast);
}
Protocols::InteractionModel::Status
DeviceEnergyManagementMockDelegate::PowerAdjustRequest(const int64_t power, const uint32_t duration, AdjustmentCauseEnum cause)
{
    mESAState                                = ESAStateEnum::kPowerAdjustActive;
    mPowerAdjustmentCapability.Value().cause = AdjustmentCauseToPowerAdjustReason(cause);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::CancelPowerAdjustRequest()
{
    // Update ESA state from kPaused to kOnline (verified by cluster)
    mESAState                                = ESAStateEnum::kOnline;
    mPowerAdjustmentCapability.Value().cause = PowerAdjustReasonEnum::kNoAdjustment;
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::StartTimeAdjustRequest(const uint32_t requestedStartTime,
                                                                                               AdjustmentCauseEnum cause)
{
    if (!mForecast.IsNull())
    {
        auto & forecast = mForecast.Value();

        // Calculate duration from current forecast
        uint32_t duration = forecast.endTime - forecast.startTime;

        // Update Forecast.startTime to requestedStartTime
        forecast.startTime = requestedStartTime;

        // Increment Forecast.forecastID (new ForecastID)
        forecast.forecastID++;

        // Update Forecast.endTime to requestedStartTime + duration
        forecast.endTime = requestedStartTime + duration;

        forecast.forecastUpdateReason = AdjustmentCauseToForecastUpdateReason(cause);
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::PauseRequest(const uint32_t duration,
                                                                                     AdjustmentCauseEnum cause)
{
    mESAState = ESAStateEnum::kPaused;

    if (!mForecast.IsNull())
    {
        mForecast.Value().forecastUpdateReason = AdjustmentCauseToForecastUpdateReason(cause);
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::ResumeRequest()
{
    // Update ESA state from kPaused to kOnline (verified by cluster)
    mESAState = ESAStateEnum::kOnline;

    // Update Forecast's forecastUpdateReason to kInternalOptimization (verified by cluster)
    if (!mForecast.IsNull())
    {
        mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::ModifyForecastRequest(
    const uint32_t forecastID, const DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
    AdjustmentCauseEnum cause)
{
    if (mForecast.IsNull())
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    if (mForecast.Value().forecastID != forecastID)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    mForecast.Value().forecastUpdateReason = AdjustmentCauseToForecastUpdateReason(cause);
    mForecast.Value().forecastID++;

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::RequestConstraintBasedForecast(
    const DataModel::DecodableList<Structs::ConstraintsStruct::Type> & constraints, AdjustmentCauseEnum cause)
{
    if (mForecast.IsNull())
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    mForecast.Value().forecastUpdateReason = AdjustmentCauseToForecastUpdateReason(cause);
    mForecast.Value().forecastID++;

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::CancelRequest()
{
    // Reset ESAState to Online
    mESAState = ESAStateEnum::kOnline;

    // Update Forecast's forecastUpdateReason to kInternalOptimization
    if (!mForecast.IsNull())
    {
        mForecast.Value().forecastUpdateReason = ForecastUpdateReasonEnum::kInternalOptimization;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
DeviceEnergyManagementMockDelegate::PowerRangeAdjustRequest(const DataModel::Nullable<int64_t> minPower,
                                                            const DataModel::Nullable<int64_t> maxPower, const uint32_t duration,
                                                            AdjustmentCauseEnum cause)
{
    // Create and set the PowerRangeAdjustment attribute
    Structs::PowerRangeAdjustStruct::Type powerRangeAdjustment;
    powerRangeAdjustment.minPower = minPower;
    powerRangeAdjustment.maxPower = maxPower;

    // Map cause to PowerAdjustReasonEnum
    switch (cause)
    {
    case AdjustmentCauseEnum::kLocalOptimization:
        powerRangeAdjustment.cause = PowerAdjustReasonEnum::kLocalOptimizationAdjustment;
        break;
    case AdjustmentCauseEnum::kGridOptimization:
        powerRangeAdjustment.cause = PowerAdjustReasonEnum::kGridOptimizationAdjustment;
        break;
    default:
        return Protocols::InteractionModel::Status::Failure;
    }

    // Set endTime as absolute epoch time (current time + duration in seconds)
    uint32_t currentTimeS = 0;
    if (chip::System::Clock::GetClock_MatterEpochS(currentTimeS) == CHIP_NO_ERROR)
    {
        powerRangeAdjustment.endTime = currentTimeS + duration;
    }
    else
    {
        // Fallback: use duration directly if clock is unavailable
        powerRangeAdjustment.endTime = duration;
    }
    mPowerRangeAdjustment.SetNonNull(powerRangeAdjustment);

    // Only update state after successful validation
    mESAState = ESAStateEnum::kPowerAdjustActive;

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status DeviceEnergyManagementMockDelegate::CancelPowerRangeAdjustRequest()
{
    // Reset ESAState to Online
    mESAState = ESAStateEnum::kOnline;

    // Clear PowerRangeAdjustment attribute
    mPowerRangeAdjustment.SetNull();

    return Protocols::InteractionModel::Status::Success;
}

ESATypeEnum DeviceEnergyManagementMockDelegate::GetESAType()
{
    return mESAType;
}

bool DeviceEnergyManagementMockDelegate::GetESACanGenerate()
{
    return mESACanGenerate;
}

ESAStateEnum DeviceEnergyManagementMockDelegate::GetESAState()
{
    return mESAState;
}

int64_t DeviceEnergyManagementMockDelegate::GetAbsMinPower()
{
    return mAbsMinPower;
}

int64_t DeviceEnergyManagementMockDelegate::GetAbsMaxPower()
{
    return mAbsMaxPower;
}

OptOutStateEnum DeviceEnergyManagementMockDelegate::GetOptOutState()
{
    return mOptOutState;
}

const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> &
DeviceEnergyManagementMockDelegate::GetPowerAdjustmentCapability()
{
    return mPowerAdjustmentCapability;
}

const DataModel::Nullable<Structs::ForecastStruct::Type> & DeviceEnergyManagementMockDelegate::GetForecast()
{
    return mForecast;
}

const DataModel::Nullable<Structs::PowerRangeAdjustStruct::Type> & DeviceEnergyManagementMockDelegate::GetPowerRangeAdjustment()
{
    return mPowerRangeAdjustment;
}

CHIP_ERROR DeviceEnergyManagementMockDelegate::SetESAState(ESAStateEnum state)
{
    mESAState = state;
    return CHIP_NO_ERROR;
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
