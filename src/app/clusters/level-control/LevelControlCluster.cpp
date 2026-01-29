/*
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

#include <app/clusters/level-control/LevelControlCluster.h>

#include <algorithm>
#include <app/ConcreteAttributePath.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/LevelControl/Attributes.h>
#include <clusters/LevelControl/Commands.h>
#include <clusters/LevelControl/Enums.h>
#include <clusters/LevelControl/Metadata.h>
#include <cmath>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>

namespace chip::app::Clusters {

using namespace chip::app::Clusters::LevelControl;
using namespace chip::Protocols::InteractionModel;

namespace {

class LevelControlValidator : public scenes::AttributeValuePairValidator
{
public:
    CHIP_ERROR Validate(const app::ConcreteClusterPath & clusterPath,
                        AttributeValuePairValidator::AttributeValuePairType & value) override
    {
        VerifyOrReturnError(clusterPath.mClusterId == LevelControl::Id, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(value.attributeID == Attributes::CurrentLevel::Id, CHIP_ERROR_INVALID_ARGUMENT);
        return CHIP_NO_ERROR;
    }
};

LevelControlValidator & GlobalLevelControlValidator()
{
    static LevelControlValidator sValidator;
    return sValidator;
}

bool IsWithOnOffCommand(CommandId commandId)
{
    return commandId == Commands::MoveToLevelWithOnOff::Id || commandId == Commands::MoveWithOnOff::Id ||
        commandId == Commands::StepWithOnOff::Id || commandId == Commands::StopWithOnOff::Id;
}

} // namespace

LevelControlCluster::LevelControlCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, LevelControl::Id }), scenes::DefaultSceneHandlerImpl(GlobalLevelControlValidator()),
    mCurrentLevel(config.mInitialCurrentLevel), mOptions(BitMask<LevelControl::OptionsBitmap>(0)),
    mOnLevel(DataModel::Nullable<uint8_t>()),
    mMinLevel(config.mFeatureMap.Has(Feature::kLighting) ? kLightingMinLevel : config.mMinLevel),
    mMaxLevel(config.mFeatureMap.Has(Feature::kLighting) ? kMaxLevel : config.mMaxLevel), mDefaultMoveRate(config.mDefaultMoveRate),
    mStartUpCurrentLevel(config.mStartUpCurrentLevel), mRemainingTime(DataModel::Nullable<uint16_t>(0)),
    mOnTransitionTime(config.mOnTransitionTime), mOffTransitionTime(config.mOffTransitionTime),
    mOnOffTransitionTime(config.mOnOffTransitionTime), mOptionalAttributes(config.mOptionalAttributes),
    mFeatureMap(config.mFeatureMap), mDelegate(config.mDelegate), mTimerDelegate(config.mTimerDelegate)
{}

void LevelControlCluster::Shutdown(ClusterShutdownType shutdownType)
{
    CancelTimer();
    DefaultServerCluster::Shutdown(shutdownType);
}

CHIP_ERROR LevelControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    DataModel::Nullable<uint8_t> currentLevel;
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id), currentLevel,
        mCurrentLevel.value());
    mCurrentLevel.SetValue(currentLevel, System::SystemClock().GetMonotonicMilliseconds64());

    if (mFeatureMap.Has(Feature::kLighting))
    {
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::StartUpCurrentLevel::Id), mStartUpCurrentLevel,
            mStartUpCurrentLevel);

        if (!mStartUpCurrentLevel.IsNull())
        {
            // Apply StartUpCurrentLevel logic (0 -> Min, Null -> Ignore, Value -> Value)
            uint8_t target = mStartUpCurrentLevel.Value();
            if (!IsValidLevel(target))
            {
                target = std::clamp<uint8_t>(target, mMinLevel, mMaxLevel);
            }

            // Use SetValue to update internal state without triggering a report or check
            mCurrentLevel.SetValue(DataModel::MakeNullable(target), System::SystemClock().GetMonotonicMilliseconds64());
        }
    }

    if (!mCurrentLevel.value().IsNull())
    {
        mDelegate.OnLevelChanged(mCurrentLevel.value().Value());
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus LevelControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Globals::Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case Globals::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureMap);
    case Attributes::CurrentLevel::Id:
        return encoder.Encode(mCurrentLevel.value());
    case Attributes::Options::Id:
        return encoder.Encode(mOptions);
    case Attributes::OnLevel::Id:
        return encoder.Encode(mOnLevel);
    case Attributes::MinLevel::Id:
        return encoder.Encode(mMinLevel);
    case Attributes::MaxLevel::Id:
        return encoder.Encode(mMaxLevel);
    case Attributes::DefaultMoveRate::Id:
        return encoder.Encode(mDefaultMoveRate);
    case Attributes::StartUpCurrentLevel::Id:
        return encoder.Encode(mStartUpCurrentLevel);
    case Attributes::RemainingTime::Id:
        return encoder.Encode(mRemainingTime.value().ValueOr(0));
    case Attributes::OnTransitionTime::Id:
        return encoder.Encode(mOnTransitionTime);
    case Attributes::OffTransitionTime::Id:
        return encoder.Encode(mOffTransitionTime);
    case Attributes::OnOffTransitionTime::Id:
        return encoder.Encode(mOnOffTransitionTime);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus LevelControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                  AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::Options::Id: {
        BitMask<OptionsBitmap> options;
        ReturnErrorOnFailure(decoder.Decode(options));
        VerifyOrReturnValue(mOptions != options, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        SetOptions(options);
        return Status::Success;
    }
    case Attributes::OnLevel::Id: {
        DataModel::Nullable<uint8_t> onLevel;
        ReturnErrorOnFailure(decoder.Decode(onLevel));
        VerifyOrReturnValue(mOnLevel != onLevel, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        SetOnLevel(onLevel);
        return Status::Success;
    }
    case Attributes::DefaultMoveRate::Id: {
        DataModel::Nullable<uint8_t> rate;
        ReturnErrorOnFailure(decoder.Decode(rate));
        VerifyOrReturnValue(mDefaultMoveRate != rate, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        ReturnErrorOnFailure(SetDefaultMoveRate(rate));
        return Status::Success;
    }
    case Attributes::StartUpCurrentLevel::Id: {
        DataModel::Nullable<uint8_t> startup;
        ReturnErrorOnFailure(decoder.Decode(startup));
        VerifyOrReturnValue(mStartUpCurrentLevel != startup, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        ReturnErrorOnFailure(SetStartUpCurrentLevel(startup));
        return Status::Success;
    }
    case Attributes::OnTransitionTime::Id: {
        DataModel::Nullable<uint16_t> onTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onTransitionTime));
        VerifyOrReturnValue(mOnTransitionTime != onTransitionTime, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        SetOnTransitionTime(onTransitionTime);
        return Status::Success;
    }
    case Attributes::OffTransitionTime::Id: {
        DataModel::Nullable<uint16_t> offTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(offTransitionTime));
        VerifyOrReturnValue(mOffTransitionTime != offTransitionTime, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        SetOffTransitionTime(offTransitionTime);
        return Status::Success;
    }
    case Attributes::OnOffTransitionTime::Id: {
        uint16_t onOffTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onOffTransitionTime));
        VerifyOrReturnValue(mOnOffTransitionTime != onOffTransitionTime,
                            DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        SetOnOffTransitionTime(onOffTransitionTime);
        return Status::Success;
    }
    default:
        return Status::UnsupportedAttribute;
    }
    return Status::Success;
}

CHIP_ERROR LevelControlCluster::Attributes(const ConcreteClusterPath & path,
                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using OptionalEntry                = AttributeListBuilder::OptionalAttributeEntry;
    OptionalEntry optionalAttributes[] = {
        { mOptionalAttributes.IsSet(Attributes::MinLevel::Id), Attributes::MinLevel::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::MaxLevel::Id), Attributes::MaxLevel::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::DefaultMoveRate::Id), Attributes::DefaultMoveRate::kMetadataEntry },
        { mFeatureMap.Has(Feature::kLighting), Attributes::StartUpCurrentLevel::kMetadataEntry },
        { mFeatureMap.Has(Feature::kLighting), Attributes::RemainingTime::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::OnTransitionTime::Id), Attributes::OnTransitionTime::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::OffTransitionTime::Id), Attributes::OffTransitionTime::kMetadataEntry },
        { mOptionalAttributes.IsSet(Attributes::OnOffTransitionTime::Id), Attributes::OnOffTransitionTime::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR LevelControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    DataModel::AcceptedCommandEntry commands[] = {
        Commands::MoveToLevel::kMetadataEntry,
        Commands::Move::kMetadataEntry,
        Commands::Step::kMetadataEntry,
        Commands::Stop::kMetadataEntry,
        // This is odd but the spec mandates these commands even if the On/Off feature is not present
        Commands::MoveToLevelWithOnOff::kMetadataEntry,
        Commands::MoveWithOnOff::kMetadataEntry,
        Commands::StepWithOnOff::kMetadataEntry,
        Commands::StopWithOnOff::kMetadataEntry,
    };
    return builder.AppendElements(Span<DataModel::AcceptedCommandEntry>(commands));
}

std::optional<DataModel::ActionReturnStatus> LevelControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::MoveToLevel::Id: {
        Commands::MoveToLevel::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return MoveToLevel(data.level, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToLevelWithOnOff::Id: {
        Commands::MoveToLevelWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return MoveToLevelWithOnOff(data.level, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::Move::Id: {
        Commands::Move::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return Move(data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveWithOnOff::Id: {
        Commands::MoveWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);

        return MoveWithOnOff(data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::Step::Id: {
        Commands::Step::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return Step(data.stepMode, data.stepSize, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::StepWithOnOff::Id: {
        Commands::StepWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return StepWithOnOff(data.stepMode, data.stepSize, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::Stop::Id: {
        Commands::Stop::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return Stop(data.optionsMask, data.optionsOverride);
    }
    case Commands::StopWithOnOff::Id: {
        Commands::StopWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return StopWithOnOff(data.optionsMask, data.optionsOverride);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus LevelControlCluster::MoveToLevelCommand(CommandId commandId, uint8_t level,
                                                                      DataModel::Nullable<uint16_t> transitionTimeDS,
                                                                      BitMask<OptionsBitmap> optionsMask,
                                                                      BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnError(IsValidLevel(level), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    if (IsWithOnOffCommand(commandId))
    {
        SetOnOff(true);
    }
    else if (!ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    CancelTimer(); // Cancel any currently active transition before starting a new one.

    if (mCurrentLevel.value().IsNull())
    {
        // If the current level is undefined (null), we cannot calculate a transition duration
        // because we don't know the starting point. The spec says "move from its current level".
        // In this case, we treat it as an immediate transition to the target level.
        CHIP_ERROR status = SetCurrentLevel(level);
        if (status == CHIP_NO_ERROR && IsWithOnOffCommand(commandId) && level == mMinLevel)
        {
            SetOnOff(false);
        }
        return status;
    }

    uint8_t currentLevel = mCurrentLevel.value().Value();
    mTargetLevel         = level;
    mCurrentCommandId    = commandId;

    // Calculate Transition Time in Milliseconds
    // Priority: Command argument > OnOffTransitionTime > 0 (Immediate)
    const uint32_t transitionTimeMs = transitionTimeDS.ValueOr(mOnOffTransitionTime) * 100;

    // Refresh CurrentLevel (might have changed due to OnOff logic) and set Direction
    currentLevel = mCurrentLevel.value().ValueOr(currentLevel);
    mIncreasing  = (mTargetLevel > currentLevel);

    // Calculate duration per step
    uint8_t totalSteps = (mIncreasing) ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel);
    mTickDurationMs    = (totalSteps > 0) ? (transitionTimeMs / totalSteps) : 0;

    // Immediate move
    if (transitionTimeMs == 0 || totalSteps == 0 || mTickDurationMs == 0)
    {
        CHIP_ERROR status = SetCurrentLevel(mTargetLevel);
        if (status == CHIP_NO_ERROR && IsWithOnOffCommand(commandId) && mTargetLevel == mMinLevel)
        {
            SetOnOff(false);
        }
        return status;
    }

    StartTransition(mTickDurationMs, transitionTimeMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::MoveCommand(CommandId commandId, MoveModeEnum moveMode,
                                                               DataModel::Nullable<uint8_t> rate,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnError(rate.IsNull() || rate.Value() != 0, Status::InvalidCommand);
    VerifyOrReturnError(!mCurrentLevel.value().IsNull(), Status::Failure);
    VerifyOrReturnError(!rate.IsNull() || !mDefaultMoveRate.IsNull(), Status::Success); // No movement if rate is unspecified

    if (IsWithOnOffCommand(commandId) && moveMode == MoveModeEnum::kUp)
    {
        SetOnOff(true);
    }
    else if (!ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    mCurrentCommandId = commandId;
    CancelTimer(); // Cancel any currently active transition before starting a new one.

    // If rate is null, use default move rate
    uint8_t currentRate = !rate.IsNull() ? rate.Value() : mDefaultMoveRate.Value();

    // Determine Direction first
    mIncreasing = (moveMode == MoveModeEnum::kUp);

    // Now determine Target and Check Constraints (safe from clobbering)
    if (mIncreasing)
    {
        mTargetLevel = mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) ? mMaxLevel : kMaxLevel;
        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.value().Value();
        VerifyOrReturnError(currentLevel < mTargetLevel, Status::Success);
    }
    else
    {
        mTargetLevel = mOptionalAttributes.IsSet(Attributes::MinLevel::Id) ? mMinLevel : 0;
        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.value().Value();
        VerifyOrReturnError(currentLevel > mTargetLevel, Status::Success);
    }

    // Estimate total transition time for RemainingTime reporting (though Move is indefinite until stop/limit)
    uint8_t currentLevel = mCurrentLevel.value().Value();
    uint8_t difference   = (mIncreasing ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel));
    mTickDurationMs      = 1000 / currentRate;

    StartTransition(mTickDurationMs, difference * mTickDurationMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::StepCommand(CommandId commandId, StepModeEnum stepMode, uint8_t stepSize,
                                                               DataModel::Nullable<uint16_t> transitionTime,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    // Spec: "On receipt of this command, if the StepSize field has a value of zero, the command has no effect
    // and a response SHALL be returned with the status code set to INVALID_COMMAND."
    VerifyOrReturnError(stepSize > 0, Status::InvalidCommand);
    VerifyOrReturnError(!mCurrentLevel.value().IsNull(), Status::Failure);

    // Spec: "Before commencing any command that has the effect of setting the CurrentLevel attribute above the minimum level...
    // the OnOff attribute... SHALL be set to TRUE"
    // For Step(Up), we might go above MinLevel (if we are at MinLevel).
    if (IsWithOnOffCommand(commandId) && stepMode == StepModeEnum::kUp)
    {
        SetOnOff(true);
    }
    // Spec: "Command execution SHALL NOT continue beyond the Options processing if...
    // The OnOff attribute... is FALSE." (Unless ExecuteIfOff is set)
    // Note: We use !IsWithOnOffCommand because WithOnOff commands are exempt from this check.
    else if (!IsWithOnOffCommand(commandId) && !ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    mCurrentCommandId = commandId;
    CancelTimer();

    mIncreasing          = (stepMode == StepModeEnum::kUp);
    uint8_t currentLevel = mCurrentLevel.value().Value();

    // Spec: "Up: Increase CurrentLevel by StepSize units, or until it reaches the maximum level..."
    // Spec: "Down: Decrease CurrentLevel by StepSize units, or until it reaches the minimum level..."
    if (mIncreasing)
    {
        int max      = mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) ? mMaxLevel : kMaxLevel;
        mTargetLevel = static_cast<uint8_t>(std::min(currentLevel + stepSize, max));
    }
    else
    {
        int min      = mOptionalAttributes.IsSet(Attributes::MinLevel::Id) ? mMinLevel : 0;
        mTargetLevel = static_cast<uint8_t>(std::max(currentLevel - stepSize, min));
    }

    // Calculate effective step duration
    uint8_t totalSteps = (mIncreasing ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel));

    // Spec: "If the TransitionTime field is equal to null, the device SHOULD move as fast as it is able."
    uint32_t transitionTimeMs = transitionTime.ValueOr(0) * 100;

    // Check if immediate transition is needed (0 time or 0 duration calculated)
    if (transitionTimeMs == 0 || totalSteps == 0 || (transitionTimeMs / totalSteps) == 0)
    {
        CHIP_ERROR status = SetCurrentLevel(mTargetLevel);

        // Spec: "If any command that has the effect of setting the CurrentLevel attribute to the minimum level...
        // the OnOff attribute... SHALL be set to FALSE"
        if (status == CHIP_NO_ERROR && IsWithOnOffCommand(commandId) && mTargetLevel == mMinLevel)
        {
            SetOnOff(false);
        }
        return status;
    }

    mTickDurationMs = transitionTimeMs / totalSteps;
    StartTransition(mTickDurationMs, transitionTimeMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::StopCommand(CommandId commandId, BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    // Spec (Options Attribute): "Command execution SHALL NOT continue beyond the Options processing if...
    // The command is one of the ‘without On/Off’ commands: ... Stop."
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    CancelTimer();
    UpdateRemainingTime(0, ReportingMode::kCommand);
    return SetCurrentLevel(mCurrentLevel.value().Value());
}

void LevelControlCluster::SetOptions(BitMask<OptionsBitmap> newOptions)
{
    VerifyOrReturn(SetAttributeValue(mOptions, newOptions, Attributes::Options::Id));
    mDelegate.OnOptionsChanged(mOptions);
}

void LevelControlCluster::SetOnLevel(DataModel::Nullable<uint8_t> newOnLevel)
{
    VerifyOrReturn(SetAttributeValue(mOnLevel, newOnLevel, Attributes::OnLevel::Id));
    mDelegate.OnOnLevelChanged(mOnLevel);
}

CHIP_ERROR LevelControlCluster::SetDefaultMoveRate(DataModel::Nullable<uint8_t> newDefaultMoveRate)
{
    // Validate constraint: Min 1
    VerifyOrReturnError(newDefaultMoveRate.ValueOr(1) >= 1, CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(SetAttributeValue(mDefaultMoveRate, newDefaultMoveRate, Attributes::DefaultMoveRate::Id), CHIP_NO_ERROR);

    mDelegate.OnDefaultMoveRateChanged(mDefaultMoveRate);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetCurrentLevel(uint8_t level)
{
    VerifyOrReturnError(IsValidLevel(level), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mCurrentLevel.value().IsNull() || mCurrentLevel.value().Value() != level, CHIP_NO_ERROR); // No change

    mCurrentLevel.SetValue(DataModel::MakeNullable(level), System::SystemClock().GetMonotonicMilliseconds64());
    NotifyAttributeChanged(Attributes::CurrentLevel::Id);
    StoreCurrentLevel(mCurrentLevel.value());
    mDelegate.OnLevelChanged(level);

    return CHIP_NO_ERROR;
}

void LevelControlCluster::StoreCurrentLevel(DataModel::Nullable<uint8_t> value)
{
    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<uint8_t>::StorageType storageValue;
    DataModel::NullableToStorage(value, storageValue);

    CHIP_ERROR err = mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue)));
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "LevelControlCluster: Failed to store CurrentLevel: %" CHIP_ERROR_FORMAT, err.Format()));
}

CHIP_ERROR LevelControlCluster::SetCurrentLevelQuietReport(DataModel::Nullable<uint8_t> newValue)
{
    VerifyOrReturnError(mCurrentLevel.value() != newValue, CHIP_NO_ERROR);

    auto now = System::SystemClock().GetMonotonicMilliseconds64();

    // Spec: "At most once per second"
    AttributeDirtyState dirtyState = mCurrentLevel.SetValue(
        newValue, now,
        QuieterReportingAttribute<uint8_t>::GetPredicateForSufficientTimeSinceLastDirty(chip::System::Clock::Milliseconds64(1000)));

    StoreCurrentLevel(mCurrentLevel.value());

    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        NotifyAttributeChanged(Attributes::CurrentLevel::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetStartUpCurrentLevel(DataModel::Nullable<uint8_t> startupLevel)
{
    VerifyOrReturnError(SetAttributeValue(mStartUpCurrentLevel, startupLevel, Attributes::StartUpCurrentLevel::Id), CHIP_NO_ERROR);
    VerifyOrReturnError(mContext != nullptr, CHIP_NO_ERROR);

    NumericAttributeTraits<uint8_t>::StorageType storageValue;
    DataModel::NullableToStorage(startupLevel, storageValue);
    return mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::StartUpCurrentLevel::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue)));
}

void LevelControlCluster::SetOnTransitionTime(DataModel::Nullable<uint16_t> onTransitionTime)
{
    SetAttributeValue(mOnTransitionTime, onTransitionTime, Attributes::OnTransitionTime::Id);
}

void LevelControlCluster::SetOffTransitionTime(DataModel::Nullable<uint16_t> offTransitionTime)
{
    SetAttributeValue(mOffTransitionTime, offTransitionTime, Attributes::OffTransitionTime::Id);
}

void LevelControlCluster::SetOnOffTransitionTime(uint16_t onOffTransitionTime)
{
    SetAttributeValue(mOnOffTransitionTime, onOffTransitionTime, Attributes::OnOffTransitionTime::Id);
}

bool LevelControlCluster::IsValidLevel(uint8_t level)
{
    bool isBelowGlobalMax = level <= kMaxLevel;
    bool validMin         = !mOptionalAttributes.IsSet(Attributes::MinLevel::Id) || level >= mMinLevel;
    bool validMax         = !mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) || level <= mMaxLevel;

    return isBelowGlobalMax && validMin && validMax;
}

void LevelControlCluster::SetOnOff(bool on)
{
    VerifyOrReturn(mFeatureMap.Has(Feature::kOnOff) && !(on && mDelegate.GetOnOff()));

    // Prevent potential callback loops
    mTemporarilyIgnoreOnOffCallbacks = true;
    mDelegate.SetOnOff(on);
    mTemporarilyIgnoreOnOffCallbacks = false;
}

void LevelControlCluster::UpdateRemainingTime(uint32_t remainingTimeMs, ReportingMode mode)
{
    VerifyOrReturn(mFeatureMap.Has(Feature::kLighting));

    // Convert ms to ds (rounding up)
    uint16_t remainingTimeDs = static_cast<uint16_t>((remainingTimeMs + 99) / 100);

    auto now = System::SystemClock().GetMonotonicMilliseconds64();

    // Spec: "Changes to this attribute SHALL only be marked as reportable in the following cases:
    // - When it changes from 0 to any value higher than 10, or
    // - When it changes, with a delta larger than 10, caused by the invoke of a command, or
    // - When it changes to 0."
    if (mRemainingTime.SetValue(DataModel::MakeNullable(remainingTimeDs), now, [this, mode](const auto & candidate) {
            // "As this attribute is not being reported during a regular countdown..."
            if (mode == ReportingMode::kTick)
            {
                return candidate.newValue.ValueOr(0) == 0 && candidate.lastDirtyValue.ValueOr(0) != 0;
            }

            VerifyOrReturnValue(mTransitionTimeMs >= 1000, false);

            auto lastDirty = candidate.lastDirtyValue.ValueOr(0);
            auto newValue  = candidate.newValue.ValueOr(0);

            return ((newValue == 0 && lastDirty != 0) || std::abs(static_cast<int>(newValue) - static_cast<int>(lastDirty)) > 10);
        }) == AttributeDirtyState::kMustReport)
    {
        NotifyAttributeChanged(Attributes::RemainingTime::Id);
    }
}

void LevelControlCluster::StartTimer(uint32_t delayMs)
{
    SuccessOrDie(mTimerDelegate.StartTimer(this, System::Clock::Milliseconds64(delayMs)));
}

void LevelControlCluster::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void LevelControlCluster::TimerFired()
{
    VerifyOrReturn(!mCurrentLevel.value().IsNull());

    mElapsedTimeMs += mTickDurationMs;

    // RemainingTime update: This is a "regular countdown", so use Tick logic (only report at 0)
    uint16_t remainingTimeMs = 0;
    if (mFeatureMap.Has(Feature::kLighting) && mTransitionTimeMs > 0 && mElapsedTimeMs < mTransitionTimeMs)
    {
        remainingTimeMs = static_cast<uint16_t>(mTransitionTimeMs - mElapsedTimeMs);
    }
    UpdateRemainingTime(remainingTimeMs, ReportingMode::kTick);

    // Calculate new level
    uint8_t currentLevel = mCurrentLevel.value().Value();
    if (mIncreasing && (currentLevel < mTargetLevel))
    {
        currentLevel++;
    }
    else if (!mIncreasing && (currentLevel > mTargetLevel))
    {
        currentLevel--;
    }

    // End of transition
    if (currentLevel == mTargetLevel)
    {
        RETURN_SAFELY_IGNORED SetCurrentLevel(currentLevel);

        UpdateRemainingTime(0, ReportingMode::kCommand); // Transition complete, ensure RemainingTime is 0

        // If reached minimum, turn off OnOff cluster
        if (IsWithOnOffCommand(mCurrentCommandId) && (currentLevel == mMinLevel || currentLevel == 0))
        {
            SetOnOff(false);
        }
        return;
    }

    mDelegate.OnLevelChanged(currentLevel);
    // Ignore error as this is a background tick.
    RETURN_SAFELY_IGNORED SetCurrentLevelQuietReport(DataModel::MakeNullable(currentLevel));

    StartTimer(mTickDurationMs);
}

void LevelControlCluster::OnOffChanged(bool isOn)
{
    VerifyOrReturn(!mCurrentLevel.value().IsNull() && !mTemporarilyIgnoreOnOffCallbacks);

    if (isOn)
    {
        // On Transition
        // 2. Determine Target Level (Capture before setting to Min)
        uint8_t target = kMaxLevel; // Default Max
        if (!mOnLevel.IsNull())
        {
            target = mOnLevel.Value();
        }
        else if (!mStoredLevel.IsNull())
        {
            target = mStoredLevel.Value();
        }

        // 1. Set to MinLevel
        // Ignore error as we are internally forcing a valid level (MinLevel) to start the transition.
        RETURN_SAFELY_IGNORED SetCurrentLevel(mMinLevel);

        // 3. Determine Transition Time
        DataModel::Nullable<uint16_t> transitionTime;
        if (mOptionalAttributes.IsSet(Attributes::OnTransitionTime::Id) && !mOnTransitionTime.IsNull())
        {
            transitionTime = mOnTransitionTime;
        }
        else if (mOptionalAttributes.IsSet(Attributes::OnOffTransitionTime::Id))
        {
            transitionTime.SetNonNull(mOnOffTransitionTime);
        }

        // 4. Move
        BitMask<OptionsBitmap> options;
        MoveToLevelCommand(Commands::MoveToLevelWithOnOff::Id, target, transitionTime, options, options);
    }
    else
    {
        // Off Transition
        // Store CurrentLevel
        mStoredLevel = mCurrentLevel.value();

        // Move to MinLevel
        DataModel::Nullable<uint16_t> transitionTime;
        if (mOptionalAttributes.IsSet(Attributes::OffTransitionTime::Id) && !mOffTransitionTime.IsNull())
        {
            transitionTime = mOffTransitionTime;
        }
        else if (mOptionalAttributes.IsSet(Attributes::OnOffTransitionTime::Id))
        {
            transitionTime.SetNonNull(mOnOffTransitionTime);
        }

        // Force execution to allow fading out even if device is technically "Off"
        BitMask<OptionsBitmap> optionsMask(OptionsBitmap::kExecuteIfOff);
        BitMask<OptionsBitmap> optionsOverride(OptionsBitmap::kExecuteIfOff);
        MoveToLevelCommand(Commands::MoveToLevelWithOnOff::Id, mMinLevel, transitionTime, optionsMask, optionsOverride);
    }
}

void LevelControlCluster::StartTransition(uint32_t tickDurationMs, uint32_t transitionTimeMs)
{
    mTransitionTimeMs = transitionTimeMs;
    mTickDurationMs   = tickDurationMs;

    // Command invoked: Set RemainingTime using full reporting rules
    UpdateRemainingTime(mTransitionTimeMs, ReportingMode::kCommand);
    StartTimer(mTickDurationMs);
}

bool LevelControlCluster::ShouldExecuteIfOff(BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    // Spec: "Command execution SHALL NOT continue beyond the Options processing if all of these criteria are true:
    // ...
    // * The On/Off cluster exists on the same endpoint as this cluster.
    // * The OnOff attribute of the On/Off cluster, on this endpoint, is FALSE.
    // * The value of the ExecuteIfOff bit is 0."

    // 1. If On/Off feature is not supported, there is no dependency, so we execute.
    // 2. If the OnOff state is On, we execute.
    if (!mFeatureMap.Has(Feature::kOnOff) || mDelegate.GetOnOff())
    {
        return true;
    }

    // 3. The device is Off. We check the ExecuteIfOff bit.
    if (optionsMask.Has(OptionsBitmap::kExecuteIfOff))
    {
        return optionsOverride.Has(OptionsBitmap::kExecuteIfOff);
    }
    return mOptions.Has(OptionsBitmap::kExecuteIfOff);
}

bool LevelControlCluster::SupportsCluster(EndpointId endpoint, ClusterId cluster)
{
    return (cluster == LevelControl::Id) && (endpoint == mPath.mEndpointId);
}

CHIP_ERROR LevelControlCluster::SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    VerifyOrReturnError(SupportsCluster(endpoint, cluster), CHIP_ERROR_INVALID_ARGUMENT);

    AttributeValuePair pairs[1];

    if (!mCurrentLevel.value().IsNull())
    {
        pairs[0].attributeID = Attributes::CurrentLevel::Id;
        pairs[0].valueUnsigned8.SetValue(mCurrentLevel.value().Value());
    }

    app::DataModel::List<AttributeValuePair> attributeValueList(pairs, !mCurrentLevel.value().IsNull() ? 1 : 0);
    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR LevelControlCluster::ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                           scenes::TransitionTimeMs timeMs)
{
    app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

    VerifyOrReturnError(SupportsCluster(endpoint, cluster), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    auto pair_iterator = attributeValueList.begin();
    while (pair_iterator.Next())
    {
        auto & decodePair = pair_iterator.GetValue();
        if (decodePair.attributeID == Attributes::CurrentLevel::Id && decodePair.valueUnsigned8.HasValue())
        {
            uint8_t level = decodePair.valueUnsigned8.Value();
            DataModel::Nullable<uint16_t> ds;
            ds.SetNonNull(static_cast<uint16_t>(timeMs / 100));

            // Scenes must be applied even if the device is Off, overriding the Options attribute.
            BitMask<OptionsBitmap> optionsMask(OptionsBitmap::kExecuteIfOff);
            BitMask<OptionsBitmap> optionsOverride(OptionsBitmap::kExecuteIfOff);

            MoveToLevelCommand(Commands::MoveToLevel::Id, level, ds, optionsMask, optionsOverride);
        }
    }
    return pair_iterator.GetStatus();
}

DataModel::ActionReturnStatus LevelControlCluster::MoveToLevel(uint8_t level, DataModel::Nullable<uint16_t> transitionTime,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    return MoveToLevelCommand(Commands::MoveToLevel::Id, level, transitionTime, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::MoveToLevelWithOnOff(uint8_t level, DataModel::Nullable<uint16_t> transitionTime,
                                                                        BitMask<OptionsBitmap> optionsMask,
                                                                        BitMask<OptionsBitmap> optionsOverride)
{
    return MoveToLevelCommand(Commands::MoveToLevelWithOnOff::Id, level, transitionTime, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::Move(MoveModeEnum moveMode, DataModel::Nullable<uint8_t> rate,
                                                        BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    return MoveCommand(Commands::Move::Id, moveMode, rate, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::MoveWithOnOff(MoveModeEnum moveMode, DataModel::Nullable<uint8_t> rate,
                                                                 BitMask<OptionsBitmap> optionsMask,
                                                                 BitMask<OptionsBitmap> optionsOverride)
{
    return MoveCommand(Commands::MoveWithOnOff::Id, moveMode, rate, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::Step(StepModeEnum stepMode, uint8_t stepSize,
                                                        DataModel::Nullable<uint16_t> transitionTime,
                                                        BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    return StepCommand(Commands::Step::Id, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::StepWithOnOff(StepModeEnum stepMode, uint8_t stepSize,
                                                                 DataModel::Nullable<uint16_t> transitionTime,
                                                                 BitMask<OptionsBitmap> optionsMask,
                                                                 BitMask<OptionsBitmap> optionsOverride)
{
    return StepCommand(Commands::StepWithOnOff::Id, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::Stop(BitMask<OptionsBitmap> optionsMask, BitMask<OptionsBitmap> optionsOverride)
{
    return StopCommand(Commands::Stop::Id, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::StopWithOnOff(BitMask<OptionsBitmap> optionsMask,
                                                                 BitMask<OptionsBitmap> optionsOverride)
{
    return StopCommand(Commands::StopWithOnOff::Id, optionsMask, optionsOverride);
}

} // namespace chip::app::Clusters
