/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/humidistat-server/HumidistatCluster.h>

#include <app/CommandHandler.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Humidistat/Attributes.h>
#include <clusters/Humidistat/Commands.h>
#include <clusters/Humidistat/Metadata.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Humidistat;
using namespace chip::app::Clusters::Humidistat::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

HumidistatCluster::HumidistatCluster(EndpointId endpointId, BitFlags<Humidistat::Feature> features,
                                     const OptionalAttributeSet & optionalAttributes) :
    HumidistatCluster(endpointId, features, optionalAttributes, StartupConfiguration())
{}

HumidistatCluster::HumidistatCluster(EndpointId endpointId, BitFlags<Humidistat::Feature> features,
                                     const OptionalAttributeSet & optionalAttributes, const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, Humidistat::Id }),
    mFeatures(features), mOptionalAttributes(optionalAttributes), mActiveOptional(ComputeActiveOptionalAttributes()),
    mMode(IsModeSupported(config.mode) ? config.mode : ModeEnum::kOff),
    mSystemState(IsSystemStateSupported(config.systemState) ? config.systemState : SystemStateEnum::kOff),
    mUserSetpoint(config.userSetpoint), mMinSetpoint(config.minSetpoint), mMaxSetpoint(config.maxSetpoint), mStep(config.step),
    mTargetSetpoint(config.targetSetpoint), mMistType(config.mistType), mContinuous(config.continuous), mSleep(config.sleep),
    mOptimal(config.optimal)
{
    // Spec constraints on Quality F (fixed) setpoint attributes.
    VerifyOrDie(config.minSetpoint <= 99);
    VerifyOrDie(config.maxSetpoint >= static_cast<chip::Percent>(config.minSetpoint + 1) && config.maxSetpoint <= 100);
    VerifyOrDie(config.step >= 1 && config.step <= static_cast<chip::Percent>(config.maxSetpoint - config.minSetpoint));
    VerifyOrDie((config.maxSetpoint - config.minSetpoint) % config.step == 0);
    VerifyOrDie(IsMistTypeSupportable(config.mistType));
    VerifyOrDie(IsMistTypeConsistentWithMode(mMode, config.mistType));

    // Snap initial setpoints to the valid step grid.
    mUserSetpoint   = SnapToNearestStep(mUserSetpoint);
    mTargetSetpoint = SnapToNearestStep(mTargetSetpoint);

    if (ShouldTargetSetpointMatchUserSetpoint())
    {
        mTargetSetpoint = mUserSetpoint;
    }
}

CHIP_ERROR HumidistatCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

