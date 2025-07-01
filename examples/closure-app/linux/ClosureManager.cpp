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

#include "ClosureManager.h"
#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"

#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {
// Define a constant for the countdown time
constexpr uint32_t kCountdownTimeSeconds          = 10;
constexpr uint32_t kMotionCountdownTimeMs         = 1000;
constexpr chip::Percent100ths kMotionPositionStep = 1000; // 10% of the total range per motion interval.

// Define the Namespace and Tag for the endpoint
// Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure.adoc
constexpr uint8_t kNamespaceClosure   = 0x44;
constexpr uint8_t kTagClosureCovering = 0x00;
// Derived from
// https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-Closure-Covering.adoc
constexpr uint8_t kNamespaceCovering   = 0x46;
constexpr uint8_t kTagCoveringVenetian = 0x03;
// Derived from https://github.com/CHIP-Specifications/connectedhomeip-spec/blob/master/src/namespaces/Namespace-ClosurePanel.adoc
constexpr uint8_t kNamespaceClosurePanel = 0x45;
constexpr uint8_t kTagClosurePanelLift   = 0x00;
constexpr uint8_t kTagClosurePanelTilt   = 0x01;

// Define the list of semantic tags for the endpoint
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosureEndpoint1TagList[] = {
    { .namespaceID = kNamespaceClosure,
      .tag         = kTagClosureCovering,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) },
    { .namespaceID = kNamespaceCovering,
      .tag         = kTagCoveringVenetian,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosurePanelEndpoint2TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelLift,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosurePanelEndpoint3TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelTilt,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Tilt"_span)) },
};

} // namespace

// Definition of the static instance of ClosureManager
ClosureManager ClosureManager::sInstance;

