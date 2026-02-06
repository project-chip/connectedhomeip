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
#include <app/clusters/on-off-server/OnOffCluster.h>
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

constexpr bool IsWithOnOffCommand(CommandId commandId)
{
    return commandId == Commands::MoveToLevelWithOnOff::Id //
        || commandId == Commands::MoveWithOnOff::Id        //
        || commandId == Commands::StepWithOnOff::Id        //
        || commandId == Commands::StopWithOnOff::Id        //
        ;
}

constexpr CommandId kInternalOffTransition = 0xFFFFFFFF; // Sentinel value to identify internal fade-to-off transitions

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
    mFeatureMap(config.mFeatureMap), mDelegate(config.mDelegate), mTimerDelegate(config.mTimerDelegate),
    mOnOffCluster(config.mOnOffCluster), mTransitionHandler(*this)
{
    VerifyOrDie(!mFeatureMap.Has(Feature::kOnOff) || mOnOffCluster != nullptr);
}

void LevelControlCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mTransitionHandler.StopTransition();
    DefaultServerCluster::Shutdown(shutdownType);
}

LevelControlCluster::TransitionHandler::~TransitionHandler()
{
    mCluster.mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR LevelControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);

    // 1. Determine the initial value for CurrentLevel
    // Start with the default/reset value (set in constructor)
    DataModel::Nullable<uint8_t> currentLevel = mCurrentLevel.value();

    // Try to load from persistence. If not found, it keeps 'currentLevel' (which is the default).
    attributePersistence.LoadNativeEndianValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id), currentLevel, currentLevel);

    // 2. Handle Lighting logic (StartUpCurrentLevel overrides persistence)
    if (mFeatureMap.Has(Feature::kLighting))
    {
        attributePersistence.LoadNativeEndianValue(
            ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::StartUpCurrentLevel::Id), mStartUpCurrentLevel,
            mStartUpCurrentLevel);

        if (!mStartUpCurrentLevel.IsNull())
        {
            currentLevel = mStartUpCurrentLevel;
        }
    }

    // 3. Validation and Clamping
    // Ensure CurrentLevel is within Min/Max bounds.
    // This handles cases where StartUpCurrentLevel is null, but the restored/initial CurrentLevel
    // is outside the valid range (e.g. initial=0 with Lighting feature min=1).
    if (!currentLevel.IsNull())
    {
        currentLevel.SetNonNull(std::clamp<uint8_t>(currentLevel.Value(), mMinLevel, mMaxLevel));
    }

    // 4. Commit to Attribute and Delegate (Single SetValue call)
    mCurrentLevel.SetValue(currentLevel, System::SystemClock().GetMonotonicMilliseconds64());

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
        SetOptions(options);
        return Status::Success;
    }
    case Attributes::OnLevel::Id: {
        DataModel::Nullable<uint8_t> onLevel;
        ReturnErrorOnFailure(decoder.Decode(onLevel));
        SetOnLevel(onLevel);
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
        SetOnTransitionTime(onTransitionTime);
        return Status::Success;
    }
    case Attributes::OffTransitionTime::Id: {
        DataModel::Nullable<uint16_t> offTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(offTransitionTime));
        SetOffTransitionTime(offTransitionTime);
        return Status::Success;
    }
    case Attributes::OnOffTransitionTime::Id: {
        uint16_t onOffTransitionTime;
        ReturnErrorOnFailure(decoder.Decode(onOffTransitionTime));
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
        // Spec bug: https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/12613
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
    VerifyOrReturnError(IsValidLevel(level), Status::ConstraintError);

    if (IsWithOnOffCommand(commandId))
    {
        ReturnErrorOnFailure(SetOnOff(true));
    }
    else if (!ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    mTransitionHandler.StopTransition(); // Cancel any currently active transition before starting a new one.

    if (mCurrentLevel.value().IsNull())
    {
        // If the current level is undefined (null), we cannot calculate a transition duration
        // because we don't know the starting point. The spec says "move from its current level".
        // In this case, we treat it as an immediate transition to the target level.
        CHIP_ERROR status = SetCurrentLevel(level, ReportingMode::kForceReport);
        if (status == CHIP_NO_ERROR && IsWithOnOffCommand(commandId) && level == mMinLevel)
        {
            ReturnErrorOnFailure(SetOnOff(false));
        }
        return status;
    }

    uint8_t currentLevel = mCurrentLevel.value().Value();
    uint8_t targetLevel  = level;

    // Calculate Transition Time in Milliseconds
    // Priority: Command argument > OnOffTransitionTime > 0 (Immediate)
    const uint32_t transitionTimeMs = transitionTimeDS.ValueOr(mOnOffTransitionTime) * 100;

    // Refresh CurrentLevel (might have changed due to OnOff logic) and set Direction
    currentLevel = mCurrentLevel.value().ValueOr(currentLevel);

    // Calculate duration per step
    auto totalSteps         = static_cast<uint8_t>(std::abs(targetLevel - currentLevel));
    uint32_t tickDurationMs = (totalSteps > 0) ? (transitionTimeMs / totalSteps) : 0;

    if (tickDurationMs > 0)
    {
        // We are doing a timed transition, start it.
        mTransitionHandler.StartTransition(commandId, currentLevel, targetLevel, transitionTimeMs, tickDurationMs);
        return Status::Success;
    }

    // Immediate move
    ReturnErrorOnFailure(SetCurrentLevel(targetLevel, ReportingMode::kForceReport));

    if ((IsWithOnOffCommand(commandId) || commandId == kInternalOffTransition) && targetLevel == mMinLevel)
    {
        ReturnErrorOnFailure(SetOnOff(false));
    }

    if (commandId == kInternalOffTransition && targetLevel == mMinLevel)
    {
        // This was an internal fade-to-off. Restoring the previous level ensures that the next
        // "On" command (which might not specify a level) restores the brightness the user expects.
        if (mOnLevel.IsNull() && !mLevelBeforeTurnedOff.IsNull())
        {
            ReturnErrorOnFailure(SetCurrentLevel(mLevelBeforeTurnedOff.Value(), ReportingMode::kForceReport));
        }
    }
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

    // If rate is null, use default move rate (one of the two is guaranteed to be non-null here because of the earlier check)
    uint8_t currentRate = !rate.IsNull() ? rate.Value() : mDefaultMoveRate.Value();
    VerifyOrReturnError(currentRate != 0, Status::ConstraintError);

    if (IsWithOnOffCommand(commandId) && moveMode == MoveModeEnum::kUp)
    {
        ReturnErrorOnFailure(SetOnOff(true));
    }
    else if (!ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    mTransitionHandler.StopTransition(); // Cancel any currently active transition before starting a new one.

    // Determine Direction first
    bool increasing = (moveMode == MoveModeEnum::kUp);
    uint8_t targetLevel;

    // Now determine Target and Check Constraints (safe from clobbering)
    if (increasing)
    {
        targetLevel = mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) ? mMaxLevel : kMaxLevel;
        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.value().Value();
        VerifyOrReturnError(currentLevel < targetLevel, Status::Success);
    }
    else
    {
        targetLevel = mOptionalAttributes.IsSet(Attributes::MinLevel::Id) ? mMinLevel : 0;
        // Check if already at target
        uint8_t currentLevel = mCurrentLevel.value().Value();
        VerifyOrReturnError(currentLevel > targetLevel, Status::Success);
    }

    // Estimate total transition time for RemainingTime reporting (though Move is indefinite until stop/limit)
    uint8_t currentLevel = mCurrentLevel.value().Value();
    uint8_t difference   = static_cast<uint8_t>(std::abs(targetLevel - currentLevel));

    // currentRate is known not to be 0 (ConstraintError check above)
    uint32_t tickDurationMs = 1000 / currentRate;
    if (tickDurationMs == 0)
    {
        tickDurationMs = 1;
    }

    mTransitionHandler.StartTransition(commandId, currentLevel, targetLevel, difference * tickDurationMs, tickDurationMs);
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
        ReturnErrorOnFailure(SetOnOff(true));
    }
    // Spec: "Command execution SHALL NOT continue beyond the Options processing if...
    // The OnOff attribute... is FALSE." (Unless ExecuteIfOff is set)
    // Note: We use !IsWithOnOffCommand because WithOnOff commands are exempt from this check.
    else if (!IsWithOnOffCommand(commandId) && !ShouldExecuteIfOff(optionsMask, optionsOverride))
    {
        return Status::Success;
    }

    mTransitionHandler.StopTransition();

    bool increasing      = (stepMode == StepModeEnum::kUp);
    uint8_t currentLevel = mCurrentLevel.value().Value();
    uint8_t targetLevel;

    // Spec: "Up: Increase CurrentLevel by StepSize units, or until it reaches the maximum level..."
    // Spec: "Down: Decrease CurrentLevel by StepSize units, or until it reaches the minimum level..."
    if (increasing)
    {
        int max     = mOptionalAttributes.IsSet(Attributes::MaxLevel::Id) ? mMaxLevel : kMaxLevel;
        targetLevel = static_cast<uint8_t>(std::min(currentLevel + stepSize, max));
    }
    else
    {
        int min     = mOptionalAttributes.IsSet(Attributes::MinLevel::Id) ? mMinLevel : 0;
        targetLevel = static_cast<uint8_t>(std::max(currentLevel - stepSize, min));
    }

    // Calculate effective step duration
    uint8_t totalSteps = (increasing ? (targetLevel - currentLevel) : (currentLevel - targetLevel));

    // Spec: "If the TransitionTime field is equal to null, the device SHOULD move as fast as it is able."
    uint32_t transitionTimeMs = transitionTime.ValueOr(0) * 100;

    // Check if immediate transition is needed (0 time or 0 duration calculated)
    if (transitionTimeMs == 0 || totalSteps == 0 || (transitionTimeMs / totalSteps) == 0)
    {
        CHIP_ERROR status = SetCurrentLevel(targetLevel, ReportingMode::kForceReport);

        // Spec: "If any command that has the effect of setting the CurrentLevel attribute to the minimum level...
        // the OnOff attribute... SHALL be set to FALSE"
        if (status == CHIP_NO_ERROR && IsWithOnOffCommand(commandId) && targetLevel == mMinLevel)
        {
            ReturnErrorOnFailure(SetOnOff(false));
        }
        return status;
    }

    uint32_t tickDurationMs = transitionTimeMs / totalSteps;
    mTransitionHandler.StartTransition(commandId, currentLevel, targetLevel, transitionTimeMs, tickDurationMs);
    return Status::Success;
}

