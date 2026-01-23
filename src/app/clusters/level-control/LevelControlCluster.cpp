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

constexpr uint8_t kLightingMinLevel = 1;
constexpr uint8_t kMaxLevel         = 254;

} // namespace

LevelControlCluster::LevelControlCluster(const Config & config) :
    DefaultServerCluster({ config.mEndpointId, LevelControl::Id }), scenes::DefaultSceneHandlerImpl(GlobalLevelControlValidator()),
    mCurrentLevel(config.mInitialCurrentLevel), mOptions(BitMask<LevelControl::OptionsBitmap>(0)),
    mOnLevel(DataModel::Nullable<uint8_t>()),
    mMinLevel(config.mFeatureMap.Has(Feature::kLighting) ? kLightingMinLevel : config.mMinLevel),
    mMaxLevel(config.mFeatureMap.Has(Feature::kLighting) ? kMaxLevel : config.mMaxLevel), mDefaultMoveRate(config.mDefaultMoveRate),
    mStartUpCurrentLevel(config.mStartUpCurrentLevel), mRemainingTime(0), mLastReportedRemainingTime(0),
    mOnTransitionTime(config.mOnTransitionTime), mOffTransitionTime(config.mOffTransitionTime),
    mOnOffTransitionTime(config.mOnOffTransitionTime), mOptionalAttributes(config.mOptionalAttributes),
    mFeatureMap(config.mFeatureMap), mDelegate(config.mDelegate), mTimerDelegate(config.mTimerDelegate)
{}

LevelControlCluster::~LevelControlCluster()
{
    CancelTimer();
}