void ClosureManager::Init()
{
    VerifyOrDie(mClosureEndpoint1.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Control Endpoint initialized successfully");

    VerifyOrDie(mClosurePanelEndpoint2.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Panel Endpoint 2 initialized successfully");

    VerifyOrDie(mClosurePanelEndpoint3.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Panel Endpoint 3 initialized successfully");

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ kClosureEndpoint1,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosureEndpoint1TagList));
    SetTagList(/* endpoint= */ kClosurePanelEndpoint2,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosurePanelEndpoint2TagList));
    SetTagList(/* endpoint= */ kClosurePanelEndpoint3,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosurePanelEndpoint3TagList));

    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (pTestEventDelegate != nullptr)
    {
        CHIP_ERROR err = pTestEventDelegate->AddHandler(&mClosureEndpoint1.GetDelegate());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to add handler for delegate: %s", chip::ErrorStr(err));
        }
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot add handler for delegate");
    }
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    // Cancel any existing timer for closure action
    DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);

    // For sample application, we are using a timer to simulate the hardware calibration action.
    // In a real application, this would be replaced with actual calibration logic and call HandleClosureActionComplete.
    VerifyOrReturnValue(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kCountdownTimeSeconds),
                                                              HandleClosureActionTimer, nullptr) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to start closure action timer"));
    mCurrentAction                 = ClosureAction::kCalibrateAction;
    mCurrentEndpointId             = kClosureEndpoint1;
    mIsCalibrationActionInProgress = true;

    ChipLogProgress(AppServer, "ClosureManager: Calibration action started for endpoint %d", mCurrentEndpointId);

    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    // Add logic to handle the Stop command
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                const Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed)
{
    // Add logic to handle the MoveTo command
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & position,
                                                                             const Optional<bool> & latch,
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             const chip::EndpointId endpointId)
{
    MainStateEnum ep1MainState;
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().GetMainState(ep1MainState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get main state for Step command on Endpoint 1"));

    // If this command is received while the MainState attribute is currently either in Disengaged, Protected, Calibrating,
    //  SetupRequired or Error, then a status code of INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

    if (mIsSetTargetActionInProgress && mCurrentActionEndpointId != endpointId)
    {
        ChipLogError(AppServer, "SetTarget action is already in progress on Endpoint %d", mCurrentActionEndpointId);
        return Status::Failure;
    }

    // Update OverallTarget of Closure based on SetTarget command.
    DataModel::Nullable<GenericOverallTargetState> overallTargetState;
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().GetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get overall target for SetTarget command"));

    if (overallTargetState.IsNull())
    {
        overallTargetState.SetNonNull(GenericOverallTargetState{});
    }

    if (position.HasValue())
    {
        // Set overallTargetState position to NullOptional as panel position change cannot be represented in OverallTarget.
        overallTargetState.Value().position.SetValue(DataModel::NullNullable);
    }

    if (latch.HasValue())
    {
        overallTargetState.Value().latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    if (speed.HasValue())
    {
        overallTargetState.Value().speed.SetValue(speed.Value());
    }

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for SetTarget command for Endpoint %d", endpointId));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for SetTarget command on Endpoint %d", endpointId));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set main state for SetTarget command on Endpoint 1"));

    (void) DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);

    // Post an event to initiate the unlatch action asynchronously.
    // Closure panel first performs the unlatch action if it is currently latched,
    // and then continues with the SetTarget action.
    // This is to ensure that the panel can move to the target position without being latched.
    mCurrentAction               = ClosureManager::ClosureAction::kPanelUnLatchAction;
    mCurrentActionEndpointId     = endpointId;
    mIsSetTargetActionInProgress = true;
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs), HandleClosureActionTimer, this);

    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                        const uint16_t & numberOfSteps,
                                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                        const chip::EndpointId endpointId)
{
    MainStateEnum ep1MainState;
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().GetMainState(ep1MainState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get main state for Step command on Endpoint 1"));

    // If this command is received while the MainState attribute is currently either in Disengaged, Protected, Calibrating,
    //  SetupRequired or Error, then a status code of INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

    if (mIsStepActionInProgress && mCurrentActionEndpointId != endpointId)
    {
        ChipLogError(AppServer, "Step action is already in progress on Endpoint %d", mCurrentActionEndpointId);
        return Status::Failure;
    }

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Update Overall Target to Null for the Closure Control on Endpoint 1
    DataModel::Nullable<GenericOverallTargetState> ep1Target;

    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().GetOverallTargetState(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get overall target for Step command"));

    if (ep1Target.IsNull())
    {
        ep1Target.SetNonNull(GenericOverallTargetState{});
    }

    ep1Target.Value().position = NullOptional; // Reset position to Null

    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().SetOverallTargetState(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for Step command"));

    (void) DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);
    mCurrentAction           = ClosureManager::ClosureAction::kStepAction;
    mCurrentActionEndpointId = endpointId;
    mIsStepActionInProgress  = true;
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs), HandleClosureActionTimer, this);
    return Status::Success;
}

void ClosureManager::HandleClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();

    switch (instance.mCurrentAction)
    {
    case ClosureAction::kCalibrateAction:
        instance.HandleCalibrateActionComplete();
        break;
    case ClosureAction::kStopAction:
        // Add logic to handle Stop action completion
        break;
    case ClosureAction::kMoveToAction:
        // Add logic to handle MoveTo action completion
        break;
    case ClosureAction::kLatchAction:
        // Add logic to handle Latch action completion
        break;
    case ClosureAction::kSetTargetAction:
        instance.HandlePanelSetTargetAction(instance.mCurrentActionEndpointId);
        break;
    case ClosureAction::kStepAction:
        instance.HandlePanelStepAction(instance.mCurrentActionEndpointId);
        break;
    case ClosureAction::kPanelUnLatchAction:
        instance.HandlePanelUnlatchAction(instance.mCurrentActionEndpointId);
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureActionTimer");
        break;
    }
}