DataModel::ActionReturnStatus LevelControlCluster::StopCommand(CommandId commandId, BitMask<OptionsBitmap> optionsMask,
                                                               BitMask<OptionsBitmap> optionsOverride)
{
    // Spec (Options Attribute): "Command execution SHALL NOT continue beyond the Options processing if...
    // The command is one of the ‘without On/Off’ commands: ... Stop."
    VerifyOrReturnValue(ShouldExecuteIfOff(optionsMask, optionsOverride), Status::Success);
    mTransitionHandler.StopTransition();
    UpdateRemainingTime(0, ReportingMode::kForceReport);
    // mCurrentLevel is guaranteed to have a value here.
    // - If we were transitioning, it had a value.
    // - If we weren't transitioning, it maintains its last state.
    // - Startup ensures it has a value (either from NVM or defaults).
    return SetCurrentLevel(mCurrentLevel.value().Value(), ReportingMode::kForceReport);
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

CHIP_ERROR LevelControlCluster::SetCurrentLevel(uint8_t level, ReportingMode reportingMode)
{
    VerifyOrReturnError(IsValidLevel(level), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    VerifyOrReturnError(mCurrentLevel.value().IsNull() || mCurrentLevel.value().Value() != level, CHIP_NO_ERROR); // No change

    auto now = System::SystemClock().GetMonotonicMilliseconds64();
    AttributeDirtyState dirtyState;

    if (reportingMode == ReportingMode::kForceReport)
    {
        dirtyState = mCurrentLevel.SetValue(DataModel::MakeNullable(level), now, [](const auto &) { return true; });
    }
    else
    {
        dirtyState = mCurrentLevel.SetValue(DataModel::MakeNullable(level), now,
                                            QuieterReportingAttribute<uint8_t>::GetPredicateForSufficientTimeSinceLastDirty(
                                                chip::System::Clock::Milliseconds64(1000)));
    }

    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        NotifyAttributeChanged(Attributes::CurrentLevel::Id);
    }
    StoreCurrentLevel(mCurrentLevel.value());
    mDelegate.OnLevelChanged(level);

    return CHIP_NO_ERROR;
}

void LevelControlCluster::StoreCurrentLevel(DataModel::Nullable<uint8_t> value)
{
    VerifyOrReturn(mContext != nullptr);

    NumericAttributeTraits<uint8_t>::StorageType storageValue;
    DataModel::NullableToStorage(value, storageValue);

    LogErrorOnFailure(mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id),
        ByteSpan(reinterpret_cast<const uint8_t *>(&storageValue), sizeof(storageValue))));
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

