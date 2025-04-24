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
#include <platform/LockTracker.h>
#include <platform/CHIPDeviceLayer.h>
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
    
    mConformance = conformance;
    mIsInitialized = true;
    
    ReturnErrorOnFailure(SetMainState(initParams.mMainState));
    ReturnErrorOnFailure(SetOverallState(initParams.mOverallState));
    
    return CHIP_NO_ERROR;
}

bool ClusterLogic::IsSupportedMainState(MainStateEnum mainState)
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


bool ClusterLogic::IsValidMainStateTransition(MainStateEnum mainState)
{
    //TODO: Implement the MainState state machin to validate transitions
    return true;
}

bool ClusterLogic::IsSupportedOverallStatePositioning(PositioningEnum positioning)
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

bool ClusterLogic::IsSupportedOverallTargetPositioning(TargetPositionEnum positioning)
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

    if (fromDelegate)
    {
        // Updates from delegate are reduce-reported to every 1s max (choice of this implementation), in addition
        // to default change-from-null, change-from-zero and increment policy.
        System::Clock::Milliseconds64 reportInterval = System::Clock::Milliseconds64(1000);
        auto predicate = mState.mCountdownTime.GetPredicateForSufficientTimeSinceLastDirty(reportInterval);
        markDirty      = (mState.mCountdownTime.SetValue(countdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }
    else
    {
        auto predicate = [](const decltype(mState.mCountdownTime)::SufficientChangePredicateCandidate &) -> bool { return true; };
        markDirty      = (mState.mCountdownTime.SetValue(countdownTime, now, predicate) == AttributeDirtyState::kMustReport);
    }

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
        PostEngageStateChangedEvent(true);
    }

    if (mainState == MainStateEnum::kDisengaged)
    { 
        PostEngageStateChangedEvent(false);
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
    }
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState.mOverallState != overallState, CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> & currentOverallState = mState.mOverallState;

    bool validateClusterOverallStateMembers     = false;
    bool validateClusterOverallStatePositioning = false;
    bool validateClusterOverallStateLatch       = false;
    bool validateClusterOverallStateSpeed       = false;
    bool validateClusterOverallStateSecureState = false;

    bool validateIncomingOverallStateMembers     = false;
    bool validateIncomingOverallStatePositioning = false;
    bool validateIncomingOverallStateLatch       = false;
    bool validateIncomingOverallStateSpeed       = false;
    bool validateIncomingOverallStateSecureState = false;

    bool requirePositioningUpdate  = false;
    bool requireLatchUpdate        = false;
    bool requireSpeedUpdate        = false;
    bool requiredSecureStateUpdate = false;

    /*
        Determine checks that need to be executed based on current and provided values
    */

    // Determine OverallState member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!currentOverallState.IsNull())
    {

        // If the OverallState has a value, we need to check internal members to validate if we need to mark the attribute as dirty.
        validateClusterOverallStateMembers = true;

        validateClusterOverallStatePositioning = currentOverallState.Value().positioning.HasValue();
        validateClusterOverallStateLatch       = currentOverallState.Value().latch.HasValue();
        validateClusterOverallStateSpeed       = currentOverallState.Value().speed.HasValue();
        validateClusterOverallStateSecureState = currentOverallState.Value().secureState.HasValue();
    }

    // Determine OverallState member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!overallState.IsNull())
    {
        // If the provided OverallState has a value, we need to check its members to validate
        validateIncomingOverallStateMembers = true;

        validateIncomingOverallStatePositioning = overallState.Value().positioning.HasValue();
        validateIncomingOverallStateLatch       = overallState.Value().latch.HasValue();
        validateIncomingOverallStateSpeed       = overallState.Value().speed.HasValue();
        validateIncomingOverallStateSecureState = overallState.Value().secureState.HasValue();
    }

    /*
        Validate incoming values, featureMap conformance and if update is necessary
    */

    // Validate the incomging Positioning value and featureMap conformance.
    if (validateIncomingOverallStatePositioning)
    {
        // If the positioning member is present in the incoming OverallState, we need to check if the Positioning
        // feature is supported by the device. If the Positioning feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        
        // We don't need to check is values are present since the check was done above.
        const DataModel::Nullable<PositioningEnum> & positioning = overallState.Value().positioning.Value();

        if (!positioning.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(positioning.Value()) != PositioningEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(IsSupportedOverallStatePositioning(positioning.Value()), CHIP_ERROR_INVALID_ARGUMENT);
        }

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requirePositioningUpdate =
            !validateClusterOverallStatePositioning || currentOverallState.Value().positioning.Value() != positioning;
    }

    // Validate the incomging Latch value and featureMap conformance.
    if (validateIncomingOverallStateLatch)
    {
        // If the latching member is present in the incoming OverallState, we need to check if the MotionLatching
        // feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        
        // We don't need to check is values are present since the check was done above.
        const DataModel::Nullable<bool> & latch = overallState.Value().latch.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterOverallStateLatch || currentOverallState.Value().latch.Value() != latch;
    }

    // Validate the incomging Speed value and featureMap conformance.
    if (validateIncomingOverallStateSpeed)
    {
        // If the speed member is present in the incoming OverallState, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        
        const DataModel::Nullable<Globals::ThreeLevelAutoEnum> & speed = overallState.Value().speed.Value();

        if (!speed.IsNull())
        {
            VerifyOrReturnError(EnsureKnownEnumValue(speed.Value()) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireSpeedUpdate = !validateClusterOverallStateSpeed || currentOverallState.Value().speed.Value() != speed;
    }

    // Validate the incomging SecureState value and featureMap conformance.
    if (validateIncomingOverallStateSecureState)
    {
        // If the secureState member is present in the OverallState, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) || mConformance.HasFeature(Feature::kMotionLatching), 
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
                            
        // We don't need to check is values are present since the check was done above.
        const DataModel::Nullable<bool> & secureState = overallState.Value().secureState.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requiredSecureStateUpdate = !validateClusterOverallStateSecureState || currentOverallState.Value().secureState.Value() != secureState;
    }

    /*
        Update the cluster state and mark it as dirty if necessary
    */

    // Cluster OverallState going from non-null to null
    if (validateClusterOverallStateMembers && !validateIncomingOverallStateMembers)
    {
        currentOverallState.SetNull();
        mMatterContext.MarkDirty(Attributes::OverallState::Id);
    }
    else if (requirePositioningUpdate || requireLatchUpdate || requireSpeedUpdate || requiredSecureStateUpdate)
    {
        currentOverallState.SetNonNull(overallState.Value());
        mMatterContext.MarkDirty(Attributes::OverallState::Id);
        
        //SecureStateChangedEvent SHALL be generated when the SecureState field in the OverallState attribute changes
        if (requiredSecureStateUpdate) {
            PostSecureStateChangedEvent(overallState.Value().secureState.Value().Value());
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverallTarget(const DataModel::Nullable<GenericOverallTarget> & overallTarget)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState.mOverallTarget != overallTarget, CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallTarget> & currentOverallTarget = mState.mOverallTarget;

    bool validateClusterOverallTargetMembers  = false;
    bool validateClusterOverallTargetPosition = false;
    bool validateClusterOverallTargetLatch    = false;
    bool validateClusterOverallTargetSpeed    = false;

    bool validateIncomingOverallTargetMembers  = false;
    bool validateIncomingOverallTargetPosition = false;
    bool validateIncomingOverallTargetLatch    = false;
    bool validateIncomingOverallTargetSpeed    = false;

    bool requirePositioningUpdate  = false;
    bool requireLatchUpdate        = false;
    bool requireSpeedUpdate        = false;

    /*
        Determine checks that need to be executed based on current and provided values
    */

    // Determine OverallTarget member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!currentOverallTarget.IsNull())
    {

        // If the OverallTarget has a value, we need to check internal members to validate if we need to mark the attribute as dirty.
        validateClusterOverallTargetMembers = true;

        validateClusterOverallTargetPosition = currentOverallTarget.Value().position.HasValue();
        validateClusterOverallTargetLatch    = currentOverallTarget.Value().latch.HasValue();
        validateClusterOverallTargetSpeed    = currentOverallTarget.Value().speed.HasValue();
    }

    // Determine OverallTarget member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!overallTarget.IsNull())
    {
        // If the provided OverallTarget has a value, we need to check its members to validate
        validateIncomingOverallTargetMembers = true;

        validateIncomingOverallTargetPosition = overallTarget.Value().position.HasValue();
        validateIncomingOverallTargetLatch    = overallTarget.Value().latch.HasValue();
        validateIncomingOverallTargetSpeed    = overallTarget.Value().speed.HasValue();
    }

    /*
        Validate incoming values, featureMap conformance and if update is necessary
    */

    // Validate the incoming Position value and FeatureMap conformance.
    if (validateIncomingOverallTargetPosition)
    {
        // If the position member is present in the incoming OverallTarget, we need to check if the Position
        // feature is supported by the device. If the Position feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        
        // We don't need to check is values are present since the check was done above.
        const TargetPositionEnum & position = overallTarget.Value().position.Value();
        
        VerifyOrReturnError(EnsureKnownEnumValue(position) != TargetPositionEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(IsSupportedOverallTargetPositioning(position), CHIP_ERROR_INVALID_ARGUMENT);
        
        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requirePositioningUpdate =
            !validateClusterOverallTargetPosition || currentOverallTarget.Value().position.Value() != position;
    }

    // Validate the incoming Latch value and FeatureMap conformance.
    if (validateIncomingOverallTargetLatch)
    {
        // If the latching member is present in the incoming OverallTarget, we need to check if the MotionLatching
        // feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        
        // We don't need to check is values are present since the check was done above.
        const bool & latch = overallTarget.Value().latch.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterOverallTargetLatch || currentOverallTarget.Value().latch.Value() != latch;
    }

    // Validate the incoming Speed value and FeatureMap conformance.
    if (validateIncomingOverallTargetSpeed)
    {
        // If the speed member is present in the incoming OverallTarget, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        
        const Globals::ThreeLevelAutoEnum & speed = overallTarget.Value().speed.Value();
        
        VerifyOrReturnError(EnsureKnownEnumValue(speed) != Globals::ThreeLevelAutoEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireSpeedUpdate = !validateClusterOverallTargetSpeed || currentOverallTarget.Value().speed.Value() != speed;
    }

    /*
        Update the cluster state and mark it as dirty if necessary
    */

    // Cluster OverallTarget going from non-null to null
    if (validateClusterOverallTargetMembers && !validateIncomingOverallTargetMembers)
    {
        currentOverallTarget.SetNull();
        mMatterContext.MarkDirty(Attributes::OverallTarget::Id);
    }
    else if (requirePositioningUpdate || requireLatchUpdate || requireSpeedUpdate)
    {
        currentOverallTarget.SetNonNull(overallTarget.Value());
        mMatterContext.MarkDirty(Attributes::OverallTarget::Id);
    }

    return CHIP_NO_ERROR;
}

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

    if (mState.mOverallState.IsNull())
    {
        overallState.SetNull();
    }
    else
    {
        overallState = mState.mOverallState;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetOverallTarget(DataModel::Nullable<GenericOverallTarget> & overallTarget)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);

    if (mState.mOverallTarget.IsNull())
    {
        overallTarget.SetNull();
    }
    else
    {
        overallTarget = mState.mOverallTarget;
    }
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::GetCurrentErrorList(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    
    for (size_t i = 0; true; i++)
    {
        ClosureErrorEnum error;

        err = mDelegate.GetCurrentErrorAtIndex(i, error);

        // Convert CHIP_ERROR_PROVIDER_LIST_EXHAUSTED to CHIP_NO_ERROR
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            err = CHIP_NO_ERROR;
            goto exit;
        }

        // Exit for other errors occurred apart from CHIP_ERROR_PROVIDER_LIST_EXHAUSTED
        SuccessOrExit(err);

        // Encode the error
        err = encoder.Encode(error);

        // Check if error occurred while trying to encode
        SuccessOrExit(err);
    }

exit:
    return err;
}

chip::Protocols::InteractionModel::Status ClusterLogic::HandleStop() 
{
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "Stop Command called before Initialization of closure");
    
    // Stop command can only be supported if closure doesnt support instantaneous features
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kInstantaneous), Status::UnsupportedCommand);
    
    Status status = Status::Success;
        
    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR,Status::Failure);
    
    // Stop action is supported only if the closure is in one of the following states Moving, WaitingForMotion or Calibrating.
    // A status code of SUCCESS SHALL always be returned, regardless if it is in above states or not.
    if ((state == MainStateEnum::kCalibrating) || (state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion))
    {
        // Set the MainState to 'Stopped' only if the stop command handling is successfully completed.
        VerifyOrReturnError(mDelegate.HandleStopCommand() == Status::Success, Status::Failure);
        
        VerifyOrReturnError(SetMainState(MainStateEnum::kStopped) == CHIP_NO_ERROR, Status::Failure,
                                ChipLogError(AppServer, "Stop Command: Failed to set MainState to Stopped"));
    }

    return status;
}

chip::Protocols::InteractionModel::Status ClusterLogic::HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                       Optional<Globals::ThreeLevelAutoEnum> speed) 
{
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "MoveTo Command called before Initialization of closure");
    
    Status status = Status::Success;
    
    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR,Status::Failure);
    
    GenericOverallTarget target;

    VerifyOrReturnError(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    if (position.HasValue() && mConformance.HasFeature(Feature::kPositioning))
    {
        VerifyOrReturnError(position.Value() != TargetPositionEnum::kUnknownEnumValue, Status::ConstraintError);

        target.position = position;
    }

    if (latch.HasValue() && mConformance.HasFeature(Feature::kMotionLatching))
    {
        // If manual intervention is required to latch, respond with INVALID_ACTION
        if (mDelegate.IsManualLatchingNeeded())
        {
            return Status::InvalidAction;
        }
        
        target.latch = latch;
    }

    if (speed.HasValue() && mConformance.HasFeature(Feature::kSpeed))
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);
        
        target.speed = speed;
    }

    // If the MoveTo command is received in any state other than 'Moving', 'WaitingForMotion', or 'Stopped', an error code INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(state == MainStateEnum::kMoving || state == MainStateEnum::kWaitingForMotion
                        || state == MainStateEnum::kStopped, Status::InvalidInState);

    // Once the MoveTo action is successfully completed, the server will set OverallTarget and MainState.
    VerifyOrReturnError(mDelegate.HandleMoveToCommand(position, latch, speed) == Status::Success, Status::Failure);
    
    DataModel::Nullable<GenericOverallTarget> setTarget;
    setTarget.SetNonNull(target);
    VerifyOrReturnError(SetOverallTarget(setTarget) == CHIP_NO_ERROR, Status::Failure);

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

    
    return status;
}

