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
#include <app/EventLogging.h>
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

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance)
{
    VerifyOrReturnError(conformance.Valid(), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    mConformance = conformance;

    mIsInitialized = true;
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
        // Calibrating requires the Calibration feature
        isSupported = mConformance.HasFeature(Feature::kCalibration);
        break;

    case MainStateEnum::kProtected:
        // Protected requires the Protection feature
        isSupported = mConformance.HasFeature(Feature::kProtection);
        break;

    case MainStateEnum::kDisengaged:
        // Disengaged requires the ManuallyOperable feature
        isSupported = mConformance.HasFeature(Feature::kManuallyOperable);
        break;

    default:
        // Unknown states are not supported
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
        // OpenedForPedestrian requires the Pedestrian feature
        isSupported = mConformance.HasFeature(Feature::kPedestrian);
        break;

    case PositioningEnum::kOpenedForVentilation:
        // OpenedForVentilation requires the Ventilation feature
        isSupported = mConformance.HasFeature(Feature::kVentilation);
        break;

    default:
        // Unknown states are not supported
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
        // Pedestrian requires the Pedestrian feature
        isSupported = mConformance.HasFeature(Feature::kPedestrian);
        break;

    case TargetPositionEnum::kVentilation:
        // Ventilation requires the Ventilation feature
        isSupported = mConformance.HasFeature(Feature::kVentilation);
        break;

    default:
        // Unknown states are not supported
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

static void onCountdownTimerTick(System::Layer * systemLayer, void * context)
{
    assertChipStackLockedByCurrentThread();
    
    auto * logic = static_cast<ClusterLogic *>(context);
    
    ClusterState state = logic->GetState();
    
    if (state.mCountdownTime.value().IsNull())
    {
        // I think this might be an error state - if mCountdownTime is NULL, this timer shouldn't be on.
        return;
    }

    ElapsedS countdownTime = state.mCountdownTime.value().Value();

    if (countdownTime)
    {
        countdownTime--;
        logic->SetCountdownTimeFromCluster(countdownTime);
    }
    else 
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(onCountdownTimerTick, logic);
        logic->HandleCountdownTimeExpired();
    }

    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onCountdownTimerTick, logic);
}

void ClusterLogic::HandleCountdownTimeExpired()
{
    ClusterState state = GetState();

    if (state.mMainState == MainStateEnum::kWaitingForMotion)
    {
        if (mDelegate.IsReadyToMove())
        {
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onCountdownTimerTick, this);
            Status status = mDelegate.HandleMoveToCommand(state.mOverallTarget.Value().position, state.mOverallTarget.Value().latch, state.mOverallTarget.Value().speed);
            if (status == Status::Success)
            {
                SetMainState(MainStateEnum::kMoving);
            } 
        } 
    }
    
    PostMovementCompletedEvent();
    HandleStop();
}

