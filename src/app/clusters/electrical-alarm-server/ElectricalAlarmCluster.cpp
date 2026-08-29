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

#include <app/clusters/electrical-alarm-server/ElectricalAlarmCluster.h>

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ElectricalAlarm/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalAlarm;
using namespace chip::app::Clusters::ElectricalAlarm::Attributes;
using namespace chip::app::Clusters::ElectricalAlarm::Commands;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalAlarm {

// ---------------------------------------------------------------------------
// Startup: validate feature constraints
// ---------------------------------------------------------------------------

CHIP_ERROR ElectricalAlarmCluster::Startup(ServerClusterContext & context)
{
    if (mDelegate == nullptr)
    {
        ChipLogError(Zcl, "ElectricalAlarm ep %u: no delegate set — ModifyEnabledAlarms and Reset commands will be auto-approved",
                     static_cast<unsigned>(mPath.mEndpointId));
    }

    // ADJUST feature is only meaningful when at least one alarm-class feature is present.
    if (mFeatureFlags.Has(Feature::kAdjustableThresholds))
    {
        const bool hasAlarmClass = mFeatureFlags.HasAny(
            Feature::kOverVoltage, Feature::kUnderVoltage, Feature::kOverFrequency, Feature::kUnderFrequency, Feature::kOverPower,
            Feature::kUnderPower, Feature::kOverCurrent, Feature::kUnderCurrent, Feature::kPowerImport, Feature::kPowerExport);
        VerifyOrReturnError(hasAlarmClass, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl,
                                         "ElectricalAlarm: AdjustableThresholds feature requires at least one alarm-class "
                                         "feature to be enabled"));
    }
    return DefaultServerCluster::Startup(context);
}

// ---------------------------------------------------------------------------
// ReadAttribute
// ---------------------------------------------------------------------------

DataModel::ActionReturnStatus ElectricalAlarmCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                    AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureFlags);
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case Attributes::Mask::Id:
        return encoder.Encode(mMask);
    case Attributes::State::Id:
        return encoder.Encode(mState);
    case Attributes::Supported::Id:
        return encoder.Encode(mSupported);
    case Attributes::Latch::Id:
        if (!mFeatureFlags.Has(Feature::kReset))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mLatch);
    case Attributes::OverVoltageThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kOverVoltage))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mOverVoltageThreshold);
    case Attributes::UnderVoltageThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kUnderVoltage))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mUnderVoltageThreshold);
    case Attributes::OverFrequencyThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kOverFrequency))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mOverFrequencyThreshold);
    case Attributes::UnderFrequencyThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kUnderFrequency))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mUnderFrequencyThreshold);
    case Attributes::OverPowerThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kOverPower))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mOverPowerThreshold);
    case Attributes::UnderPowerThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kUnderPower))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mUnderPowerThreshold);
    case Attributes::OverCurrentThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kOverCurrent))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mOverCurrentThreshold);
    case Attributes::UnderCurrentThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kUnderCurrent))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mUnderCurrentThreshold);
    case Attributes::PowerImportThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kPowerImport))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mPowerImportThreshold);
    case Attributes::PowerExportThreshold::Id:
        if (!mFeatureFlags.Has(Feature::kPowerExport))
        {
            return Status::UnsupportedAttribute;
        }
        return encoder.Encode(mPowerExportThreshold);
    default:
        return Status::UnsupportedAttribute;
    }
}

// ---------------------------------------------------------------------------
// Attributes
// ---------------------------------------------------------------------------

CHIP_ERROR ElectricalAlarmCluster::Attributes(const ConcreteClusterPath & path,
                                              ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatureFlags.Has(Feature::kReset), Attributes::Latch::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kOverVoltage), Attributes::OverVoltageThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kUnderVoltage), Attributes::UnderVoltageThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kOverFrequency), Attributes::OverFrequencyThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kUnderFrequency), Attributes::UnderFrequencyThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kOverPower), Attributes::OverPowerThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kUnderPower), Attributes::UnderPowerThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kOverCurrent), Attributes::OverCurrentThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kUnderCurrent), Attributes::UnderCurrentThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kPowerImport), Attributes::PowerImportThreshold::kMetadataEntry },
        { mFeatureFlags.Has(Feature::kPowerExport), Attributes::PowerExportThreshold::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

// ---------------------------------------------------------------------------
// AcceptedCommands
// ---------------------------------------------------------------------------

CHIP_ERROR ElectricalAlarmCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    // ModifyEnabledAlarms is always available (spec: optionalConform with no feature gate).
    ReturnErrorOnFailure(builder.AppendElements({ Commands::ModifyEnabledAlarms::kMetadataEntry }));
    if (mFeatureFlags.Has(Feature::kReset))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::Reset::kMetadataEntry }));
    }
    if (mFeatureFlags.Has(Feature::kAdjustableThresholds))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SetElectricalAlarmThresholds::kMetadataEntry }));
    }
    return CHIP_NO_ERROR;
}

