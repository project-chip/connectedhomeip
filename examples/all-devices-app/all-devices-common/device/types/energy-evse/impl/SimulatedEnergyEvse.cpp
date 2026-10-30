/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/energy-evse/impl/SimulatedEnergyEvse.h>

#include <lib/support/BitMask.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

namespace {

using namespace Clusters::EnergyEvseMode;
using ModeTagStructType = Clusters::detail::Structs::ModeTagStruct::Type;

constexpr uint8_t kModeManual                    = 0;
constexpr uint8_t kModeTimeOfUse                 = 1;
constexpr uint8_t kModeSolarCharging             = 2;
constexpr uint8_t kModeTimeOfUseAndSolarCharging = 3;

const ModeTagStructType kModeTagsManual[]                    = { { .value = to_underlying(ModeTag::kManual) } };
const ModeTagStructType kModeTagsTimeOfUse[]                 = { { .value = to_underlying(ModeTag::kTimeOfUse) } };
const ModeTagStructType kModeTagsSolarCharging[]             = { { .value = to_underlying(ModeTag::kSolarCharging) } };
const ModeTagStructType kModeTagsTimeOfUseAndSolarCharging[] = {
    { .value = to_underlying(ModeTag::kTimeOfUse) },
    { .value = to_underlying(ModeTag::kSolarCharging) },
};

struct ModeOptionEntry
{
    CharSpan label;
    uint8_t mode;
    Span<const ModeTagStructType> modeTags;
};

const ModeOptionEntry kEvseModeOptions[] = {
    { "Manual"_span, kModeManual, Span(kModeTagsManual) },
    { "Auto-scheduled"_span, kModeTimeOfUse, Span(kModeTagsTimeOfUse) },
    { "Solar"_span, kModeSolarCharging, Span(kModeTagsSolarCharging) },
    { "Auto-scheduled with Solar charging"_span, kModeTimeOfUseAndSolarCharging, Span(kModeTagsTimeOfUseAndSolarCharging) },
};

} // namespace

SimulatedEnergyEvse::SimulatedEnergyEvse(const Context & context) :
    EnergyEvse(EnergyEvse::Config{
        .timerDelegate          = context.timerDelegate,
        .diagnosticDataProvider = context.diagnosticDataProvider,
        .evseDelegate           = *this,
        .evseModeDelegate       = *this,
        .evseFeatures           = BitMask<Clusters::EnergyEvse::Feature>(Clusters::EnergyEvse::Feature::kChargingPreferences),
        .evseOptionalCmds       = {},
        .evseOptionalAttrs      = {},
    })
{}

CHIP_ERROR SimulatedEnergyEvse::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, GetTimerDelegate()).WithDelegate(this));
    return provider.AddCluster(mIdentifyCluster.Registration());
}

void SimulatedEnergyEvse::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

