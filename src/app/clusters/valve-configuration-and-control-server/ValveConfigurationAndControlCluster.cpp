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

#include "ValveConfigurationAndControlCluster.h"

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

ValveConfigurationAndControlCluster::ValveConfigurationAndControlCluster(EndpointId endpoint, ValveContext context) :
    DefaultServerCluster({ endpoint, ValveConfigurationAndControl::Id }), mFeatures(context.features),
    mOptionalAttributeSet(context.optionalAttributeSet), mDefaultOpenDuration(context.config.defaultOpenDuration),
    mDefaultOpenLevel(context.config.defaultOpenLevel), mLevelStep(context.config.levelStep), mDelegate(context.delegate),
    mTsTracker(context.tsTracker)
{}

CHIP_ERROR ValveConfigurationAndControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    // This feature shall not be supported if the node doesn't support the TimeSync cluster.
    if (mFeatures.Has(Feature::kTimeSync))
    {
        VerifyOrReturnError((mTsTracker != nullptr), CHIP_ERROR_INVALID_ARGUMENT);
    }

    // The RemainingDuration attribute shall be reported when:
    // - It changes from null or vice versa (default handling for QuieterReportingAttribute)
    // - It changes to 0
    // - It increases.
    mRemainingDuration.policy()
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero)
        .Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement);

    // Try to get the stored value for the DefaultOpenDuration attribute.
    AttributePersistence attrPersistence{ context.attributeStorage };
    const DataModel::Nullable<uint32_t> defaultOpenDuration = mDefaultOpenDuration;
    attrPersistence.LoadNativeEndianValue(
        { mPath.mEndpointId, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::DefaultOpenDuration::Id },
        mDefaultOpenDuration, defaultOpenDuration);

    // If Level feature is enabled and optional attribute is set, try to get value for
    // DefaultOpenLevel
    if (mFeatures.Has(Feature::kLevel) && mOptionalAttributeSet.IsSet(Attributes::DefaultOpenLevel::Id))
    {
        Percent defaultOpenLevel = mDefaultOpenLevel;
        attrPersistence.LoadNativeEndianValue(
            { mPath.mEndpointId, ValveConfigurationAndControl::Id, ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id },
            mDefaultOpenLevel, defaultOpenLevel);
    }

    return CHIP_NO_ERROR;
}

void ValveConfigurationAndControlCluster::Shutdown(ClusterShutdownType shutdownType)
{
    // Call CancelTimer in the instance shutdown to avoid "use after free" scenarios.
    DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);
    DefaultServerCluster::Shutdown(shutdownType);
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
        AttributePersistence persistence{ mContext->attributeStorage };
        return persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, mDefaultOpenDuration);
    }

    if (request.path.mAttributeId == ValveConfigurationAndControl::Attributes::DefaultOpenLevel::Id)
    {
        Percent defaultOpenLevel;
        ReturnErrorOnFailure(decoder.Decode(defaultOpenLevel));
        VerifyOrReturnValue(defaultOpenLevel != mDefaultOpenLevel, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);
        // TODO(#40708): Currently the `DecodeAndStoreNativeEndianValue` function doesn't allow performing specific checks
        // on provided values; update this logic once a fix for
        // https://github.com/project-chip/connectedhomeip/issues/40708 is merged.
        // In the current tests for this cluster this specific part is not validated.
        // https://github.com/CHIP-Specifications/chip-test-plans/issues/5835
        VerifyOrReturnError(ValueCompliesWithLevelStep(defaultOpenLevel), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        mDefaultOpenLevel = defaultOpenLevel;
        return mContext->attributeStorage.WriteValue(
            request.path, { reinterpret_cast<const uint8_t *>(&mDefaultOpenLevel), sizeof(mDefaultOpenLevel) });
    }

    return Status::UnsupportedWrite;
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
        return HandleCloseCommand();
    case ValveConfigurationAndControl::Commands::Open::Id:
        return HandleOpenCommand(request, input_arguments, handler);
    default:
        return Status::UnsupportedCommand;
    }
}