// ---------------------------------------------------------------------------
// InvokeCommand
// ---------------------------------------------------------------------------

std::optional<DataModel::ActionReturnStatus> ElectricalAlarmCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                   TLV::TLVReader & input_arguments,
                                                                                   CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::Reset::Id: {
        Commands::Reset::DecodableType data;
        if (data.Decode(input_arguments) != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
        return HandleReset(data.alarms);
    }
    case Commands::ModifyEnabledAlarms::Id: {
        Commands::ModifyEnabledAlarms::DecodableType data;
        if (data.Decode(input_arguments) != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
        return HandleModifyEnabledAlarms(data.mask);
    }
    case Commands::SetElectricalAlarmThresholds::Id: {
        Commands::SetElectricalAlarmThresholds::DecodableType data;
        if (data.Decode(input_arguments) != CHIP_NO_ERROR)
        {
            return Status::InvalidCommand;
        }
        return HandleSetThresholds(data);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

// ---------------------------------------------------------------------------
// Alarm Base attribute setters
// ---------------------------------------------------------------------------

Status ElectricalAlarmCluster::SetSupportedValue(BitMask<AlarmBitmap> supported)
{
    // Narrow Latch and Mask before committing Supported so the cascade path (SetMaskValue →
    // SetStateValue) always sees a consistent mSupported. The narrowed arguments are strict
    // subsets of the new Supported value, so neither cascade call can fail its subset guards.
    if (mFeatureFlags.Has(Feature::kReset) && !supported.HasAll(mLatch))
    {
        SetAttributeValue(mLatch, mLatch & supported, Attributes::Latch::Id);
    }

    if (!supported.HasAll(mMask))
    {
        // SetMaskValue validates mask ⊆ mSupported; update mSupported first so the check passes.
        SetAttributeValue(mSupported, supported, Attributes::Supported::Id);
        return SetMaskValue(mMask & supported);
    }

    SetAttributeValue(mSupported, supported, Attributes::Supported::Id);
    return Status::Success;
}

Status ElectricalAlarmCluster::SetMaskValue(BitMask<AlarmBitmap> mask)
{
    if (!mSupported.HasAll(mask))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: Mask contains unsupported bits");
        return Status::Failure;
    }
    SetAttributeValue(mMask, mask, Attributes::Mask::Id);

    // State must be a subset of Mask; trim newly-suppressed alarms.
    if (!mask.HasAll(mState))
    {
        return SetStateValue(mState & mask, /*ignoreLatchState=*/true);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetLatchValue(BitMask<AlarmBitmap> latch)
{
    if (!mFeatureFlags.Has(Feature::kReset))
    {
        return Status::UnsupportedAttribute;
    }
    if (!mSupported.HasAll(latch))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: Latch contains unsupported bits");
        return Status::Failure;
    }
    SetAttributeValue(mLatch, latch, Attributes::Latch::Id);
    return Status::Success;
}

Status ElectricalAlarmCluster::SetStateValue(BitMask<AlarmBitmap> newState, bool ignoreLatchState)
{
    if (!mSupported.HasAll(newState))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: Alarm is not supported");
        return Status::Failure;
    }
    if (!mMask.HasAll(newState))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: Alarm is suppressed");
        return Status::Failure;
    }

    BitMask<AlarmBitmap> finalNewState = newState;
    if (!ignoreLatchState && mFeatureFlags.Has(Feature::kReset))
    {
        // Preserve bits that are currently latched active.
        finalNewState.Set(mLatch & mState);
    }

    BitMask<AlarmBitmap> becameActive;
    becameActive.Set(finalNewState).Clear(mState);
    BitMask<AlarmBitmap> becameInactive;
    becameInactive.Set(mState).Clear(finalNewState);

    SetAttributeValue(mState, finalNewState, Attributes::State::Id);
    if (becameActive.HasAny() || becameInactive.HasAny())
    {
        SendNotifyEvent(becameActive, becameInactive, mState, mMask);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::ResetLatchedAlarms(BitMask<AlarmBitmap> alarms)
{
    if (!mFeatureFlags.Has(Feature::kReset))
    {
        return Status::UnsupportedCommand;
    }
    if (!mSupported.HasAll(alarms))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: Reset bitmap contains unsupported alarms");
        return Status::Failure;
    }
    BitMask<AlarmBitmap> newState = mState;
    newState.Clear(alarms);
    return SetStateValue(newState, /*ignoreLatchState=*/true);
}

// ---------------------------------------------------------------------------
// Threshold setters
// ---------------------------------------------------------------------------

Status ElectricalAlarmCluster::SetOverVoltageThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverVoltage), Status::UnsupportedAttribute);
    SetAttributeValue(mOverVoltageThreshold, value, Attributes::OverVoltageThreshold::Id);
    mOverVoltageThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderVoltageThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderVoltage), Status::UnsupportedAttribute);
    SetAttributeValue(mUnderVoltageThreshold, value, Attributes::UnderVoltageThreshold::Id);
    mUnderVoltageThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverFrequencyThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverFrequency), Status::UnsupportedAttribute);
    SetAttributeValue(mOverFrequencyThreshold, value, Attributes::OverFrequencyThreshold::Id);
    mOverFrequencyThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderFrequencyThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderFrequency), Status::UnsupportedAttribute);
    SetAttributeValue(mUnderFrequencyThreshold, value, Attributes::UnderFrequencyThreshold::Id);
    mUnderFrequencyThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverPowerThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverPower), Status::UnsupportedAttribute);
    SetAttributeValue(mOverPowerThreshold, value, Attributes::OverPowerThreshold::Id);
    mOverPowerThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderPowerThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderPower), Status::UnsupportedAttribute);
    SetAttributeValue(mUnderPowerThreshold, value, Attributes::UnderPowerThreshold::Id);
    mUnderPowerThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverCurrentThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverCurrent), Status::UnsupportedAttribute);
    SetAttributeValue(mOverCurrentThreshold, value, Attributes::OverCurrentThreshold::Id);
    mOverCurrentThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderCurrentThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderCurrent), Status::UnsupportedAttribute);
    SetAttributeValue(mUnderCurrentThreshold, value, Attributes::UnderCurrentThreshold::Id);
    mUnderCurrentThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetPowerImportThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kPowerImport), Status::UnsupportedAttribute);
    SetAttributeValue(mPowerImportThreshold, value, Attributes::PowerImportThreshold::Id);
    mPowerImportThresholdSet = true;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetPowerExportThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kPowerExport), Status::UnsupportedAttribute);
    SetAttributeValue(mPowerExportThreshold, value, Attributes::PowerExportThreshold::Id);
    mPowerExportThresholdSet = true;
    return Status::Success;
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

