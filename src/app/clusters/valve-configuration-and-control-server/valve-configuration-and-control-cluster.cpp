/**
 *
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

#include "valve-configuration-and-control-cluster.h"

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ValveConfigurationAndControl/Commands.h>
#include <clusters/ValveConfigurationAndControl/Events.h>
#include <lib/support/CodeUtils.h>

#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ValveConfigurationAndControl;
using chip::Protocols::InteractionModel::Status;

ValveConfigurationAndControlCluster::ValveConfigurationAndControlCluster(EndpointId endpoint,
                                                                         BitFlags<ValveConfigurationAndControl::Feature> features,
                                                                         OptionalAttributeSet optionalAttributeSet,
                                                                         TimeSyncTracker * tsTracker) :
    DefaultServerCluster({ endpoint, ValveConfigurationAndControl::Id }),
    mFeatures(features), mOptionalAttributeSet(optionalAttributeSet), mDelegate(nullptr), mTsTracker(tsTracker)
{}

CHIP_ERROR ValveConfigurationAndControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // This feature shall not be supported if the cluster doesn't support the TimeSync cluster.
    if (mFeatures.Has(Feature::kTimeSync))
    {
        VerifyOrReturnError((mTsTracker != nullptr && mTsTracker->IsTimeSyncClusterSupported()), CHIP_ERROR_INVALID_ARGUMENT);
    }

    // The RemainingDuration attribute shall be reported when:
    // - It changes from Null or  vice versa (default handling for QuieterReportingAttribute)
    // - It changes to 0
    // - It increases.
    mRemainingDuration.policy()
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero)
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement);

    // Try to get the stored value for the DefaultOpenDuration attribute.
    MutableByteSpan defaultOpenDurationBytes(reinterpret_cast<uint8_t *>(&mDefaultOpenDuration), sizeof(mDefaultOpenDuration));
    const DataModel::Nullable<uint32_t> defaultOpenDuration = mDefaultOpenDuration;
    if (context.attributeStorage.ReadValue({ mPath.mEndpointId, ValveConfigurationAndControl::Id,
                                             ValveConfigurationAndControl::Attributes::DefaultOpenDuration::Id },
                                           defaultOpenDurationBytes) != CHIP_NO_ERROR)
    {
        mDefaultOpenDuration = defaultOpenDuration;
    }

    // If Level feature is enabled, try to get the DefaultOpenLevel value.
    if (mFeatures.Has(Feature::kLevel))
    {
        AttributePersistence attrPersistence{ context.attributeStorage };
        Percent defaultOpenLevel = mDefaultOpenLevel;
        attrPersistence.LoadNativeEndianValue(
            { mPath.mEndpointId, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id },
            mDefaultOpenLevel, defaultOpenLevel);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ValveConfigurationAndControlCluster::Attributes(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const bool isDefaultOpenLevelSupported =
        (mFeatures.Has(Feature::kLevel) && mOptionalAttributeSet.IsSet(Attributes::DefaultOpenLevel::Id));
    const bool isLevelStepSupported = (mFeatures.Has(Feature::kLevel) && mOptionalAttributeSet.IsSet(Attributes::LevelStep::Id));

    AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = {
        { mFeatures.Has(Feature::kTimeSync), Attributes::AutoCloseTime::kMetadataEntry },
        { mFeatures.Has(Feature::kLevel), Attributes::CurrentLevel::kMetadataEntry },
        { mFeatures.Has(Feature::kLevel), Attributes::TargetLevel::kMetadataEntry },
        { isDefaultOpenLevelSupported, Attributes::DefaultOpenLevel::kMetadataEntry },
        { mOptionalAttributeSet.IsSet(Attributes::ValveFault::Id), Attributes::ValveFault::kMetadataEntry },
        { isLevelStepSupported, Attributes::LevelStep::kMetadataEntry }
    };

    return listBuilder.Append(Span(ValveConfigurationAndControl::Attributes::kMandatoryMetadata), Span(optionalAttributeEntries));
}

DataModel::ActionReturnStatus ValveConfigurationAndControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                 AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ValveConfigurationAndControl::Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case ValveConfigurationAndControl::Attributes::ClusterRevision::Id:
        return encoder.Encode(ValveConfigurationAndControl::kRevision);
    case ValveConfigurationAndControl::Attributes::OpenDuration::Id:
        return encoder.Encode(mOpenDuration);
    case ValveConfigurationAndControl::Attributes::DefaultOpenDuration::Id:
        return encoder.Encode(mDefaultOpenDuration);
    case ValveConfigurationAndControl::Attributes::AutoCloseTime::Id:
        return encoder.Encode(mAutoCloseTime);
    case ValveConfigurationAndControl::Attributes::RemainingDuration::Id:
        return encoder.Encode(mRemainingDuration.value());
    case ValveConfigurationAndControl::Attributes::CurrentState::Id:
        return encoder.Encode(mCurrentState);
    case ValveConfigurationAndControl::Attributes::TargetState::Id:
        return encoder.Encode(mTargetState);
    case ValveConfigurationAndControl::Attributes::CurrentLevel::Id:
        return encoder.Encode(mCurrentLevel);
    case ValveConfigurationAndControl::Attributes::TargetLevel::Id:
        return encoder.Encode(mTargetLevel);
    case ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id:
        return encoder.Encode(mDefaultOpenLevel);
    case ValveConfigurationAndControl::Attributes::ValveFault::Id:
        return encoder.Encode(mValveFault);
    case ValveConfigurationAndControl::Attributes::LevelStep::Id:
        return encoder.Encode(mLevelStep);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus ValveConfigurationAndControlCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                                  AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus ValveConfigurationAndControlCluster::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                                             AttributeValueDecoder & decoder)
{

    if (request.path.mAttributeId == ValveConfigurationAndControl::Attributes::DefaultOpenDuration::Id)
    {
        DataModel::Nullable<uint32_t> defaultOpenDuration;
        ReturnErrorOnFailure(decoder.Decode(defaultOpenDuration));
        VerifyOrReturnValue(defaultOpenDuration != mDefaultOpenDuration,
                            DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        mDefaultOpenDuration = defaultOpenDuration;
        return mContext->attributeStorage.WriteValue(
            request.path, { reinterpret_cast<const uint8_t *>(&mDefaultOpenDuration), sizeof(mDefaultOpenDuration) });
    }

    if (request.path.mAttributeId == ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id)
    {
        Percent defaultOpenLevel;
        ReturnErrorOnFailure(decoder.Decode(defaultOpenLevel));
        VerifyOrReturnValue(defaultOpenLevel != mDefaultOpenLevel, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

        // If the LevelStep attribute is Set, verify that the provided DefaultOpenLevel complies with the spec.
        if (mOptionalAttributeSet.IsSet(Attributes::LevelStep::Id))
        {
            VerifyOrReturnError(ValueCompliesWithLevelStep(defaultOpenLevel), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        }

        mDefaultOpenLevel = defaultOpenLevel;
        return mContext->attributeStorage.WriteValue(
            request.path, { reinterpret_cast<const uint8_t *>(&mDefaultOpenLevel), sizeof(mDefaultOpenLevel) });
    }

    return Protocols::InteractionModel::Status::UnsupportedWrite;
}

CHIP_ERROR ValveConfigurationAndControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                                 ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        ValveConfigurationAndControl::Commands::Open::kMetadataEntry, ValveConfigurationAndControl::Commands::Close::kMetadataEntry
    };

    return builder.ReferenceExisting(kAcceptedCommands);
}

std::optional<DataModel::ActionReturnStatus>
ValveConfigurationAndControlCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                   CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case ValveConfigurationAndControl::Commands::Close::Id:
        return HandleCloseCommand(request, handler);
    case ValveConfigurationAndControl::Commands::Open::Id:
        return HandleOpenCommand(request, input_arguments, handler);
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

// Command Handlers
std::optional<DataModel::ActionReturnStatus>
ValveConfigurationAndControlCluster::HandleCloseCommand(const DataModel::InvokeRequest & request, CommandHandler * handler)
{
    // Cancel time if running.
    DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);

    // Check if there is any Fault registered
    if (mValveFault.HasAny())
    {
        ReturnErrorOnFailure(handler->AddClusterSpecificFailure(request.path, to_underlying(StatusCodeEnum::kFailureDueToFault)));
        return std::nullopt;
    }

    return CloseValve();
}

CHIP_ERROR ValveConfigurationAndControlCluster::CloseValve()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // OpenDuration and RemainingDuration should be set to Null
    SaveAndReportIfChanged(mOpenDuration, DataModel::NullNullable, Attributes::OpenDuration::Id);
    SetRemainingDuration(DataModel::NullNullable);

    // TargetState should be set to Closed and CurrentState to Transitioning
    SaveAndReportIfChanged(mTargetState, DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kClosed), Attributes::TargetState::Id);
    SaveAndReportIfChanged(mCurrentState, DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kTransitioning),
                           Attributes::CurrentState::Id);

    // If TimeSync is enabled, AutoCloseTime should be set to Null
    if (mFeatures.Has(Feature::kTimeSync))
    {
        SaveAndReportIfChanged(mAutoCloseTime, DataModel::NullNullable, Attributes::AutoCloseTime::Id);
    }

    // Set the TargetLevel to 0
    if (mFeatures.Has(Feature::kLevel))
    {
        SaveAndReportIfChanged(mTargetLevel, Percent(0), Attributes::TargetLevel::Id);
    }

    if (mDelegate != nullptr)
    {
        err = mDelegate->HandleCloseValve();
    }

    // If there was an error, we know nothing about the current state
    if (err != CHIP_NO_ERROR)
    {
        SaveAndReportIfChanged(mCurrentLevel, DataModel::NullNullable, Attributes::CurrentLevel::Id);
        SaveAndReportIfChanged(mCurrentState, DataModel::NullNullable, Attributes::CurrentState::Id);
    }

    // Emit the Transition state.
    emitValveChangeEvent(ValveStateEnum::kTransitioning);

    return err;
}

std::optional<DataModel::ActionReturnStatus>
ValveConfigurationAndControlCluster::HandleOpenCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                       CommandHandler * handler)
{
    Commands::Open::DecodableType commandData;
    DataModel::Nullable<chip::Percent> openTargetLevel;
    DataModel::Nullable<chip::Percent> openCurrentLevel;

    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    // Check if there is any Fault registered
    if (mValveFault.HasAny())
    {
        ReturnErrorOnFailure(handler->AddClusterSpecificFailure(request.path, to_underlying(StatusCodeEnum::kFailureDueToFault)));
        return std::nullopt;
    }

    // Set the states accordingly, TargetState to Open and CurrentState to Transitioning
    SaveAndReportIfChanged(mTargetState, DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kOpen), Attributes::TargetState::Id);
    SaveAndReportIfChanged(mCurrentState, DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kTransitioning),
                           Attributes::CurrentState::Id);

    // Check if the provided openDuration has a value
    if (commandData.openDuration.HasValue())
    {
        // Save the duration if provided, it can be Null or an actual value.
        SaveAndReportIfChanged(mOpenDuration, commandData.openDuration.Value(), Attributes::OpenDuration::Id);
    }
    else
    {
        // If no value is provided, use the DefaultOpenDuration attribute value.
        SaveAndReportIfChanged(mOpenDuration, mDefaultOpenDuration, Attributes::OpenDuration::Id);
    }

    // Check if the OpenDuration value is Null
    if (mOpenDuration.IsNull())
    {
        // If null, set the remaining duration as Null
        SetRemainingDuration(DataModel::NullNullable);
    }
    else
    {
        // If the OpenDuration value is provided, set the RemainingDuration attribute to the value of OpenDuration.
        SetRemainingDuration(mOpenDuration.Value());
    }

    // If TimeSync feature is enabled, set the value according the OpenDuration attribute.
    if (mFeatures.Has(Feature::kTimeSync))
    {
        ReturnErrorOnFailure(SetAutoCloseTime(mOpenDuration));
    }

    // Check rules for TargetLevel if enabled
    if (mFeatures.Has(Feature::kLevel))
    {
        ReturnErrorOnFailure(GetAdjustedTargetLevel(commandData.targetLevel, openTargetLevel));
    }

    if (mDelegate != nullptr)
    {
        openCurrentLevel = mDelegate->HandleOpenValve(openTargetLevel);
    }

    // Update the TargetLevel and set the CurrentLevel to the returned from the Delegate call.
    if (mFeatures.Has(Feature::kLevel))
    {
        SaveAndReportIfChanged(mTargetLevel, openTargetLevel, Attributes::TargetLevel::Id);

        if (!openCurrentLevel.IsNull())
        {
            UpdateCurrentLevel(openCurrentLevel.Value());
        }
    }

    // Start countdown if applicable (e.g. OpenDuration is not Null).
    HandleUpdateRemainingDurationInternal();

    return Status::Success;
}

// Internal functions.
CHIP_ERROR ValveConfigurationAndControlCluster::SetAutoCloseTime(DataModel::Nullable<uint32_t> openDuration)
{
    if (!openDuration.IsNull() && mTsTracker->IsValidUTCTime())
    {
        // We have a synchronized UTC time in the TimeSync cluster, we can proceed to set the AutoCloseTime attribute
        System::Clock::Microseconds64 utcTime;
        uint64_t chipEpochTime;
        ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
        VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);

        uint64_t time = openDuration.Value() * chip::kMicrosecondsPerSecond;
        SaveAndReportIfChanged(mAutoCloseTime, DataModel::Nullable<uint64_t>(time + chipEpochTime), Attributes::AutoCloseTime::Id);
    }
    else
    {
        // No synchronized time or OpenDuration is null, setting the AutoCloseTime attribute to Null
        SaveAndReportIfChanged(mAutoCloseTime, DataModel::NullNullable, Attributes::AutoCloseTime::Id);
    }

    return CHIP_NO_ERROR;
}

void ValveConfigurationAndControlCluster::UpdateAutoCloseTime(uint64_t time)
{
    if (!mRemainingDuration.value().IsNull() && mRemainingDuration.value().Value() != 0)
    {
        uint64_t closingTime = mRemainingDuration.value().Value() * chip::kMicrosecondsPerSecond + time;
        SaveAndReportIfChanged(mAutoCloseTime, DataModel::Nullable<uint64_t>(closingTime), Attributes::AutoCloseTime::Id);
    }
}
void ValveConfigurationAndControlCluster::HandleUpdateRemainingDuration(System::Layer * systemLayer, void * context)
{
    auto * logic = static_cast<ValveConfigurationAndControlCluster *>(context);
    logic->HandleUpdateRemainingDurationInternal();
}

void ValveConfigurationAndControlCluster::HandleUpdateRemainingDurationInternal()
{
    // Start by cancelling the timer in case this was called from a command handler
    // We will start a new timer if required.
    DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);

    if (mOpenDuration.IsNull() || mRemainingDuration.value().IsNull())
    {
        // If openDuration is NULL, RemainingDuration should also be Null and this timer shouldn't be running.
        SetRemainingDuration(DataModel::NullNullable);
        return;
    }

    if (mDelegate != nullptr)
    {
        mDelegate->HandleRemainingDurationTick(mRemainingDuration.value().Value());
    }

    // Check the value of RemainingDuration, when reaches 0 the valve shall be closed.
    if (mRemainingDuration.value().Value() > 0)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), HandleUpdateRemainingDuration, this));
        SetRemainingDuration(mRemainingDuration.value().Value() - 1);
    }
    else
    {
        LogErrorOnFailure(CloseValve());
        DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);
    }
}

void ValveConfigurationAndControlCluster::SetRemainingDuration(const DataModel::Nullable<ElapsedS> & remainingDuration)
{
    System::Clock::Milliseconds64 now = System::SystemClock().GetMonotonicMilliseconds64();
    AttributeDirtyState dirtyState    = mRemainingDuration.SetValue(remainingDuration, now);
    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        NotifyAttributeChanged(Attributes::RemainingDuration::Id);
    }
}

// The rules to get the TargetLevel from the command data
// - if no value is provided
//   - Use the DefaultOpenLevel attribute if implemented.
//   - If DefaultOpenLevel is not implemented, set it to 100
// - if the TargetLevel is provided
//   - Validate that the TargetLevel and LevelStep are compatible.
CHIP_ERROR
ValveConfigurationAndControlCluster::GetAdjustedTargetLevel(const Optional<Percent> & targetLevel,
                                                            DataModel::Nullable<chip::Percent> & adjustedTargetLevel) const
{
    if (!targetLevel.HasValue())
    {
        if (mOptionalAttributeSet.IsSet(Attributes::DefaultOpenLevel::Id))
        {
            adjustedTargetLevel = mDefaultOpenLevel;
            return CHIP_NO_ERROR;
        }
        adjustedTargetLevel = 100u;
        return CHIP_NO_ERROR;
    }

    // targetLevel has a value
    VerifyOrReturnError(ValueCompliesWithLevelStep(targetLevel.Value()), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    adjustedTargetLevel = targetLevel.Value();
    return CHIP_NO_ERROR;
}

// According to the spec, when using a TargetLevel while the attribute LevelStep is set
// the value of TargetLevel should be 0, 100 or a multiple of LevelStep.
bool ValveConfigurationAndControlCluster::ValueCompliesWithLevelStep(const uint8_t value) const
{
    if (mOptionalAttributeSet.IsSet(Attributes::LevelStep::Id))
    {
        if ((value != 100u) && ((value % mLevelStep) != 0))
        {
            return false;
        }
    }
    return true;
}

void ValveConfigurationAndControlCluster::SetDelegate(Delegate * delegate)
{
    mDelegate = delegate;
}

void ValveConfigurationAndControlCluster::UpdateCurrentState(const ValveConfigurationAndControl::ValveStateEnum currentState)
{
    SaveAndReportIfChanged(mCurrentState, DataModel::Nullable<ValveStateEnum>(currentState), Attributes::CurrentState::Id);

    if (mTargetState.ValueOr(ValveStateEnum::kUnknownEnumValue) == currentState)
    {
        SaveAndReportIfChanged(mTargetState, DataModel::NullNullable, Attributes::TargetState::Id);
    }

    emitValveChangeEvent(currentState);
}

void ValveConfigurationAndControlCluster::UpdateCurrentLevel(chip::Percent currentLevel)
{
    if (mFeatures.Has(Feature::kLevel))
    {
        SaveAndReportIfChanged(mCurrentLevel, currentLevel, Attributes::CurrentLevel::Id);

        if (!mTargetLevel.IsNull() && mCurrentLevel == mTargetLevel)
        {
            SaveAndReportIfChanged(mTargetLevel, DataModel::NullNullable, Attributes::TargetLevel::Id);
            UpdateCurrentState(currentLevel == 0 ? ValveStateEnum::kClosed : ValveStateEnum::kOpen);
        }
    }
}

CHIP_ERROR ValveConfigurationAndControlCluster::SetValveLevel(DataModel::Nullable<Percent> level,
                                                              DataModel::Nullable<uint32_t> openDuration)
{
    // Check for the AutoCloseTime feature
    if (mFeatures.Has(Feature::kTimeSync))
    {
        VerifyOrReturnValue(mTsTracker->IsTimeSyncClusterSupported(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(SetAutoCloseTime(openDuration));
    }

    // level can only be null if LVL feature is not supported
    if (mFeatures.Has(Feature::kLevel) && !level.IsNull())
    {
        SaveAndReportIfChanged(mTargetLevel, level, Attributes::TargetLevel::Id);
    }

    SaveAndReportIfChanged(mOpenDuration, openDuration, Attributes::OpenDuration::Id);

    SetRemainingDuration(openDuration);
    // set targetstate to open
    // Set the states accordingly, TargetState to Open and CurrentState to Transitioning
    SaveAndReportIfChanged(mTargetState, DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kOpen), Attributes::TargetState::Id);
    SaveAndReportIfChanged(mCurrentState, DataModel::Nullable<ValveStateEnum>(ValveStateEnum::kTransitioning),
                           Attributes::CurrentState::Id);

    // start movement towards target
    emitValveChangeEvent(ValveConfigurationAndControl::ValveStateEnum::kTransitioning);

    if (mDelegate != nullptr)
    {
        DataModel::Nullable<Percent> cLevel = mDelegate->HandleOpenValve(level);
        if (mFeatures.Has(Feature::kLevel) && !cLevel.IsNull())
        {
            UpdateCurrentLevel(cLevel.Value());
        }
    }

    // start countdown
    HandleUpdateRemainingDurationInternal();

    return CHIP_NO_ERROR;
}

void ValveConfigurationAndControlCluster::emitValveChangeEvent(ValveConfigurationAndControl::ValveStateEnum currentState)
{
    ValveConfigurationAndControl::Events::ValveStateChanged::Type event;
    event.valveState = currentState;
    
    // Check if Level feature and add current level if enabled
    if(mFeatures.Has(Feature::kLevel) && !mCurrentLevel.IsNull())
    {
        event.valveLevel = MakeOptional<Percent>(mCurrentLevel.Value());
    }

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ValveConfigurationAndControlCluster::EmitValveFault(chip::BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ValveConfigurationAndControl::Events::ValveFault::Type event;
    event.valveFault = fault;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    mValveFault = fault;
}
