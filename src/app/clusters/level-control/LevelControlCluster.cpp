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
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/LevelControl/Attributes.h>
#include <clusters/LevelControl/Commands.h>
#include <clusters/LevelControl/Enums.h>
#include <clusters/LevelControl/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>

namespace chip::app::Clusters {

using namespace chip::app::Clusters::LevelControl;

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

} // namespace

LevelControlCluster::LevelControlCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, LevelControl::Id }), scenes::DefaultSceneHandlerImpl(GlobalLevelControlValidator()),
    mCurrentLevel(config.mInitialCurrentLevel), mOptions(BitMask<LevelControl::OptionsBitmap>(0)),
    mOnLevel(DataModel::Nullable<uint8_t>()), mMinLevel(config.mMinLevel), mMaxLevel(config.mMaxLevel),
    mDefaultMoveRate(config.mDefaultMoveRate), mStartUpCurrentLevel(config.mStartUpCurrentLevel),
    mRemainingTime(DataModel::MakeNullable<uint16_t>(0)), mOnTransitionTime(config.mOnTransitionTime),
    mOffTransitionTime(config.mOffTransitionTime), mOnOffTransitionTime(config.mOnOffTransitionTime),
    mOptionalAttributes(config.mOptionalAttributes), mFeatureMap(config.mFeatureMap), mDelegate(config.mDelegate),
    mTimerDelegate(config.mTimerDelegate)
{}

LevelControlCluster::~LevelControlCluster()
{
    CancelTimer();
}

CHIP_ERROR LevelControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    if (mFeatureMap.Has(Feature::kLighting) && !mStartUpCurrentLevel.IsNull())
    {
        // Apply StartUpCurrentLevel logic (0 -> Min, Null -> Ignore, Value -> Value)
        uint8_t target = mStartUpCurrentLevel.Value();
        if (mOptionalAttributes.IsSet(Attributes::MinLevel::Id) && target < mMinLevel)
            target = mMinLevel;
        if (mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) && target > mMaxLevel)
            target = mMaxLevel;

        // Use SetValue to update internal state without triggering a report or check
        mCurrentLevel.SetNonNull(target);
        mDelegate.OnLevelChanged(target);
    }
    return CHIP_NO_ERROR;
}

// ... Standard Attribute Methods ...

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
        return encoder.Encode(mCurrentLevel);
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
        return encoder.Encode(mRemainingTime);
    case Attributes::OnTransitionTime::Id:
        return encoder.Encode(mOnTransitionTime);
    case Attributes::OffTransitionTime::Id:
        return encoder.Encode(mOffTransitionTime);
    case Attributes::OnOffTransitionTime::Id:
        return encoder.Encode(mOnOffTransitionTime);
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
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
        return SetOptions(options);
    }
    case Attributes::OnLevel::Id: {
        DataModel::Nullable<uint8_t> onLevel;
        ReturnErrorOnFailure(decoder.Decode(onLevel));
        return SetOnLevel(onLevel);
    }
    case Attributes::DefaultMoveRate::Id: {
        DataModel::Nullable<uint8_t> rate;
        ReturnErrorOnFailure(decoder.Decode(rate));
        return SetDefaultMoveRate(rate);
    }
    case Attributes::StartUpCurrentLevel::Id: {
        DataModel::Nullable<uint8_t> startup;
        ReturnErrorOnFailure(decoder.Decode(startup));
        return SetStartUpCurrentLevel(startup);
    }
    case Attributes::OnTransitionTime::Id: {
        DataModel::Nullable<uint16_t> onTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onTransitionTime));
        return SetOnTransitionTime(onTransitionTime);
    }
    case Attributes::OffTransitionTime::Id: {
        DataModel::Nullable<uint16_t> offTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(offTransitionTime));
        return SetOffTransitionTime(offTransitionTime);
    }
    case Attributes::OnOffTransitionTime::Id: {
        uint16_t onOffTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onOffTransitionTime));
        return SetOnOffTransitionTime(onOffTransitionTime);
    }
    default:
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
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
    DataModel::AcceptedCommandEntry commands[8];
    size_t count = 0;

    commands[count++] = Commands::MoveToLevel::kMetadataEntry;
    commands[count++] = Commands::Move::kMetadataEntry;
    commands[count++] = Commands::Step::kMetadataEntry;
    commands[count++] = Commands::Stop::kMetadataEntry;

    if (mFeatureMap.Has(Feature::kOnOff))
    {
        commands[count++] = Commands::MoveToLevelWithOnOff::kMetadataEntry;
        commands[count++] = Commands::MoveWithOnOff::kMetadataEntry;
        commands[count++] = Commands::StepWithOnOff::kMetadataEntry;
        commands[count++] = Commands::StopWithOnOff::kMetadataEntry;
    }

    return builder.AppendElements(Span<DataModel::AcceptedCommandEntry>(commands, count));
}

