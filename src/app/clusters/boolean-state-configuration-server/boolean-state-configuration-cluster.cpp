/**
 *    Copyright (c) 2023-2025 Project CHIP Authors
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
#include <app/clusters/boolean-state-configuration-server/boolean-state-configuration-cluster.h>

#include <app/SafeAttributePersistenceProvider.h>
#include <app/data-model/Decode.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/BooleanStateConfiguration/AttributeIds.h>
#include <clusters/BooleanStateConfiguration/Commands.h>
#include <clusters/BooleanStateConfiguration/Events.h>
#include <clusters/BooleanStateConfiguration/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

#include <algorithm>

using namespace chip::app::DataModel;
using namespace chip::app::Clusters::BooleanStateConfiguration;
using namespace chip::app::Clusters::BooleanStateConfiguration::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip::app::Clusters {

constexpr uint8_t kAllKnownAlarmModes =
    static_cast<uint8_t>(AlarmModeBitmap::kAudible) | static_cast<uint8_t>(AlarmModeBitmap::kVisual);

BooleanStateConfigurationCluster::BooleanStateConfigurationCluster(EndpointId endpointId,
                                                                   BitMask<BooleanStateConfiguration::Feature> features,
                                                                   OptionalAttributesSet optionalAttributes,
                                                                   const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, BooleanStateConfiguration::Id }),
    mFeatures(features), mOptionalAttributes([&features, &optionalAttributes]() -> FullOptionalAttributesSet {
        // constructs the attribute set, that once constructed stays const
        AttributeSet enabledOptionalAttributes;

        if (features.Has(Feature::kSensitivityLevel))
        {
            enabledOptionalAttributes.ForceSet<CurrentSensitivityLevel::Id>();
            enabledOptionalAttributes.ForceSet<SupportedSensitivityLevels::Id>();
            if (optionalAttributes.IsSet(DefaultSensitivityLevel::Id))
            {
                enabledOptionalAttributes.ForceSet<DefaultSensitivityLevel::Id>();
            }
        }

        if (features.Has(Feature::kVisual) || features.Has(Feature::kAudible))
        {
            enabledOptionalAttributes.ForceSet<AlarmsActive::Id>();
            enabledOptionalAttributes.ForceSet<AlarmsSupported::Id>();

            if (optionalAttributes.IsSet(AlarmsEnabled::Id))
            {
                enabledOptionalAttributes.ForceSet<AlarmsEnabled::Id>();
            }
        }

        if (features.Has(Feature::kAlarmSuppress))
        {
            enabledOptionalAttributes.ForceSet<AlarmsSuppressed::Id>();
        }

        if (optionalAttributes.IsSet(SensorFault::Id))
        {
            enabledOptionalAttributes.ForceSet<SensorFault::Id>();
        }

        return enabledOptionalAttributes;
    }()),
    mSupportedSensitivityLevels(
        std::clamp(config.supportedSensitivityLevels, kMinSupportedSensitivityLevels, kMaxSupportedSensitivityLevels)),
    mDefaultSensitivityLevel(std::min(config.defaultSensitivityLevel, static_cast<uint8_t>(mSupportedSensitivityLevels - 1))),
    mAlarmsSupported(config.alarmsSupported)
{}

CHIP_ERROR BooleanStateConfigurationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (GetSafeAttributePersistenceProvider()->ReadScalarValue({ mPath.mEndpointId, mPath.mClusterId, CurrentSensitivityLevel::Id },
                                                               mCurrentSensitivityLevel) != CHIP_NO_ERROR)
    {
        mCurrentSensitivityLevel = mDefaultSensitivityLevel;
    }

    if (mCurrentSensitivityLevel >= mSupportedSensitivityLevels)
    {
        mCurrentSensitivityLevel = mSupportedSensitivityLevels - 1;
    }

    // alarms enabled persistence was handled by ember previously (as opposed to AAI usage of sensitivity level)
    // TODO: this is VERY inconvenient/strange and we should really fix this inconsistence
    AttributePersistence attributePersistence(context.attributeStorage);
    AlarmModeBitMask::IntegerType alarmsEnabled;
    attributePersistence.LoadNativeEndianValue({ mPath.mEndpointId, mPath.mClusterId, AlarmsEnabled::Id }, alarmsEnabled,
                                               AlarmModeBitMask::IntegerType(0));
    mAlarmsEnabled = AlarmModeBitMask(alarmsEnabled);

    // internal state validation:
    if (mFeatures.Has(Feature::kAlarmSuppress))
    {
        // alarm Suppression requires visual/audible alarms
        VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), CHIP_ERROR_INCORRECT_STATE);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BooleanStateConfigurationCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{

    if (mFeatures.HasAny(Feature::kAudible, Feature::kVisual))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::EnableDisableAlarm::kMetadataEntry }));
    }

    if (mFeatures.Has(Feature::kAlarmSuppress))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::SuppressAlarm::kMetadataEntry }));
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

            AlarmModeBitMask::IntegerType rawAlarmsEnabled = mAlarmsEnabled.Raw();
            if (CHIP_ERROR err = mContext->attributeStorage.WriteValue({ mPath.mEndpointId, mPath.mClusterId, AlarmsEnabled::Id },
                                                                       { &rawAlarmsEnabled, sizeof(rawAlarmsEnabled) });
                err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement, "Failed to persist alarms enabled: %" CHIP_ERROR_FORMAT, err.Format());
            }
            OnClusterAttributeChanged(AlarmsEnabled::Id);
        }

        if (mDelegate != nullptr)
        {
            // TODO: For backwards compatibility with previous code, we ignore the return code
            //       from the delegate handler. This feels off though...
            TEMPORARY_RETURN_IGNORED mDelegate->HandleEnableDisableAlarms(alarms);
        }

        // This inverts the bits (0x03 is the current max bitmap):
        //   - every "known" bit that is set to 0 in the request will be set to 1 in the `alarmsToDisable`
        const BitMask<BooleanStateConfiguration::AlarmModeBitmap> alarmsToDisable{ static_cast<uint8_t>(~alarms.Raw() &
                                                                                                        kAllKnownAlarmModes) };

        bool generateEvent = false;
        if (mAlarmsActive.HasAny(alarmsToDisable))
        {
            mAlarmsActive.Clear(alarmsToDisable);
            OnClusterAttributeChanged(AlarmsActive::Id);
            generateEvent = true;
        }
        if (mAlarmsSuppressed.HasAny(alarmsToDisable))
        {
            mAlarmsSuppressed.Clear(alarmsToDisable);
            OnClusterAttributeChanged(AlarmsSuppressed::Id);
            generateEvent = true;
        }

        if (generateEvent)
        {
            GenerateAlarmsStateChangedEvent();
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

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span<const AttributeEntry>{ optionalAttributesMeta }, mOptionalAttributes);
}

void BooleanStateConfigurationCluster::GenerateAlarmsStateChangedEvent()
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

void BooleanStateConfigurationCluster::GenerateSensorFault(SensorFaultBitMask fault)
{
    VerifyOrReturn(mContext != nullptr);

    BooleanStateConfiguration::Events::SensorFault::Type event;
    event.sensorFault = fault;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);

    if (mOptionalAttributes.IsSet(SensorFault::Id) && (mSensorFault != fault))
    {
        mSensorFault = fault;
        OnClusterAttributeChanged(SensorFault::Id);
    }
}

CHIP_ERROR BooleanStateConfigurationCluster::SetCurrentSensitivityLevel(uint8_t level)
{
    VerifyOrReturnError(level < mSupportedSensitivityLevels, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mCurrentSensitivityLevel != level, CHIP_NO_ERROR);

    mCurrentSensitivityLevel = level;
    OnClusterAttributeChanged(CurrentSensitivityLevel::Id);

    // TODO: we should migrate this to not use `Safe` attribute persistence and use
    //       a common persistence layer.
    return GetSafeAttributePersistenceProvider()->WriteScalarValue(
        { mPath.mEndpointId, mPath.mClusterId, CurrentSensitivityLevel::Id }, level);
}

void BooleanStateConfigurationCluster::OnClusterAttributeChanged(AttributeId attributeId)
{
    NotifyAttributeChanged(attributeId);
    if (mDelegate != nullptr)
    {
        mDelegate->OnAttributeChanged(attributeId, this);
    }
}

Status BooleanStateConfigurationCluster::SetAlarmsActive(AlarmModeBitMask alarms)
{
    VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), Status::Failure);
    VerifyOrReturnError(mAlarmsEnabled.HasAll(alarms), Status::Failure);

    // No change is a noop
    VerifyOrReturnError(mAlarmsActive != alarms, Status::Success);

    mAlarmsActive = alarms;
    OnClusterAttributeChanged(AlarmsActive::Id);
    GenerateAlarmsStateChangedEvent();

    return Status::Success;
}

Status BooleanStateConfigurationCluster::SetAllEnabledAlarmsActive()
{
    VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), Status::Failure);
    VerifyOrReturnError(mAlarmsEnabled.HasAny(), Status::Success);

    mAlarmsActive = mAlarmsEnabled;
    OnClusterAttributeChanged(AlarmsActive::Id);
    GenerateAlarmsStateChangedEvent();
    return Status::Success;
}

void BooleanStateConfigurationCluster::ClearAllAlarms()
{
    VerifyOrReturn(mAlarmsActive.HasAny() || mAlarmsSuppressed.HasAny());

    if (mAlarmsActive.HasAny())
    {
        mAlarmsActive.ClearAll();
        OnClusterAttributeChanged(AlarmsActive::Id);
    }
    if (mAlarmsSuppressed.HasAny())
    {
        mAlarmsSuppressed.ClearAll();
        OnClusterAttributeChanged(AlarmsSuppressed::Id);
    }

    GenerateAlarmsStateChangedEvent();
}

Status BooleanStateConfigurationCluster::SuppressAlarms(AlarmModeBitMask alarms)
{
    // Need SPRS feature and that is only available if [VIS | AUD]. These are all checked here.
    VerifyOrReturnError(mFeatures.Has(Feature::kAlarmSuppress), Status::UnsupportedCommand);
    VerifyOrReturnError(mFeatures.HasAny(Feature::kAudible, Feature::kVisual), Status::UnsupportedCommand);

    // can only suppress valid active alarms
    VerifyOrReturnError(mAlarmsSupported.HasAll(alarms), Status::ConstraintError);
    VerifyOrReturnError(mAlarmsActive.HasAll(alarms), Status::InvalidInState);

    // validate this is not a NOOP
    VerifyOrReturnError(!mAlarmsSuppressed.HasAll(alarms), Status::Success);

    if (mDelegate != nullptr)
    {
        // TODO: To preserve original logic, we ignore error code from the
        //       delegate, however this feels off.
        TEMPORARY_RETURN_IGNORED mDelegate->HandleSuppressAlarm(alarms);
    }

    mAlarmsSuppressed.Set(alarms);
    OnClusterAttributeChanged(AlarmsSuppressed::Id);
    GenerateAlarmsStateChangedEvent();
    return Status::Success;
}

} // namespace chip::app::Clusters
