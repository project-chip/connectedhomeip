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
    // ADJUST feature is only meaningful when at least one alarm-class feature is present.
    if (mFeatureFlags.Has(Feature::kAdjustableThresholds))
    {
        const bool hasAlarmClass = mFeatureFlags.HasAny(Feature::kOverVoltage, Feature::kUnderVoltage, Feature::kOverFrequency,
                                                        Feature::kUnderFrequency, Feature::kOverPower, Feature::kUnderPower,
                                                        Feature::kOverCurrent, Feature::kUnderCurrent, Feature::kPowerImport,
                                                        Feature::kPowerExport);
        VerifyOrReturnError(hasAlarmClass, CHIP_ERROR_INCORRECT_STATE,
                            ChipLogError(Zcl, "ElectricalAlarm: AdjustableThresholds feature requires at least one alarm-class "
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
    SetAttributeValue(mSupported, supported, Attributes::Supported::Id);

    // Latch must be a subset of Supported.
    if (mFeatureFlags.Has(Feature::kReset) && !supported.HasAll(mLatch))
    {
        SetAttributeValue(mLatch, mLatch & supported, Attributes::Latch::Id);
    }

    // Mask must be a subset of Supported; cascade to State.
    if (!supported.HasAll(mMask))
    {
        // SetMaskValue cascades to State — call it to get the cascade.
        return SetMaskValue(mMask & supported);
    }
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

    mState = finalNewState;
    if (becameActive.HasAny() || becameInactive.HasAny())
    {
        SendNotifyEvent(becameActive, becameInactive, mState, mMask);
    }
    return Status::Success;
}

Status ElectricalAlarmCluster::ResetLatchedAlarms(BitMask<AlarmBitmap> alarms)
{
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
    mOverVoltageThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderVoltageThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderVoltage), Status::UnsupportedAttribute);
    mUnderVoltageThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverFrequencyThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverFrequency), Status::UnsupportedAttribute);
    mOverFrequencyThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderFrequencyThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderFrequency), Status::UnsupportedAttribute);
    mUnderFrequencyThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverPowerThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverPower), Status::UnsupportedAttribute);
    mOverPowerThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderPowerThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderPower), Status::UnsupportedAttribute);
    mUnderPowerThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetOverCurrentThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kOverCurrent), Status::UnsupportedAttribute);
    mOverCurrentThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetUnderCurrentThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kUnderCurrent), Status::UnsupportedAttribute);
    mUnderCurrentThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetPowerImportThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kPowerImport), Status::UnsupportedAttribute);
    mPowerImportThreshold = value;
    return Status::Success;
}

Status ElectricalAlarmCluster::SetPowerExportThreshold(int64_t value)
{
    VerifyOrReturnValue(mFeatureFlags.Has(Feature::kPowerExport), Status::UnsupportedAttribute);
    mPowerExportThreshold = value;
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
    if (!mDelegate.ModifyEnabledAlarmsCallback(mask))
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
    if (!mDelegate.ResetAlarmsCallback(alarms))
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

    // Validate cross-pair ordering: Over must be > Under for each measurement type.
    // The check is only applied when both sides are known: either supplied in this command
    // or previously persisted (non-zero stored value). When only one side is provided and
    // the other side's stored value is still at the default (0), there is no baseline to
    // compare against, so we skip the cross-pair constraint for that direction.
    auto checkPair = [&](const Optional<int64_t> & over, const Optional<int64_t> & under, int64_t storedOver,
                         int64_t storedUnder) -> Status {
        if (!over.HasValue() && !under.HasValue())
        {
            return Status::Success;
        }
        // Skip cross-pair check when only one side is supplied and the other has no stored baseline.
        const bool overKnown  = over.HasValue() || (storedOver != 0);
        const bool underKnown = under.HasValue() || (storedUnder != 0);
        if (!overKnown || !underKnown)
        {
            return Status::Success;
        }
        int64_t resolvedOver  = over.ValueOr(storedOver);
        int64_t resolvedUnder = under.ValueOr(storedUnder);
        if (resolvedOver <= resolvedUnder)
        {
            return Status::ConstraintError;
        }
        return Status::Success;
    };

    Status s;
    if ((s = checkPair(data.overVoltageThreshold, data.underVoltageThreshold, mOverVoltageThreshold, mUnderVoltageThreshold)) !=
        Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overFrequencyThreshold, data.underFrequencyThreshold, mOverFrequencyThreshold,
                       mUnderFrequencyThreshold)) != Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overPowerThreshold, data.underPowerThreshold, mOverPowerThreshold, mUnderPowerThreshold)) !=
        Status::Success)
    {
        return s;
    }
    if ((s = checkPair(data.overCurrentThreshold, data.underCurrentThreshold, mOverCurrentThreshold, mUnderCurrentThreshold)) !=
        Status::Success)
    {
        return s;
    }
    // PowerImport >= 0 and PowerExport <= 0 are enforced by attribute min/max in ZAP;
    // the import/export pair uses minOf/maxOf so only validate when both are supplied.
    if (data.powerImportThreshold.HasValue() && data.powerExportThreshold.HasValue())
    {
        if (data.powerImportThreshold.Value() <= data.powerExportThreshold.Value())
        {
            return Status::ConstraintError;
        }
    }

    if (!mDelegate.SetElectricalAlarmThresholdsCallback(data))
    {
        return Status::Failure;
    }

    // Persist each present field.
    if (data.overVoltageThreshold.HasValue())
    {
        mOverVoltageThreshold = data.overVoltageThreshold.Value();
    }
    if (data.underVoltageThreshold.HasValue())
    {
        mUnderVoltageThreshold = data.underVoltageThreshold.Value();
    }
    if (data.overFrequencyThreshold.HasValue())
    {
        mOverFrequencyThreshold = data.overFrequencyThreshold.Value();
    }
    if (data.underFrequencyThreshold.HasValue())
    {
        mUnderFrequencyThreshold = data.underFrequencyThreshold.Value();
    }
    if (data.overPowerThreshold.HasValue())
    {
        mOverPowerThreshold = data.overPowerThreshold.Value();
    }
    if (data.underPowerThreshold.HasValue())
    {
        mUnderPowerThreshold = data.underPowerThreshold.Value();
    }
    if (data.overCurrentThreshold.HasValue())
    {
        mOverCurrentThreshold = data.overCurrentThreshold.Value();
    }
    if (data.underCurrentThreshold.HasValue())
    {
        mUnderCurrentThreshold = data.underCurrentThreshold.Value();
    }
    if (data.powerImportThreshold.HasValue())
    {
        mPowerImportThreshold = data.powerImportThreshold.Value();
    }
    if (data.powerExportThreshold.HasValue())
    {
        mPowerExportThreshold = data.powerExportThreshold.Value();
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