// Command Handlers
std::optional<DataModel::ActionReturnStatus> ValveConfigurationAndControlCluster::HandleCloseCommand()
{
    // If there is a fault that prevents the cluster to perform the action, return FailureDueToFault.
    // This logic keeps the previous implementation of this cluster, however it may be too strict since
    // any fault will cause the command to fail.
    if (mValveFault.HasAny())
    {
        return Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kFailureDueToFault);
    }

    return CloseValve();
}

CHIP_ERROR ValveConfigurationAndControlCluster::CloseValve()
{
    // OpenDuration and RemainingDuration shall be set to null.
    SetAttributeValue(mOpenDuration, DataModel::Nullable<uint32_t>{}, Attributes::OpenDuration::Id);
    SetRemainingDuration(DataModel::NullNullable);

    // If TimeSync feature is supported, AutoCloseTime shall be set to null.
    if (mFeatures.Has(Feature::kTimeSync))
    {
        SetAttributeValue(mAutoCloseTime, DataModel::Nullable<uint64_t>{}, Attributes::AutoCloseTime::Id);
    }

    // TargetState shall be set to Closed and CurrentState to Transitioning.
    SetAttributeValue(mTargetState, DataModel::MakeNullable(ValveStateEnum::kClosed), Attributes::TargetState::Id);
    SetCurrentState(ValveStateEnum::kTransitioning);

    // If Level feature is supported, TargetLevel shall be set to 0.
    if (mFeatures.Has(Feature::kLevel))
    {
        SetAttributeValue(mTargetLevel, DataModel::MakeNullable(Percent(0)), Attributes::TargetLevel::Id);
    }

    // Cancel timer if running.
    DeviceLayer::SystemLayer().CancelTimer(HandleUpdateRemainingDuration, this);

    VerifyOrReturnError(mDelegate != nullptr, CHIP_NO_ERROR);
    return mDelegate->HandleCloseValve();
}

std::optional<DataModel::ActionReturnStatus>
ValveConfigurationAndControlCluster::HandleOpenCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                       CommandHandler * handler)
{
    Commands::Open::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    // Check the "min 1" constraint in the command fields.
    // This mirrors the previous implementation for this cluster, however we need to validate properly the scenarios
    // where the TargetLevel field is set while the LVL feature is not enabled.
    // https://github.com/project-chip/connectedhomeip/issues/42777
    VerifyOrReturnValue((!commandData.openDuration.HasValue() ? true : commandData.openDuration.Value().ValueOr(1) > 0),
                        Status::ConstraintError);
    VerifyOrReturnValue(commandData.targetLevel.ValueOr(1) > 0, Status::ConstraintError);

    // If there is a fault that prevents the cluster to perform the action, return FailureDueToFault.
    // This logic keeps the previous implementation of this cluster, however it may be too strict since
    // any fault will cause the command to fail.
    if (mValveFault.HasAny())
    {
        return Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kFailureDueToFault);
    }

    // In the spec, the setting of the TargetState and CurrentState goes before the handling of the
    // fields of the command (checking and setting default values), however this was deferred to the OpenValve function to keep
    // backwards compatibility. Also this avoids setting the attributes if the targetLevel field doesn't have a valid value (in LVL
    // enabled valves).
    // Issue https://github.com/project-chip/connectedhomeip/issues/42777 created to follow up on the SDK changes.
    // Issue https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/12666 to follow up on the spec.
    // Issue https://github.com/CHIP-Specifications/chip-test-plans/issues/5837 to follow up on the test changes.

    // Check the rules for the OpenDuration field of the command.
    // This value will be used to set the OpenDuration attribute, initialize the RemainingDuration attribute and
    // calculate the AutoCloseTime attribute (if supported) in the OpenValve function
    DataModel::Nullable<uint32_t> openDuration;
    if (commandData.openDuration.HasValue())
    {
        // Save the duration if provided.
        openDuration = commandData.openDuration.Value();
    }
    else
    {
        // If no value is provided, use the DefaultOpenDuration attribute value.
        openDuration = mDefaultOpenDuration;
    }

    // Check rules for TargetLevel, if enabled
    // After validation and if the value is correct, the TargetLevel and CurrentLevel attributes
    // will be set in the OpenValve function.
    DataModel::Nullable<Percent> openTargetLevel;
    if (mFeatures.Has(Feature::kLevel))
    {
        // The rules to get the TargetLevel from the command data
        // If no value is provided
        if (!commandData.targetLevel.HasValue())
        {
            // Use the DefaultOpenLevel attribute if implemented.
            if (mOptionalAttributeSet.IsSet(Attributes::DefaultOpenLevel::Id))
            {
                openTargetLevel = mDefaultOpenLevel;
            }
            else
            {
                // If DefaultOpenLevel is not implemented, set it to 100
                openTargetLevel = kMaxLevelValuePercent;
            }
        }
        else
        {
            // If TargetLevel is provided
            // Validate that the TargetLevel and LevelStep are compatible.
            VerifyOrReturnError(ValueCompliesWithLevelStep(commandData.targetLevel.Value()),
                                CHIP_IM_GLOBAL_STATUS(ConstraintError));
            openTargetLevel = commandData.targetLevel.Value();
        }
    }

    // Use the OpenValve function to handle the setting of internal values.
    return OpenValve(openTargetLevel, openDuration);
}