std::optional<DataModel::ActionReturnStatus> LevelControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                TLV::TLVReader & input_arguments,
                                                                                CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::MoveToLevel::Id: {
        Commands::MoveToLevel::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return MoveToLevelHandler(request.path.mCommandId, data.level, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToLevelWithOnOff::Id: {
        Commands::MoveToLevelWithOnOff::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return MoveToLevelHandler(request.path.mCommandId, data.level, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::Move::Id: {
        Commands::Move::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return MoveHandler(request.path.mCommandId, data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveWithOnOff::Id: {
        Commands::MoveWithOnOff::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return MoveHandler(request.path.mCommandId, data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::Step::Id: {
        Commands::Step::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return StepHandler(request.path.mCommandId, data.stepMode, data.stepSize, data.transitionTime, data.optionsMask,
                           data.optionsOverride);
    }
    case Commands::StepWithOnOff::Id: {
        Commands::StepWithOnOff::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return StepHandler(request.path.mCommandId, data.stepMode, data.stepSize, data.transitionTime, data.optionsMask,
                           data.optionsOverride);
    }
    case Commands::Stop::Id: {
        Commands::Stop::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return StopHandler(request.path.mCommandId, data.optionsMask, data.optionsOverride);
    }
    case Commands::StopWithOnOff::Id: {
        Commands::StopWithOnOff::DecodableType data;
        if (DataModel::Decode(input_arguments, data) != CHIP_NO_ERROR)
            return DataModel::ActionReturnStatus(CHIP_IM_GLOBAL_STATUS(InvalidCommand));
        return StopHandler(request.path.mCommandId, data.optionsMask, data.optionsOverride);
    }
    }
    return std::nullopt;
}

DataModel::ActionReturnStatus LevelControlCluster::MoveToLevelHandler(CommandId commandId, uint8_t level,
                                                                      DataModel::Nullable<uint16_t> transitionTimeDS,
                                                                      BitMask<OptionsBitmap> optionsMask,
                                                                      BitMask<OptionsBitmap> optionsOverride)
{
    // Check if command execution is allowed by On/Off state (ExecuteIfOff bit)
    if (!ShouldExecuteIfOff(commandId, optionsMask, optionsOverride))
        return Protocols::InteractionModel::Status::Success;

    // Cancel any currently active transition before starting a new one.
    CancelTimer();

    // Validate level against Min/Max constraints
    if (level > 254)
        return Protocols::InteractionModel::Status::ConstraintError;
    if (mOptionalAttributes.IsSet(Attributes::MinLevel::Id) && level < mMinLevel)
        return Protocols::InteractionModel::Status::ConstraintError;
    if (mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) && level > mMaxLevel)
        return Protocols::InteractionModel::Status::ConstraintError;

    if (mCurrentLevel.IsNull())
        return Protocols::InteractionModel::Status::Failure;

    uint8_t currentLevel = mCurrentLevel.Value();
    mTargetLevel         = level;
    mCurrentCommandId    = commandId;
    mIncreasing          = (mTargetLevel > currentLevel);

    // Calculate Transition Time in Milliseconds
    // Priority: Command argument > OnOffTransitionTime > 0 (Immediate)
    uint32_t transitionTimeMs = 0;
    if (!transitionTimeDS.IsNull())
    {
        transitionTimeMs = transitionTimeDS.Value() * 100;
    }
    else
    {
        transitionTimeMs = mOnOffTransitionTime * 100;
    }

    mTransitionTimeMs = transitionTimeMs;
    mElapsedTimeMs    = 0;

    // Handle "With On/Off" commands: Turn On if moving up.
    if ((commandId == Commands::MoveToLevelWithOnOff::Id) && mIncreasing)
    {
        UpdateOnOff(true, true /* temporarilyIgnoreOnOffCallbacks */);

        // If turning on, ensure we start at least at MinLevel (Spec: "set CurrentLevel to MinLevel")
        if (!mCurrentLevel.IsNull() && mCurrentLevel.Value() < mMinLevel)
        {
            SetCurrentLevel(mMinLevel);
        }

        if (!mCurrentLevel.IsNull())
        {
            currentLevel = mCurrentLevel.Value();
        }
    }

    // Immediate move
    if (mTransitionTimeMs == 0)
        return SetCurrentLevel(mTargetLevel);

    // Calculate duration per step
    uint8_t totalSteps = (mIncreasing) ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel);
    if (totalSteps == 0)
        return Protocols::InteractionModel::Status::Success;

    mEventDurationMs = mTransitionTimeMs / totalSteps;
    if (mEventDurationMs == 0)
        mEventDurationMs = 1;

    // Update RemainingTime immediately for start of transition
    UpdateRemainingTime(mTransitionTimeMs, true);

    StartTimer(mEventDurationMs);
    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::MoveHandler(CommandId commandId, MoveModeEnum moveMode,
                                                               DataModel::Nullable<uint8_t> rate,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    if (!ShouldExecuteIfOff(commandId, optionsMask, optionsOverride))
        return Protocols::InteractionModel::Status::Success;

    if (!rate.IsNull() && rate.Value() == 0)
        return Protocols::InteractionModel::Status::InvalidCommand;
    if (mCurrentLevel.IsNull())
        return Protocols::InteractionModel::Status::Failure;

    mCurrentCommandId = commandId;
    CancelTimer();

    // Determine Rate (units/s)
    uint8_t currentRate = 0;
    if (!rate.IsNull())
        currentRate = rate.Value();
    else if (!mDefaultMoveRate.IsNull())
        currentRate = mDefaultMoveRate.Value();

    if (currentRate == 0)
        return Protocols::InteractionModel::Status::Success;

    mEventDurationMs = 1000 / currentRate;
    mElapsedTimeMs   = 0;

    // Determine Direction first
    if (moveMode == MoveModeEnum::kUp)
    {
        mIncreasing = true;
    }
    else
    {
        mIncreasing = false;
    }

    if ((commandId == Commands::MoveWithOnOff::Id) && mIncreasing)
    {
        UpdateOnOff(true, true /* temporarilyIgnoreOnOffCallbacks */);

        if (!mCurrentLevel.IsNull() && mCurrentLevel.Value() < mMinLevel)
        {
            SetCurrentLevel(mMinLevel);
        }
    }

    // Now determine Target and Check Constraints (safe from clobbering)
    if (mIncreasing)
    {
        mTargetLevel = mMaxLevel;
        if (mOptionalAttributes.IsSet(Attributes::MaxLevel::Id))
            mTargetLevel = mMaxLevel;
        else
            mTargetLevel = 254; // Default max

        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.Value();
        if (currentLevel >= mTargetLevel)
            return Protocols::InteractionModel::Status::Success;
    }
    else
    {
        mTargetLevel = 0;
        if (mOptionalAttributes.IsSet(Attributes::MinLevel::Id))
            mTargetLevel = mMinLevel;

        uint8_t currentLevel = mCurrentLevel.Value();
        if (currentLevel <= mTargetLevel)
            return Protocols::InteractionModel::Status::Success;
    }

    // Estimate total transition time for RemainingTime reporting (though Move is indefinite until stop/limit)
    uint8_t currentLevel = mCurrentLevel.Value();
    uint8_t difference   = 0;
    if (mIncreasing)
        difference = mTargetLevel - currentLevel;
    else
        difference = currentLevel - mTargetLevel;

    mTransitionTimeMs = difference * mEventDurationMs;
    UpdateRemainingTime(mTransitionTimeMs, true);

    StartTimer(mEventDurationMs);
    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::StepHandler(CommandId commandId, StepModeEnum stepMode, uint8_t stepSize,
                                                               DataModel::Nullable<uint16_t> transitionTime,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    if (!ShouldExecuteIfOff(commandId, optionsMask, optionsOverride))
        return Protocols::InteractionModel::Status::Success;

    if (stepSize == 0)
        return Protocols::InteractionModel::Status::InvalidCommand;
    if (mCurrentLevel.IsNull())
        return Protocols::InteractionModel::Status::Failure;

    mCurrentCommandId = commandId;
    CancelTimer();

    uint32_t transitionTimeMs = 0;
    if (!transitionTime.IsNull())
        transitionTimeMs = transitionTime.Value() * 100;

    // Determine Direction first
    if (stepMode == StepModeEnum::kUp)
    {
        mIncreasing = true;
    }
    else
    {
        mIncreasing = false;
    }

    if ((commandId == Commands::StepWithOnOff::Id) && mIncreasing)
    {
        UpdateOnOff(true, true /* temporarilyIgnoreOnOffCallbacks */);

        if (!mCurrentLevel.IsNull() && mCurrentLevel.Value() < mMinLevel)
        {
            SetCurrentLevel(mMinLevel);
        }
    }

    // Recalculate everything based on potential new current level
    uint8_t currentLevel = mCurrentLevel.Value();

    // Calculate Target based on Step Size
    if (mIncreasing)
    {
        uint16_t next = currentLevel + stepSize;
        uint8_t max   = mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) ? mMaxLevel : 254;
        if (next > max)
            next = max;
        mTargetLevel = static_cast<uint8_t>(next);
    }
    else
    {
        int16_t next = currentLevel - stepSize;
        uint8_t min  = mOptionalAttributes.IsSet(Attributes::MinLevel::Id) ? mMinLevel : 0;
        if (next < min)
            next = min;
        mTargetLevel = static_cast<uint8_t>(next);
    }

    mTransitionTimeMs = transitionTimeMs;

    if (mTransitionTimeMs == 0)
        return SetCurrentLevel(mTargetLevel);

    uint8_t totalSteps = (mIncreasing) ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel);
    if (totalSteps == 0)
        return Protocols::InteractionModel::Status::Success;

    mEventDurationMs = mTransitionTimeMs / totalSteps;
    if (mEventDurationMs == 0)
        mEventDurationMs = 1;

    UpdateRemainingTime(mTransitionTimeMs, true);
    StartTimer(mEventDurationMs);
    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::StopHandler(CommandId commandId, BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    if (!ShouldExecuteIfOff(commandId, optionsMask, optionsOverride))
        return Protocols::InteractionModel::Status::Success;

    CancelTimer();
    // Force report of current level and reset remaining time
    SetCurrentLevelQuietReport(mCurrentLevel, true /*isEndOfTransition*/);
    UpdateRemainingTime(0, false);
    return Protocols::InteractionModel::Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::SetOptions(BitMask<OptionsBitmap> newOptions)
{
    if (mOptions == newOptions)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mOptions = newOptions;
    mDelegate.OnOptionsChanged(mOptions);
    return NotifyAttributeChangedIfSuccess(Attributes::Options::Id, Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus LevelControlCluster::SetOnLevel(DataModel::Nullable<uint8_t> newOnLevel)
{
    if (mOnLevel == newOnLevel)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mOnLevel = newOnLevel;
    mDelegate.OnOnLevelChanged(mOnLevel);
    return NotifyAttributeChangedIfSuccess(Attributes::OnLevel::Id, Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus LevelControlCluster::SetDefaultMoveRate(DataModel::Nullable<uint8_t> newDefaultMoveRate)
{
    // Validate constraint: Min 1
    if (!newDefaultMoveRate.IsNull() && newDefaultMoveRate.Value() < 1)
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    if (mDefaultMoveRate == newDefaultMoveRate)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mDefaultMoveRate = newDefaultMoveRate;
    mDelegate.OnDefaultMoveRateChanged(mDefaultMoveRate);
    return NotifyAttributeChangedIfSuccess(Attributes::DefaultMoveRate::Id, Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus LevelControlCluster::SetCurrentLevel(uint8_t level)
{
    // Validate constraints against Min/Max
    if (mOptionalAttributes.IsSet(Attributes::MinLevel::Id) && level < mMinLevel)
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    if (mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) && level > mMaxLevel)
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);

    if (mCurrentLevel.IsNull() || mCurrentLevel.Value() != level)
    {
        mDelegate.OnLevelChanged(level);
        return SetCurrentLevelQuietReport(DataModel::MakeNullable(level), true);
    }
    return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
}

DataModel::ActionReturnStatus LevelControlCluster::SetStartUpCurrentLevel(DataModel::Nullable<uint8_t> startupLevel)
{
    if (mStartUpCurrentLevel == startupLevel)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mStartUpCurrentLevel = startupLevel;
    return NotifyAttributeChangedIfSuccess(Attributes::StartUpCurrentLevel::Id, Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus LevelControlCluster::SetOnTransitionTime(DataModel::Nullable<uint16_t> onTransitionTime)
{
    if (mOnTransitionTime == onTransitionTime)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mOnTransitionTime = onTransitionTime;
    return NotifyAttributeChangedIfSuccess(Attributes::OnTransitionTime::Id, Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus LevelControlCluster::SetOffTransitionTime(DataModel::Nullable<uint16_t> offTransitionTime)
{
    if (mOffTransitionTime == offTransitionTime)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mOffTransitionTime = offTransitionTime;
    return NotifyAttributeChangedIfSuccess(Attributes::OffTransitionTime::Id, Protocols::InteractionModel::Status::Success);
}

DataModel::ActionReturnStatus LevelControlCluster::SetOnOffTransitionTime(uint16_t onOffTransitionTime)
{
    if (mOnOffTransitionTime == onOffTransitionTime)
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    mOnOffTransitionTime = onOffTransitionTime;
    return NotifyAttributeChangedIfSuccess(Attributes::OnOffTransitionTime::Id, Protocols::InteractionModel::Status::Success);
}

void LevelControlCluster::UpdateOnOff(bool on, bool temporarilyIgnoreOnOffCallbacks)
{
    if (mFeatureMap.Has(Feature::kOnOff))
    {
        if (temporarilyIgnoreOnOffCallbacks)
            mTemporarilyIgnoreOnOffCallbacks = true;

        mDelegate.SetOnOff(on);

        if (temporarilyIgnoreOnOffCallbacks)
            mTemporarilyIgnoreOnOffCallbacks = false;
    }
}

bool LevelControlCluster::ShouldExecuteIfOff(CommandId commandId, BitMask<OptionsBitmap> optionsMask,
                                             BitMask<OptionsBitmap> optionsOverride)
{
    if (!mFeatureMap.Has(Feature::kOnOff))
        return true;

    // Commands that bypass the check
    if (commandId == Commands::MoveToLevelWithOnOff::Id || commandId == Commands::MoveWithOnOff::Id ||
        commandId == Commands::StepWithOnOff::Id || commandId == Commands::StopWithOnOff::Id)
    {
        return true;
    }

    if (mDelegate.GetOnOff())
        return true;

    bool executeIfOff = false;
    if (mOptions.Has(OptionsBitmap::kExecuteIfOff))
        executeIfOff = true;
    if (optionsMask.Has(OptionsBitmap::kExecuteIfOff))
    {
        if (optionsOverride.Has(OptionsBitmap::kExecuteIfOff))
        {
            executeIfOff = true;
        }
        else
        {
            executeIfOff = false;
        }
    }

    return executeIfOff;
}

void LevelControlCluster::TimerFired()
{
    HandleTick();
}

bool LevelControlCluster::SupportsCluster(EndpointId endpoint, ClusterId cluster)
{
    return (cluster == LevelControl::Id) && (endpoint == mPath.mEndpointId);
}

CHIP_ERROR LevelControlCluster::SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    if (!SupportsCluster(endpoint, cluster))
        return CHIP_ERROR_INVALID_ARGUMENT;

    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::CurrentLevel::Id;
    if (!mCurrentLevel.IsNull())
        pairs[0].valueUnsigned8.SetValue(mCurrentLevel.Value());
    else
        pairs[0].valueUnsigned8.SetValue(0);

    app::DataModel::List<AttributeValuePair> attributeValueList(pairs);
    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR LevelControlCluster::ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                           scenes::TransitionTimeMs timeMs)
{
    app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

    if (!SupportsCluster(endpoint, cluster))
        return CHIP_ERROR_INVALID_ARGUMENT;

    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    auto pair_iterator = attributeValueList.begin();
    while (pair_iterator.Next())
    {
        auto & decodePair = pair_iterator.GetValue();
        if (decodePair.attributeID == Attributes::CurrentLevel::Id)
        {
            if (decodePair.valueUnsigned8.HasValue())
            {
                uint8_t level = decodePair.valueUnsigned8.Value();
                DataModel::Nullable<uint16_t> ds;
                if (timeMs > 0)
                    ds.SetNonNull(static_cast<uint16_t>(timeMs / 100));

                // Use default options for scene?
                BitMask<OptionsBitmap> optionsMask;
                BitMask<OptionsBitmap> optionsOverride;

                MoveToLevelHandler(Commands::MoveToLevel::Id, level, ds, optionsMask, optionsOverride);
            }
        }
    }
    return pair_iterator.GetStatus();
}

DataModel::ActionReturnStatus LevelControlCluster::SetCurrentLevelQuietReport(DataModel::Nullable<uint8_t> newValue,
                                                                              bool isEndOfTransition)
{
    if (mCurrentLevel != newValue)
    {
        uint64_t nowMs    = System::SystemClock().GetMonotonicMilliseconds64().count();
        bool shouldReport = false;

        if (isEndOfTransition)
        {
            shouldReport = true;
        }
        else if (mCurrentLevel.IsNull() != newValue.IsNull())
        {
            shouldReport = true;
        }
        else if (nowMs - mLastReportTimeMs >= 1000)
        {
            shouldReport = true;
        }

        mCurrentLevel = newValue;

        if (shouldReport)
        {
            mLastReportTimeMs = nowMs;
            return NotifyAttributeChangedIfSuccess(Attributes::CurrentLevel::Id, Protocols::InteractionModel::Status::Success);
        }
    }
    return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
}

void LevelControlCluster::UpdateRemainingTime(uint32_t remainingTimeMs, bool isNewTransition)
{
    if (mFeatureMap.Has(Feature::kLighting))
    {
        // Convert ms to ds (rounding up)
        uint16_t remainingTimeDs = static_cast<uint16_t>((remainingTimeMs + 99) / 100);

        if (mRemainingTime.IsNull() || mRemainingTime.Value() != remainingTimeDs)
        {
            mRemainingTime.SetNonNull(remainingTimeDs);
            NotifyAttributeChanged(Attributes::RemainingTime::Id);
        }
    }
}

void LevelControlCluster::StartTimer(uint32_t delayMs)
{
    RETURN_SAFELY_IGNORED mTimerDelegate.StartTimer(this, System::Clock::Milliseconds64(delayMs));
}

void LevelControlCluster::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void LevelControlCluster::HandleTick()
{
    if (mCurrentLevel.IsNull())
        return;

    mElapsedTimeMs += mEventDurationMs;

    // RemainingTime update
    uint16_t remainingTimeMs = 0;
    if (mFeatureMap.Has(Feature::kLighting) && mTransitionTimeMs > 0 && mElapsedTimeMs < mTransitionTimeMs)
    {
        remainingTimeMs = static_cast<uint16_t>(mTransitionTimeMs - mElapsedTimeMs);
    }
    UpdateRemainingTime(remainingTimeMs, false);

    uint8_t currentLevel = mCurrentLevel.Value();

    if (mIncreasing)
    {
        if (currentLevel < mTargetLevel)
            currentLevel++;
    }
    else
    {
        if (currentLevel > mTargetLevel)
            currentLevel--;
    }

    bool isEndOfTransition = (currentLevel == mTargetLevel);

    mDelegate.OnLevelChanged(currentLevel);
    SetCurrentLevelQuietReport(DataModel::MakeNullable(currentLevel), isEndOfTransition);

    if (isEndOfTransition)
    {
        UpdateRemainingTime(0, false);
        if (mCurrentCommandId == Commands::MoveToLevelWithOnOff::Id || mCurrentCommandId == Commands::MoveWithOnOff::Id ||
            mCurrentCommandId == Commands::StepWithOnOff::Id)
        {
            if (currentLevel == mMinLevel || currentLevel == 0)
                UpdateOnOff(false);
        }
        return;
    }

    StartTimer(mEventDurationMs);
}

void LevelControlCluster::OnOffChanged(bool isOn)
{
    if (mCurrentLevel.IsNull())
        return;
    if (mTemporarilyIgnoreOnOffCallbacks)
        return;

    if (isOn)
    {
        // On Transition
        // 2. Determine Target Level (Capture before setting to Min)
        uint8_t target = 254; // Default Max
        if (!mOnLevel.IsNull())
            target = mOnLevel.Value();
        else if (!mStoredLevel.IsNull())
            target = mStoredLevel.Value();
        else
            target = mCurrentLevel.Value();

        // 1. Set to MinLevel
        SetCurrentLevel(mMinLevel);

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
        MoveToLevelHandler(Commands::MoveToLevelWithOnOff::Id, target, transitionTime, options, options);
    }
    else
    {
        // Off Transition
        // Store CurrentLevel
        mStoredLevel = mCurrentLevel;

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

        BitMask<OptionsBitmap> options;
        MoveToLevelHandler(Commands::MoveToLevelWithOnOff::Id, mMinLevel, transitionTime, options, options);
    }
}

} // namespace chip::app::Clusters