CHIP_ERROR ClusterLogic::SetMainState(MainStateEnum mainState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsSupportedMainState(mainState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsValidMainStateTransition(mainState), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnValue(mainState != mState.mMainState, CHIP_NO_ERROR);

    // EngageStateChanged event SHALL be generated when the MainStateEnum attribute changes state to and from disengaged
    // Present state is Disengaged and will be changed to new state, so trigger EngageStateChangedEvent 
    if (mState.mMainState == MainStateEnum::kDisengaged)
    {
        // As Closure is transition from Disengaged state to new State, EngageValue should be true.
        PostEngageStateChangedEvent(true);
    }
    // New state will be Disengaged , so trigger EngageStateChangedEvent
    if (mainState == MainStateEnum::kDisengaged)
    {
        // As Closure is transition to Disengaged state, EngageValue should be false.  
        PostEngageStateChangedEvent(false);
    }

    mState.mMainState = mainState;
    mMatterContext.MarkDirty(Attributes::MainState::Id);
    
    
    if (mConformance.HasFeature(Feature::kInstantaneous)) {
        
        if (mainState == MainStateEnum::kCalibrating){
            SetCountdownTimeFromCluster(mDelegate.GetCalibrationCountdownTime());
        } else if (mainState == MainStateEnum::kMoving) {
            SetCountdownTimeFromCluster(mDelegate.GetMovingCountdownTime());
        } else if (mainState == MainStateEnum::kWaitingForMotion) {
            SetCountdownTimeFromCluster(mDelegate.GetWaitingForMotionCountdownTime());
        } else {
            SetCountdownTimeFromCluster(DataModel::NullNullable);
        }
    
        if (mainState == MainStateEnum::kCalibrating || mainState == MainStateEnum::kMoving || mainState == MainStateEnum::kWaitingForMotion)
        {
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(1), onCountdownTimerTick, this);
        }   
    }
    
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnValue(mState.mOverallState != overallState, CHIP_NO_ERROR);

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
        Validate FeatureMap Conformance
    */

    // Validate if the Positioning feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStatePositioning || validateIncomingOverallStatePositioning)
    {
        //TODO: why are we checking the current position value, why it should impact the set. 
        //TODO: ideally if we dont have ppositioningfeature the current.state should not be present.
        //TODO: if currentstate has positioning then we are missing check somewhere else.
        // If the positioning member is present in either the current or incoming OverallState, we need to check if the Positioning
        // feature is supported by the device. If the Positioning feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the MotionLatching feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStateLatch || validateIncomingOverallStateLatch)
    {
        //TODO: Same comment as above
        // If the latching member is present in either the current or incoming OverallState, we need to check if the MotionLatching
        // feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the Speed feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStateSpeed || validateIncomingOverallStateSpeed)
    {
        //TODO: Same comment as above
        // If the speed member is present in either the current or incoming OverallState, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the Positioning or MotionLatching feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStateSecureState || validateIncomingOverallStateSecureState)
    {
        //TODO: Same comment as above
        // If the secureState member is present in either the current or incoming OverallState, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) || mConformance.HasFeature(Feature::kMotionLatching), 
                            CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    /*
        Validate incoming values and if update is necessary
    */

    // Validate the incomging Positioning value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallStatePositioning)
    {
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

    // Validate the incomging Latch value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallStateLatch)
    {
        // We don't need to check is values are present since the check was done above.
        const DataModel::Nullable<bool> & latch = overallState.Value().latch.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterOverallStateLatch || currentOverallState.Value().latch.Value() != latch;
    }

    // Validate the incomging Speed value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallStateSpeed)
    {
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

    // Validate the incomging SecureState value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallStateSecureState)
    {
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
    VerifyOrReturnValue(mState.mOverallTarget != overallTarget, CHIP_NO_ERROR);

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
        Validate FeatureMap Conformance
    */

    // Validate if the Position feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallTargetPosition || validateIncomingOverallTargetPosition)
    {
        //TODO: why are we checking the current position value, why it should impact the set. 
        //TODO: ideally if we dont have ppositioningfeature the current.state should not be present.
        //TODO: if currentstate has positioning then we are missing check somewhere else.
        // If the position member is present in either the current or incoming OverallTarget, we need to check if the Position
        // feature is supported by the device. If the Position feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the MotionLatching feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallTargetLatch || validateIncomingOverallTargetLatch)
    {
        //TODO: Same comment as above
        // If the latching member is present in either the current or incoming OverallTarget, we need to check if the MotionLatching
        // feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the Speed feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallTargetSpeed || validateIncomingOverallTargetSpeed)
    {
        //TODO: Same comment as above
        // If the speed member is present in either the current or incoming OverallTarget, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    /*
        Validate incoming values and if update is necessary
    */

    // Validate the incomging Position value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallTargetPosition)
    {
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

    // Validate the incomging Latch value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallTargetLatch)
    {
        // We don't need to check is values are present since the check was done above.
        const bool & latch = overallTarget.Value().latch.Value();

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterOverallTargetLatch || currentOverallTarget.Value().latch.Value() != latch;
    }

    // Validate the incomging Speed value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallTargetSpeed)
    {
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

CHIP_ERROR ClusterLogic::SetCurrentErrorList(const ClosureErrorEnum error)
{
    assertChipStackLockedByCurrentThread();
    
    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    
    VerifyOrReturnError(EnsureKnownEnumValue(error) != ClosureErrorEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(mDelegate.SetCurrentErrorInList(error));
    ReportCurrentErrorListChange();
    
    //TODO: GetErrorList and PostErrorEvent
    
    ReturnErrorOnFailure(SetMainState(MainStateEnum::kError));
    
    return CHIP_NO_ERROR;
}

void ClusterLogic::ReportCurrentErrorListChange()
{
    mMatterContext.MarkDirty(Attributes::CurrentErrorList::Id);
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

    if (mState.mOverallState.IsNull())
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
    Status status = Status::Success;
    
    // Stop command can only be supported if closure doesnt support instantaneous features
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kInstantaneous), Status::UnsupportedCommand);
    
    MainStateEnum state;
    VerifyOrReturnValue(GetMainState(state) == CHIP_NO_ERROR,Status::Failure);

    //VerifyOrReturnValue(CheckCommandStateCompatibility(Commands::Stop::Id, state), Status::InvalidInState);
    
    // Stop action is supported only if the closure is in one of the following states Moving, WaitingForMotion or Calibrating.
    // A status code of SUCCESS SHALL always be returned, regardless if it is in above states or not.
    if ((state == MainStateEnum::kCalibrating) || (state == MainStateEnum::kMoving) || (state == MainStateEnum::kWaitingForMotion))
    {
        status = mDelegate.HandleStopCommand();
        // once stop action is successful on closure will set MainState to stopped.
        if (status == Status::Success) {
            VerifyOrReturnValue(SetMainState(MainStateEnum::kStopped) == CHIP_NO_ERROR,Status::Failure,
                                ChipLogError(AppServer, "Stop Command: Failed to set MainState to Stopped"));
        } 
    }

    return status;
    
}

chip::Protocols::InteractionModel::Status ClusterLogic::HandleMoveTo(Optional<TargetPositionEnum> position, Optional<bool> latch,
                                                       Optional<Globals::ThreeLevelAutoEnum> speed) 
{
    Status status = Status::Success;
    
    MainStateEnum state;
    VerifyOrReturnValue(GetMainState(state) == CHIP_NO_ERROR,Status::Failure);
    
    DataModel::Nullable<GenericOverallTarget> target;

    // If all command parameters don't have a value, return InvalidCommand
    VerifyOrReturnValue(position.HasValue() || latch.HasValue() || speed.HasValue(), Status::InvalidCommand);

    if (position.HasValue())
    {
        VerifyOrReturnError(position.Value() != TargetPositionEnum::kUnknownEnumValue, Status::ConstraintError);

        // If Positioning(PS) feature is not supported, it SHALL return a status code SUCCESS.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), Status::Success);

        target.Value().position = position;
    }

    if (latch.HasValue())
    {
        // If MotionLatching (LT) feature is not supported, the server SHALL return a status code SUCCESS,
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), Status::Success);

        // If manual intervention is required to latch, respond with INVALID_ACTION
        if (mDelegate.IsManualLatchingNeeded())
        {
            return Status::InvalidAction;
        }
        
        target.Value().latch = latch;
    }

    if (speed.HasValue())
    {
        VerifyOrReturnError(speed.Value() != Globals::ThreeLevelAutoEnum::kUnknownEnumValue, Status::ConstraintError);

        // If Speed (SP) feature is not supported, the server SHALL return a status code SUCCESS,
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), Status::Success);
        
        target.Value().speed = speed;
    }

    // If MoveTo command is received in any state other than Moving, WaitingForMotion or Stopped, an error code INVALID_IN_STATE SHALL be returned
    VerifyOrReturnValue(state != MainStateEnum::kMoving && state != MainStateEnum::kWaitingForMotion
                        && state != MainStateEnum::kStopped, Status::InvalidInState);

    // TODO: Check if Closure need to perform motion or not.
    // if the closure is already in the target position, Motion is not required
    // If Motion is required, then ignore next checks and start checking if closure is ready to move or not.
    // If Motion is not required, then check for Errors on closure.
    // If there are no errors on closure, set MainState to Stopped and return SUCCESS.
    // If there are errors on closure, set MainState to Error and return SUCCESS.

    

    status = mDelegate.HandleMoveToCommand(position, latch, speed);
    
    // once MoveTo action is successful.Server will set OverallTarget and the MainState.
    if (status == Status::Success) 
    {
        VerifyOrReturnValue(SetOverallTarget(target) == CHIP_NO_ERROR, Status::Failure);
        
        // MainState is set to Moving or WaitingforMotion based on closure.
        if (mDelegate.IsReadyToMove())
        {
            // If the closure is ready to move, set MainState to Moving
            VerifyOrReturnValue(SetMainState(MainStateEnum::kCalibrating) == CHIP_NO_ERROR,Status::Failure,
                                ChipLogError(AppServer, "MoveTo Command: Failed to set MainState to Moving"));
        }
        else
        {
            // If closure needs to pre-stage before moving, then set MainState to WaitingForMoving
            SetMainState(MainStateEnum::kWaitingForMotion);
        }
            VerifyOrReturnValue(SetMainState(MainStateEnum::kCalibrating) == CHIP_NO_ERROR,Status::Failure,
                                ChipLogError(AppServer, "Calibrate Command: Failed to set MainState to Calibrating"));
    }
    
    return status;
}

chip::Protocols::InteractionModel::Status ClusterLogic::HandleCalibrate()
{
    Status status = Status::Success;
    
    // Calibrate command can only be supported if closure supports Calibration feature.
    VerifyOrReturnError(!mConformance.HasFeature(Feature::kCalibration), Status::UnsupportedCommand);
    
    MainStateEnum state;
    VerifyOrReturnValue(GetMainState(state) == CHIP_NO_ERROR,Status::Failure);
    
    //If Calibrate command is received when already in the Calibrating state, the server SHALL respond with a status code of SUCCESS.
    if (state == MainStateEnum::kCalibrating) 
    {
        return Status::Success;
    }
    
    // If Calibrate command is invoked in any state other than Stopped, the server SHALL respond with INVALID_IN_STATE.
    // this check excludes Claibrating Mainstate as its already validated above.
    VerifyOrReturnValue(state != MainStateEnum::kStopped,Status::InvalidInState);
    
    status = mDelegate.HandleCalibrateCommand();
    
    // once Calibrate action is successful on closure will set MainState to stopped.
    if (status == Status::Success) {
        VerifyOrReturnValue(SetMainState(MainStateEnum::kCalibrating) == CHIP_NO_ERROR,Status::Failure,
                                ChipLogError(AppServer, "Calibrate Command: Failed to set MainState to Calibrating"));
    } 
    
    return status;
    
}


CHIP_ERROR ClusterLogic::PostOperationalErrorEvent(const DataModel::List<const ClosureErrorEnum> errorState)
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = SetMainState(MainStateEnum::kError);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement,
                     "ClosureControlCLuster: Operation error event set MainState as Error failed %" CHIP_ERROR_FORMAT,
                     err.Format());
    }

    Events::OperationalError::Type event{ .errorState = errorState };
    EventNumber eventNumber;
    err = LogEvent(event, mMatterContext.GetEndpointId(), eventNumber);
    
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "ClosureControlCLuster: Operation error log event failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR ClusterLogic::PostMovementCompletedEvent()
{
    // Movement complete event can only be triggered,if the Positioning feature is supported and Instantenous feature is not supported.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE otherwise.
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
    Events::MovementCompleted::Type event{};
    EventNumber eventNumber;
    
    CHIP_ERROR err = LogEvent(event, mMatterContext.GetEndpointId(), eventNumber);
    
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "ClosureControlCLuster: Movement complete log event failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR ClusterLogic::PostEngageStateChangedEvent(const bool engageValue)
{
    
    // EngageState changed Event can only be triggered,if the ManuallyOperable feature is supported.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE otherwise.
    VerifyOrReturnError(mConformance.HasFeature(Feature::kManuallyOperable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
    Events::EngageStateChanged::Type event{ .engageValue = engageValue };
    EventNumber eventNumber;
    
    CHIP_ERROR err = LogEvent(event, mMatterContext.GetEndpointId(), eventNumber);
    
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "ClosureControlCLuster: Engage State log event failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR ClusterLogic::PostSecureStateChangedEvent(const bool secureValue)
{
    // SecureState changed event can only be triggered,if the Positioning or MotionLatching feature is supported.
    // Return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE otherwise.
    VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning) && !mConformance.HasFeature(Feature::kInstantaneous), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
    Events::SecureStateChanged::Type event{ .secureValue = secureValue };
    EventNumber eventNumber;
    
    CHIP_ERROR err = LogEvent(event, mMatterContext.GetEndpointId(), eventNumber);
    
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "ClosureControlCLuster: Secured state log event failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}


} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
