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

namespace {
constexpr uint8_t kCurrentErrorListSize = 10;
} // namespace

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
    ReturnErrorOnFailure(SetOverallState(initParams.mOverallState));

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

bool ClusterLogic::IsValidMainStateTransition(MainStateEnum mainState) const
{
    // TODO: Implement the MainState state machine to validate transitions
    return true;
}

bool ClusterLogic::IsSupportedOverallStatePositioning(PositioningEnum positioning) const
{
    bool isSupported = false;

    switch (positioning)
    {
    case PositioningEnum::kFullyClosed:
    case PositioningEnum::kFullyOpened:
    case PositioningEnum::kPartiallyOpened:
    case PositioningEnum::kOpenedAtSignature:
        // Mandatory states are always supported
        isSupported = true;
        break;

    case PositioningEnum::kOpenedForPedestrian:
        isSupported = mConformance.HasFeature(Feature::kPedestrian);
        break;

    case PositioningEnum::kOpenedForVentilation:
        isSupported = mConformance.HasFeature(Feature::kVentilation);
        break;

    default:
        isSupported = false;
        break;
    }

    return isSupported;
}

bool ClusterLogic::IsSupportedOverallTargetPositioning(TargetPositionEnum positioning) const
{
    bool isSupported = false;

    switch (positioning)
    {
    case TargetPositionEnum::kCloseInFull:
    case TargetPositionEnum::kOpenInFull:
    case TargetPositionEnum::kSignature:
        // Mandatory states are always supported
        isSupported = true;
        break;

    case TargetPositionEnum::kPedestrian:
        isSupported = mConformance.HasFeature(Feature::kPedestrian);
        break;

    case TargetPositionEnum::kVentilation:
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
    VerifyOrReturnError(IsValidMainStateTransition(mainState), CHIP_ERROR_INCORRECT_STATE);
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

CHIP_ERROR ClusterLogic::SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState.mOverallState != overallState, CHIP_NO_ERROR);

    if (!overallState.IsNull())
    {
        const GenericOverallState & incomingOverallState = overallState.Value();

        // Validate the incoming Positioning value and FeatureMap conformance.
        if (incomingOverallState.positioning.HasValue())
        {
            // If the positioning member is present in the incoming OverallState, we need to check if the Positioning
            // feature is supported by the closure. If the Positioning feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingOverallState.positioning.Value().IsNull())
            {
                VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallState.positioning.Value().Value()) !=
                                        PositioningEnum::kUnknownEnumValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
                VerifyOrReturnError(IsSupportedOverallStatePositioning(incomingOverallState.positioning.Value().Value()),
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }
        }

        // Validate the incoming Latch FeatureMap conformance.
        if (incomingOverallState.latch.HasValue())
        {
            // If the latch member is present in the incoming OverallState, we need to check if the MotionLatching
            // feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Validate the incoming Speed value and FeatureMap conformance.
        if (incomingOverallState.speed.HasValue())
        {
            // If the speed member is present in the incoming OverallState, we need to check if the Speed feature is
            // supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            if (!incomingOverallState.speed.Value().IsNull())
            {
                VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallState.speed.Value().Value()) !=
                                        Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                    CHIP_ERROR_INVALID_ARGUMENT);
            }
        }

        // Validate the incoming SecureState FeatureMap conformance.
        if (incomingOverallState.secureState.HasValue())
        {
            // If the secureState member is present in the OverallState, we need to check if the Speed feature is
            // supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) || mConformance.HasFeature(Feature::kMotionLatching),
                                CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }
    }

    mState.mOverallState = overallState;
    mMatterContext.MarkDirty(Attributes::OverallState::Id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverallTarget(const DataModel::Nullable<GenericOverallTarget> & overallTarget)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState.mOverallTarget != overallTarget, CHIP_NO_ERROR);

    if (!overallTarget.IsNull())
    {
        const GenericOverallTarget & incomingOverallTarget = overallTarget.Value();

        // Validate the incoming Position value and FeatureMap conformance.
        if (incomingOverallTarget.position.HasValue())
        {
            // If the position member is present in the incoming OverallTarget, we need to check if the Position
            // feature is supported by the closure. If the Position feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallTarget.position.Value()) !=
                                    TargetPositionEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);

            VerifyOrReturnError(IsSupportedOverallTargetPositioning(incomingOverallTarget.position.Value()),
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // Validate the incoming Latch FeatureMap conformance.
        if (incomingOverallTarget.latch.HasValue())
        {
            // If the latch member is present in the incoming OverallTarget, we need to check if the MotionLatching
            // feature is supported by the closure. If the MotionLatching feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        }

        // Validate the incoming Speed value and FeatureMap conformance.
        if (incomingOverallTarget.speed.HasValue())
        {
            // If the speed member is present in the incoming OverallTarget, we need to check if the Speed feature is
            // supported by the closure. If the Speed feature is not supported, return an error.
            VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

            VerifyOrReturnError(EnsureKnownEnumValue(incomingOverallTarget.speed.Value()) !=
                                    Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

    mState.mOverallTarget = overallTarget;
    mMatterContext.MarkDirty(Attributes::OverallTarget::Id);

    return CHIP_NO_ERROR;
}

// TODO: Move the CountdownTime handling to Delegate
CHIP_ERROR ClusterLogic::GetCountdownTime(DataModel::Nullable<ElapsedS> & countdownTime)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

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

CHIP_ERROR ClusterLogic::GetOverallState(DataModel::Nullable<GenericOverallState> & overallState)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    overallState = mState.mOverallState;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetOverallTarget(DataModel::Nullable<GenericOverallTarget> & overallTarget)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    overallTarget = mState.mOverallTarget;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetCurrentErrorList(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    // List can contain at most only 10 Error
    for (size_t i = 0; i < kCurrentErrorListSize; i++)
    {
        ClosureErrorEnum error;

        CHIP_ERROR err = mDelegate.GetCurrentErrorAtIndex(i, error);

        // Convert CHIP_ERROR_PROVIDER_LIST_EXHAUSTED to CHIP_NO_ERROR
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        // Return for other errors occurred apart from CHIP_ERROR_PROVIDER_LIST_EXHAUSTED
        ReturnErrorOnFailure(err);

        // Encode the error
        ReturnErrorOnFailure(encoder.Encode(error));
    }

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
        // Set the MainState to 'Stopped' only if the delegate call to HandleMoveToCommand is successful.
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

    GenericOverallTarget target;

    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    if (position.HasValue())
    {
        VerifyOrReturnError(position.Value() != TargetPositionEnum::kUnknownEnumValue, Status::ConstraintError);

        if (mConformance.HasFeature(Feature::kPositioning))
        {
            target.position = position;
        }
    }

    if (latch.HasValue() && mConformance.HasFeature(Feature::kMotionLatching))
    {
        // If manual intervention is required to latch, respond with INVALID_IN_STATE
        if (mDelegate.IsManualLatchingNeeded())
        {
            return Status::InvalidInState;
        }

        target.latch = latch;
    }

    if (speed.HasValue())
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);

        if (mConformance.HasFeature(Feature::kSpeed))
        {
            target.speed = speed;
        }
    }

    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR, Status::Failure);

    // If the MoveTo command is received in any state other than 'Moving', 'WaitingForMotion', or 'Stopped', an error code
    // INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion ||
                            state == MainStateEnum::kStopped,
                        Status::InvalidInState);

    // Set MainState and OverallTarget only if the delegate call to HandleMoveToCommand is successful
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

    VerifyOrReturnError(SetOverallTarget(DataModel::MakeNullable(target)) == CHIP_NO_ERROR, Status::Failure);

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

    // If the Calibrate command is invoked in any state other than 'Stopped', the server shall respond with INVALID_IN_STATE.
    // This check excludes the 'Calibrating' MainState as it is already validated above
    VerifyOrReturnError(state == MainStateEnum::kStopped, Status::InvalidInState);

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
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

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
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    Events::SecureStateChanged::Type event{ .secureValue = secureValue };
    ReturnErrorOnFailure(mMatterContext.GenerateEvent(event));

    return CHIP_NO_ERROR;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