void HumidistatCluster::LoadPersistentAttributes()
{
    AttributePersistence attrPersistence{ mContext->attributeStorage };

    const auto defaultMode = mMode;
    if (!attrPersistence.LoadNativeEndianValue<ModeEnum>(ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Mode::Id), mMode,
                                                         defaultMode))
    {
        ChipLogDetail(Zcl, "Humidistat: Unable to load Mode attribute, using default");
    }
    if (!IsModeSupported(mMode))
    {
        ChipLogDetail(Zcl, "Humidistat: Loaded unsupported Mode value %u, forcing Off", static_cast<unsigned>(mMode));
        mMode = ModeEnum::kOff;
    }

    const auto defaultSystemState = mSystemState;
    if (!attrPersistence.LoadNativeEndianValue<SystemStateEnum>(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, SystemState::Id), mSystemState, defaultSystemState))
    {
        ChipLogDetail(Zcl, "Humidistat: Unable to load SystemState attribute, using default");
    }
    if (!IsSystemStateSupported(mSystemState))
    {
        ChipLogDetail(Zcl, "Humidistat: Loaded unsupported SystemState value %u, forcing Off", static_cast<unsigned>(mSystemState));
        mSystemState = SystemStateEnum::kOff;
    }

    if (mFeatures.Has(Feature::kSensor))
    {
        const auto defaultUserSetpoint = mUserSetpoint;
        if (!attrPersistence.LoadNativeEndianValue<chip::Percent>(
                ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, UserSetpoint::Id), mUserSetpoint, defaultUserSetpoint))
        {
            ChipLogDetail(Zcl, "Humidistat: Unable to load UserSetpoint attribute, using default");
        }
        mUserSetpoint = SnapToNearestStep(mUserSetpoint);
    }

    if (mFeatures.Has(Feature::kHumidifier))
    {
        uint8_t rawMistType              = mMistType.Raw();
        const uint8_t defaultRawMistType = rawMistType;
        if (!attrPersistence.LoadNativeEndianValue<uint8_t>(ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, MistType::Id),
                                                            rawMistType, defaultRawMistType))
        {
            ChipLogDetail(Zcl, "Humidistat: Unable to load MistType attribute, using default");
        }

        chip::BitMask<MistTypeBitmap> loadedMistType(rawMistType);

        // Clear any bits not supported by the current feature set to guard against stale persisted data.
        if (!mFeatures.Has(Feature::kColdMist))
        {
            loadedMistType.Clear(MistTypeBitmap::kMistCold);
        }
        if (!mFeatures.Has(Feature::kWarmMist))
        {
            loadedMistType.Clear(MistTypeBitmap::kMistWarm);
        }
        // Spec: MistType SHALL be zero when Mode is not Humidifier.
        if (mMode != ModeEnum::kHumidifier)
        {
            loadedMistType.ClearAll();
        }
        else if (!loadedMistType.HasAny())
        {
            ChipLogDetail(Zcl, "Humidistat: Loaded empty MistType in Humidifier mode, using startup default");
            loadedMistType = mMistType;
        }
        mMistType = loadedMistType;
    }

    if (mFeatures.Has(Feature::kContinuous))
    {
        const auto defaultContinuous = mContinuous;
        if (!attrPersistence.LoadNativeEndianValue<bool>(ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Continuous::Id),
                                                         mContinuous, defaultContinuous))
        {
            ChipLogDetail(Zcl, "Humidistat: Unable to load Continuous attribute, using default");
        }
    }

    if (mOptionalAttributes.IsSet(Sleep::Id))
    {
        const auto defaultSleep = mSleep;
        if (!attrPersistence.LoadNativeEndianValue<bool>(ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Sleep::Id),
                                                         mSleep, defaultSleep))
        {
            ChipLogDetail(Zcl, "Humidistat: Unable to load Sleep attribute, using default");
        }
    }

    if (mFeatures.Has(Feature::kOptimal))
    {
        const auto defaultOptimal = mOptimal;
        if (!attrPersistence.LoadNativeEndianValue<bool>(ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Optimal::Id),
                                                         mOptimal, defaultOptimal))
        {
            ChipLogDetail(Zcl, "Humidistat: Unable to load Optimal attribute, using default");
        }
    }

    if (ShouldTargetSetpointMatchUserSetpoint())
    {
        mTargetSetpoint = mUserSetpoint;
    }
}

bool HumidistatCluster::IsModeSupported(Humidistat::ModeEnum mode) const
{
    switch (mode)
    {
    case ModeEnum::kOff:
        return true;
    case ModeEnum::kHumidifier:
        return mFeatures.Has(Feature::kHumidifier);
    case ModeEnum::kDehumidifier:
        return mFeatures.Has(Feature::kDehumidifier);
    case ModeEnum::kAuto:
        return mFeatures.Has(Feature::kAuto);
    case ModeEnum::kFanOnly:
        return mFeatures.Has(Feature::kFanOnly);
    default:
        return false;
    }
}

bool HumidistatCluster::IsSystemStateSupported(Humidistat::SystemStateEnum systemState) const
{
    switch (systemState)
    {
    case SystemStateEnum::kOff:
    case SystemStateEnum::kIdle:
        return true;
    case SystemStateEnum::kHumidifying:
        return mFeatures.Has(Feature::kHumidifier);
    case SystemStateEnum::kDehumidifying:
        return mFeatures.Has(Feature::kDehumidifier);
    case SystemStateEnum::kFan:
        return mFeatures.Has(Feature::kFanOnly);
    default:
        return false;
    }
}

bool HumidistatCluster::IsMistTypeConsistentWithMode(Humidistat::ModeEnum mode,
                                                     chip::BitMask<Humidistat::MistTypeBitmap> mistType) const
{
    if (mode == ModeEnum::kHumidifier)
    {
        return mistType.HasAny();
    }

    return !mistType.HasAny();
}

bool HumidistatCluster::ShouldTargetSetpointMatchUserSetpoint() const
{
    return mFeatures.Has(Feature::kSensor) && !mSleep && !mOptimal;
}

