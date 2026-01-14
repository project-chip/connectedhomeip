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

#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
// Need the `nogncheck` because it's inter-cluster dependency and this
// breaks GN deps checks since that doesn't know how to deal with #ifdef'd includes :(.
#include "app/clusters/time-synchronization-server/time-synchronization-server.h" // nogncheck

#endif // ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER

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

void ValveConfigurationAndControlCluster::SetRemainingDuration(const DataModel::Nullable<ElapsedS> & remainingDuration)
{
    System::Clock::Milliseconds64 now = System::SystemClock().GetMonotonicMilliseconds64();
    AttributeDirtyState dirtyState    = mRemainingDuration.SetValue(remainingDuration, now);
    if (dirtyState == AttributeDirtyState::kMustReport)
    {
        NotifyAttributeChanged(Attributes::RemainingDuration::Id);
    }
}


ValveConfigurationAndControlCluster::ValveConfigurationAndControlCluster(
    EndpointId endpoint, BitFlags<ValveConfigurationAndControl::Feature> features, OptionalAttributeSet optionalAttributeSet,
    const StartupConfiguration & config) :
    DefaultServerCluster({ endpoint, ValveConfigurationAndControl::Id }),
    mFeatures(features), mOptionalAttributeSet(optionalAttributeSet), mDefaultOpenDuration(config.defaultOpenDuration),
    mDefaultOpenLevel(config.defaultOpenLevel), mLevelStep(config.levelStep), mDelegate(nullptr)
{}

CHIP_ERROR ValveConfigurationAndControlCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

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

void ValveConfigurationAndControlCluster::emitValveStateChangedEvent(ValveConfigurationAndControl::ValveStateEnum newState)
{
    ValveConfigurationAndControl::Events::ValveStateChanged::Type event;
    event.valveState = newState;

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ValveConfigurationAndControlCluster::emitValveFaultEvent(BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault)
{
    ValveConfigurationAndControl::Events::ValveFault::Type event;
    event.valveFault = fault;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    mValveFault = fault;
}

void ValveConfigurationAndControlCluster::onValveConfigurationAndControlTick(System::Layer * systemLayer, void * context)
{
    auto * instance = static_cast<ValveConfigurationAndControlCluster *>(context);

    if (instance->mRemainingDuration.value().Value() > 0)
    {
        instance->SetRemainingDuration(instance->mRemainingDuration.value().Value() - 1);
        instance->startRemainingDurationTick();
    }
    else
    {
        instance->SetRemainingDuration(DataModel::NullNullable);
    }
}

void ValveConfigurationAndControlCluster::startRemainingDurationTick()
{
    VerifyOrReturn(!mRemainingDuration.value().IsNull());
    VerifyOrReturn(mDelegate != nullptr);

    mDelegate->HandleRemainingDurationTick(mRemainingDuration.value().Value());

    if (mRemainingDuration.value().Value() > 0)
    {
        LogErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onValveConfigurationAndControlTick, this));
    }
    else
    {
        LogErrorOnFailure(CloseValve());
        DeviceLayer::SystemLayer().CancelTimer(onValveConfigurationAndControlTick, this);
    }
}

void ValveConfigurationAndControlCluster::SetDelegate(Delegate * delegate)
{
    mDelegate = delegate;
}

CHIP_ERROR ValveConfigurationAndControlCluster::CloseValve()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TargetState should be set to Closed and CurrentState to Transitioning
    SaveAndReportIfChanged(mTargetState, DataModel::MakeNullable(ValveStateEnum::kClosed), Attributes::TargetState::Id);
    SetCurrentState(DataModel::MakeNullable(ValveStateEnum::kTransitioning));

    // OpenDuration and RemainingDuration should be set to null
    SaveAndReportIfChanged(mOpenDuration, DataModel::NullNullable, Attributes::OpenDuration::Id);

    // Set the TargetLevel to 0
    if (mFeatures.Has(Feature::kLevel))
    {
        SaveAndReportIfChanged(mTargetLevel, Percent(0), Attributes::TargetLevel::Id);
    }

    // If TimeSync is enabled, AutoCloseTime should be set to null
    if (mFeatures.Has(Feature::kTimeSync))
    {
        SaveAndReportIfChanged(mAutoCloseTime, DataModel::NullNullable, Attributes::AutoCloseTime::Id);
    }

    SetRemainingDuration(DataModel::NullNullable);

    // Cancel timer if running.
    DeviceLayer::SystemLayer().CancelTimer(onValveConfigurationAndControlTick, this);

    if (mDelegate != nullptr)
    {
        err = mDelegate->HandleCloseValve();
    }

    return err;
}

