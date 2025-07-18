/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
#include <clusters/ClosureControl/Metadata.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/LockTracker.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

using namespace Protocols::InteractionModel;

/*
    ClusterLogic Implementation
*/

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance, const ClusterInitParameters & initParams)
{
    VerifyOrReturnError(!mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(conformance.Valid(), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);

    mConformance   = conformance;
    mIsInitialized = true;

    ReturnErrorOnFailure(SetMainState(initParams.mMainState));
    ReturnErrorOnFailure(SetOverallCurrentState(initParams.mOverallCurrentState));

    return CHIP_NO_ERROR;
}

bool ClusterLogic::IsSupportedMainState(MainStateEnum mainState) const
{
    bool isSupported = false;

    switch (mainState)
    {
    case MainStateEnum::kStopped:
    case MainStateEnum::kMoving:
    case MainStateEnum::kWaitingForMotion:
    case MainStateEnum::kError:
    case MainStateEnum::kSetupRequired:
        // Mandatory states are always supported
        isSupported = true;
        break;

    case MainStateEnum::kCalibrating:
        isSupported = mConformance.HasFeature(Feature::kCalibration);
        break;

    case MainStateEnum::kProtected:
        isSupported = mConformance.HasFeature(Feature::kProtection);
        break;

    case MainStateEnum::kDisengaged:
        // Disengaged requires the ManuallyOperable feature
        isSupported = mConformance.HasFeature(Feature::kManuallyOperable);
        break;

    default:
        isSupported = false;
        break;
    }

    return isSupported;
}

bool ClusterLogic::IsSupportedOverallCurrentStatePositioning(CurrentPositionEnum positioning) const
{
    bool isSupported = false;

    switch (positioning)
    {
    case CurrentPositionEnum::kFullyClosed:
    case CurrentPositionEnum::kFullyOpened:
    case CurrentPositionEnum::kPartiallyOpened:
    case CurrentPositionEnum::kOpenedAtSignature:
        // Mandatory states are always supported
        isSupported = true;
        break;

    case CurrentPositionEnum::kOpenedForPedestrian:
        isSupported = mConformance.HasFeature(Feature::kPedestrian);
        break;

    case CurrentPositionEnum::kOpenedForVentilation:
        isSupported = mConformance.HasFeature(Feature::kVentilation);
        break;

    default:
        isSupported = false;
        break;
    }

    return isSupported;
}

bool ClusterLogic::IsSupportedOverallTargetStatePositioning(TargetPositionEnum positioning) const
{
    bool isSupported = false;

    switch (positioning)
    {
    case TargetPositionEnum::kMoveToFullyClosed:
    case TargetPositionEnum::kMoveToFullyOpen:
    case TargetPositionEnum::kMoveToSignaturePosition:
        // Mandatory states are always supported
        isSupported = true;
        break;

    case TargetPositionEnum::kMoveToPedestrianPosition:
        isSupported = mConformance.HasFeature(Feature::kPedestrian);
        break;

    case TargetPositionEnum::kMoveToVentilationPosition:
        isSupported = mConformance.HasFeature(Feature::kVentilation);
        break;

    default:
        isSupported = false;
        break;
    }

    return isSupported;
}

CHIP_ERROR ClusterLogic::SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime, bool fromDelegate)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    auto now       = System::SystemClock().GetMonotonicTimestamp();
    bool markDirty = false;

    // TODO: Delegate specific handling logic will be added if needed after after spec issue resolution.
    //       https://github.com/CHIP-Specifications/connectedhomeip-spec/issues/11603

    auto predicate = [](const decltype(mState.mCountdownTime)::SufficientChangePredicateCandidate &) -> bool { return true; };
    markDirty      = (mState.mCountdownTime.SetValue(countdownTime, now, predicate) == AttributeDirtyState::kMustReport);

    if (markDirty)
    {
        mMatterContext.MarkDirty(Attributes::CountdownTime::Id);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetMainState(MainStateEnum mainState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsSupportedMainState(mainState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(mainState != mState.mMainState, CHIP_NO_ERROR);

    // EngageStateChanged event SHALL be generated when the MainStateEnum attribute changes state to and from disengaged state
    if (mState.mMainState == MainStateEnum::kDisengaged)
    {
        GenerateEngageStateChangedEvent(true);
    }

    if (mainState == MainStateEnum::kDisengaged)
    {
        GenerateEngageStateChangedEvent(false);
    }

    mState.mMainState = mainState;
    mMatterContext.MarkDirty(Attributes::MainState::Id);

    if (!mConformance.HasFeature(Feature::kInstantaneous))
    {
        if (mainState == MainStateEnum::kCalibrating)
        {
            SetCountdownTimeFromCluster(mDelegate.GetCalibrationCountdownTime());
        }
        else if (mainState == MainStateEnum::kMoving)
        {
            SetCountdownTimeFromCluster(mDelegate.GetMovingCountdownTime());
        }
        else if (mainState == MainStateEnum::kWaitingForMotion)
        {
            SetCountdownTimeFromCluster(mDelegate.GetWaitingForMotionCountdownTime());
        }
        else
        {
            // Reset the countdown time to 0 when the main state is not in motion or calibration.
            SetCountdownTimeFromCluster(DataModel::Nullable<ElapsedS>(0));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverallCurrentState(const DataModel::Nullable<GenericOverallCurrentState> & overallCurrentState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
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

        // TODO: SecureState field Value based on conditions validation will be done after the specification issue #11805
        // resolution.

        // SecureStateChanged event SHALL be generated when the SecureState field in the OverallCurrentState attribute changes
        if (!incomingOverallCurrentState.secureState.IsNull())
        {
            if (mState.mOverallCurrentState.IsNull() || mState.mOverallCurrentState.Value().secureState.IsNull())
            {
                // As secureState field is not set in present current state and incoming current state has value, we generate the
                // event
                GenerateSecureStateChangedEvent(incomingOverallCurrentState.secureState.Value());
            }
            else
            {
                // If the secureState field is set in both present and incoming current state, we generate the event only if the
                // value has changed.
                if (mState.mOverallCurrentState.Value().secureState.Value() != incomingOverallCurrentState.secureState.Value())
                {
                    GenerateSecureStateChangedEvent(incomingOverallCurrentState.secureState.Value());
                }
            }
        }
    }

    mState.mOverallCurrentState = overallCurrentState;
    mMatterContext.MarkDirty(Attributes::OverallCurrentState::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverallTargetState(const DataModel::Nullable<GenericOverallTargetState> & overallTarget)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
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

    mState.mOverallTargetState = overallTarget;
    mMatterContext.MarkDirty(Attributes::OverallTargetState::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetLatchControlModes(const BitFlags<LatchControlModesBitmap> & latchControlModes)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    mState.mLatchControlModes = latchControlModes;
    mMatterContext.MarkDirty(Attributes::LatchControlModes::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::AddErrorToCurrentErrorList(ClosureErrorEnum error)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(EnsureKnownEnumValue(error) != ClosureErrorEnum::kUnknownEnumValue, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mState.mCurrentErrorCount < kCurrentErrorListMaxSize, CHIP_ERROR_PROVIDER_LIST_EXHAUSTED,
                        ChipLogError(AppServer, "Error list is full"));
    // Check for duplicates
    for (size_t i = 0; i < mState.mCurrentErrorCount; ++i)
    {
        VerifyOrReturnError(mState.mCurrentErrorList[i] != error, CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED,
                            ChipLogError(AppServer, "Error already exists in the list"));
    }
    mState.mCurrentErrorList[mState.mCurrentErrorCount++] = error;
    DataModel::List<const ClosureErrorEnum> currentErrorList(mState.mCurrentErrorList, mState.mCurrentErrorCount);
    mMatterContext.MarkDirty(Attributes::CurrentErrorList::Id);
    ReturnLogErrorOnFailure(GenerateOperationalErrorEvent(currentErrorList));
    return CHIP_NO_ERROR;
}

void ClusterLogic::ClearCurrentErrorList()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "ClearCurrentErrorList called before Initialization of closure");
    // Clearing the error list array by setting all elements to kUnknownEnumValue
    for (size_t i = 0; i < mState.mCurrentErrorCount; ++i)
    {
        mState.mCurrentErrorList[i] = ClosureErrorEnum::kUnknownEnumValue;
    }
    // Reset the current error count to 0
    mState.mCurrentErrorCount = 0;
    mMatterContext.MarkDirty(Attributes::CurrentErrorList::Id);
}

// TODO: Move the CountdownTime handling to Delegate
CHIP_ERROR ClusterLogic::GetCountdownTime(DataModel::Nullable<ElapsedS> & countdownTime)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    countdownTime = mState.mCountdownTime.value();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetMainState(MainStateEnum & mainState)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    mainState = mState.mMainState;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetOverallCurrentState(DataModel::Nullable<GenericOverallCurrentState> & overallCurrentState)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    overallCurrentState = mState.mOverallCurrentState;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetOverallTargetState(DataModel::Nullable<GenericOverallTargetState> & overallTarget)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    overallTarget = mState.mOverallTargetState;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetCurrentErrorList(Span<ClosureErrorEnum> & outputSpan)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(outputSpan.size() == kCurrentErrorListMaxSize, CHIP_ERROR_BUFFER_TOO_SMALL,
                        ChipLogError(AppServer, "Output buffer size is not equal to kCurrentErrorListMaxSize"));
    for (size_t i = 0; i < mState.mCurrentErrorCount; ++i)
    {
        outputSpan[i] = mState.mCurrentErrorList[i];
    }
    outputSpan.reduce_size(mState.mCurrentErrorCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::ReadCurrentErrorListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    for (size_t i = 0; i < mState.mCurrentErrorCount; i++)
    {
        ClosureErrorEnum error = mState.mCurrentErrorList[i];
        // Encode the error
        ReturnErrorOnFailure(encoder.Encode(error));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetLatchControlModes(BitFlags<LatchControlModesBitmap> & latchControlModes)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    latchControlModes = mState.mLatchControlModes;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetFeatureMap(BitFlags<Feature> & featureMap)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    featureMap = mConformance.FeatureMap();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision)
{
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    clusterRevision = ClosureControl::kRevision;
    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status ClusterLogic::HandleStop()
{
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "Stop Command called before Initialization of closure");

    // Stop command can only be supported if closure doesnt support instantaneous features
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kInstantaneous), Status::UnsupportedCommand);

    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR, Status::Failure);

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

Protocols::InteractionModel::Status ClusterLogic::HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                               Optional<Globals::ThreeLevelAutoEnum> speed)
{
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "MoveTo Command called before Initialization of closure");

    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState;
    DataModel::Nullable<GenericOverallTargetState> overallTargetState;
    VerifyOrReturnError(GetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnError(GetOverallCurrentState(overallCurrentState) == CHIP_NO_ERROR, Status::Failure);
    VerifyOrReturnError(!overallCurrentState.IsNull(), Status::InvalidInState,
                        ChipLogError(AppServer, "OverallCurrentState is null on endpoint : %d", mMatterContext.GetEndpointId()));

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

    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR, Status::Failure);

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
                                             mMatterContext.GetEndpointId()));
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

Protocols::InteractionModel::Status ClusterLogic::HandleCalibrate()
{
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "Calibrate Command called before Initialization of closure");

    VerifyOrReturnError(mConformance.HasFeature(Feature::kCalibration), Status::UnsupportedCommand);

    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR, Status::Failure);

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

CHIP_ERROR ClusterLogic::GenerateOperationalErrorEvent(const DataModel::List<const ClosureErrorEnum> & errorState)
{
    ReturnErrorOnFailure(SetMainState(MainStateEnum::kError));

    Events::OperationalError::Type event{ .errorState = errorState };
    ReturnErrorOnFailure(mMatterContext.GenerateEvent(event));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GenerateMovementCompletedEvent()
{
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kInstantaneous), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    Events::MovementCompleted::Type event{};
    ReturnErrorOnFailure(mMatterContext.GenerateEvent(event));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GenerateEngageStateChangedEvent(const bool engageValue)
{
    VerifyOrReturnError(mConformance.HasFeature(Feature::kManuallyOperable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    Events::EngageStateChanged::Type event{ .engageValue = engageValue };
    ReturnErrorOnFailure(mMatterContext.GenerateEvent(event));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GenerateSecureStateChangedEvent(const bool secureValue)
{
    Events::SecureStateChanged::Type event{ .secureValue = secureValue };
    ReturnErrorOnFailure(mMatterContext.GenerateEvent(event));

    return CHIP_NO_ERROR;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