void ClosureManager::HandlePanelUnlatchAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint instance based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * panelEp = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(panelEp != nullptr, ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState  = DataModel::NullNullable;

    VerifyOrReturn(panelEp->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint %d", endpointId));
    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint %d", endpointId));

    VerifyOrReturn(panelEp->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target for Endpoint %d", endpointId));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target is not set for Endpoint %d", endpointId));

    // If currently latched (true) and target is unlatched (false), Perform unlatch action and call timer with SET_TARGET_ACTION
    // to continue with the SetTarget action.
    if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
        panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull() &&
        (panelCurrentState.Value().latch.Value().Value() && !panelTargetState.Value().latch.Value().Value()))
    {
        DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState = DataModel::NullNullable;

        VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(ep1OverallCurrentState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
        VerifyOrReturn(!ep1OverallCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint 1"));

        // In Real application, this would be replaced with actual unlatch logic.
        ChipLogProgress(AppServer, "Performing unlatch action");

        ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
        mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1OverallCurrentState);

        panelCurrentState.Value().latch.SetValue(false);
        panelEp->GetLogic().SetCurrentState(panelCurrentState);

        ChipLogProgress(AppServer, "Unlatched action completed");
    }

    // Cancel any active timers before starting the panel movement
    (void) DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);

    instance.HandlePanelSetTargetAction(endpointId);
}

ClosureDimensionEndpoint * ClosureManager::GetCurrentPanelInstance(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();
    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpoint())
    {
        return &instance.mClosurePanelEndpoint2;
    }
    else if (endpointId == instance.mClosurePanelEndpoint3.GetEndpoint())
    {
        return &instance.mClosurePanelEndpoint3;
    }
    return nullptr;
}

void ClosureManager::HandlePanelSetTargetAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint instance based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(ep != nullptr, ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState  = DataModel::NullNullable;

    VerifyOrReturn(ep->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint %d", endpointId));
    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint %d", endpointId));

    VerifyOrReturn(ep->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target for Endpoint %d", endpointId));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target is not set for Endpoint %d", endpointId));

    bool panelProgressPossible                            = false;
    DataModel::Nullable<chip::Percent100ths> nextPosition = DataModel::NullNullable;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(panelCurrentState.Value(), panelTargetState.Value(), nextPosition))
    {
        VerifyOrReturn(!nextPosition.IsNull(), ChipLogError(AppServer, "Next position is not set for Endpoint %d", endpointId));

        panelCurrentState.Value().position.SetValue(DataModel::MakeNullable(nextPosition.Value()));
        ep->GetLogic().SetCurrentState(panelCurrentState);

        panelProgressPossible = (nextPosition.Value() != panelTargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", endpointId, nextPosition.Value(),
                        panelTargetState.Value().position.Value().Value());
    }
    if (panelProgressPossible)
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);
        mCurrentAction     = ClosureManager::ClosureAction::kSetTargetAction;
        mCurrentEndpointId = endpointId;
        // Start the timer to continue with the SetTarget action
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleClosureActionTimer, this);
        return;
    }

    // If currently unlatched (false) and target is latched (true), latch after completing motion
    if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
        panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull())
    {
        if (!panelCurrentState.Value().latch.Value().Value() && panelTargetState.Value().latch.Value().Value())
        {
            DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState = DataModel::NullNullable;
            VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(ep1OverallCurrentState) == CHIP_NO_ERROR,
                           ChipLogError(AppServer, "Failed to get overall current state for Endpoint 1"));
            VerifyOrReturn(!ep1OverallCurrentState.IsNull(),
                           ChipLogError(AppServer, "Overall current state is not set for Endpoint 1"));

            // In Real application, this would be replaced with actual latch logic.
            ChipLogProgress(AppServer, "Performing latch action");

            ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1OverallCurrentState);

            panelCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            ep->GetLogic().SetCurrentState(panelCurrentState);

            ChipLogProgress(AppServer, "Latch action completed");
        }
    }
    // If the target position is reached, call the HandlePanelSetTargetActionComplete method to complete the action
    HandlePanelSetTargetActionComplete();
}