CHIP_ERROR ValveConfigurationAndControlCluster::SetValveLevel(DataModel::Nullable<Percent> level,
                                                              DataModel::Nullable<uint32_t> openDuration)
{
    // Check for the AutoCloseTime feature
    if (mFeatures.Has(Feature::kTimeSync))
    {
#ifdef ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
        auto timeSynchronization = TimeSynchronization::GetClusterInstance();
        VerifyOrReturnValue(timeSynchronization != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        if (!openDuration.IsNull() &&
            timeSynchronization->GetGranularity() != TimeSynchronization::GranularityEnum::kNoTimeGranularity)
        {
            System::Clock::Microseconds64 utcTime;
            uint64_t chipEpochTime;
            ReturnErrorOnFailure(System::SystemClock().GetClock_RealTime(utcTime));
            VerifyOrReturnError(UnixEpochToChipEpochMicros(utcTime.count(), chipEpochTime), CHIP_ERROR_INVALID_TIME);

            uint64_t time = openDuration.Value() * chip::kMicrosecondsPerSecond;
            SaveAndReportIfChanged(mAutoCloseTime, DataModel::MakeNullable(time + chipEpochTime), Attributes::AutoCloseTime::Id);
        }
        else
        {
            SaveAndReportIfChanged(mAutoCloseTime, DataModel::NullNullable, Attributes::AutoCloseTime::Id);
        }
#else
        return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // ZCL_USING_TIME_SYNCHRONIZATION_CLUSTER_SERVER
    }

    // Set target level
    if (mFeatures.Has(Feature::kLevel) && !level.IsNull())
    {
        SaveAndReportIfChanged(mTargetLevel, level, Attributes::TargetLevel::Id);
    }

    // Set OpenDuration to the provided value (can be null).
    SaveAndReportIfChanged(mOpenDuration, openDuration, Attributes::OpenDuration::Id);

    // Set the RemainingDuration to the value of OpenDuration (either a value or null)
    SetRemainingDuration(mOpenDuration);

    // Set the states accordingly, TargetState to Open and CurrentState to Transitioning
    SaveAndReportIfChanged(mTargetState, DataModel::MakeNullable(ValveStateEnum::kOpen), Attributes::TargetState::Id);
    SetCurrentState(DataModel::MakeNullable(ValveStateEnum::kTransitioning));


    if (mDelegate != nullptr)
    {
        DataModel::Nullable<Percent> cLevel = mDelegate->HandleOpenValve(level);
        if (mFeatures.Has(Feature::kLevel) && !cLevel.IsNull())
        {
            // Update CurrentLevel to the one returned from the Delegate
            UpdateCurrentLevel(cLevel.Value());
        }
    }

    // Start countdown
    startRemainingDurationTick();

    return CHIP_NO_ERROR;
}

void ValveConfigurationAndControlCluster::UpdateCurrentLevel(Percent currentLevel)
{
    VerifyOrReturn(mFeatures.Has(Feature::kLevel));
    SaveAndReportIfChanged(mCurrentLevel, currentLevel, Attributes::CurrentLevel::Id);

    if (mCurrentLevel == mTargetLevel)
    {
        SaveAndReportIfChanged(mTargetLevel, DataModel::NullNullable, Attributes::TargetLevel::Id);
        UpdateCurrentState(currentLevel == 0 ? ValveStateEnum::kClosed : ValveStateEnum::kOpen);
    }
}

void ValveConfigurationAndControlCluster::UpdateCurrentState(const ValveConfigurationAndControl::ValveStateEnum currentState)
{
    SetCurrentState(DataModel::MakeNullable(currentState));

    if (mTargetState == currentState)
    {
        SaveAndReportIfChanged(mTargetState, DataModel::NullNullable, Attributes::TargetState::Id);
    }
}

void ValveConfigurationAndControlCluster::UpdateAutoCloseTime(uint64_t epochTime)
{
    if (!mRemainingDuration.value().IsNull() && mRemainingDuration.value().Value() != 0)
    {
        uint64_t closingTime = mRemainingDuration.value().Value() * kMicrosecondsPerSecond + epochTime;
        SaveAndReportIfChanged(mAutoCloseTime, DataModel::MakeNullable(closingTime), Attributes::AutoCloseTime::Id);
    }
}

std::optional<DataModel::ActionReturnStatus>
ValveConfigurationAndControlCluster::HandleOpenCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                       CommandHandler * handler)
{
    Commands::Open::DecodableType commandData;
    DataModel::Nullable<Percent> openTargetLevel;
    DataModel::Nullable<uint32_t> openDuration;

    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    // Check if there is any Fault registered
    if (mValveFault.HasAny())
    {
        return Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kFailureDueToFault);
    }

    // Verify "min 1" constraint on TargetLevel field.
    VerifyOrReturnValue(commandData.targetLevel.HasValue() ? commandData.targetLevel.Value() > 0 : true, Status::ConstraintError);

    if (commandData.openDuration.HasValue())
    {
        // Check if the provided openDuration has a value and validate the "min 1" constraint in this field.
        // Save the duration if provided, it can be null or an actual value.
        VerifyOrReturnValue(commandData.openDuration.Value().IsNull() ? true : commandData.openDuration.Value().Value() > 0,
                            Status::ConstraintError);
        openDuration = commandData.openDuration.Value();
    }
    else
    {
        // If no value is provided, use the DefaultOpenDuration attribute value.
        openDuration = mDefaultOpenDuration;
    }

    // Check rules for TargetLevel if enabled
    if (mFeatures.Has(Feature::kLevel))
    {
        if (commandData.targetLevel.HasValue())
        {
            openTargetLevel.SetNonNull(commandData.targetLevel.Value());
        }
        else
        {
            openTargetLevel.SetNonNull(mDefaultOpenLevel);
        }
    }

    // Use the SetValveLevel function to handle the setting of internal values.
    return SetValveLevel(openTargetLevel, openDuration);
}

std::optional<DataModel::ActionReturnStatus> ValveConfigurationAndControlCluster::HandleCloseCommand()
{
    // Check if there is any Fault registered
    if (mValveFault.HasAny())
    {
        return Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kFailureDueToFault);
    }

    return CloseValve();
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

// Function to handle the StateChange that also allows to generate an event if needed.
void ValveConfigurationAndControlCluster::SetCurrentState(
    const DataModel::Nullable<ValveConfigurationAndControl::ValveStateEnum> & newState)
{
    VerifyOrReturn(mCurrentState != newState);
    mCurrentState = newState;
    NotifyAttributeChanged(Attributes::CurrentState::Id);

    // Only emit ValveStateChanged when the new state is non-null; transitions to null are not reported.
    if (!mCurrentState.IsNull())
    {
        emitValveStateChangedEvent(mCurrentState.Value());
    }
}