Status ElectricalAlarmCluster::HandleModifyEnabledAlarms(BitMask<AlarmBitmap> mask)
{
    if (!mSupported.HasAll(mask))
    {
        return Status::InvalidCommand;
    }
    if (mDelegate != nullptr && !mDelegate->ModifyEnabledAlarmsCallback(mask))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: delegate rejected ModifyEnabledAlarms");
        return Status::Failure;
    }
    return SetMaskValue(mask);
}

Status ElectricalAlarmCluster::HandleReset(BitMask<AlarmBitmap> alarms)
{
    if (!mFeatureFlags.Has(Feature::kReset))
    {
        return Status::UnsupportedCommand;
    }
    if (mDelegate != nullptr && !mDelegate->ResetAlarmsCallback(alarms))
    {
        ChipLogProgress(Zcl, "ElectricalAlarm: delegate rejected Reset");
        return Status::Failure;
    }
    return ResetLatchedAlarms(alarms);
}

Status ElectricalAlarmCluster::HandleSetThresholds(const Commands::SetElectricalAlarmThresholds::DecodableType & data)
{
    if (!mFeatureFlags.Has(Feature::kAdjustableThresholds))
    {
        return Status::UnsupportedCommand;
    }

    // Validate cross-pair ordering. Both the incoming optional value AND any previously-persisted
    // value count as a known baseline. mXxxThresholdSet flags distinguish an explicit zero from
    // "not yet set" (0 is a valid threshold value per the spec).
    //
    // strictOver=true:  over must be strictly > under (voltage, frequency, power, current pairs).
    // strictOver=false: over must be >= under (import/export — both may be 0 simultaneously, since
    //                   import=0 and export=0 is a valid quiescent state per attribute constraints).
    auto checkPair = [&](const Optional<int64_t> & over, const Optional<int64_t> & under, int64_t storedOver, bool storedOverSet,
                         int64_t storedUnder, bool storedUnderSet, bool strictOver = true) -> Status {
        if (!over.HasValue() && !under.HasValue())
        {
            return Status::Success;
        }
        const bool overKnown  = over.HasValue() || storedOverSet;
        const bool underKnown = under.HasValue() || storedUnderSet;
        if (!overKnown || !underKnown)
        {
            return Status::Success;
        }
        int64_t resolvedOver  = over.ValueOr(storedOver);
        int64_t resolvedUnder = under.ValueOr(storedUnder);
        const bool violated   = strictOver ? (resolvedOver <= resolvedUnder) : (resolvedOver < resolvedUnder);
        return violated ? Status::ConstraintError : Status::Success;
    };

    Status s;
    if ((s = checkPair(data.overVoltageThreshold, data.underVoltageThreshold, mOverVoltageThreshold, mOverVoltageThresholdSet,
                       mUnderVoltageThreshold, mUnderVoltageThresholdSet)) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overFrequencyThreshold, data.underFrequencyThreshold, mOverFrequencyThreshold,
                       mOverFrequencyThresholdSet, mUnderFrequencyThreshold, mUnderFrequencyThresholdSet)) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overPowerThreshold, data.underPowerThreshold, mOverPowerThreshold, mOverPowerThresholdSet,
                       mUnderPowerThreshold, mUnderPowerThresholdSet)) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overCurrentThreshold, data.underCurrentThreshold, mOverCurrentThreshold, mOverCurrentThresholdSet,
                       mUnderCurrentThreshold, mUnderCurrentThresholdSet)) != Status::Success)
    {
        return s;
    }
    // PowerImport >= 0 and PowerExport <= 0 by attribute min/max in ZAP. Import=0 and export=0 is a
    // valid quiescent state (no power flowing), so use strictOver=false (>= rather than >).
    if ((s = checkPair(data.powerImportThreshold, data.powerExportThreshold, mPowerImportThreshold, mPowerImportThresholdSet,
                       mPowerExportThreshold, mPowerExportThresholdSet, /*strictOver=*/false)) != Status::Success)
    {
        return s;
    }

    if (mDelegate != nullptr && !mDelegate->SetElectricalAlarmThresholdsCallback(data))
    {
        return Status::Failure;
    }

    // Persist each present field via the setter so SetAttributeValue fires for subscriptions.
    // If a field is present in the command but the corresponding alarm-class feature is absent,
    // the setter returns UnsupportedAttribute — treat that as InvalidCommand so the caller
    // knows the field was not accepted rather than silently dropping it.
    auto applyThreshold = [](Status result) -> Status {
        if (result == Status::UnsupportedAttribute)
        {
            return Status::InvalidCommand;
        }
        return result;
    };
    if (data.overVoltageThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetOverVoltageThreshold(data.overVoltageThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.underVoltageThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetUnderVoltageThreshold(data.underVoltageThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.overFrequencyThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetOverFrequencyThreshold(data.overFrequencyThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.underFrequencyThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetUnderFrequencyThreshold(data.underFrequencyThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.overPowerThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetOverPowerThreshold(data.overPowerThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.underPowerThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetUnderPowerThreshold(data.underPowerThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.overCurrentThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetOverCurrentThreshold(data.overCurrentThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.underCurrentThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetUnderCurrentThreshold(data.underCurrentThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.powerImportThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetPowerImportThreshold(data.powerImportThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }
    if (data.powerExportThreshold.HasValue())
    {
        VerifyOrReturnValue(applyThreshold(SetPowerExportThreshold(data.powerExportThreshold.Value())) == Status::Success,
                            Status::InvalidCommand);
    }

    return Status::Success;
}

// ---------------------------------------------------------------------------
// Event
// ---------------------------------------------------------------------------

void ElectricalAlarmCluster::SendNotifyEvent(BitMask<AlarmBitmap> becameActive, BitMask<AlarmBitmap> becameInactive,
                                             BitMask<AlarmBitmap> newState, BitMask<AlarmBitmap> mask)
{
    if (mContext == nullptr)
    {
        return;
    }
    Events::Notify::Type event{ .active = becameActive, .inactive = becameInactive, .state = newState, .mask = mask };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace ElectricalAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
