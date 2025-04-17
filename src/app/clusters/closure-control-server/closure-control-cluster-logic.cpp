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

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

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
    // Implement the MainState state machin to validate transitions
    return true;
}

bool ClusterLogic::IsSupportedPositioning(PositioningEnum positioning)
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

CHIP_ERROR ClusterLogic::SetMainState(MainStateEnum mainState)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mIsInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsSupportedMainState(mainState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    VerifyOrReturnError(IsValidMainStateTransition(mainState), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnValue(mainState != mState.mMainState, CHIP_NO_ERROR);

    mState.mMainState = mainState;
    mMatterContext.MarkDirty(Attributes::MainState::Id);

    // TODO: Trigger CountdownTime update

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
    // TODO: Add Support for secure state

    bool validateIncomingOverallStateMembers     = false;
    bool validateIncomingOverallStatePositioning = false;
    bool validateIncomingOverallStateLatch       = false;
    bool validateIncomingOverallStateSpeed       = false;
    // TODO: Add Support for secure state

    bool requirePositioningUpdate = false;
    bool requireLatchUpdate       = false;
    bool requireSpeedUpdate       = false;
    // TODO: Add Support for secure state

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
        validateClusterOverallStateLatch       = currentOverallState.Value().latching.HasValue();
        validateClusterOverallStateSpeed       = currentOverallState.Value().speed.HasValue();
        // TODO: Add Support for secure state
    }

    // Determine OverallState member values, if a value is present, if it is null or if it has a valid value, to determine what
    // checks need to be executed.
    if (!overallState.IsNull())
    {
        // If the provided OverallState has a value, we need to check its members to validate
        validateIncomingOverallStateMembers = true;

        validateIncomingOverallStatePositioning = overallState.Value().positioning.HasValue();
        validateIncomingOverallStateLatch       = overallState.Value().latching.HasValue();
        validateIncomingOverallStateSpeed       = overallState.Value().speed.HasValue();
        // TODO: Add Support for secure state
    }

    /*
        Validate FeatureMap Conformance
    */

    // Validate if the Positioning feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStatePositioning || validateIncomingOverallStatePositioning)
    {
        // If the positioning member is present in either the current or incoming OverallState, we need to check if the Positioning
        // feature is supported by the device. If the Positioning feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kPositioning), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the MotionLatching feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStateLatch || validateIncomingOverallStateLatch)
    {
        // If the latching member is present in either the current or incoming OverallState, we need to check if the MotionLatching
        // feature is supported by the device. If the MotionLatching feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kMotionLatching), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Validate if the Speed feature is required based on the set values and FeatureMap conformance.
    if (validateClusterOverallStateSpeed || validateIncomingOverallStateSpeed)
    {
        // If the speed member is present in either the current or incoming OverallState, we need to check if the Speed feature is
        // supported by the device. If the Speed feature is not supported, return an error.
        VerifyOrReturnError(mConformance.HasFeature(Feature::kSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // TODO: Add Support for secure state

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
            VerifyOrReturnError(IsSupportedPositioning(positioning.Value()), CHIP_ERROR_INVALID_ARGUMENT);
        }

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requirePositioningUpdate =
            !validateClusterOverallStatePositioning || currentOverallState.Value().positioning.Value() != positioning;
    }

    // Validate the incomging Latching value - We don't need to check feature since the check was done above.
    if (validateIncomingOverallStateLatch)
    {
        // We don't need to check is values are present since the check was done above.
        const DataModel::Nullable<LatchingEnum> & latch = overallState.Value().latching.Value();

        if (!latch.IsNull())
        {
            // TODO: Update this when the XML is updated to use the latest spec
            VerifyOrReturnError(EnsureKnownEnumValue(latch.Value()) != LatchingEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
        }

        // We need to update the cluster positioning value if
        // - cluster does not have value; the value will necessarily be updated
        // - cluster and incoming values are different
        requireLatchUpdate = !validateClusterOverallStateLatch || currentOverallState.Value().latching.Value() != latch;
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

    // TODO: Add Support for secure state

    /*
        Update the cluster state and mark it as dirty if necessary
    */

    // Cluster OverallState going from non-null to null
    if (validateClusterOverallStateMembers && !validateIncomingOverallStateMembers)
    {
        currentOverallState.SetNull();
        mMatterContext.MarkDirty(Attributes::OverallState::Id);
    }
    else if (requirePositioningUpdate || requireLatchUpdate || requireSpeedUpdate)
    {
        currentOverallState.SetNonNull(overallState.Value());
        mMatterContext.MarkDirty(Attributes::OverallState::Id);
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR ClusterLogic::SetOverallTarget(const DataModel::Nullable<GenericOverallTarget> & overallTarget)
{
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
    // TODO: Implement logic to retrieve OverallTarget
    return CHIP_NO_ERROR;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