CHIP_ERROR LevelControlCluster::SetOnOff(bool on)
{
    VerifyOrReturnError(mFeatureMap.Has(Feature::kOnOff), CHIP_NO_ERROR);
    VerifyOrReturnError(on != GetOnOff(), CHIP_NO_ERROR);

    // Prevent potential callback loops
    mTemporarilyIgnoreOnOffCallbacks = true;
    CHIP_ERROR err                   = mOnOffCluster->SetOnOff(on);
    mTemporarilyIgnoreOnOffCallbacks = false;
    return err;
}

bool LevelControlCluster::GetOnOff()
{
    VerifyOrReturnError(mFeatureMap.Has(Feature::kOnOff), false);
    return mOnOffCluster->GetOnOff();
}

void LevelControlCluster::OnOffStartup(bool on)
{
    // Per spec, On/Off and Level Control are intrinsically independent variables.
    // Each cluster handles its own initialization via StartUpOnOff and
    // StartUpCurrentLevel attributes. Coupling logic (e.g., OnLevel) is
    // strictly command-based and does not apply to the initial power-up state.
    // The application may implement custom logic if desired and use the cluster's public API
    // to set state after boot.
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
            if (mode == ReportingMode::kQuietReport)
            {
                return candidate.newValue.ValueOr(0) == 0 && candidate.lastDirtyValue.ValueOr(0) != 0;
            }

            // Transitions shorter than 1 second (10ds) will never satisfy the "higher than 10" requirement for the initial report,
            // so we filter them out early to avoid unnecessary processing.
            VerifyOrReturnValue(mTransitionHandler.GetTransitionTimeMs() >= 1000, false);

            auto lastDirty = candidate.lastDirtyValue.ValueOr(0);
            auto newValue  = candidate.newValue.ValueOr(0);

            return ((newValue == 0 && lastDirty != 0) || (newValue > lastDirty ? newValue - lastDirty : lastDirty - newValue) > 10);
        }) == AttributeDirtyState::kMustReport)
    {
        NotifyAttributeChanged(Attributes::RemainingTime::Id);
    }
}