void ClosureManager::HandlePanelStepAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint instance based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(ep != nullptr, ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));
    StepDirectionEnum stepDirection = ep->GetDelegate().GetStepCommandTargetDirection();

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState;

    VerifyOrReturn(ep->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Step action"));
    VerifyOrReturn(ep->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Step action"));

    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is null, Step action Failed"));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target state  is null, Step action Failed"));
    VerifyOrReturn(panelCurrentState.Value().position.HasValue() && !panelCurrentState.Value().position.Value().IsNull(),
                   ChipLogError(AppServer, "Current or target position is not set, Step action Failed"));
    VerifyOrReturn(panelTargetState.Value().position.HasValue() && !panelTargetState.Value().position.Value().IsNull(),
                   ChipLogError(AppServer, "Current or target position is not set, Step action Failed"));

    chip::Percent100ths currentPosition = panelCurrentState.Value().position.Value().Value();
    chip::Percent100ths targetPosition  = panelTargetState.Value().position.Value().Value();

    ChipLogProgress(AppServer, "Current Position: %d, Target Position: %d", currentPosition, targetPosition);

    bool panelTargetReached = (currentPosition == targetPosition);
    ChipLogProgress(AppServer, "Panel Target Reached: %s", panelTargetReached ? "true" : "false");

    if (!panelTargetReached)
    {
        chip::Percent100ths nextCurrentPosition;
        chip::Percent100ths stepValue;
        VerifyOrReturn(ep->GetLogic().GetStepValue(stepValue) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get step value for Step action"));

        // Compute the next position
        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            nextCurrentPosition = std::min(static_cast<chip::Percent100ths>(currentPosition + stepValue), targetPosition);
        }
        else
        {
            nextCurrentPosition = std::max(static_cast<chip::Percent100ths>(currentPosition - stepValue), targetPosition);
        }

        panelCurrentState.Value().position.SetValue(DataModel::MakeNullable(nextCurrentPosition));
        ep->GetLogic().SetCurrentState(panelCurrentState);

        // Cancel any existing timer before starting a new action
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);
        instance.mCurrentAction           = ClosureManager::ClosureAction::kStepAction;
        instance.mCurrentActionEndpointId = endpointId;
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(1), HandleClosureActionTimer, this);
        return;
    }

    // If the target position is reached, we can complete the action
    HandlePanelStepActionComplete();
}

void ClosureManager::HandleCalibrateActionComplete()
{
    ChipLogProgress(AppServer, "HandleCalibrateActionComplete called");

    mClosureEndpoint1.OnCalibrateActionComplete();
    mClosurePanelEndpoint2.OnCalibrateActionComplete();
    mClosurePanelEndpoint3.OnCalibrateActionComplete();
    mIsCalibrationActionInProgress = false;
    mCurrentAction                 = ClosureAction::kInvalidAction;
    mCurrentEndpointId             = chip::kInvalidEndpointId;
}

void ClosureManager::HandleStopActionComplete()
{
    // Add logic to handle Stop action completion
}

void ClosureManager::HandleMoveToActionComplete()
{
    // Add logic to handle MoveTo action completion
}

void ClosureManager::HandlePanelSetTargetActionComplete()
{
    ChipLogProgress(AppServer, "HandleSetTargetActionComplete called");

    ClosureManager & instance = ClosureManager::GetInstance();
    instance.mClosureEndpoint1.OnPanelMotionActionComplete();
    if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint2.GetEndpoint())
    {
        instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
    }
    else if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint3.GetEndpoint())
    {
        instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
    }
    instance.mIsSetTargetActionInProgress = false;
    instance.mCurrentAction               = ClosureAction::kInvalidAction;
    instance.mCurrentEndpointId           = chip::kInvalidEndpointId;

    ChipLogProgress(AppServer, "SetTarget action completed for Endpoint %d", instance.mCurrentEndpointId);
}

void ClosureManager::HandlePanelStepActionComplete()
{
    ChipLogProgress(AppServer, "HandleStepActionComplete called");

    ClosureManager & instance = ClosureManager::GetInstance();
    instance.mClosureEndpoint1.OnPanelMotionActionComplete();
    if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint2.GetEndpoint())
    {
        instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
    }
    else if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint3.GetEndpoint())
    {
        instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
    }
    instance.mIsStepActionInProgress = false;
    instance.mCurrentAction          = ClosureAction::kInvalidAction;
    instance.mCurrentEndpointId      = chip::kInvalidEndpointId;

    ChipLogProgress(AppServer, "Step action completed for Endpoint %d", instance.mCurrentEndpointId);
}
