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

#include <app/clusters/energy-evse-server/tests/MockEvseDelegate.h>

#include <app/clusters/energy-evse-server/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

Protocols::InteractionModel::Status MockEvseDelegate::Disable()
{
    VerifyOrReturnValue(mCluster != nullptr, Protocols::InteractionModel::Status::Failure);

    DataModel::Nullable<uint32_t> disableTime(0);
    TEMPORARY_RETURN_IGNORED mCluster->SetChargingEnabledUntil(disableTime);
    TEMPORARY_RETURN_IGNORED mCluster->SetDischargingEnabledUntil(disableTime);
    TEMPORARY_RETURN_IGNORED mCluster->SetSupplyState(SupplyStateEnum::kDisabled);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MockEvseDelegate::EnableCharging(const DataModel::Nullable<uint32_t> & enableChargeTime,
                                                                     const int64_t & minimumChargeCurrent,
                                                                     const int64_t & maximumChargeCurrent)
{
    VerifyOrReturnValue(mCluster != nullptr, Protocols::InteractionModel::Status::Failure);

    // If there is currently an error present on the EVSE, return FAILURE
    if (mCluster->GetFaultState() != FaultStateEnum::kNoError)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // If Diagnostics are currently active, return FAILURE
    if (mCluster->GetSupplyState() == SupplyStateEnum::kDisabledDiagnostics)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // Update SupplyState based on previous state:
    // - If previously Disabled -> ChargingEnabled
    // - If previously DischargingEnabled -> Enabled (both charging and discharging)
    SupplyStateEnum currentSupplyState = mCluster->GetSupplyState();
    if (currentSupplyState == SupplyStateEnum::kDisabled || currentSupplyState == SupplyStateEnum::kDisabledError)
    {
        TEMPORARY_RETURN_IGNORED mCluster->SetSupplyState(SupplyStateEnum::kChargingEnabled);
    }
    else if (currentSupplyState == SupplyStateEnum::kDischargingEnabled)
    {
        TEMPORARY_RETURN_IGNORED mCluster->SetSupplyState(SupplyStateEnum::kEnabled);
    }
    // If already ChargingEnabled or Enabled, keep the current state

    // Update ChargingEnabledUntil to the provided timestamp (can be null for indefinite)
    TEMPORARY_RETURN_IGNORED mCluster->SetChargingEnabledUntil(enableChargeTime);

    // Store the charge current limits
    TEMPORARY_RETURN_IGNORED mCluster->SetMinimumChargeCurrent(minimumChargeCurrent);
    TEMPORARY_RETURN_IGNORED mCluster->SetMaximumChargeCurrent(maximumChargeCurrent);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MockEvseDelegate::EnableDischarging(const DataModel::Nullable<uint32_t> & enableDischargeTime,
                                                                        const int64_t & maximumDischargeCurrent)
{
    VerifyOrReturnValue(mCluster != nullptr, Protocols::InteractionModel::Status::Failure);

    // If there is currently an error present on the EVSE, return FAILURE
    if (mCluster->GetFaultState() != FaultStateEnum::kNoError)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // If Diagnostics are currently active, return FAILURE
    if (mCluster->GetSupplyState() == SupplyStateEnum::kDisabledDiagnostics)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // Update SupplyState based on previous state:
    // - If previously Disabled -> DischargingEnabled
    // - If previously ChargingEnabled -> Enabled (both charging and discharging)
    SupplyStateEnum currentSupplyState = mCluster->GetSupplyState();
    if (currentSupplyState == SupplyStateEnum::kDisabled || currentSupplyState == SupplyStateEnum::kDisabledError)
    {
        TEMPORARY_RETURN_IGNORED mCluster->SetSupplyState(SupplyStateEnum::kDischargingEnabled);
    }
    else if (currentSupplyState == SupplyStateEnum::kChargingEnabled)
    {
        TEMPORARY_RETURN_IGNORED mCluster->SetSupplyState(SupplyStateEnum::kEnabled);
    }
    // If already DischargingEnabled or Enabled, keep the current state

    // Update DischargingEnabledUntil to the provided timestamp (can be null for indefinite)
    TEMPORARY_RETURN_IGNORED mCluster->SetDischargingEnabledUntil(enableDischargeTime);

    // Store the discharge current limit
    TEMPORARY_RETURN_IGNORED mCluster->SetMaximumDischargeCurrent(maximumDischargeCurrent);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MockEvseDelegate::StartDiagnostics()
{
    VerifyOrReturnValue(mCluster != nullptr, Protocols::InteractionModel::Status::Failure);

    // EVSE SHALL enter Diagnostics state only if SupplyState is Disabled
    if (mCluster->GetSupplyState() != SupplyStateEnum::kDisabled)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // Set SupplyState to DisabledDiagnostics
    TEMPORARY_RETURN_IGNORED mCluster->SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MockEvseDelegate::SetTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules)
{
    Structs::ChargingTargetStruct::Type tmpTargets[7][kMaxTargetsPerDay];
    size_t tmpTargetsPerDay[7] = { 0 };
    size_t newTargetsCount     = 0;
    BitMask<TargetDayOfWeekBitmap> daysModified;

    auto iter = chargingTargetSchedules.begin();
    while (iter.Next())
    {
        auto & schedule = iter.GetValue();
        daysModified.Set(schedule.dayOfWeekForSequence);

        // For each day in the bitmask, collect targets into temp storage
        for (uint8_t day = 0; day < 7; day++)
        {
            if (!schedule.dayOfWeekForSequence.Has(static_cast<TargetDayOfWeekBitmap>(1 << day)))
            {
                continue;
            }

            // Reset temp count for this day (replacing targets)
            tmpTargetsPerDay[day] = 0;

            auto targetsIter = schedule.chargingTargets.begin();
            while (targetsIter.Next() && tmpTargetsPerDay[day] < kMaxTargetsPerDay)
            {
                if (newTargetsCount >= kMaxTotalTargets)
                {
                    return Protocols::InteractionModel::Status::ResourceExhausted;
                }

                auto & target                          = targetsIter.GetValue();
                tmpTargets[day][tmpTargetsPerDay[day]] = target;
                tmpTargetsPerDay[day]++;
                newTargetsCount++;
            }
        }
    }

    mTotalTargetsCount = 0;
    mDaysWithTargets   = 0;
    for (uint8_t day = 0; day < 7; day++)
    {
        if (daysModified.Has(static_cast<TargetDayOfWeekBitmap>(1 << day)))
        {
            // This day was modified - use temp state
            mTargetsPerDay[day] = tmpTargetsPerDay[day];
            for (size_t i = 0; i < tmpTargetsPerDay[day]; i++)
            {
                mTargets[day][i] = tmpTargets[day][i];
            }
        }

        // Recalculate totals for all days
        mTotalTargetsCount += mTargetsPerDay[day];
        if (mTargetsPerDay[day] > 0)
        {
            mDaysWithTargets |= static_cast<uint8_t>(1 << day);
        }
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MockEvseDelegate::LoadTargets()
{
    // In a real implementation, this would load from persistent storage
    // For mock, targets are already in memory
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status
MockEvseDelegate::GetTargets(DataModel::List<const Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules)
{
    // Build response from stored targets
    size_t scheduleCount = 0;

    for (uint8_t day = 0; day < 7; day++)
    {
        if (mTargetsPerDay[day] > 0)
        {
            mSchedules[scheduleCount].dayOfWeekForSequence = static_cast<TargetDayOfWeekBitmap>(1 << day);
            mSchedules[scheduleCount].chargingTargets =
                DataModel::List<const Structs::ChargingTargetStruct::Type>(mTargets[day], mTargetsPerDay[day]);
            scheduleCount++;
        }
    }

    chargingTargetSchedules = DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(mSchedules, scheduleCount);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status MockEvseDelegate::ClearTargets()
{
    // Clear all stored targets
    for (uint8_t day = 0; day < 7; day++)
    {
        mTargetsPerDay[day] = 0;
    }
    mTotalTargetsCount = 0;
    mDaysWithTargets   = 0;

    return Protocols::InteractionModel::Status::Success;
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