void HumidistatCluster::SyncTargetSetpointToUserSetpoint()
{
    VerifyOrReturn(ShouldTargetSetpointMatchUserSetpoint());
    VerifyOrReturn(mActiveOptional.IsSet(TargetSetpoint::Id));

    if (SetAttributeValue(mTargetSetpoint, mUserSetpoint, TargetSetpoint::Id) && (mDelegate != nullptr))
    {
        mDelegate->OnTargetSetpointChanged(mTargetSetpoint);
    }
}

bool HumidistatCluster::IsMistTypeSupportable(chip::BitMask<Humidistat::MistTypeBitmap> mistType) const
{
    // Reject any bits that are not defined in the spec (MistCold=bit0, MistWarm=bit1).
    if (!mistType.HasOnly(MistTypeBitmap::kMistCold, MistTypeBitmap::kMistWarm))
    {
        return false;
    }
    if (mistType.Has(MistTypeBitmap::kMistWarm) && !mFeatures.Has(Feature::kWarmMist))
    {
        return false;
    }
    if (mistType.Has(MistTypeBitmap::kMistCold) && !mFeatures.Has(Feature::kColdMist))
    {
        return false;
    }
    return true;
}

HumidistatCluster::FullOptionalAttributeSet HumidistatCluster::ComputeActiveOptionalAttributes() const
{
    AttributeSet active(mOptionalAttributes);

    if (mFeatures.Has(Feature::kSensor))
    {
        active.ForceSet<UserSetpoint::Id>();
        active.ForceSet<MinSetpoint::Id>();
        active.ForceSet<MaxSetpoint::Id>();
        active.ForceSet<Step::Id>();
    }

    if (mFeatures.Has(Feature::kHumidifier))
    {
        active.ForceSet<MistType::Id>();
    }

    if (mFeatures.Has(Feature::kContinuous))
    {
        active.ForceSet<Continuous::Id>();
    }

    // Spec: TargetSetpoint conformance:
    //   - Mandatory when OPTIMAL is enabled (OPTIMAL requires SENSOR per spec constraints).
    //   - Optional when SENSOR is present but OPTIMAL is not enabled (user can explicitly request it).
    //   - Hidden otherwise (requires SENSOR).
    if (mFeatures.Has(Feature::kSensor) && (mFeatures.Has(Feature::kOptimal) || mOptionalAttributes.IsSet(TargetSetpoint::Id)))
    {
        active.ForceSet<TargetSetpoint::Id>();
    }

    if (mFeatures.Has(Feature::kOptimal))
    {
        active.ForceSet<Optimal::Id>();
    }

    return FullOptionalAttributeSet(active);
}