void LevelControlCluster::TransitionHandler::StartTransition(CommandId commandId, uint8_t initialLevel, uint8_t targetLevel,
                                                             uint32_t transitionTimeMs, uint32_t stepDurationMs)
{
    mCurrentCommandId = commandId;
    mInitialLevel     = initialLevel;
    mTargetLevel      = targetLevel;
    mTransitionTimeMs = transitionTimeMs;
    mTickDurationMs   = stepDurationMs;
    mElapsedTimeMs    = 0;

    mTransitionStartTimeMs = System::SystemClock().GetMonotonicMilliseconds64().count();

    // Command invoked: Set RemainingTime using full reporting rules
    mCluster.UpdateRemainingTime(mTransitionTimeMs, LevelControlCluster::ReportingMode::kForceReport);
    SuccessOrDie(mCluster.mTimerDelegate.StartTimer(this, System::Clock::Milliseconds64(mTickDurationMs)));
}

void LevelControlCluster::TransitionHandler::StopTransition()
{
    mCluster.mTimerDelegate.CancelTimer(this);
    mCluster.UpdateRemainingTime(0, LevelControlCluster::ReportingMode::kForceReport);
}

void LevelControlCluster::TransitionHandler::TimerFired()
{
    VerifyOrReturn(!mCluster.mCurrentLevel.value().IsNull());

    uint64_t now = System::SystemClock().GetMonotonicMilliseconds64().count();
    // Check for monotonic clock rollover or backward jump
    if (now < mTransitionStartTimeMs)
    {
        mTransitionStartTimeMs = now; // Default to restart reference
        uint32_t remainingMs   = static_cast<uint32_t>(mCluster.GetRemainingTime()) * 100;
        // Try to recover start time based on remaining time if available
        if (mCluster.mFeatureMap.Has(Feature::kLighting) && (remainingMs < mTransitionTimeMs))
        {
            mTransitionStartTimeMs -= (mTransitionTimeMs - remainingMs);
        }
    }

    uint64_t elapsed = now - mTransitionStartTimeMs;
    mElapsedTimeMs   = static_cast<uint32_t>(elapsed);

    // RemainingTime update
    uint16_t remainingTimeMs = 0;
    if (mCluster.mFeatureMap.Has(Feature::kLighting) && mTransitionTimeMs > 0 && mElapsedTimeMs < mTransitionTimeMs)
    {
        remainingTimeMs = static_cast<uint16_t>(mTransitionTimeMs - mElapsedTimeMs);
    }
    mCluster.UpdateRemainingTime(remainingTimeMs, LevelControlCluster::ReportingMode::kQuietReport);

    // Calculate new level based on time interpolation
    uint8_t currentLevel = mInitialLevel;

    if (mElapsedTimeMs >= mTransitionTimeMs)
    {
        currentLevel = mTargetLevel;
    }
    else if (mTransitionTimeMs > 0)
    {
        // Interpolate
        // Use 64-bit math for the intermediate multiplication to avoid overflow, though with uint8 levels it's unlikely to overflow
        // 32-bit. We avoid floating point to stick to integer arithmetic.
        const int32_t initial = mInitialLevel;
        const int32_t target  = mTargetLevel;
        const int32_t delta   = target - initial;

        // change = delta * (elapsed / total)
        // Reordered to: (delta * elapsed) / total
        const int32_t change = static_cast<int32_t>((static_cast<int64_t>(delta) * mElapsedTimeMs) / mTransitionTimeMs);

        currentLevel = static_cast<uint8_t>(initial + change);
    }

    // End of transition
    if (currentLevel == mTargetLevel || mElapsedTimeMs >= mTransitionTimeMs)
    {
        // Safe to ignore error: mTargetLevel was validated when starting the transition.
        RETURN_SAFELY_IGNORED mCluster.SetCurrentLevel(mTargetLevel, LevelControlCluster::ReportingMode::kForceReport);

        mCluster.UpdateRemainingTime(0, LevelControlCluster::ReportingMode::kForceReport); // Transition complete

        // If reached minimum, turn off OnOff cluster
        if ((IsWithOnOffCommand(mCurrentCommandId) || mCurrentCommandId == kInternalOffTransition) &&
            (mTargetLevel == mCluster.mMinLevel || mTargetLevel == 0))
        {
            LogErrorOnFailure(mCluster.SetOnOff(false));
        }

        if (mCurrentCommandId == kInternalOffTransition && mTargetLevel == mCluster.mMinLevel)
        {
            // Internal fade-to-off complete. Restore previous level.
            if (mCluster.mOnLevel.IsNull() && !mCluster.mLevelBeforeTurnedOff.IsNull())
            {
                RETURN_SAFELY_IGNORED mCluster.SetCurrentLevel(mCluster.mLevelBeforeTurnedOff.Value(),
                                                               LevelControlCluster::ReportingMode::kForceReport);
            }
        }
        return;
    }

    // Intermediate tick
    RETURN_SAFELY_IGNORED mCluster.SetCurrentLevel(currentLevel, LevelControlCluster::ReportingMode::kQuietReport);

    // StartTimer is safe here because this method is called when the timer has already fired (and thus is not active),
    // and if we are here it means we are continuing the same transition. If a new transition starts via StartTransition,
    // it will cancel any existing timer first.
    SuccessOrDie(mCluster.mTimerDelegate.StartTimer(this, System::Clock::Milliseconds64(mTickDurationMs)));
}

void LevelControlCluster::OnOnOffChanged(bool isOn)
{
    VerifyOrReturn(!mCurrentLevel.value().IsNull() && !mTemporarilyIgnoreOnOffCallbacks);

    if (isOn)
    {
        // On Transition
        // 2. Determine Target Level (Capture before setting to Min)
        const uint8_t target = mOnLevel.ValueOr(mLevelBeforeTurnedOff.ValueOr(kMaxLevel));

        // 1. Set to MinLevel
        // Ignore error as we are internally forcing a valid level (MinLevel) to start the transition.
        RETURN_SAFELY_IGNORED SetCurrentLevel(mMinLevel, ReportingMode::kForceReport);

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
        mLevelBeforeTurnedOff = mCurrentLevel.value();

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
        // Use kInternalOffTransition to differentiate this from a user-requested MoveToLevel.
        // This allows us to restore the pre-off level after the transition completes.
        MoveToLevelCommand(kInternalOffTransition, mMinLevel, transitionTime, optionsMask, optionsOverride);
    }
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
    if (!mFeatureMap.Has(Feature::kOnOff) || GetOnOff())
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