CHIP_ERROR ValveConfigurationAndControlCluster::OpenValve(DataModel::Nullable<Percent> targetLevel,
                                                          DataModel::Nullable<uint32_t> openDuration)
{
    // Check for the AutoCloseTime feature and set it to the UTC time plus OpenDuration.
    if (mFeatures.Has(Feature::kTimeSync))
    {
        ReturnErrorOnFailure(SetAutoCloseTime(openDuration));
    }

    // Set TargetState to Open and CurrentState to Transitioning
    SetAttributeValue(mTargetState, DataModel::MakeNullable(ValveStateEnum::kOpen), Attributes::TargetState::Id);
    SetCurrentState(ValveStateEnum::kTransitioning);

    // Set OpenDuration to the provided value (can be null).
    SetAttributeValue(mOpenDuration, openDuration, Attributes::OpenDuration::Id);

    // Set the RemainingDuration to the value of OpenDuration (either a value or null)
    SetRemainingDuration(mOpenDuration);

    // Set target level
    if (mFeatures.Has(Feature::kLevel))
    {
        // If the LVL feature is enabled this function should actually have a TargetLevel
        VerifyOrDie(!targetLevel.IsNull());
        SetAttributeValue(mTargetLevel, targetLevel, Attributes::TargetLevel::Id);
    }

    if (mDelegate != nullptr)
    {
        DataModel::Nullable<Percent> cLevel = mDelegate->HandleOpenValve(targetLevel);
        if (mFeatures.Has(Feature::kLevel) && !cLevel.IsNull())
        {
            // Update CurrentLevel to the one returned from the Delegate
            UpdateCurrentLevel(cLevel.Value());
        }
    }

    // Start countdown if applicable (e.g. OpenDuration is not null).
    HandleUpdateRemainingDurationInternal();

    return CHIP_NO_ERROR;
}

// Internal functions.
CHIP_ERROR ValveConfigurationAndControlCluster::SetAutoCloseTime(DataModel::Nullable<uint32_t> openDuration)
{
    VerifyOrReturnValue(mTsTracker != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    if (!openDuration.IsNull() && mTsTracker->IsValidUTCTime())
    {
        // We have a synchronized UTC time in the TimeSync cluster, we can proceed to set the AutoCloseTime attribute
        System::Clock::Microseconds64 utcTime;
        uint64_t chipEpochTime;
        ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
        VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);

        uint64_t time = openDuration.Value() * kMicrosecondsPerSecond;
        SetAttributeValue(mAutoCloseTime, DataModel::MakeNullable(time + chipEpochTime), Attributes::AutoCloseTime::Id);
    }
    else
    {
        // No synchronized time or OpenDuration is null, setting the AutoCloseTime attribute to null
        SetAttributeValue(mAutoCloseTime, DataModel::Nullable<uint64_t>{}, Attributes::AutoCloseTime::Id);
    }

    return CHIP_NO_ERROR;
}

