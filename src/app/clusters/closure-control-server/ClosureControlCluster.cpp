/*
 *
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

#include "ClosureControlCluster.h"

#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ClosureControl/Attributes.h>
#include <clusters/ClosureControl/Commands.h>
#include <clusters/ClosureControl/Metadata.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureControl::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

ClosureControlCluster::ClosureControlCluster(EndpointId endpointId, const Context & context) :
    DefaultServerCluster({ endpointId, ClosureControl::Id }), mDelegate(context.delegate), mTimerDelegate(context.timerDelegate),
    mConformance(context.conformance)
{
    VerifyOrDieWithMsg(context.conformance.Valid(), AppServer, "Invalid conformance");
    VerifyOrDieWithMsg(SetMainState(context.initParams.mMainState) == CHIP_NO_ERROR, AppServer, "Failed to set main state");
    VerifyOrDieWithMsg(SetOverallCurrentState(context.initParams.mOverallCurrentState) == CHIP_NO_ERROR, AppServer,
                       "Failed to set overall current state");
}

ClosureControlCluster::~ClosureControlCluster() {}

CHIP_ERROR ClosureControlCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using OptionalEntry                = AttributeListBuilder::OptionalAttributeEntry;
    OptionalEntry optionalAttributes[] = {
        { mConformance.OptionalAttributes().IsSet(Attributes::CountdownTime::Id), Attributes::CountdownTime::kMetadataEntry },
        { mConformance.HasFeature(Feature::kMotionLatching), Attributes::LatchControlModes::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(ClosureControl::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ClosureControlCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    const ClusterConformance & conformance = mConformance;

    static constexpr DataModel::AcceptedCommandEntry kMandatoryCommands[] = {
        ClosureControl::Commands::MoveTo::kMetadataEntry,
    };

    static constexpr DataModel::AcceptedCommandEntry kStopCommand[] = {
        ClosureControl::Commands::Stop::kMetadataEntry,
    };

    static constexpr DataModel::AcceptedCommandEntry kCalibrateCommand[] = {
        ClosureControl::Commands::Calibrate::kMetadataEntry,
    };

    if (!conformance.HasFeature(Feature::kInstantaneous))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kStopCommand));
    }

    ReturnErrorOnFailure(builder.ReferenceExisting(kMandatoryCommands));

    if (conformance.HasFeature(Feature::kCalibration))
    {
        ReturnErrorOnFailure(builder.ReferenceExisting(kCalibrateCommand));
    }

    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ClosureControlCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(ClosureControl::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(GetFeatureMap());
    case MainState::Id:
        return encoder.Encode(GetMainState());
    case CountdownTime::Id:
        return encoder.Encode(GetCountdownTime());
    case CurrentErrorList::Id:
        return encoder.EncodeList(
            [this](const auto & subEncoder) -> CHIP_ERROR { return ReadCurrentErrorListAttribute(subEncoder); });
    case OverallCurrentState::Id:
        return encoder.Encode(GetOverallCurrentState());
    case OverallTargetState::Id:
        return encoder.Encode(GetOverallTargetState());
    case LatchControlModes::Id:
        return encoder.Encode(GetLatchControlModes());
    default:
        return Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> ClosureControlCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  chip::TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    VerifyOrReturnValue(handler != nullptr, Status::Failure);

    switch (request.path.mCommandId)
    {
    case Commands::Stop::Id:
        return HandleStop();
    case Commands::MoveTo::Id: {
        Commands::MoveTo::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleMoveTo(commandData.position, commandData.latch, commandData.speed);
    }
    case Commands::Calibrate::Id:
        return HandleCalibrate();
    default:
        return Status::UnsupportedCommand;
    }
}

bool ClosureControlCluster::IsSupportedMainState(MainStateEnum mainState) const
{
    switch (mainState)
    {
    case MainStateEnum::kStopped:
    case MainStateEnum::kMoving:
    case MainStateEnum::kWaitingForMotion:
    case MainStateEnum::kError:
    case MainStateEnum::kSetupRequired:
        // Mandatory states are always supported
        return true;
    case MainStateEnum::kCalibrating:
        return mConformance.HasFeature(Feature::kCalibration);
    case MainStateEnum::kProtected:
        return mConformance.HasFeature(Feature::kProtection);
    case MainStateEnum::kDisengaged:
        return mConformance.HasFeature(Feature::kManuallyOperable);
    default:
        return false;
    }
}

bool ClosureControlCluster::IsSupportedOverallCurrentStatePositioning(CurrentPositionEnum positioning) const
{
    switch (positioning)
    {
    case CurrentPositionEnum::kFullyClosed:
    case CurrentPositionEnum::kFullyOpened:
    case CurrentPositionEnum::kPartiallyOpened:
    case CurrentPositionEnum::kOpenedAtSignature:
        // Mandatory positions are always supported
        return true;
    case CurrentPositionEnum::kOpenedForPedestrian:
        return mConformance.HasFeature(Feature::kPedestrian);
    case CurrentPositionEnum::kOpenedForVentilation:
        return mConformance.HasFeature(Feature::kVentilation);
    default:
        return false;
    }
}

bool ClosureControlCluster::IsSupportedOverallTargetStatePositioning(TargetPositionEnum positioning) const
{
    switch (positioning)
    {
    case TargetPositionEnum::kMoveToFullyClosed:
    case TargetPositionEnum::kMoveToFullyOpen:
    case TargetPositionEnum::kMoveToSignaturePosition:
        // Mandatory positions are always supported
        return true;
    case TargetPositionEnum::kMoveToPedestrianPosition:
        return mConformance.HasFeature(Feature::kPedestrian);
    case TargetPositionEnum::kMoveToVentilationPosition:
        return mConformance.HasFeature(Feature::kVentilation);
    default:
        return false;
    }
}

CHIP_ERROR ClosureControlCluster::SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime, bool fromDelegate)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    auto now       = mTimerDelegate.GetCurrentMonotonicTimestamp();
    bool markDirty = false;

    // When fromDelegate=true (delegate updates), we rely on the QuieterReportingAttribute policies
    // to determine if reporting is needed (increment, change to/from zero, null changes).
    // When fromDelegate=false (MainState change), we force reporting since the tracked operation changed.

    auto predicate = [fromDelegate](const decltype(mState.mCountdownTime)::SufficientChangePredicateCandidate &) -> bool {
        // Force reporting when the tracked operation changes due to MainState change
        return !fromDelegate;
    };
    VerifyOrReturnError(mDelegate.OnCountdownTimeChanged(countdownTime), CHIP_ERROR_INCORRECT_STATE);
    markDirty = (mState.mCountdownTime.SetValue(countdownTime, now, predicate) == AttributeDirtyState::kMustReport);

    if (markDirty)
    {
        NotifyAttributeChanged(Attributes::CountdownTime::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::SetMainState(MainStateEnum mainState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(IsSupportedMainState(mainState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mainState != mState.mMainState, CHIP_NO_ERROR);

    // EngageStateChanged event SHALL be generated when the MainStateEnum attribute changes state to and from disengaged state
    if (mState.mMainState == MainStateEnum::kDisengaged)
    {
        ReturnErrorOnFailure(GenerateEngageStateChangedEvent(true));
    }

    if (mainState == MainStateEnum::kDisengaged)
    {
        ReturnErrorOnFailure(GenerateEngageStateChangedEvent(false));
    }

    VerifyOrReturnError(mDelegate.OnMainStateChanged(mainState), CHIP_ERROR_INCORRECT_STATE);
    mState.mMainState = mainState;
    NotifyAttributeChanged(Attributes::MainState::Id);

    if (!mConformance.HasFeature(Feature::kInstantaneous))
    {
        switch (mainState)
        {
        case MainStateEnum::kCalibrating:
            return SetCountdownTimeFromCluster(mDelegate.GetCalibrationCountdownTime());
        case MainStateEnum::kMoving:
            return SetCountdownTimeFromCluster(mDelegate.GetMovingCountdownTime());
        case MainStateEnum::kWaitingForMotion:
            return SetCountdownTimeFromCluster(mDelegate.GetWaitingForMotionCountdownTime());
        default:
            // Reset the countdown time to 0 when the main state is not in motion or calibration.
            return SetCountdownTimeFromCluster(DataModel::Nullable<ElapsedS>(0));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR
ClosureControlCluster::SetOverallCurrentState(const DataModel::Nullable<GenericOverallCurrentState> & overallCurrentState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mState.mOverallCurrentState != overallCurrentState, CHIP_NO_ERROR);

    if (!overallCurrentState.IsNull())
    {
        const GenericOverallCurrentState & incomingOverallCurrentState = overallCurrentState.Value();

        // Validate the incoming Positioning value and FeatureMap conformance.
        if (incomingOverallCurrentState.position.HasValue())
        {
            // If the positioning member is present in the incoming OverallCurrentState, we need to check if the Positioning
            // feature is supported by the closure. If the Positioning feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingOverallCurrentState.position.Value().IsNull())
            {
                VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallCurrentState.position.Value().Value()) !=
                                        CurrentPositionEnum::kUnknownEnumValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
                VerifyOrReturnError(IsSupportedOverallCurrentStatePositioning(incomingOverallCurrentState.position.Value().Value()),
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }
        }

        // Validate the incoming Latch FeatureMap conformance.
        if (incomingOverallCurrentState.latch.HasValue())
        {
            // If the latch member is present in the incoming OverallCurrentState, we need to check if the MotionLatching
            // feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Validate the incoming Speed value and FeatureMap conformance.
        if (incomingOverallCurrentState.speed.HasValue())
        {
            // If the speed member is present in the incoming OverallCurrentState, we need to check if the Speed feature is
            // supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallCurrentState.speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // SecureState can only be true if the closure meets the required conditions for a secure state, preventing unauthorized or
        // undetectable access.
        if (!incomingOverallCurrentState.secureState.IsNull() && incomingOverallCurrentState.secureState.Value())
        {
            // secure state requires the closure to meet all of the following conditions based on feature support:
            // If the Positioning feature is supported, then the Position field of OverallCurrentState is FullyClosed.
            // If the MotionLatching feature is supported, then the Latch field of OverallCurrentState is True.

            if (mConformance.HasFeature(Feature::kPositioning))
            {
                VerifyOrReturnError(incomingOverallCurrentState.position.HasValue() &&
                                        !incomingOverallCurrentState.position.Value().IsNull(),
                                    CHIP_ERROR_INVALID_ARGUMENT);
                VerifyOrReturnError(incomingOverallCurrentState.position.Value().Value() == CurrentPositionEnum::kFullyClosed,
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }

            if (mConformance.HasFeature(Feature::kMotionLatching))
            {
                VerifyOrReturnError(incomingOverallCurrentState.latch.HasValue() &&
                                        !incomingOverallCurrentState.latch.Value().IsNull(),
                                    CHIP_ERROR_INVALID_ARGUMENT);
                VerifyOrReturnError(incomingOverallCurrentState.latch.Value().Value() == true, CHIP_ERROR_INVALID_ARGUMENT);
            }
        }

        // SecureStateChanged event SHALL be generated when the SecureState field in the OverallCurrentState attribute changes
        if (!incomingOverallCurrentState.secureState.IsNull())
        {
            if (mState.mOverallCurrentState.IsNull() || mState.mOverallCurrentState.Value().secureState.IsNull())
            {
                // As secureState field is not set in present current state and incoming current state has value, we generate the
                // event
                ReturnErrorOnFailure(GenerateSecureStateChangedEvent(incomingOverallCurrentState.secureState.Value()));
            }
            else
            {
                // If the secureState field is set in both present and incoming current state, we generate the event only if the
                // value has changed.
                if (mState.mOverallCurrentState.Value().secureState.Value() != incomingOverallCurrentState.secureState.Value())
                {
                    ReturnErrorOnFailure(GenerateSecureStateChangedEvent(incomingOverallCurrentState.secureState.Value()));
                }
            }
        }
    }

    VerifyOrReturnError(mDelegate.OnOverallCurrentStateChanged(overallCurrentState), CHIP_ERROR_INCORRECT_STATE);
    SetAttributeValue(mState.mOverallCurrentState, overallCurrentState, Attributes::OverallCurrentState::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::SetOverallTargetState(const DataModel::Nullable<GenericOverallTargetState> & overallTarget)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mState.mOverallTargetState != overallTarget, CHIP_NO_ERROR);

    if (!overallTarget.IsNull())
    {
        const GenericOverallTargetState & incomingOverallTargetState = overallTarget.Value();

        // Validate the incoming Position value and FeatureMap conformance.
        if (incomingOverallTargetState.position.HasValue())
        {
            // If the position member is present in the incoming OverallTargetState, we need to check if the Position
            // feature is supported by the closure. If the Position feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingOverallTargetState.position.Value().IsNull())
            {
                VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallTargetState.position.Value().Value()) !=
                                        TargetPositionEnum::kUnknownEnumValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
                VerifyOrReturnError(IsSupportedOverallTargetStatePositioning(incomingOverallTargetState.position.Value().Value()),
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }
        }

        // Validate the incoming Latch FeatureMap conformance.
        if (incomingOverallTargetState.latch.HasValue())
        {
            // If the latch member is present in the incoming OverallTargetState, we need to check if the MotionLatching
            // feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Validate the incoming Speed value and FeatureMap conformance.
        if (incomingOverallTargetState.speed.HasValue())
        {
            // If the speed member is present in the incoming OverallTargetState, we need to check if the Speed feature is
            // supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallTargetState.speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    VerifyOrReturnError(mDelegate.OnOverallTargetStateChanged(overallTarget), CHIP_ERROR_INCORRECT_STATE);
    SetAttributeValue(mState.mOverallTargetState, overallTarget, Attributes::OverallTargetState::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::SetLatchControlModes(const BitFlags<LatchControlModesBitmap> & latchControlModes)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    SetAttributeValue(mState.mLatchControlModes, latchControlModes, Attributes::LatchControlModes::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::AddErrorToCurrentErrorList(ClosureErrorEnum error)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(EnsureKnownEnumValue(error) != ClosureErrorEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState.mCurrentErrorCount < kCurrentErrorListMaxSize, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED,
                        ChipLogError(AppServer, "Error list is full"));
    // Check for duplicates
    for (size_t i = 0; i < mState.mCurrentErrorCount; ++i)
    {
        VerifyOrReturnError(mState.mCurrentErrorList[i] != error, CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED,
                            ChipLogError(AppServer, "Error already exists in the list"));
    }
    VerifyOrReturnError(mDelegate.OnCurrentErrorListChanged(
                            DataModel::List<const ClosureErrorEnum>(mState.mCurrentErrorList, mState.mCurrentErrorCount)),
                        CHIP_ERROR_INCORRECT_STATE);
    mState.mCurrentErrorList[mState.mCurrentErrorCount++] = error;
    DataModel::List<const ClosureErrorEnum> currentErrorList(mState.mCurrentErrorList, mState.mCurrentErrorCount);
    NotifyAttributeChanged(Attributes::CurrentErrorList::Id);
    return GenerateOperationalErrorEvent(currentErrorList);
}

void ClosureControlCluster::ClearCurrentErrorList()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturn(mDelegate.OnCurrentErrorListChanged(
        DataModel::List<const ClosureErrorEnum>(mState.mCurrentErrorList, mState.mCurrentErrorCount)));
    // Clearing the error list array by setting all elements to kUnknownEnumValue
    for (size_t i = 0; i < mState.mCurrentErrorCount; ++i)
    {
        mState.mCurrentErrorList[i] = ClosureErrorEnum::kUnknownEnumValue;
    }
    // Reset the current error count to 0
    SetAttributeValue(mState.mCurrentErrorCount, size_t(0), Attributes::CurrentErrorList::Id);
}

// TODO: Move the CountdownTime handling to Delegate
DataModel::Nullable<ElapsedS> ClosureControlCluster::GetCountdownTime() const
{
    VerifyOrReturnValue(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                        DataModel::NullNullable,
                        ChipLogError(AppServer, "Cluster should support Positioning and not Instantaneous feature"));
    return mState.mCountdownTime.value();
}

MainStateEnum ClosureControlCluster::GetMainState() const
{
    return mState.mMainState;
}

DataModel::Nullable<GenericOverallCurrentState> ClosureControlCluster::GetOverallCurrentState() const
{
    return mState.mOverallCurrentState;
}

DataModel::Nullable<GenericOverallTargetState> ClosureControlCluster::GetOverallTargetState() const
{
    return mState.mOverallTargetState;
}

CHIP_ERROR ClosureControlCluster::GetCurrentErrorList(Span<ClosureErrorEnum> & outputSpan)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(outputSpan.size() == kCurrentErrorListMaxSize, CHIP_ERROR_BUFFER_TOO_SMALL,
                        ChipLogError(AppServer, "Output buffer size is not equal to kCurrentErrorListMaxSize"));
    for (size_t i = 0; i < mState.mCurrentErrorCount; ++i)
    {
        outputSpan[i] = mState.mCurrentErrorList[i];
    }
    outputSpan.reduce_size(mState.mCurrentErrorCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::ReadCurrentErrorListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    assertChipStackLockedByCurrentThread();

    for (size_t i = 0; i < mState.mCurrentErrorCount; i++)
    {
        ClosureErrorEnum error = mState.mCurrentErrorList[i];
        // Encode the error
        ReturnErrorOnFailure(encoder.Encode(error));
    }
    return CHIP_NO_ERROR;
}

BitFlags<LatchControlModesBitmap> ClosureControlCluster::GetLatchControlModes() const
{
    VerifyOrReturnValue(mConformance.HasFeature(Feature::kMotionLatching), BitFlags<LatchControlModesBitmap>(),
                        ChipLogError(AppServer, "LatchControlModes feature is not supported"));
    return mState.mLatchControlModes;
}

BitFlags<Feature> ClosureControlCluster::GetFeatureMap() const
{

    return mConformance.FeatureMap();
}

Protocols::InteractionModel::Status ClosureControlCluster::HandleStop()
{

    // Stop command can only be supported if closure doesnt support instantaneous features
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kInstantaneous), Status::UnsupportedCommand);

    MainStateEnum state = GetMainState();

    // Stop action is supported only if the closure is in one of the following states Moving, WaitingForMotion or Calibrating.
    // A status code of SUCCESS SHALL always be returned, regardless if it is in above states or not.
    if ((state == MainStateEnum::kCalibrating) || (state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion))
    {
        // Set the MainState to 'Stopped' only if the delegate call to HandleStopCommand is successful.
        Status status = mDelegate.HandleStopCommand();
        VerifyOrReturnValue(status == Status::Success, status);

        VerifyOrReturnError(SetMainState(MainStateEnum::kStopped) == CHIP_NO_ERROR, Status::Failure,
                            ChipLogError(AppServer, "Stop Command: Failed to set MainState to Stopped"));
    }

    return Status::Success;
}

Protocols::InteractionModel::Status ClosureControlCluster::HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                                        Optional<Globals::ThreeLevelAutoEnum> speed)
{

    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState = GetOverallCurrentState();
    DataModel::Nullable<GenericOverallTargetState> overallTargetState   = GetOverallTargetState();
    VerifyOrReturnError(!overallCurrentState.IsNull(), Status::InvalidInState,
                        ChipLogError(AppServer, "OverallCurrentState is null on endpoint : %d", GetEndpointId()));

    if (overallTargetState.IsNull())
    {
        // If overallTargetState is null, we need to initialize to default value.
        // This is to ensure that we can set the position, latch, and speed values in the overallTargetState.
        overallTargetState.SetNonNull(GenericOverallTargetState{});
    }

    if (position.HasValue() && mConformance.HasFeature(Feature::kPositioning))
    {
        VerifyOrReturnError(position.Value() != TargetPositionEnum::kUnknownEnumValue, Status::ConstraintError);

        overallTargetState.Value().position.SetValue(DataModel::MakeNullable(position.Value()));
    }

    if (latch.HasValue() && mConformance.HasFeature(Feature::kMotionLatching))
    {
        // If latch value is true and the Remote Latching feature is not supported, or
        // if latch value is false and the Remote Unlatching feature is not supported, return InvalidInState.
        if ((latch.Value() && !mState.mLatchControlModes.Has(LatchControlModesBitmap::kRemoteLatching)) ||
            (!latch.Value() && !mState.mLatchControlModes.Has(LatchControlModesBitmap::kRemoteUnlatching)))
        {
            return Status::InvalidInState;
        }

        overallTargetState.Value().latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    if (speed.HasValue() && mConformance.HasFeature(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);

        overallTargetState.Value().speed.SetValue(speed.Value());
    }

    MainStateEnum state = GetMainState();

    // If the MoveTo command is received in any state other than 'Moving', 'WaitingForMotion', or 'Stopped', an error code
    // INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion ||
                            state == MainStateEnum::kStopped,
                        Status::InvalidInState);

    if (mConformance.HasFeature(Feature::kMotionLatching))
    {
        // If this command requests a position change while the Latch field of the OverallCurrentState is True (Latched), and the
        // Latch field of this command is not set to False (Unlatched), a status code of INVALID_IN_STATE SHALL be returned.
        if (position.HasValue() && overallCurrentState.Value().latch.HasValue() &&
            !overallCurrentState.Value().latch.Value().IsNull() && overallCurrentState.Value().latch.Value().Value())
        {
            VerifyOrReturnError(latch.HasValue() && !latch.Value(), Status::InvalidInState,
                                ChipLogError(AppServer,
                                             "Latch is True in OverallCurrentState, but MoveTo command does not set latch to False "
                                             "when position change is requested on endpoint : %d",
                                             GetEndpointId()));
        }
    }

    // Set MainState and OverallTargetState only if the delegate call to HandleMoveToCommand is successful
    Status status = mDelegate.HandleMoveToCommand(position, latch, speed);
    VerifyOrReturnValue(status == Status::Success, status);

    if (mDelegate.IsReadyToMove())
    {
        VerifyOrReturnError(SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                            ChipLogError(AppServer, "MoveTo Command: Failed to set MainState to Moving"));
    }
    else
    {
        VerifyOrReturnError(SetMainState(MainStateEnum::kWaitingForMotion) == CHIP_NO_ERROR, Status::Failure,
                            ChipLogError(AppServer, "MoveTo Command: Failed to set MainState to kWaitingForMotion"));
    }

    VerifyOrReturnError(SetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure);

    return Status::Success;
}

Protocols::InteractionModel::Status ClosureControlCluster::HandleCalibrate()
{
    VerifyOrReturnError(mConformance.HasFeature(Feature::kCalibration), Status::UnsupportedCommand);

    MainStateEnum state = GetMainState();

    // If Calibrate command is received when already in the Calibrating state,
    // the server SHALL respond with a status code of SUCCESS.
    VerifyOrReturnValue(state != MainStateEnum::kCalibrating, Status::Success);

    // If the Calibrate command is invoked in any state other than Stopped or SetupRequired,
    // the server SHALL respond with INVALID_IN_STATE and there SHALL be no other effect.
    // This check excludes the 'Calibrating' MainState as it is already validated above
    VerifyOrReturnError(state == MainStateEnum::kStopped || state == MainStateEnum::kSetupRequired, Status::InvalidInState);

    // Set the MainState to 'Calibrating' only if the delegate call to HandleCalibrateCommand is successful
    Status status = mDelegate.HandleCalibrateCommand();
    VerifyOrReturnValue(status == Status::Success, status);

    VerifyOrReturnError(SetMainState(MainStateEnum::kCalibrating) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Calibrate Command: Failed to set MainState to Calibrating"));

    return Status::Success;
}

CHIP_ERROR ClosureControlCluster::GenerateOperationalErrorEvent(const DataModel::List<const ClosureErrorEnum> & errorState)
{
    ReturnErrorOnFailure(SetMainState(MainStateEnum::kError));
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Events::OperationalError::Type event{ .errorState = errorState };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::GenerateMovementCompletedEvent()
{
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kInstantaneous), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Events::MovementCompleted::Type event{};
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::GenerateEngageStateChangedEvent(const bool engageValue)
{
    VerifyOrReturnError(mConformance.HasFeature(Feature::kManuallyOperable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Events::EngageStateChanged::Type event{ .engageValue = engageValue };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureControlCluster::GenerateSecureStateChangedEvent(const bool secureValue)
{
    VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Events::SecureStateChanged::Type event{ .secureValue = secureValue };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, GetEndpointId());

    return CHIP_NO_ERROR;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
