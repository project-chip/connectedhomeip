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
    if (mOverVoltageThreshold != value)
    {
        mOverVoltageThreshold = value;
        NotifyAttributeChanged(Attributes::OverVoltageThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderVoltageThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderVoltage), Status::UnsupportedAttribute);
    if (mUnderVoltageThreshold != value)
    {
        mUnderVoltageThreshold = value;
        NotifyAttributeChanged(Attributes::UnderVoltageThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverFrequencyThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverFrequency), Status::UnsupportedAttribute);
    if (mOverFrequencyThreshold != value)
    {
        mOverFrequencyThreshold = value;
        NotifyAttributeChanged(Attributes::OverFrequencyThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderFrequencyThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderFrequency), Status::UnsupportedAttribute);
    if (mUnderFrequencyThreshold != value)
    {
        mUnderFrequencyThreshold = value;
        NotifyAttributeChanged(Attributes::UnderFrequencyThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverPowerThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverPower), Status::UnsupportedAttribute);
    if (mOverPowerThreshold != value)
    {
        mOverPowerThreshold = value;
        NotifyAttributeChanged(Attributes::OverPowerThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderPowerThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderPower), Status::UnsupportedAttribute);
    if (mUnderPowerThreshold != value)
    {
        mUnderPowerThreshold = value;
        NotifyAttributeChanged(Attributes::UnderPowerThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverCurrentThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverCurrent), Status::UnsupportedAttribute);
    if (mOverCurrentThreshold != value)
    {
        mOverCurrentThreshold = value;
        NotifyAttributeChanged(Attributes::OverCurrentThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderCurrentThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderCurrent), Status::UnsupportedAttribute);
    if (mUnderCurrentThreshold != value)
    {
        mUnderCurrentThreshold = value;
        NotifyAttributeChanged(Attributes::UnderCurrentThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetPowerImportThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kPowerImport), Status::UnsupportedAttribute);
    if (mPowerImportThreshold != value)
    {
        mPowerImportThreshold = value;
        NotifyAttributeChanged(Attributes::PowerImportThreshold::Id);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::SetPowerExportThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kPowerExport), Status::UnsupportedAttribute);
    if (mPowerExportThreshold != value)
    {
        mPowerExportThreshold = value;
        NotifyAttributeChanged(Attributes::PowerExportThreshold::Id);
    }
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
    if (!mSupported.HasAll(alarms))
    {
        return Status::InvalidCommand;
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

    // Validate that every supplied field targets a feature that is present. Do this before any
    // constraint check or state mutation so the command is either fully accepted or fully rejected —
    // applying some fields and then failing on a later one would leave state partially mutated.
    if ((data.overVoltageThreshold.HasValue() && !mFeatureFlags.Has(Feature::kOverVoltage)) ||
        (data.underVoltageThreshold.HasValue() && !mFeatureFlags.Has(Feature::kUnderVoltage)) ||
        (data.overFrequencyThreshold.HasValue() && !mFeatureFlags.Has(Feature::kOverFrequency)) ||
        (data.underFrequencyThreshold.HasValue() && !mFeatureFlags.Has(Feature::kUnderFrequency)) ||
        (data.overPowerThreshold.HasValue() && !mFeatureFlags.Has(Feature::kOverPower)) ||
        (data.underPowerThreshold.HasValue() && !mFeatureFlags.Has(Feature::kUnderPower)) ||
        (data.overCurrentThreshold.HasValue() && !mFeatureFlags.Has(Feature::kOverCurrent)) ||
        (data.underCurrentThreshold.HasValue() && !mFeatureFlags.Has(Feature::kUnderCurrent)) ||
        (data.powerImportThreshold.HasValue() && !mFeatureFlags.Has(Feature::kPowerImport)) ||
        (data.powerExportThreshold.HasValue() && !mFeatureFlags.Has(Feature::kPowerExport)))
    {
        return Status::InvalidCommand;
    }

    // Validate absolute per-field constraints before cross-pair ordering.
    // PowerImportThreshold min 0, PowerExportThreshold max 0 (from spec and XML).
    if (data.powerImportThreshold.HasValue() && data.powerImportThreshold.Value() < 0)
    {
        return Status::ConstraintError;
    }
    if (data.powerExportThreshold.HasValue() && data.powerExportThreshold.Value() > 0)
    {
        return Status::ConstraintError;
    }

    // Validate cross-pair ordering. If a feature is enabled, the stored Fallback value is always
    // a valid baseline — use feature presence rather than a "has been explicitly written" flag.
    //
    // For voltage/frequency/power/current pairs: over must be strictly > under (over >= under + 1).
    // For import/export: (0, 0) is the valid quiescent state; otherwise over must be > under.
    auto checkPair = [&](const Optional<int64_t> & over, const Optional<int64_t> & under, int64_t storedOver,
                         bool overFeatureEnabled, int64_t storedUnder, bool underFeatureEnabled) -> Status {
        if (!over.HasValue() && !under.HasValue())
        {
            return Status::Success;
        }
        const bool overKnown  = over.HasValue() || overFeatureEnabled;
        const bool underKnown = under.HasValue() || underFeatureEnabled;
        if (!overKnown || !underKnown)
        {
            return Status::Success;
        }
        int64_t resolvedOver  = over.ValueOr(storedOver);
        int64_t resolvedUnder = under.ValueOr(storedUnder);
        return (resolvedOver <= resolvedUnder) ? Status::ConstraintError : Status::Success;
    };

    Status s;
    if ((s = checkPair(data.overVoltageThreshold, data.underVoltageThreshold, mOverVoltageThreshold,
                       mFeatureFlags.Has(Feature::kOverVoltage), mUnderVoltageThreshold,
                       mFeatureFlags.Has(Feature::kUnderVoltage))) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overFrequencyThreshold, data.underFrequencyThreshold, mOverFrequencyThreshold,
                       mFeatureFlags.Has(Feature::kOverFrequency), mUnderFrequencyThreshold,
                       mFeatureFlags.Has(Feature::kUnderFrequency))) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overPowerThreshold, data.underPowerThreshold, mOverPowerThreshold,
                       mFeatureFlags.Has(Feature::kOverPower), mUnderPowerThreshold,
                       mFeatureFlags.Has(Feature::kUnderPower))) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overCurrentThreshold, data.underCurrentThreshold, mOverCurrentThreshold,
                       mFeatureFlags.Has(Feature::kOverCurrent), mUnderCurrentThreshold,
                       mFeatureFlags.Has(Feature::kUnderCurrent))) != Status::Success)
    {
        return s;
    }
    // PowerImport/Export: (0, 0) is valid quiescent state; otherwise import must be > export.
    if (data.powerImportThreshold.HasValue() || data.powerExportThreshold.HasValue())
    {
        int64_t resolvedImport = data.powerImportThreshold.ValueOr(mPowerImportThreshold);
        int64_t resolvedExport = data.powerExportThreshold.ValueOr(mPowerExportThreshold);
        if (!(resolvedImport == 0 && resolvedExport == 0) && resolvedImport <= resolvedExport)
        {
            return Status::ConstraintError;
        }
    }

    if (mDelegate != nullptr && !mDelegate->SetElectricalAlarmThresholdsCallback(data))
    {
        return Status::Failure;
    }

    // Commit: all fields have been validated; apply each present field. Feature presence was
    // already checked above so each setter returns Success.
    if (data.overVoltageThreshold.HasValue())
    {
        SetOverVoltageThreshold(data.overVoltageThreshold.Value());
    }
    if (data.underVoltageThreshold.HasValue())
    {
        SetUnderVoltageThreshold(data.underVoltageThreshold.Value());
    }
    if (data.overFrequencyThreshold.HasValue())
    {
        SetOverFrequencyThreshold(data.overFrequencyThreshold.Value());
    }
    if (data.underFrequencyThreshold.HasValue())
    {
        SetUnderFrequencyThreshold(data.underFrequencyThreshold.Value());
    }
    if (data.overPowerThreshold.HasValue())
    {
        SetOverPowerThreshold(data.overPowerThreshold.Value());
    }
    if (data.underPowerThreshold.HasValue())
    {
        SetUnderPowerThreshold(data.underPowerThreshold.Value());
    }
    if (data.overCurrentThreshold.HasValue())
    {
        SetOverCurrentThreshold(data.overCurrentThreshold.Value());
    }
    if (data.underCurrentThreshold.HasValue())
    {
        SetUnderCurrentThreshold(data.underCurrentThreshold.Value());
    }
    if (data.powerImportThreshold.HasValue())
    {
        SetPowerImportThreshold(data.powerImportThreshold.Value());
    }
    if (data.powerExportThreshold.HasValue())
    {
        SetPowerExportThreshold(data.powerExportThreshold.Value());
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