void ValveConfigurationAndControlCluster::UpdateAutoCloseTime(uint64_t epochTime)
{
    if (!mRemainingDuration.value().IsNull() && mRemainingDuration.value().Value() != 0)
    {
        uint64_t closingTime = mRemainingDuration.value().Value() * kMicrosecondsPerSecond + epochTime;
        SetAttributeValue(mAutoCloseTime, DataModel::MakeNullable(closingTime), Attributes::AutoCloseTime::Id);
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
        // If openDuration is null, RemainingDuration should also be null and this timer shouldn't be running.
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

// Function to handle the StateChange that also allows to generate an event if needed.
void ValveConfigurationAndControlCluster::SetCurrentState(const ValveStateEnum & newState)
{
    VerifyOrReturn(mCurrentState != newState);
    mCurrentState = newState;
    NotifyAttributeChanged(Attributes::CurrentState::Id);

    EmitValveChangeEvent(newState);
}

// According to the spec, when using a TargetLevel while the attribute LevelStep is set
// the value of TargetLevel should be 0, 100 or a multiple of LevelStep.
bool ValveConfigurationAndControlCluster::ValueCompliesWithLevelStep(const uint8_t value) const
{
    if (!mOptionalAttributeSet.IsSet(Attributes::LevelStep::Id))
    {
        return true;
    }
    return (value == kMaxLevelValuePercent) || (value % mLevelStep) == 0;
}

void ValveConfigurationAndControlCluster::SetDelegate(Delegate * delegate)
{
    mDelegate = delegate;
}

void ValveConfigurationAndControlCluster::UpdateCurrentState(const ValveConfigurationAndControl::ValveStateEnum currentState)
{
    SetCurrentState(currentState);

    if (mTargetState == currentState)
    {
        SetAttributeValue(mTargetState, DataModel::Nullable<ValveStateEnum>{}, Attributes::TargetState::Id);
    }
}

void ValveConfigurationAndControlCluster::UpdateCurrentLevel(Percent currentLevel)
{
    VerifyOrReturn(mFeatures.Has(Feature::kLevel));
    SetAttributeValue(mCurrentLevel, DataModel::MakeNullable(currentLevel), Attributes::CurrentLevel::Id);

    if (mCurrentLevel == mTargetLevel)
    {
        SetAttributeValue(mTargetLevel, DataModel::Nullable<Percent>{}, Attributes::TargetLevel::Id);
        UpdateCurrentState(currentLevel == 0 ? ValveStateEnum::kClosed : ValveStateEnum::kOpen);
    }
}

// Name taken from previous cluster implementation, need to change to a more clear one and review the rest
// to ensure that they are also fitting (https://github.com/project-chip/connectedhomeip/issues/42777)
void ValveConfigurationAndControlCluster::EmitValveChangeEvent(ValveConfigurationAndControl::ValveStateEnum newState)
{
    ValveConfigurationAndControl::Events::ValveStateChanged::Type event;
    event.valveState = newState;

    // Check if Level feature and add current level if enabled
    if (mFeatures.Has(Feature::kLevel) && !mCurrentLevel.IsNull())
    {
        event.valveLevel = MakeOptional<Percent>(mCurrentLevel.Value());
    }

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ValveConfigurationAndControlCluster::SetValveFault(BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ValveConfigurationAndControl::Events::ValveFault::Type event;
    event.valveFault = fault;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    mValveFault = fault;
}