// Clusters::EnergyEvse::Delegate
Protocols::InteractionModel::Status SimulatedEnergyEvse::Disable()
{
    DataModel::Nullable<uint32_t> disableTime(0);
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetChargingEnabledUntil(disableTime);
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetDischargingEnabledUntil(disableTime);
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kDisabled);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::EnableCharging(const DataModel::Nullable<uint32_t> & enableChargeTime,
                                                                        const int64_t & minimumChargeCurrent,
                                                                        const int64_t & maximumChargeCurrent)
{
    if (EnergyEvseCluster().GetFaultState() != Clusters::EnergyEvse::FaultStateEnum::kNoError)
    {
        return Protocols::InteractionModel::Status::Failure;
    }
    if (EnergyEvseCluster().GetSupplyState() == Clusters::EnergyEvse::SupplyStateEnum::kDisabledDiagnostics)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    const auto currentSupplyState = EnergyEvseCluster().GetSupplyState();
    if (currentSupplyState == Clusters::EnergyEvse::SupplyStateEnum::kDisabled ||
        currentSupplyState == Clusters::EnergyEvse::SupplyStateEnum::kDisabledError)
    {
        TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kChargingEnabled);
    }
    else if (currentSupplyState == Clusters::EnergyEvse::SupplyStateEnum::kDischargingEnabled)
    {
        TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kEnabled);
    }

    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetChargingEnabledUntil(enableChargeTime);
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetMinimumChargeCurrent(minimumChargeCurrent);
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetMaximumChargeCurrent(maximumChargeCurrent);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::EnableDischarging(const DataModel::Nullable<uint32_t> & enableDischargeTime,
                                                                           const int64_t & maximumDischargeCurrent)
{
    if (!EnergyEvseCluster().Features().Has(Clusters::EnergyEvse::Feature::kV2x))
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    if (EnergyEvseCluster().GetFaultState() != Clusters::EnergyEvse::FaultStateEnum::kNoError)
    {
        return Protocols::InteractionModel::Status::Failure;
    }
    if (EnergyEvseCluster().GetSupplyState() == Clusters::EnergyEvse::SupplyStateEnum::kDisabledDiagnostics)
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    const auto currentSupplyState = EnergyEvseCluster().GetSupplyState();
    if (currentSupplyState == Clusters::EnergyEvse::SupplyStateEnum::kDisabled ||
        currentSupplyState == Clusters::EnergyEvse::SupplyStateEnum::kDisabledError)
    {
        TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kDischargingEnabled);
    }
    else if (currentSupplyState == Clusters::EnergyEvse::SupplyStateEnum::kChargingEnabled)
    {
        TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kEnabled);
    }

    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetDischargingEnabledUntil(enableDischargeTime);
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetMaximumDischargeCurrent(maximumDischargeCurrent);

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::StartDiagnostics()
{
    if (!EnergyEvseCluster().SupportsStartDiagnostics())
    {
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
    TEMPORARY_RETURN_IGNORED EnergyEvseCluster().SetSupplyState(Clusters::EnergyEvse::SupplyStateEnum::kDisabledDiagnostics);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::SetTargets(
    const DataModel::DecodableList<Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::DecodableType> &
        chargingTargetSchedules)
{
    Clusters::EnergyEvse::Structs::ChargingTargetStruct::Type tmpTargets[7][kMaxTargetsPerDay];
    size_t tmpTargetsPerDay[7] = { 0 };
    size_t newTargetsCount     = 0;
    BitMask<Clusters::EnergyEvse::TargetDayOfWeekBitmap> daysModified;

    auto iter = chargingTargetSchedules.begin();
    while (iter.Next())
    {
        auto & schedule = iter.GetValue();
        daysModified.Set(schedule.dayOfWeekForSequence);

        for (uint8_t day = 0; day < 7; day++)
        {
            if (!schedule.dayOfWeekForSequence.Has(static_cast<Clusters::EnergyEvse::TargetDayOfWeekBitmap>(1 << day)))
            {
                continue;
            }

            tmpTargetsPerDay[day] = 0;

            auto targetsIter = schedule.chargingTargets.begin();
            while (targetsIter.Next() && tmpTargetsPerDay[day] < kMaxTargetsPerDay)
            {
                if (newTargetsCount >= (7 * kMaxTargetsPerDay))
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
        if (daysModified.Has(static_cast<Clusters::EnergyEvse::TargetDayOfWeekBitmap>(1 << day)))
        {
            mTargetsPerDay[day] = tmpTargetsPerDay[day];
            for (size_t i = 0; i < tmpTargetsPerDay[day]; i++)
            {
                mTargets[day][i] = tmpTargets[day][i];
            }
        }

        mTotalTargetsCount += mTargetsPerDay[day];
        if (mTargetsPerDay[day] > 0)
        {
            mDaysWithTargets |= static_cast<uint8_t>(1 << day);
        }
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::LoadTargets()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::GetTargets(
    DataModel::List<const Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::Type> & chargingTargetSchedules)
{
    size_t scheduleCount = 0;
    for (uint8_t day = 0; day < 7; ++day)
    {
        if (mTargetsPerDay[day] > 0)
        {
            mSchedules[scheduleCount].dayOfWeekForSequence =
                BitMask<Clusters::EnergyEvse::TargetDayOfWeekBitmap>(static_cast<Clusters::EnergyEvse::TargetDayOfWeekBitmap>(1 << day));
            mSchedules[scheduleCount].chargingTargets =
                DataModel::List<const Clusters::EnergyEvse::Structs::ChargingTargetStruct::Type>(mTargets[day],
                                                                                                 mTargetsPerDay[day]);
            scheduleCount++;
        }
    }
    chargingTargetSchedules =
        DataModel::List<const Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::Type>(mSchedules, scheduleCount);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status SimulatedEnergyEvse::ClearTargets()
{
    mTotalTargetsCount = 0;
    mDaysWithTargets   = 0;
    for (size_t i = 0; i < 7; ++i)
    {
        mTargetsPerDay[i] = 0;
    }
    return Protocols::InteractionModel::Status::Success;
}

void SimulatedEnergyEvse::OnStateChanged(Clusters::EnergyEvse::StateEnum newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: State changed to %u", to_underlying(newValue));
}

void SimulatedEnergyEvse::OnSupplyStateChanged(Clusters::EnergyEvse::SupplyStateEnum newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: SupplyState changed to %u", to_underlying(newValue));
}

void SimulatedEnergyEvse::OnFaultStateChanged(Clusters::EnergyEvse::FaultStateEnum newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: FaultState changed to %u", to_underlying(newValue));
}

void SimulatedEnergyEvse::OnChargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: ChargingEnabledUntil changed");
}

void SimulatedEnergyEvse::OnDischargingEnabledUntilChanged(DataModel::Nullable<uint32_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: DischargingEnabledUntil changed");
}

void SimulatedEnergyEvse::OnCircuitCapacityChanged(int64_t newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: CircuitCapacity changed");
}

void SimulatedEnergyEvse::OnMinimumChargeCurrentChanged(int64_t newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: MinimumChargeCurrent changed");
}

void SimulatedEnergyEvse::OnMaximumChargeCurrentChanged(int64_t newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: MaximumChargeCurrent changed");
}

void SimulatedEnergyEvse::OnMaximumDischargeCurrentChanged(int64_t newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: MaximumDischargeCurrent changed");
}

void SimulatedEnergyEvse::OnUserMaximumChargeCurrentChanged(int64_t newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: UserMaximumChargeCurrent changed");
}

void SimulatedEnergyEvse::OnRandomizationDelayWindowChanged(uint32_t newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: RandomizationDelayWindow changed");
}

void SimulatedEnergyEvse::OnNextChargeStartTimeChanged(DataModel::Nullable<uint32_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: NextChargeStartTime changed");
}

void SimulatedEnergyEvse::OnNextChargeTargetTimeChanged(DataModel::Nullable<uint32_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: NextChargeTargetTime changed");
}

void SimulatedEnergyEvse::OnNextChargeRequiredEnergyChanged(DataModel::Nullable<int64_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: NextChargeRequiredEnergy changed");
}

void SimulatedEnergyEvse::OnNextChargeTargetSoCChanged(DataModel::Nullable<Percent> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: NextChargeTargetSoC changed");
}

void SimulatedEnergyEvse::OnApproximateEVEfficiencyChanged(DataModel::Nullable<uint16_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: ApproximateEVEfficiency changed");
}

void SimulatedEnergyEvse::OnStateOfChargeChanged(DataModel::Nullable<Percent> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: StateOfCharge changed");
}

void SimulatedEnergyEvse::OnBatteryCapacityChanged(DataModel::Nullable<int64_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: BatteryCapacity changed");
}

void SimulatedEnergyEvse::OnVehicleIDChanged(DataModel::Nullable<CharSpan> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: VehicleID changed");
}

void SimulatedEnergyEvse::OnSessionIDChanged(DataModel::Nullable<uint32_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: SessionID changed");
}

void SimulatedEnergyEvse::OnSessionDurationChanged(DataModel::Nullable<uint32_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: SessionDuration changed");
}

void SimulatedEnergyEvse::OnSessionEnergyChargedChanged(DataModel::Nullable<int64_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: SessionEnergyCharged changed");
}

void SimulatedEnergyEvse::OnSessionEnergyDischargedChanged(DataModel::Nullable<int64_t> newValue)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: SessionEnergyDischarged changed");
}

// Clusters::ModeBase::AppDelegate
CHIP_ERROR SimulatedEnergyEvse::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedEnergyEvse::GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kEvseModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    return CopyCharSpanToMutableCharSpan(kEvseModeOptions[modeIndex].label, label);
}

CHIP_ERROR SimulatedEnergyEvse::GetModeValueByIndex(uint8_t modeIndex, uint8_t & value)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kEvseModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    value = kEvseModeOptions[modeIndex].mode;
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedEnergyEvse::GetModeTagsByIndex(uint8_t modeIndex,
                                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & tags)
{
    VerifyOrReturnError(modeIndex < MATTER_ARRAY_SIZE(kEvseModeOptions), CHIP_ERROR_PROVIDER_LIST_EXHAUSTED);
    const auto & modeTags = kEvseModeOptions[modeIndex].modeTags;
    VerifyOrReturnError(tags.size() >= modeTags.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    std::copy(modeTags.begin(), modeTags.end(), tags.begin());
    tags.reduce_size(modeTags.size());
    return CHIP_NO_ERROR;
}

void SimulatedEnergyEvse::HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response)
{
    ChipLogProgress(DeviceLayer, "EnergyEvseMode: ChangeToMode to %u", newMode);
    response.status = to_underlying(Clusters::ModeBase::StatusCode::kSuccess);
}

void SimulatedEnergyEvse::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: OnIdentifyStart");
}

void SimulatedEnergyEvse::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: OnIdentifyStop");
}

void SimulatedEnergyEvse::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "EnergyEvse: OnTriggerEffect");
}

} // namespace chip::app