CHIP_ERROR HumidistatCluster::SetMode(Humidistat::ModeEnum mode)
{
    VerifyOrReturnError(IsModeSupported(mode), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    const bool shouldClearMistType = mFeatures.Has(Feature::kHumidifier) && (mode != ModeEnum::kHumidifier) && mMistType.HasAny();

    if (SetAttributeValue(mMode, mode, Mode::Id))
    {
        if (mContext != nullptr)
        {
            uint8_t value = static_cast<uint8_t>(mMode);
            LogErrorOnFailure(mContext->attributeStorage.WriteValue(
                ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Mode::Id), { &value, sizeof(value) }));
        }

        if (mDelegate != nullptr)
        {
            mDelegate->OnModeChanged(mMode);
        }
    }

    VerifyOrReturnValue(shouldClearMistType, CHIP_NO_ERROR);

    // Spec: "If the value of Mode is not set to Humidifier, all bits of MistType SHALL be set to zero."
    const chip::BitMask<MistTypeBitmap> clearedMistType{};
    if (SetAttributeValue(mMistType, clearedMistType, MistType::Id) && (mContext != nullptr))
    {
        uint8_t value = mMistType.Raw();
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, MistType::Id), { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnMistTypeChanged(mMistType);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::SetSystemState(Humidistat::SystemStateEnum systemState)
{
    VerifyOrReturnError(IsSystemStateSupported(systemState), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    VerifyOrReturnValue(SetAttributeValue(mSystemState, systemState, SystemState::Id), CHIP_NO_ERROR);
    if (mContext != nullptr)
    {
        uint8_t value = static_cast<uint8_t>(mSystemState);
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, SystemState::Id), { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnSystemStateChanged(mSystemState);
    }

    return CHIP_NO_ERROR;
}

chip::Percent HumidistatCluster::SnapToNearestStep(chip::Percent value) const
{
    if (value > mMaxSetpoint)
    {
        return mMaxSetpoint;
    }
    chip::Percent offset    = static_cast<chip::Percent>(value - mMinSetpoint);
    chip::Percent remainder = static_cast<chip::Percent>(offset % mStep);
    if (remainder == 0)
    {
        return value;
    }
    // Round to nearest valid step-aligned value.
    chip::Percent lower = static_cast<chip::Percent>(value - remainder);
    chip::Percent upper = static_cast<chip::Percent>(lower + mStep);
    if (upper > mMaxSetpoint)
    {
        return lower;
    }
    return (remainder * 2 <= mStep) ? lower : upper;
}

CHIP_ERROR HumidistatCluster::SetUserSetpoint(chip::Percent userSetpoint)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kSensor), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(userSetpoint >= mMinSetpoint && userSetpoint <= mMaxSetpoint, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Spec: if not step-aligned, snap to the nearest valid value and succeed.
    userSetpoint = SnapToNearestStep(userSetpoint);

    VerifyOrReturnValue(SetAttributeValue(mUserSetpoint, userSetpoint, UserSetpoint::Id), CHIP_NO_ERROR);

    if (mContext != nullptr)
    {
        uint8_t value = mUserSetpoint;
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, UserSetpoint::Id), { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnUserSetpointChanged(mUserSetpoint);
    }

    SyncTargetSetpointToUserSetpoint();

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::SetMistType(chip::BitMask<Humidistat::MistTypeBitmap> mistType)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kHumidifier), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(IsMistTypeConsistentWithMode(mMode, mistType), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(IsMistTypeSupportable(mistType), CHIP_IM_GLOBAL_STATUS(InvalidInState));

    VerifyOrReturnValue(SetAttributeValue(mMistType, mistType, MistType::Id), CHIP_NO_ERROR);
    if (mContext != nullptr)
    {
        uint8_t value = mMistType.Raw();
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, MistType::Id), { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnMistTypeChanged(mMistType);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::SetTargetSetpoint(chip::Percent targetSetpoint)
{
    // TargetSetpoint is a device-firmware-controlled attribute requiring the SENSOR feature and must be active.
    VerifyOrReturnError(mFeatures.Has(Feature::kSensor), CHIP_IM_GLOBAL_STATUS(InvalidInState));
    VerifyOrReturnError(mActiveOptional.IsSet(TargetSetpoint::Id), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    VerifyOrReturnError(targetSetpoint >= mMinSetpoint && targetSetpoint <= mMaxSetpoint, CHIP_IM_GLOBAL_STATUS(ConstraintError));

    targetSetpoint = SnapToNearestStep(targetSetpoint);

    VerifyOrReturnValue(SetAttributeValue(mTargetSetpoint, targetSetpoint, TargetSetpoint::Id), CHIP_NO_ERROR);

    if (mDelegate != nullptr)
    {
        mDelegate->OnTargetSetpointChanged(mTargetSetpoint);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::SetContinuous(bool continuous)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kContinuous), CHIP_IM_GLOBAL_STATUS(InvalidInState));

    VerifyOrReturnValue(SetAttributeValue(mContinuous, continuous, Continuous::Id), CHIP_NO_ERROR);
    if (mContext != nullptr)
    {
        uint8_t value = mContinuous ? 1 : 0;
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Continuous::Id), { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnContinuousChanged(mContinuous);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::SetSleep(bool sleep)
{
    VerifyOrReturnError(mOptionalAttributes.IsSet(Sleep::Id), CHIP_IM_GLOBAL_STATUS(InvalidInState));

    VerifyOrReturnValue(SetAttributeValue(mSleep, sleep, Sleep::Id), CHIP_NO_ERROR);
    if (mContext != nullptr)
    {
        uint8_t value = mSleep ? 1 : 0;
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Sleep::Id),
                                                                { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnSleepChanged(mSleep);
    }

    SyncTargetSetpointToUserSetpoint();

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::SetOptimal(bool optimal)
{
    VerifyOrReturnError(mFeatures.Has(Feature::kOptimal), CHIP_IM_GLOBAL_STATUS(InvalidInState));

    VerifyOrReturnValue(SetAttributeValue(mOptimal, optimal, Optimal::Id), CHIP_NO_ERROR);
    if (mContext != nullptr)
    {
        uint8_t value = mOptimal ? 1 : 0;
        LogErrorOnFailure(mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, Humidistat::Id, Optimal::Id), { &value, sizeof(value) }));
    }

    if (mDelegate != nullptr)
    {
        mDelegate->OnOptimalChanged(mOptimal);
    }

    SyncTargetSetpointToUserSetpoint();

    return CHIP_NO_ERROR;
}

CHIP_ERROR HumidistatCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    static constexpr DataModel::AttributeEntry kOptionalEntries[] = {
        UserSetpoint::kMetadataEntry, MinSetpoint::kMetadataEntry,    MaxSetpoint::kMetadataEntry,
        Step::kMetadataEntry,         TargetSetpoint::kMetadataEntry, MistType::kMetadataEntry,
        Continuous::kMetadataEntry,   Sleep::kMetadataEntry,          Optimal::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Humidistat::Attributes::kMandatoryMetadata), Span(kOptionalEntries), mActiveOptional);
}

CHIP_ERROR HumidistatCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        Humidistat::Commands::SetSettings::kMetadataEntry,
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}

DataModel::ActionReturnStatus HumidistatCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Humidistat::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);

    case Humidistat::Attributes::ClusterRevision::Id:
        return encoder.Encode(Humidistat::kRevision);

    case Mode::Id:
        return encoder.Encode(mMode);

    case SystemState::Id:
        return encoder.Encode(mSystemState);

    case UserSetpoint::Id:
        return encoder.Encode(mUserSetpoint);

    case MinSetpoint::Id:
        return encoder.Encode(mMinSetpoint);

    case MaxSetpoint::Id:
        return encoder.Encode(mMaxSetpoint);

    case Step::Id:
        return encoder.Encode(mStep);

    case TargetSetpoint::Id:
        return encoder.Encode(mTargetSetpoint);

    case MistType::Id:
        return encoder.Encode(mMistType);

    case Continuous::Id:
        return encoder.Encode(mContinuous);

    case Sleep::Id:
        return encoder.Encode(mSleep);

    case Optimal::Id:
        return encoder.Encode(mOptimal);

    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> HumidistatCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                              chip::TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::SetSettings::Id:
        return HandleSetSettings(input_arguments);
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus HumidistatCluster::HandleSetSettings(chip::TLV::TLVReader & input_arguments)
{
    Commands::SetSettings::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    ModeEnum effectiveMode = mMode;
    if (commandData.mode.HasValue())
    {
        VerifyOrReturnError(IsModeSupported(commandData.mode.Value()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        effectiveMode = commandData.mode.Value();
    }

    // Spec: "If the value of Mode is not set to Humidifier, all bits of MistType SHALL be set to zero."
    if (commandData.mistType.HasValue() && mFeatures.Has(Feature::kHumidifier))
    {
        VerifyOrReturnError(IsMistTypeConsistentWithMode(effectiveMode, commandData.mistType.Value()),
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // Spec: unsupported bits SHALL result in INVALID_IN_STATE.
        VerifyOrReturnError(IsMistTypeSupportable(commandData.mistType.Value()), CHIP_IM_GLOBAL_STATUS(InvalidInState));
    }

    if (commandData.userSetpoint.HasValue() && mFeatures.Has(Feature::kSensor))
    {
        chip::Percent requested = commandData.userSetpoint.Value();
        // Validate the raw requested value before snapping, to avoid partial application.
        VerifyOrReturnError(requested >= mMinSetpoint && requested <= mMaxSetpoint, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    if (commandData.mode.HasValue())
    {
        ReturnErrorOnFailure(SetMode(commandData.mode.Value()));
    }

    if (commandData.mistType.HasValue() && mFeatures.Has(Feature::kHumidifier))
    {
        ReturnErrorOnFailure(SetMistType(commandData.mistType.Value()));
    }

    if (commandData.continuous.HasValue() && mFeatures.Has(Feature::kContinuous))
    {
        ReturnErrorOnFailure(SetContinuous(commandData.continuous.Value()));
    }

    if (commandData.sleep.HasValue() && mOptionalAttributes.IsSet(Sleep::Id))
    {
        ReturnErrorOnFailure(SetSleep(commandData.sleep.Value()));
    }

    if (commandData.optimal.HasValue() && mFeatures.Has(Feature::kOptimal))
    {
        ReturnErrorOnFailure(SetOptimal(commandData.optimal.Value()));
    }

    if (commandData.userSetpoint.HasValue() && mFeatures.Has(Feature::kSensor))
    {
        ReturnErrorOnFailure(SetUserSetpoint(commandData.userSetpoint.Value()));
    }

    return CHIP_NO_ERROR;
}

} // namespace chip::app::Clusters
