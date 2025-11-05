/**
 *
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
 *
 */

#include "boolean-state-configuration-cluster.h"

#include <app/SafeAttributePersistenceProvider.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/BooleanStateConfiguration/AttributeIds.h>
#include <clusters/BooleanStateConfiguration/Commands.h>
#include <clusters/BooleanStateConfiguration/Events.h>
#include <clusters/BooleanStateConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

// TODO:
//   - startup load the initial sensitivity values
//   - do we need to validate min/max supported sensitivity levels?
//
// static constexpr uint8_t kMinSupportedSensitivityLevels = 2;
// static constexpr uint8_t kMaxSupportedSensitivityLevels = 10;

using namespace chip::app::DataModel;
using namespace chip::app::Clusters::BooleanStateConfiguration;
using namespace chip::app::Clusters::BooleanStateConfiguration::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip::app::Clusters {

CHIP_ERROR BooleanStateConfigurationCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{

    if (mFeatures.HasAny(Feature::kAudible, Feature::kVisual))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::EnableDisableAlarm::kMetadataEntry }));

        // Alarm suppression has [VIS | AUD] conformance, so checks are nested here
        if (mFeatures.Has(Feature::kAlarmSuppress))
        {
            ReturnErrorOnFailure(builder.AppendElements({ Commands::SuppressAlarm::kMetadataEntry }));
        }
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
BooleanStateConfigurationCluster::InvokeCommand(const DataModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments,
                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SuppressAlarm::Id: {
        Commands::SuppressAlarm::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));

        return SuppressAlarms(request_data.alarmsToSuppress);
    }
    case Commands::EnableDisableAlarm::Id: {
        Commands::EnableDisableAlarm::DecodableType request_data;
        ReturnErrorOnFailure(request_data.Decode(input_arguments));

        const auto alarms = request_data.alarmsToEnableDisable;

        VerifyOrReturnError(mAlarmsSupported.HasAll(alarms), Status::ConstraintError);

        if (mAlarmsEnabled != alarms)
        {
            mAlarmsEnabled = alarms;
            NotifyAttributeChanged(AlarmsEnabled::Id);
        }

        if (mDelegate != nullptr)
        {
            // TODO: For backwards compatibility with previous code, we ignore the return code
            //       from the delegate handler. This feels off though...
            (void) mDelegate->HandleEnableDisableAlarms(alarms);
        }

        // This inverts the bits (0x03 is the current max bitmap):
        //   - every "known" bit that is set to 0 in the request will be set to 1 in the `alarmsToDisable`
        BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsToDisable(static_cast<uint8_t>(~alarms.Raw() & 0x03));

        bool emit = false;
        if (mAlarmsActive.HasAny(alarmsToDisable))
        {
            mAlarmsActive.Clear(alarmsToDisable);
            NotifyAttributeChanged(AlarmsActive::Id);
            emit = true;
        }
        if (mAlarmsSuppressed.HasAny(alarmsToDisable))
        {
            mAlarmsSuppressed.Clear(alarmsToDisable);
            NotifyAttributeChanged(AlarmsSuppressed::Id);
            emit = true;
        }

        if (emit)
        {
            EmitAlarmsStateChangedEvent();
        }
        return Status::Success;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

ActionReturnStatus BooleanStateConfigurationCluster::ReadAttribute(const ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(BooleanStateConfiguration::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case CurrentSensitivityLevel::Id:
        return encoder.Encode(mCurrentSensitivityLevel);
    case SupportedSensitivityLevels::Id:
        return encoder.Encode(mSupportedSensitivityLevels);
    case DefaultSensitivityLevel::Id:
        return encoder.Encode(mDefaultSensitivityLevel);
    case AlarmsActive::Id:
        return encoder.Encode(mAlarmsActive);
    case AlarmsSuppressed::Id:
        return encoder.Encode(mAlarmsSuppressed);
    case AlarmsEnabled::Id:
        return encoder.Encode(mAlarmsEnabled);
    case AlarmsSupported::Id:
        return encoder.Encode(mAlarmsSupported);
    case SensorFault::Id:
        return encoder.Encode(mSensorFault);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

ActionReturnStatus BooleanStateConfigurationCluster::WriteAttribute(const WriteAttributeRequest & request,
                                                                    AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case CurrentSensitivityLevel::Id: {
        uint8_t value;
        ReturnErrorOnFailure(decoder.Decode(value));
        return SetCurrentSensitivityLevel(value);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

CHIP_ERROR BooleanStateConfigurationCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<AttributeEntry> & builder)
{
    constexpr AttributeEntry optionalAttributesMeta[] = {
        CurrentSensitivityLevel::kMetadataEntry,    //
        SupportedSensitivityLevels::kMetadataEntry, //
        DefaultSensitivityLevel::kMetadataEntry,    //
        AlarmsActive::kMetadataEntry,               //
        AlarmsSuppressed::kMetadataEntry,           //
        AlarmsEnabled::kMetadataEntry,              //
        AlarmsSupported::kMetadataEntry,            //
        SensorFault::kMetadataEntry,                //
    };

    AttributeSet enabledOptionalAttributes;

    if (mFeatures.Has(Feature::kSensitivityLevel))
    {
        enabledOptionalAttributes.ForceSet<CurrentSensitivityLevel::Id>();
        enabledOptionalAttributes.ForceSet<SupportedSensitivityLevels::Id>();
        if (mOptionalAttributes.IsSet(DefaultSensitivityLevel::Id))
        {
            enabledOptionalAttributes.ForceSet<DefaultSensitivityLevel::Id>();
        }
    }

    if (mFeatures.Has(Feature::kVisual) || mFeatures.Has(Feature::kAudible))
    {
        enabledOptionalAttributes.ForceSet<AlarmsActive::Id>();
        enabledOptionalAttributes.ForceSet<AlarmsSupported::Id>();

        if (mOptionalAttributes.IsSet(AlarmsEnabled::Id))
        {
            enabledOptionalAttributes.ForceSet<AlarmsEnabled::Id>();
        }
    }

    if (mFeatures.Has(Feature::kAlarmSuppress))
    {
        enabledOptionalAttributes.ForceSet<AlarmsSuppressed::Id>();
    }

    if (mOptionalAttributes.IsSet(SensorFault::Id))
    {
        enabledOptionalAttributes.ForceSet<SensorFault::Id>();
    }

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span<const AttributeEntry>{ optionalAttributesMeta },
                              enabledOptionalAttributes);
}

void BooleanStateConfigurationCluster::EmitAlarmsStateChangedEvent()
{
    VerifyOrReturn(mContext != nullptr);
    VerifyOrReturn(mFeatures.HasAny(Feature::kAudible, Feature::kVisual));

    BooleanStateConfiguration::Events::AlarmsStateChanged::Type event;
    event.alarmsActive = mAlarmsActive;

    if (mFeatures.Has(Feature::kAlarmSuppress))
    {
        event.alarmsSuppressed.SetValue(mAlarmsSuppressed);
    }
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void BooleanStateConfigurationCluster::EmitSensorFault(SensorFaultBitMask fault)
{
    VerifyOrReturn(mContext != nullptr);

    BooleanStateConfiguration::Events::SensorFault::Type event;
    event.sensorFault = fault;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

CHIP_ERROR BooleanStateConfigurationCluster::SetCurrentSensitivityLevel(uint8_t level)
{
    VerifyOrReturnError(level < mSupportedSensitivityLevels, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mCurrentSensitivityLevel != level, CHIP_NO_ERROR);

    mCurrentSensitivityLevel = level;
    NotifyAttributeChanged(CurrentSensitivityLevel::Id);

    // TODO: we should migrate this to not use `Safe` attribute persistence and use
    //       a common persistence layer.
    return GetSafeAttributePersistenceProvider()->WriteScalarValue(
        { mPath.mEndpointId, mPath.mClusterId, CurrentSensitivityLevel::Id }, level);
}

Status BooleanStateConfigurationCluster::SetAlarmsActive(AlarmModeBitMask alarms)
{
    VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), Status::Failure);
    VerifyOrReturnError(mAlarmsEnabled.HasAll(alarms), Status::Failure);

    // No change is a noop
    VerifyOrReturnError(mAlarmsActive != alarms, Status::Success);

    mAlarmsActive = alarms;
    NotifyAttributeChanged(AlarmsActive::Id);
    EmitAlarmsStateChangedEvent();

    return Status::Success;
}

Status BooleanStateConfigurationCluster::SetAllEnabledAlarmsActive()
{
    VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), Status::Failure);
    VerifyOrReturnError(mAlarmsEnabled.HasAny(), Status::Success);

    mAlarmsActive = mAlarmsEnabled;
    NotifyAttributeChanged(AlarmsActive::Id);
    EmitAlarmsStateChangedEvent();
    return Status::Success;
}

void BooleanStateConfigurationCluster::ClearAllAlarms()
{
    VerifyOrReturn(mAlarmsActive.HasAny() || mAlarmsSuppressed.HasAny());

    if (mAlarmsActive.HasAny())
    {
        mAlarmsActive.ClearAll();
        NotifyAttributeChanged(AlarmsActive::Id);
    }
    if (mAlarmsSuppressed.HasAny())
    {
        mAlarmsActive.ClearAll();
        NotifyAttributeChanged(AlarmsSuppressed::Id);
    }

    EmitAlarmsStateChangedEvent();
}

Status BooleanStateConfigurationCluster::SuppressAlarms(AlarmModeBitMask alarms)
{
    // Need SPRS feature and that is only available if [VIS | AUD]. These are all checked here.
    VerifyOrReturnError(mFeatures.HasAll(Feature::kAlarmSuppress), Status::UnsupportedCommand);
    VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), Status::UnsupportedCommand);

    // can only suppress valid active alarms
    VerifyOrReturnError(mAlarmsSupported.HasAll(alarms), Status::ConstraintError);
    VerifyOrReturnError(mAlarmsActive.HasAll(alarms), Status::ConstraintError);

    // validate this is not a NOOP
    VerifyOrReturnError(!mAlarmsSupported.HasAll(alarms), Status::Success);

    if (mDelegate != nullptr)
    {
        // TODO: To preserve original logic, we ignore error code from the
        //       delegate, however this feels off.
        (void) mDelegate->HandleSuppressAlarm(alarms);
    }

    mAlarmsSuppressed.Set(alarms);
    NotifyAttributeChanged(AlarmsSuppressed::Id);
    EmitAlarmsStateChangedEvent();
    return Status::Success;
}

} // namespace chip::app::Clusters
