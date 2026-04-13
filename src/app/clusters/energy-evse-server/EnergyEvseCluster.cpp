/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/EnergyEvse/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

CHIP_ERROR EnergyEvseCluster::Startup(ServerClusterContext & context)
{
    if (mDelegate.GetEndpointId() != mPath.mEndpointId)
    {
        ChipLogError(Zcl, "EVSE: EndpointId mismatch - delegate has %d, cluster has %d", mDelegate.GetEndpointId(),
                     mPath.mEndpointId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return DefaultServerCluster::Startup(context);
}

CHIP_ERROR EnergyEvseCluster::SetState(StateEnum newValue)
{
    VerifyOrReturnError(mState != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue < StateEnum::kUnknownEnumValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mState = newValue;
    mDelegate.OnStateChanged(newValue);
    NotifyAttributeChanged(State::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetSupplyState(SupplyStateEnum newValue)
{
    VerifyOrReturnError(mSupplyState != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue < SupplyStateEnum::kUnknownEnumValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mSupplyState = newValue;
    mDelegate.OnSupplyStateChanged(newValue);
    NotifyAttributeChanged(SupplyState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetFaultState(FaultStateEnum newValue)
{
    VerifyOrReturnError(mFaultState != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue < FaultStateEnum::kUnknownEnumValue, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mFaultState = newValue;
    mDelegate.OnFaultStateChanged(newValue);
    NotifyAttributeChanged(FaultState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetChargingEnabledUntil(DataModel::Nullable<uint32_t> newValue)
{
    VerifyOrReturnError(mChargingEnabledUntil != newValue, CHIP_NO_ERROR);
    mChargingEnabledUntil = newValue;
    mDelegate.OnChargingEnabledUntilChanged(newValue);
    NotifyAttributeChanged(ChargingEnabledUntil::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetDischargingEnabledUntil(DataModel::Nullable<uint32_t> newValue)
{
    VerifyOrReturnError(mDischargingEnabledUntil != newValue, CHIP_NO_ERROR);
    mDischargingEnabledUntil = newValue;
    mDelegate.OnDischargingEnabledUntilChanged(newValue);
    NotifyAttributeChanged(DischargingEnabledUntil::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetCircuitCapacity(int64_t newValue)
{
    VerifyOrReturnError(mCircuitCapacity != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue >= 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mCircuitCapacity = newValue;
    mDelegate.OnCircuitCapacityChanged(newValue);
    NotifyAttributeChanged(CircuitCapacity::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetMinimumChargeCurrent(int64_t newValue)
{
    VerifyOrReturnError(mMinimumChargeCurrent != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue >= 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mMinimumChargeCurrent = newValue;
    mDelegate.OnMinimumChargeCurrentChanged(newValue);
    NotifyAttributeChanged(MinimumChargeCurrent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetMaximumChargeCurrent(int64_t newValue)
{
    VerifyOrReturnError(mMaximumChargeCurrent != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue >= 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mMaximumChargeCurrent = newValue;
    mDelegate.OnMaximumChargeCurrentChanged(newValue);
    NotifyAttributeChanged(MaximumChargeCurrent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetMaximumDischargeCurrent(int64_t newValue)
{
    VerifyOrReturnError(mMaximumDischargeCurrent != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue >= 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mMaximumDischargeCurrent = newValue;
    mDelegate.OnMaximumDischargeCurrentChanged(newValue);
    NotifyAttributeChanged(MaximumDischargeCurrent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetUserMaximumChargeCurrent(int64_t newValue)
{
    VerifyOrReturnError(mUserMaximumChargeCurrent != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue >= 0, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mUserMaximumChargeCurrent = newValue;
    mDelegate.OnUserMaximumChargeCurrentChanged(newValue);
    NotifyAttributeChanged(UserMaximumChargeCurrent::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetRandomizationDelayWindow(uint32_t newValue)
{
    VerifyOrReturnError(mRandomizationDelayWindow != newValue, CHIP_NO_ERROR);
    VerifyOrReturnError(newValue <= kMaxRandomizationDelayWindowSec, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    mRandomizationDelayWindow = newValue;
    mDelegate.OnRandomizationDelayWindowChanged(newValue);
    NotifyAttributeChanged(RandomizationDelayWindow::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetNextChargeStartTime(DataModel::Nullable<uint32_t> newValue)
{
    VerifyOrReturnError(mNextChargeStartTime != newValue, CHIP_NO_ERROR);
    mNextChargeStartTime = newValue;
    mDelegate.OnNextChargeStartTimeChanged(newValue);
    NotifyAttributeChanged(NextChargeStartTime::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetNextChargeTargetTime(DataModel::Nullable<uint32_t> newValue)
{
    VerifyOrReturnError(mNextChargeTargetTime != newValue, CHIP_NO_ERROR);
    mNextChargeTargetTime = newValue;
    mDelegate.OnNextChargeTargetTimeChanged(newValue);
    NotifyAttributeChanged(NextChargeTargetTime::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetNextChargeRequiredEnergy(DataModel::Nullable<int64_t> newValue)
{
    VerifyOrReturnError(mNextChargeRequiredEnergy != newValue, CHIP_NO_ERROR);
    mNextChargeRequiredEnergy = newValue;
    mDelegate.OnNextChargeRequiredEnergyChanged(newValue);
    NotifyAttributeChanged(NextChargeRequiredEnergy::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetNextChargeTargetSoC(DataModel::Nullable<Percent> newValue)
{
    VerifyOrReturnError(mNextChargeTargetSoC != newValue, CHIP_NO_ERROR);
    mNextChargeTargetSoC = newValue;
    mDelegate.OnNextChargeTargetSoCChanged(newValue);
    NotifyAttributeChanged(NextChargeTargetSoC::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetApproximateEVEfficiency(DataModel::Nullable<uint16_t> newValue)
{
    VerifyOrReturnError(mApproximateEVEfficiency != newValue, CHIP_NO_ERROR);
    mApproximateEVEfficiency = newValue;
    mDelegate.OnApproximateEVEfficiencyChanged(newValue);
    NotifyAttributeChanged(ApproximateEVEfficiency::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetStateOfCharge(DataModel::Nullable<Percent> newValue)
{
    VerifyOrReturnError(mStateOfCharge != newValue, CHIP_NO_ERROR);
    mStateOfCharge = newValue;
    mDelegate.OnStateOfChargeChanged(newValue);
    NotifyAttributeChanged(StateOfCharge::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetBatteryCapacity(DataModel::Nullable<int64_t> newValue)
{
    VerifyOrReturnError(mBatteryCapacity != newValue, CHIP_NO_ERROR);
    mBatteryCapacity = newValue;
    mDelegate.OnBatteryCapacityChanged(newValue);
    NotifyAttributeChanged(BatteryCapacity::Id);
    return CHIP_NO_ERROR;
}

DataModel::Nullable<CharSpan> EnergyEvseCluster::GetVehicleID() const
{
    if (mVehicleID.IsNull())
    {
        return DataModel::NullNullable;
    }
    return DataModel::MakeNullable(CharSpan(mVehicleIDBuffer, mVehicleID.Value().size()));
}

CHIP_ERROR EnergyEvseCluster::SetVehicleID(DataModel::Nullable<CharSpan> newValue)
{
    if (newValue.IsNull())
    {
        if (mVehicleID.IsNull())
        {
            return CHIP_NO_ERROR;
        }
        mVehicleID = DataModel::NullNullable;
    }
    else
    {
        VerifyOrReturnError(newValue.Value().size() <= kMaxVehicleIDBufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
        if (!mVehicleID.IsNull() && newValue.Value().data_equal(mVehicleID.Value()))
        {
            return CHIP_NO_ERROR;
        }
        memcpy(mVehicleIDBuffer, newValue.Value().data(), newValue.Value().size());
        mVehicleID = DataModel::MakeNullable(CharSpan(mVehicleIDBuffer, newValue.Value().size()));
    }
    mDelegate.OnVehicleIDChanged(mVehicleID);
    NotifyAttributeChanged(VehicleID::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetSessionID(DataModel::Nullable<uint32_t> newValue)
{
    VerifyOrReturnError(mSessionID != newValue, CHIP_NO_ERROR);
    mSessionID = newValue;
    mDelegate.OnSessionIDChanged(newValue);
    NotifyAttributeChanged(SessionID::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetSessionDuration(DataModel::Nullable<uint32_t> newValue)
{
    // We do not check for equality here because the tests expect reports at session boundaries (start/stop) even when the value has
    // not changed (e.g. energy stayed at 0 because no charging occurred).
    mSessionDuration = newValue;
    mDelegate.OnSessionDurationChanged(newValue);
    NotifyAttributeChanged(SessionDuration::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetSessionEnergyCharged(DataModel::Nullable<int64_t> newValue)
{
    // We do not check for equality here because the tests expect reports at session boundaries (start/stop) even when the value has
    // not changed (e.g. energy stayed at 0 because no charging occurred).
    mSessionEnergyCharged = newValue;
    mDelegate.OnSessionEnergyChargedChanged(newValue);
    NotifyAttributeChanged(SessionEnergyCharged::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::SetSessionEnergyDischarged(DataModel::Nullable<int64_t> newValue)
{
    // We do not check for equality here because the tests expect reports at session boundaries (start/stop) even when the value has
    // not changed (e.g. energy stayed at 0 because no charging occurred).
    mSessionEnergyDischarged = newValue;
    mDelegate.OnSessionEnergyDischargedChanged(newValue);
    NotifyAttributeChanged(SessionEnergyDischarged::Id);
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus EnergyEvseCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case State::Id:
        return encoder.Encode(mState);
    case SupplyState::Id:
        return encoder.Encode(mSupplyState);
    case FaultState::Id:
        return encoder.Encode(mFaultState);
    case ChargingEnabledUntil::Id:
        return encoder.Encode(mChargingEnabledUntil);
    case DischargingEnabledUntil::Id:
        return encoder.Encode(mDischargingEnabledUntil);
    case CircuitCapacity::Id:
        return encoder.Encode(mCircuitCapacity);
    case MinimumChargeCurrent::Id:
        return encoder.Encode(mMinimumChargeCurrent);
    case MaximumChargeCurrent::Id:
        return encoder.Encode(mMaximumChargeCurrent);
    case MaximumDischargeCurrent::Id:
        return encoder.Encode(mMaximumDischargeCurrent);
    case UserMaximumChargeCurrent::Id:
        return encoder.Encode(mUserMaximumChargeCurrent);
    case RandomizationDelayWindow::Id:
        return encoder.Encode(mRandomizationDelayWindow);
    case NextChargeStartTime::Id:
        return encoder.Encode(mNextChargeStartTime);
    case NextChargeTargetTime::Id:
        return encoder.Encode(mNextChargeTargetTime);
    case NextChargeRequiredEnergy::Id:
        return encoder.Encode(mNextChargeRequiredEnergy);
    case NextChargeTargetSoC::Id:
        return encoder.Encode(mNextChargeTargetSoC);
    case ApproximateEVEfficiency::Id:
        return encoder.Encode(mApproximateEVEfficiency);
    case StateOfCharge::Id:
        return encoder.Encode(mStateOfCharge);
    case BatteryCapacity::Id:
        return encoder.Encode(mBatteryCapacity);
    case VehicleID::Id:
        return encoder.Encode(GetVehicleID());
    case SessionID::Id:
        return encoder.Encode(mSessionID);
    case SessionDuration::Id:
        return encoder.Encode(mSessionDuration);
    case SessionEnergyCharged::Id:
        return encoder.Encode(mSessionEnergyCharged);
    case SessionEnergyDischarged::Id:
        return encoder.Encode(mSessionEnergyDischarged);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus EnergyEvseCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case UserMaximumChargeCurrent::Id: {
        UserMaximumChargeCurrent::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mUserMaximumChargeCurrent != value, ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        return SetUserMaximumChargeCurrent(value);
    }
    case RandomizationDelayWindow::Id: {
        RandomizationDelayWindow::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mRandomizationDelayWindow != value, ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        return SetRandomizationDelayWindow(value);
    }
    case ApproximateEVEfficiency::Id: {
        ApproximateEVEfficiency::TypeInfo::DecodableType value;
        ReturnErrorOnFailure(decoder.Decode(value));
        VerifyOrReturnValue(mApproximateEVEfficiency != value, ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        return SetApproximateEVEfficiency(value);
    }
    default:
        return Status::UnsupportedAttribute;
    }
}
std::optional<DataModel::ActionReturnStatus> EnergyEvseCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    using namespace Commands;

    switch (request.path.mCommandId)
    {
    case Disable::Id:
        return HandleDisable(request, input_arguments, handler);
    case EnableCharging::Id:
        return HandleEnableCharging(request, input_arguments, handler);
    case EnableDischarging::Id:
        return HandleEnableDischarging(request, input_arguments, handler);
    case StartDiagnostics::Id:
        return HandleStartDiagnostics(request, input_arguments, handler);
    case SetTargets::Id:
        return HandleSetTargets(request, input_arguments, handler);
    case GetTargets::Id:
        return HandleGetTargets(request, input_arguments, handler);
    case ClearTargets::Id:
        return HandleClearTargets(request, input_arguments, handler);
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR EnergyEvseCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        // V2x feature attributes
        { mFeatureFlags.Has(Feature::kV2x), DischargingEnabledUntil::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kV2x), MaximumDischargeCurrent::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kV2x), SessionEnergyDischarged::kMetadataEntry },
        // ChargingPreferences feature attributes
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeStartTime::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeTargetTime::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeRequiredEnergy::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kChargingPreferences), NextChargeTargetSoC::kMetadataEntry },
        { (mFeatureFlags.Has(Feature::kChargingPreferences) &&
           mOptionalAttrs.Has(OptionalAttributes::kSupportsApproximateEvEfficiency)),
          ApproximateEVEfficiency::kMetadataEntry },
        // SoCReporting feature attributes
        { mFeatureFlags.Has(Feature::kSoCReporting), StateOfCharge::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kSoCReporting), BatteryCapacity::kMetadataEntry },
        // PlugAndCharge feature attribute
        { mFeatureFlags.Has(Feature::kPlugAndCharge), VehicleID::kMetadataEntry },
        // Optional attributes (not feature-based)
        { mOptionalAttrs.Has(OptionalAttributes::kSupportsUserMaximumChargingCurrent), UserMaximumChargeCurrent::kMetadataEntry },
        { mOptionalAttrs.Has(OptionalAttributes::kSupportsRandomizationWindow), RandomizationDelayWindow::kMetadataEntry },
    };
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR EnergyEvseCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Commands;
    ReturnErrorOnFailure(builder.AppendElements({ Disable::kMetadataEntry, EnableCharging::kMetadataEntry }));
    if (mFeatureFlags.Has(Feature::kV2x))
    {
        ReturnErrorOnFailure(builder.AppendElements({ EnableDischarging::kMetadataEntry }));
    }
    if (mOptionalCmds.Has(OptionalCommands::kSupportsStartDiagnostics))
    {
        ReturnErrorOnFailure(builder.AppendElements({ StartDiagnostics::kMetadataEntry }));
    }
    if (mFeatureFlags.Has(Feature::kChargingPreferences))
    {
        ReturnErrorOnFailure(
            builder.AppendElements({ SetTargets::kMetadataEntry, GetTargets::kMetadataEntry, ClearTargets::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR EnergyEvseCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace Commands;
    if (mFeatureFlags.Has(Feature::kChargingPreferences))
    {
        ReturnErrorOnFailure(builder.AppendElements({ GetTargetsResponse::Id }));
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleDisable(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return mDelegate.Disable();
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleEnableCharging(const DataModel::InvokeRequest & request,
                                                                      TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::EnableCharging::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    auto & chargingEnabledUntil = commandData.chargingEnabledUntil;
    auto & minimumChargeCurrent = commandData.minimumChargeCurrent;
    auto & maximumChargeCurrent = commandData.maximumChargeCurrent;

    VerifyOrReturnValue(minimumChargeCurrent >= kMinimumChargeCurrentLimit, Status::ConstraintError);
    VerifyOrReturnValue(maximumChargeCurrent >= kMinimumChargeCurrentLimit, Status::ConstraintError);
    VerifyOrReturnValue(minimumChargeCurrent <= maximumChargeCurrent, Status::ConstraintError);

    return mDelegate.EnableCharging(chargingEnabledUntil, minimumChargeCurrent, maximumChargeCurrent);
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleEnableDischarging(const DataModel::InvokeRequest & request,
                                                                         TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::EnableDischarging::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    auto & dischargingEnabledUntil = commandData.dischargingEnabledUntil;
    auto & maximumDischargeCurrent = commandData.maximumDischargeCurrent;

    VerifyOrReturnValue(maximumDischargeCurrent >= kMinimumChargeCurrentLimit, Status::ConstraintError);

    return mDelegate.EnableDischarging(dischargingEnabledUntil, maximumDischargeCurrent);
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleStartDiagnostics(const DataModel::InvokeRequest & request,
                                                                        TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return mDelegate.StartDiagnostics();
}

Status EnergyEvseCluster::ValidateTargets(
    const DataModel::DecodableList<Structs::ChargingTargetScheduleStruct::DecodableType> & chargingTargetSchedules)
{
    uint8_t dayOfWeekBitmap = 0;

    auto iter = chargingTargetSchedules.begin();
    while (iter.Next())
    {
        auto & entry    = iter.GetValue();
        uint8_t bitmask = entry.dayOfWeekForSequence.GetField(static_cast<TargetDayOfWeekBitmap>(kDayOfWeekBitmapMask));
        ChipLogProgress(AppServer, "DayOfWeekForSequence = 0x%02x", bitmask);

        VerifyOrReturnValue((dayOfWeekBitmap & bitmask) == 0, Status::ConstraintError,
                            ChipLogError(AppServer, "DayOfWeekForSequence bit already set"));
        dayOfWeekBitmap |= bitmask;

        auto iterInner   = entry.chargingTargets.begin();
        uint8_t innerIdx = 0;
        while (iterInner.Next())
        {
            auto & targetStruct          = iterInner.GetValue();
            uint16_t minutesPastMidnight = targetStruct.targetTimeMinutesPastMidnight;
            ChipLogProgress(AppServer, "[%d] MinutesPastMidnight : %d", innerIdx,
                            static_cast<short unsigned int>(minutesPastMidnight));

            VerifyOrReturnValue(minutesPastMidnight <= kMaxMinutesPastMidnight, Status::ConstraintError,
                                ChipLogError(AppServer, "MinutesPastMidnight invalid: %d", static_cast<int>(minutesPastMidnight)));

            if (mFeatureFlags.Has(Feature::kSoCReporting))
            {
                VerifyOrReturnValue(targetStruct.targetSoC.HasValue(), Status::InvalidCommand,
                                    ChipLogError(AppServer, "SoCReporting enabled but TargetSoC missing"));
                VerifyOrReturnValue(
                    targetStruct.targetSoC.Value() <= kMaxTargetSoCPercent, Status::ConstraintError,
                    ChipLogError(AppServer, "TargetSoC invalid: %d", static_cast<int>(targetStruct.targetSoC.Value())));
            }
            else
            {
                VerifyOrReturnValue(!targetStruct.targetSoC.HasValue() || targetStruct.targetSoC.Value() == kMaxTargetSoCPercent,
                                    Status::ConstraintError,
                                    ChipLogError(AppServer, "TargetSoC must be 100 if SOC feature not supported"));
            }

            VerifyOrReturnValue(targetStruct.targetSoC.HasValue() || targetStruct.addedEnergy.HasValue(), Status::Failure,
                                ChipLogError(AppServer, "Must have one of AddedEnergy or TargetSoC"));

            VerifyOrReturnValue(
                !targetStruct.addedEnergy.HasValue() || targetStruct.addedEnergy.Value() >= 0, Status::ConstraintError,
                ChipLogError(AppServer, "AddedEnergy invalid: %ld", static_cast<long>(targetStruct.addedEnergy.Value())));
            innerIdx++;
        }

        VerifyOrReturnValue(innerIdx <= kEvseTargetsMaxTargetsPerDay, Status::ResourceExhausted,
                            ChipLogError(AppServer, "Too many targets: %d", innerIdx));
        VerifyOrReturnValue(iterInner.GetStatus() == CHIP_NO_ERROR, Status::InvalidCommand);
    }

    VerifyOrReturnValue(iter.GetStatus() == CHIP_NO_ERROR, Status::InvalidCommand);

    return Status::Success;
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleSetTargets(const DataModel::InvokeRequest & request,
                                                                  TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    Commands::SetTargets::DecodableType commandData;
    ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

    auto & chargingTargetSchedules = commandData.chargingTargetSchedules;

    Status status = ValidateTargets(chargingTargetSchedules);
    VerifyOrReturnValue(status == Status::Success, status, ChipLogError(AppServer, "SetTargets validation failed"));

    return mDelegate.SetTargets(chargingTargetSchedules);
}

std::optional<DataModel::ActionReturnStatus> EnergyEvseCluster::HandleGetTargets(const DataModel::InvokeRequest & request,
                                                                                 TLV::TLVReader & input_arguments,
                                                                                 CommandHandler * handler)
{
    Commands::GetTargetsResponse::Type response;
    Status status = mDelegate.GetTargets(response.chargingTargetSchedules);
    VerifyOrReturnValue(status == Status::Success, status);

    handler->AddResponse(request.path, response);
    return std::nullopt;
}

DataModel::ActionReturnStatus EnergyEvseCluster::HandleClearTargets(const DataModel::InvokeRequest & request,
                                                                    TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return mDelegate.ClearTargets();
}

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