chip::Protocols::InteractionModel::Status ClusterLogic::HandleCalibrate()
{
    VerifyOrDieWithMsg(mIsInitialized, AppServer, "Calibrate Command called before Initialization of closure");
    
    Status status = Status::Success;
    
    VerifyOrReturnError(mConformance.HasFeature(Feature::kCalibration), Status::UnsupportedCommand);
    
    MainStateEnum state;
    VerifyOrReturnError(GetMainState(state) == CHIP_NO_ERROR,Status::Failure);
    
    //If Calibrate command is received when already in the Calibrating state, the server SHALL respond with a status code of SUCCESS.
    if (state == MainStateEnum::kCalibrating) 
    {
        return Status::Success;
    }
    
    // If the Calibrate command is invoked in any state other than 'Stopped', the server shall respond with INVALID_IN_STATE.
    // This check excludes the 'Calibrating' MainState as it is already validated above
    VerifyOrReturnError(state == MainStateEnum::kStopped,Status::InvalidInState);
    
    // Set the MainState to 'Calibrating' only if the Calibrate command handling is successfully completed.
    VerifyOrReturnError(mDelegate.HandleCalibrateCommand() == Status::Success, Status::Failure);
    
    VerifyOrReturnError(SetMainState(MainStateEnum::kCalibrating) == CHIP_NO_ERROR, Status::Failure,
                                ChipLogError(AppServer, "Calibrate Command: Failed to set MainState to Calibrating"));
    
    return status;
    
}


CHIP_ERROR ClusterLogic::PostOperationalErrorEvent(const DataModel::List<const ClosureErrorEnum> & errorState)
{
    ReturnErrorOnFailure(SetMainState(MainStateEnum::kError));

    Events::OperationalError::Type event{ .errorState = errorState };
    ReturnErrorOnFailure(mMatterContext.LogClosureEvent(event));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::PostMovementCompletedEvent()
{
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous),
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
    Events::MovementCompleted::Type event{};
    ReturnErrorOnFailure(mMatterContext.LogClosureEvent(event));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::PostEngageStateChangedEvent(const bool engageValue)
{
    VerifyOrReturnError(mConformance.HasFeature(Feature::kManuallyOperable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
    Events::EngageStateChanged::Type event{ .engageValue = engageValue };
    ReturnErrorOnFailure(mMatterContext.LogClosureEvent(event));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::PostSecureStateChangedEvent(const bool secureValue)
{
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
    Events::SecureStateChanged::Type event{ .secureValue = secureValue };
    ReturnErrorOnFailure(mMatterContext.LogClosureEvent(event));

    return CHIP_NO_ERROR;
}


} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