CHIP_ERROR LevelControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    attributePersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::OnLevel::Id),
                                               mOnLevel, mOnLevel);
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id), mCurrentLevel, mCurrentLevel);

    if (mFeatureMap.Has(Feature::kLighting))
    {
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::StartUpCurrentLevel::Id), mStartUpCurrentLevel,
            mStartUpCurrentLevel);

        if (!mStartUpCurrentLevel.IsNull())
        {
            // Apply StartUpCurrentLevel logic (0 -> Min, Null -> Ignore, Value -> Value)
            uint8_t target = mStartUpCurrentLevel.Value();
            if (mOptionalAttributes.IsSet(Attributes::MinLevel::Id) && target < mMinLevel)
            {
                target = mMinLevel;
            }
            if (mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) && target > mMaxLevel)
            {
                target = mMaxLevel;
            }

            // Use SetValue to update internal state without triggering a report or check
            mCurrentLevel.SetNonNull(target);
        }
    }

    if (!mCurrentLevel.IsNull())
    {
        mDelegate.OnLevelChanged(mCurrentLevel.Value());
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
        ReturnErrorOnFailure(SetOptions(options));
        return Status::Success;
    }
    case Attributes::OnLevel::Id: {
        DataModel::Nullable<uint8_t> onLevel;
        ReturnErrorOnFailure(decoder.Decode(onLevel));
        ReturnErrorOnFailure(SetOnLevel(onLevel));
        return Status::Success;
    }
    case Attributes::DefaultMoveRate::Id: {
        DataModel::Nullable<uint8_t> rate;
        ReturnErrorOnFailure(decoder.Decode(rate));
        ReturnErrorOnFailure(SetDefaultMoveRate(rate));
        return Status::Success;
    }
    case Attributes::StartUpCurrentLevel::Id: {
        DataModel::Nullable<uint8_t> startup;
        ReturnErrorOnFailure(decoder.Decode(startup));
        ReturnErrorOnFailure(SetStartUpCurrentLevel(startup));
        return Status::Success;
    }
    case Attributes::OnTransitionTime::Id: {
        DataModel::Nullable<uint16_t> onTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onTransitionTime));
        ReturnErrorOnFailure(SetOnTransitionTime(onTransitionTime));
        return Status::Success;
    }
    case Attributes::OffTransitionTime::Id: {
        DataModel::Nullable<uint16_t> offTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(offTransitionTime));
        ReturnErrorOnFailure(SetOffTransitionTime(offTransitionTime));
        return Status::Success;
    }
    case Attributes::OnOffTransitionTime::Id: {
        uint16_t onOffTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onOffTransitionTime));
        ReturnErrorOnFailure(SetOnOffTransitionTime(onOffTransitionTime));
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
        return MoveToLevelCommand(request.path.mCommandId, data.level, data.transitionTime, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveToLevelWithOnOff::Id: {
        Commands::MoveToLevelWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return MoveToLevelWithOnOffCommand(request.path.mCommandId, data.level, data.transitionTime, data.optionsMask,
                                           data.optionsOverride);
    }
    case Commands::Move::Id: {
        Commands::Move::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return MoveCommand(request.path.mCommandId, data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::MoveWithOnOff::Id: {
        Commands::MoveWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return MoveWithOnOffCommand(request.path.mCommandId, data.moveMode, data.rate, data.optionsMask, data.optionsOverride);
    }
    case Commands::Step::Id: {
        Commands::Step::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return StepCommand(request.path.mCommandId, data.stepMode, data.stepSize, data.transitionTime, data.optionsMask,
                           data.optionsOverride);
    }
    case Commands::StepWithOnOff::Id: {
        Commands::StepWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return StepWithOnOffCommand(request.path.mCommandId, data.stepMode, data.stepSize, data.transitionTime, data.optionsMask,
                                    data.optionsOverride);
    }
    case Commands::Stop::Id: {
        Commands::Stop::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return StopCommand(request.path.mCommandId, data.optionsMask, data.optionsOverride);
    }
    case Commands::StopWithOnOff::Id: {
        Commands::StopWithOnOff::DecodableType data;
        VerifyOrReturnError(DataModel::Decode(input_arguments, data) == CHIP_NO_ERROR, Status::InvalidCommand);
        return StopCommand(request.path.mCommandId, data.optionsMask, data.optionsOverride);
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
    // Check if command execution is allowed by On/Off state (ExecuteIfOff bit) or ignore
    VerifyOrReturnError(ShouldExecuteIfOff(commandId, optionsMask, optionsOverride), Status::Success);

    CancelTimer(); // Cancel any currently active transition before starting a new one.

    // Validate level against Min/Max constraints
    if ((level > kMaxLevel) || (mOptionalAttributes.IsSet(Attributes::MinLevel::Id) && level < mMinLevel) ||
        (mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) && level > mMaxLevel))
    {
        return Status::ConstraintError;
    }

    VerifyOrReturnError(!mCurrentLevel.IsNull(), Status::Failure);

    uint8_t currentLevel = mCurrentLevel.Value();
    mTargetLevel         = level;
    mCurrentCommandId    = commandId;

    // Calculate Transition Time in Milliseconds
    // Priority: Command argument > OnOffTransitionTime > 0 (Immediate)
    const uint32_t transitionTimeMs = transitionTimeDS.ValueOr(mOnOffTransitionTime) * 100;

    // Refresh CurrentLevel (might have changed due to OnOff logic) and set Direction
    currentLevel = mCurrentLevel.ValueOr(currentLevel);
    mIncreasing  = (mTargetLevel > currentLevel);

    // Calculate duration per step
    uint8_t totalSteps = (mIncreasing) ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel);
    if (totalSteps > 0)
    {
        mEventDurationMs = transitionTimeMs / totalSteps;
    }
    else
    {
        mEventDurationMs = 0;
    }

    // Immediate move
    if (transitionTimeMs == 0 || totalSteps == 0 || mEventDurationMs == 0)
    {
        return SetCurrentLevel(mTargetLevel);
    }

    ScheduleTimer(mEventDurationMs, transitionTimeMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::MoveToLevelWithOnOffCommand(CommandId commandId, uint8_t level,
                                                                               DataModel::Nullable<uint16_t> transitionTimeDS,
                                                                               BitMask<OptionsBitmap> optionsMask,
                                                                               BitMask<OptionsBitmap> optionsOverride)
{
    EnsureOn();

    auto status = MoveToLevelCommand(commandId, level, transitionTimeDS, optionsMask, optionsOverride);
    VerifyOrReturnValue(status.IsSuccess(), status);
    VerifyOrReturnError(mTargetLevel == mMinLevel, Status::Success);

    if ((!transitionTimeDS.IsNull() && transitionTimeDS.Value() == 0) || (transitionTimeDS.IsNull() && mOnOffTransitionTime == 0))
    {
        UpdateOnOff(false, true);
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus LevelControlCluster::MoveCommand(CommandId commandId, MoveModeEnum moveMode,
                                                               DataModel::Nullable<uint8_t> rate,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    // Check if command execution is allowed by On/Off state (ExecuteIfOff bit) or ignore
    VerifyOrReturnError(ShouldExecuteIfOff(commandId, optionsMask, optionsOverride), Status::Success);
    if (!rate.IsNull() && rate.Value() == 0)
    {
        return Status::InvalidCommand;
    }
    VerifyOrReturnError(!mCurrentLevel.IsNull(), Status::Failure);

    mCurrentCommandId = commandId;
    CancelTimer();

    // Determine Rate (units/s)
    uint8_t currentRate = 0;
    if (!rate.IsNull())
    {
        currentRate = rate.Value();
    }
    else if (!mDefaultMoveRate.IsNull())
    {
        currentRate = mDefaultMoveRate.Value();
    }

    VerifyOrReturnError(currentRate != 0, Status::Success); // No movement if rate is unspecified

    // Determine Direction first
    mIncreasing = moveMode == MoveModeEnum::kUp;

    // Now determine Target and Check Constraints (safe from clobbering)
    if (mIncreasing)
    {
        mTargetLevel = mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) ? mMaxLevel : kMaxLevel;
        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.Value();
        VerifyOrReturnError(currentLevel < mTargetLevel, Status::Success);
    }
    else
    {
        mTargetLevel = mOptionalAttributes.IsSet(Attributes::MinLevel::Id) ? mMinLevel : 0;
        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.Value();
        VerifyOrReturnError(currentLevel > mTargetLevel, Status::Success);
    }

    // Estimate total transition time for RemainingTime reporting (though Move is indefinite until stop/limit)
    uint8_t currentLevel = mCurrentLevel.Value();
    uint8_t difference   = 0;
    if (mIncreasing)
        difference = mTargetLevel - currentLevel;
    else
        difference = currentLevel - mTargetLevel;

    mEventDurationMs = 1000 / currentRate;
    ScheduleTimer(mEventDurationMs, difference * mEventDurationMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::MoveWithOnOffCommand(CommandId commandId, MoveModeEnum moveMode,
                                                                        DataModel::Nullable<uint8_t> rate,
                                                                        BitMask<OptionsBitmap> optionsMask,
                                                                        BitMask<OptionsBitmap> optionsOverride)
{
    if (moveMode == MoveModeEnum::kUp)
    {
        EnsureOn();
    }
    return MoveCommand(commandId, moveMode, rate, optionsMask, optionsOverride);
}

DataModel::ActionReturnStatus LevelControlCluster::StepCommand(CommandId commandId, StepModeEnum stepMode, uint8_t stepSize,
                                                               DataModel::Nullable<uint16_t> transitionTime,
                                                               BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    // Check if command execution is allowed by On/Off state (ExecuteIfOff bit) or ignore
    VerifyOrReturnError(ShouldExecuteIfOff(commandId, optionsMask, optionsOverride), Status::Success);
    VerifyOrReturnError(stepSize > 0, Status::InvalidCommand);
    VerifyOrReturnError(!mCurrentLevel.IsNull(), Status::Failure);

    mCurrentCommandId = commandId;
    CancelTimer();

    uint32_t transitionTimeMs = 0;
    if (!transitionTime.IsNull())
    {
        transitionTimeMs = transitionTime.Value() * 100;
    }

    // Determine Direction first
    mIncreasing = stepMode == StepModeEnum::kUp ? true : false;

    // Recalculate everything based on potential new current level
    uint8_t currentLevel = mCurrentLevel.Value();

    // Calculate Target based on Step Size
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

    if (transitionTimeMs == 0)
    {
        return SetCurrentLevel(mTargetLevel);
    }

    uint8_t totalSteps = (mIncreasing) ? (mTargetLevel - currentLevel) : (currentLevel - mTargetLevel);
    if (totalSteps == 0)
    {
        return Status::Success;
    }

    mEventDurationMs = transitionTimeMs / totalSteps;
    ScheduleTimer(mEventDurationMs, transitionTimeMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::StepWithOnOffCommand(CommandId commandId, StepModeEnum stepMode,
                                                                        uint8_t stepSize,
                                                                        DataModel::Nullable<uint16_t> transitionTime,
                                                                        BitMask<OptionsBitmap> optionsMask,
                                                                        BitMask<OptionsBitmap> optionsOverride)
{
    if (stepMode == StepModeEnum::kUp)
    {
        EnsureOn();
    }

    auto status = StepCommand(commandId, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
    VerifyOrReturnValue(status.IsSuccess(), status);
    VerifyOrReturnError(mTargetLevel == mMinLevel, Status::Success);

    if (!transitionTime.IsNull() && transitionTime.Value() == 0)
    {
        UpdateOnOff(false);
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus LevelControlCluster::StopCommand(CommandId commandId, BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    // Check if command execution is allowed by On/Off state (ExecuteIfOff bit) or ignore
    VerifyOrReturnError(ShouldExecuteIfOff(commandId, optionsMask, optionsOverride), Status::Success);

    CancelTimer();
    // Force report of current level and reset remaining time
    // Ignore error as we are stopping and best-effort reporting is acceptable.
    RETURN_SAFELY_IGNORED SetCurrentLevelQuietReport(mCurrentLevel, true /*isEndOfTransition*/);
    UpdateRemainingTime(0, false);
    return Status::Success;
}

CHIP_ERROR LevelControlCluster::SetOptions(BitMask<OptionsBitmap> newOptions)
{
    VerifyOrReturnError(mOptions != newOptions, CHIP_NO_ERROR);
    mOptions = newOptions;
    mDelegate.OnOptionsChanged(mOptions);
    NotifyAttributeChanged(Attributes::Options::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetOnLevel(DataModel::Nullable<uint8_t> newOnLevel)
{
    VerifyOrReturnError(mOnLevel != newOnLevel, CHIP_NO_ERROR);
    mOnLevel = newOnLevel;
    mDelegate.OnOnLevelChanged(mOnLevel);
    NotifyAttributeChanged(Attributes::OnLevel::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetDefaultMoveRate(DataModel::Nullable<uint8_t> newDefaultMoveRate)
{
    // Validate constraint: Min 1
    if (!newDefaultMoveRate.IsNull() && newDefaultMoveRate.Value() < 1)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    VerifyOrReturnError(mDefaultMoveRate != newDefaultMoveRate, CHIP_NO_ERROR);
    mDefaultMoveRate = newDefaultMoveRate;
    mDelegate.OnDefaultMoveRateChanged(mDefaultMoveRate);
    NotifyAttributeChanged(Attributes::DefaultMoveRate::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetCurrentLevel(uint8_t level)
{
    // Validate constraints against Min/Max
    if ((mOptionalAttributes.IsSet(Attributes::MinLevel::Id) && level < mMinLevel) || level > kMaxLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }
    if ((mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) && level > mMaxLevel) || level < mMinLevel)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (mCurrentLevel.IsNull() || mCurrentLevel.Value() != level)
    {
        mDelegate.OnLevelChanged(level);
        return SetCurrentLevelQuietReport(DataModel::MakeNullable(level), true);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetStartUpCurrentLevel(DataModel::Nullable<uint8_t> startupLevel)
{
    VerifyOrReturnError(mStartUpCurrentLevel != startupLevel, CHIP_NO_ERROR);
    mStartUpCurrentLevel = startupLevel;
    NotifyAttributeChanged(Attributes::StartUpCurrentLevel::Id);

    VerifyOrReturnError(mContext != nullptr, CHIP_NO_ERROR);

    NumericAttributeTraits<uint8_t>::StorageType storageValue;
    DataModel::NullableToStorage(startupLevel, storageValue);
    return mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::StartUpCurrentLevel::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue)));
}

CHIP_ERROR LevelControlCluster::SetOnTransitionTime(DataModel::Nullable<uint16_t> onTransitionTime)
{
    VerifyOrReturnError(mOnTransitionTime != onTransitionTime, CHIP_NO_ERROR);
    mOnTransitionTime = onTransitionTime;
    NotifyAttributeChanged(Attributes::OnTransitionTime::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetOffTransitionTime(DataModel::Nullable<uint16_t> offTransitionTime)
{
    VerifyOrReturnError(mOffTransitionTime != offTransitionTime, CHIP_NO_ERROR);
    mOffTransitionTime = offTransitionTime;
    NotifyAttributeChanged(Attributes::OffTransitionTime::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlCluster::SetOnOffTransitionTime(uint16_t onOffTransitionTime)
{
    VerifyOrReturnError(mOnOffTransitionTime != onOffTransitionTime, CHIP_NO_ERROR);
    mOnOffTransitionTime = onOffTransitionTime;
    NotifyAttributeChanged(Attributes::OnOffTransitionTime::Id);
    return CHIP_NO_ERROR;
}

void LevelControlCluster::UpdateOnOff(bool on, bool temporarilyIgnoreOnOffCallbacks)
{
    ReturnOnFailure(!mFeatureMap.Has(Feature::kOnOff));
    // Prevent potential callback loops
    mTemporarilyIgnoreOnOffCallbacks = temporarilyIgnoreOnOffCallbacks;
    mDelegate.SetOnOff(on);
    mTemporarilyIgnoreOnOffCallbacks = false;
}

bool LevelControlCluster::ShouldExecuteIfOff(CommandId commandId, BitMask<OptionsBitmap> optionsMask,
                                             BitMask<OptionsBitmap> optionsOverride)
{
    VerifyOrReturnValue(mFeatureMap.Has(Feature::kOnOff), true);

    // If it's ON or it's one of the *WithOnOffCommands
    if (mDelegate.GetOnOff() || commandId == Commands::MoveToLevelWithOnOff::Id || commandId == Commands::MoveWithOnOff::Id ||
        commandId == Commands::StepWithOnOff::Id || commandId == Commands::StopWithOnOff::Id)
    {
        return true;
    }

    // Check for ExecuteIfOff bit in Options attribute or command overrides
    bool executeIfOff = mOptions.Has(OptionsBitmap::kExecuteIfOff);
    if (optionsMask.Has(OptionsBitmap::kExecuteIfOff))
    {
        executeIfOff = optionsOverride.Has(OptionsBitmap::kExecuteIfOff) ? true : false;
    }

    return executeIfOff;
}

bool LevelControlCluster::SupportsCluster(EndpointId endpoint, ClusterId cluster)
{
    return (cluster == LevelControl::Id) && (endpoint == mPath.mEndpointId);
}

CHIP_ERROR LevelControlCluster::SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    if (!SupportsCluster(endpoint, cluster))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

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
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

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

CHIP_ERROR LevelControlCluster::SetCurrentLevelQuietReport(DataModel::Nullable<uint8_t> newValue, bool isEndOfTransition)
{
    VerifyOrReturnError(mCurrentLevel != newValue, CHIP_NO_ERROR);

    uint64_t nowMs    = System::SystemClock().GetMonotonicMilliseconds64().count();
    bool shouldReport = false;

    if (isEndOfTransition || (mCurrentLevel.IsNull() != newValue.IsNull()) || (nowMs - mLastReportTimeMs >= 1000))
    {
        shouldReport = true;
    }

    mCurrentLevel = newValue;

    // Store if changed (N attribute)
    if (mContext)
    {
        NumericAttributeTraits<uint8_t>::StorageType storageValue;
        DataModel::NullableToStorage(mCurrentLevel, storageValue);
        // Ignore write errors, as we can't really do anything about them here and
        // we've already updated the in-memory state.
        (void) mContext->attributeStorage.WriteValue(
            ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id),
            ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue)));
    }

    if (shouldReport)
    {
        mLastReportTimeMs = nowMs;
        NotifyAttributeChanged(Attributes::CurrentLevel::Id);
    }
    return CHIP_NO_ERROR;
}

void LevelControlCluster::UpdateRemainingTime(uint32_t remainingTimeMs, bool isNewTransition)
{
    VerifyOrReturn(mFeatureMap.Has(Feature::kLighting));

    // Convert ms to ds (rounding up)
    uint16_t remainingTimeDs = static_cast<uint16_t>((remainingTimeMs + 99) / 100);
    mRemainingTime           = remainingTimeDs;

    bool shouldReport = false;

    // "For commands with a transition time or changes to the transition time less than 1 second,
    // changes to this attribute SHALL NOT be reported."
    if (mTransitionTimeMs >= 1000)
    {
        // Case 1: Changes from 0 to any value higher than 10
        if (mLastReportedRemainingTime == 0 && remainingTimeDs > 10)
        {
            shouldReport = true;
        }
        // Case 2: Changes with a delta larger than 10, caused by the invoke of a command
        else if (isNewTransition)
        {
            if (remainingTimeDs > mLastReportedRemainingTime + 10 || mLastReportedRemainingTime > remainingTimeDs + 10)
            {
                shouldReport = true;
            }
        }
        // Case 3: Changes to 0
        else if (remainingTimeDs == 0 && mLastReportedRemainingTime != 0)
        {
            shouldReport = true;
        }
    }

    if (shouldReport)
    {
        mLastReportedRemainingTime = remainingTimeDs;
        NotifyAttributeChanged(Attributes::RemainingTime::Id);
    }
}

void LevelControlCluster::StartTimer(uint32_t delayMs)
{
    VerifyOrDie(mTimerDelegate.StartTimer(this, System::Clock::Milliseconds64(delayMs)) == CHIP_NO_ERROR);
}

void LevelControlCluster::CancelTimer()
{
    mTimerDelegate.CancelTimer(this);
}

void LevelControlCluster::TimerFired()
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
    // Ignore error as this is a background tick.
    RETURN_SAFELY_IGNORED SetCurrentLevelQuietReport(DataModel::MakeNullable(currentLevel), isEndOfTransition);

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
        uint8_t target = kMaxLevel; // Default Max
        if (!mOnLevel.IsNull())
            target = mOnLevel.Value();
        else if (!mStoredLevel.IsNull())
            target = mStoredLevel.Value();

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
        MoveToLevelWithOnOffCommand(Commands::MoveToLevelWithOnOff::Id, target, transitionTime, options, options);
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

        // Force execution to allow fading out even if device is technically "Off"
        BitMask<OptionsBitmap> optionsMask(OptionsBitmap::kExecuteIfOff);
        BitMask<OptionsBitmap> optionsOverride(OptionsBitmap::kExecuteIfOff);
        MoveToLevelCommand(Commands::MoveToLevelWithOnOff::Id, mMinLevel, transitionTime, optionsMask, optionsOverride);
    }
}

void LevelControlCluster::EnsureOn()
{
    if (mDelegate.GetOnOff())
        return;

    UpdateOnOff(true, true /* temporarilyIgnoreOnOffCallbacks */);
    if (!mCurrentLevel.IsNull() && mCurrentLevel.Value() < mMinLevel)
    {
        // Ignore error as we are fixing up an invalid state (Current < Min) by forcing to Min.
        RETURN_SAFELY_IGNORED SetCurrentLevel(mMinLevel);
    }
}

void LevelControlCluster::ScheduleTimer(uint32_t durationMs, uint32_t transitionTimeMs)
{
    mTransitionTimeMs = transitionTimeMs;
    mEventDurationMs  = durationMs;
    UpdateRemainingTime(mTransitionTimeMs, true);
    StartTimer(mEventDurationMs);
}

} // namespace chip::app::Clusters
