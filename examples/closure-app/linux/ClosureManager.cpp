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
constexpr uint32_t kMotionCountdownTimeMs         = 1000; // 1 second for each motion.
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
    // Cancel any existing timers for closure actions over all endpoints
    DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);

    mEp1CurrentAction                 = ClosureAction::kCalibrateAction;
    mIsCalibrationActionInProgress = true;

    // For sample application, we are using a timer to simulate the hardware calibration action.
    // In a real application, this would be replaced with actual calibration logic and call HandleClosureActionComplete.
    VerifyOrReturnValue(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kCountdownTimeSeconds),
                                                              HandleEp1ClosureActionTimer, nullptr) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to start closure action timer"));
    

    ChipLogProgress(AppServer, "ClosureManager: Calibration action started for endpoint 1");
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    // Cancel any existing timers for closure action
    DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
    mEp1CurrentAction = ClosureAction::kStopAction;
    HandleStopActionComplete();
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                const Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed)
{
    // Cancel any existing timer for closure action
    (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);

    // Update the target state for the closure panels based on the MoveTo command.
    // This closure sample app assumes that the closure panels are represented by two endpoints:
    // - Endpoint 2: Represents the Closure Dimension Cluster for the first panel.
    // - Endpoint 3: Represents the Closure Dimension Cluster for the second panel.

    // For sample app, MoveTo command to Fullopen , will set target position of both panels to 0
    //  MoveTo command to Fullclose will set target position of both panels to 10000
    //  We simulate harware action by using timer for 1 sec and updating the current state of the panels after the timer expires.
    //  till we reach the target position.

    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().GetTargetState(ep2TargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().GetTargetState(ep3TargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturnError(!ep2CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturnError(!ep3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    // Create target struct for the panels if the target state is not set.
    GenericDimensionStateStruct ep2Target = ep2TargetState.IsNull() ? GenericDimensionStateStruct() : ep2TargetState.Value();
    GenericDimensionStateStruct ep3Target = ep3TargetState.IsNull() ? GenericDimensionStateStruct() : ep3TargetState.Value();

    if (position.HasValue())
    {
        // Set the Closure panel target position for the panels based on the MoveTo Command position.
        // For Sample App,TargetPositionEnum is mapped to specific positions for the panels.
        chip::Percent100ths ep2Position;
        chip::Percent100ths ep3Position;

        switch (position.Value())
        {
        case TargetPositionEnum::kMoveToFullyClosed:
            ep2Position = static_cast<chip::Percent100ths>(10000);
            ep3Position = static_cast<chip::Percent100ths>(10000);
            break;
        case TargetPositionEnum::kMoveToFullyOpen:
            ep2Position = static_cast<chip::Percent100ths>(0);
            ep3Position = static_cast<chip::Percent100ths>(0);
            break;
        case TargetPositionEnum::kMoveToPedestrianPosition:
            ep2Position = static_cast<chip::Percent100ths>(3000);
            ep3Position = static_cast<chip::Percent100ths>(3000);
            break;
        case TargetPositionEnum::kMoveToSignaturePosition:
            ep2Position = static_cast<chip::Percent100ths>(2000);
            ep3Position = static_cast<chip::Percent100ths>(2000);
            break;
        case TargetPositionEnum::kMoveToVentilationPosition:
            ep2Position = static_cast<chip::Percent100ths>(1000);
            ep3Position = static_cast<chip::Percent100ths>(1000);
            break;
        default:
            ChipLogError(AppServer, "Invalid target position received in OnMoveToCommand");
            return Status::Failure;
        }

        ep2Target.position.SetValue(DataModel::MakeNullable(ep2Position));
        ep3Target.position.SetValue(DataModel::MakeNullable(ep3Position));
    }

    if (latch.HasValue())
    {
        ep2Target.latch.SetValue(DataModel::MakeNullable(latch.Value()));
        ep3Target.latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    if (speed.HasValue())
    {
        ep2Target.speed.SetValue(speed.Value());
        ep3Target.speed.SetValue(speed.Value());
    }

    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().SetTargetState(DataModel::MakeNullable(ep2Target)) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().SetTargetState(DataModel::MakeNullable(ep3Target)) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 3"));
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    mEp1CurrentAction       = ClosureAction::kMoveToAction;
    mEp1MotionInProgress    = true;
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs), HandleEp1ClosureActionTimer, this);
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & pos,
                                                                             const Optional<bool> & latch,
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             const chip::EndpointId endpointId)
{
    // Add logic to handle the SetTarget command
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                        const uint16_t & numberOfSteps,
                                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                        const chip::EndpointId endpointId)
{
    // Add logic to handle the Step command
    return Status::Success;
}

void ClosureManager::HandleEp1ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp1ClosureActionTimer called with current action: %d", static_cast<int>(instance.mEp1CurrentAction));

    switch (instance.mEp1CurrentAction)
    {
    case ClosureAction::kCalibrateAction:
        instance.HandleCalibrateActionComplete();
        break;
    case ClosureAction::kStopAction:
        // Add logic to handle Stop action completion
        break;
    case ClosureAction::kMoveToAction:
        instance.HandleClosureMotionAction();
        break;
    case ClosureAction::kLatchAction:
        // Add logic to handle Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp1ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleEp2ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp2ClosureActionTimer called with current action: %d", static_cast<int>(instance.mEp2CurrentAction));

    switch (instance.mEp2CurrentAction)
    {
    case ClosureAction::kSetTargetAction:
        // Add logic to handle SetTarget action completion
        break;
    case ClosureAction::kStepAction:
        // Add logic to handle Step action completion
        break;
    case ClosureAction::kPanelLatchAction:
        // Add logic to handle Panel Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp3ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleEp3ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp3ClosureActionTimer called with current action: %d", static_cast<int>(instance.mEp3CurrentAction));

    switch (instance.mEp3CurrentAction)
    {
    case ClosureAction::kSetTargetAction:
        // Add logic to handle SetTarget action completion
        break;
    case ClosureAction::kStepAction:
        // Add logic to handle Step action completion
        break;
    case ClosureAction::kPanelLatchAction:
        // Add logic to handle Panel Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp3ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleClosureMotionAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;

    DataModel::Nullable<GenericOverallTargetState> ep1TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;

    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(ep1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallTargetState(ep1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetTargetState(ep2TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetTargetState(ep3TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturn(!ep1CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!ep2CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!ep3CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    VerifyOrReturn(!ep1TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 1"));
    VerifyOrReturn(!ep2TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 2"));
    VerifyOrReturn(!ep3TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 3"));

    // check if closure (endpoint 1) need unlatch before starting the motion action.
    if (ep1CurrentState.Value().latch.HasValue() && !ep1CurrentState.Value().latch.Value().IsNull() &&
        ep1TargetState.Value().latch.HasValue() && !ep1TargetState.Value().latch.Value().IsNull())
    {
        // If currently latched (true) and target is unlatched (false), unlatch first before moving
        if (ep1CurrentState.Value().latch.Value().Value() && !ep1TargetState.Value().latch.Value().Value())
        {
            // In Real application, this would be replaced with actual unlatch logic.
            ChipLogProgress(AppServer, "Performing unlatch action");
            ep1CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1CurrentState);
            ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "Unlatched action completed");
        }
    }

    // Once Closure is unlatched, we can proceed with the motion action for endpoints 2 and 3.
    DataModel::Nullable<chip::Percent100ths> ep2NextPosition = DataModel::NullNullable;
    DataModel::Nullable<chip::Percent100ths> ep3NextPosition = DataModel::NullNullable;

    bool isEndPoint2ProgressPossible = false;
    bool isEndPoint3ProgressPossible = false;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(ep2CurrentState.Value(), ep2TargetState.Value(), ep2NextPosition))
    {
        VerifyOrReturn(!ep2NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 2"));
        ep2CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep2NextPosition.Value()));
        instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(ep2CurrentState);
        isEndPoint2ProgressPossible = (ep2NextPosition.Value() != ep2TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", ep2NextPosition.Value(),
                        ep2TargetState.Value().position.Value().Value());
    }

    // Get the Next Current State to be set for the endpoint 3, if target postion is not reached.
    if (GetPanelNextPosition(ep3CurrentState.Value(), ep3TargetState.Value(), ep3NextPosition))
    {
        VerifyOrReturn(!ep3NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 3"));
        ep3CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep3NextPosition.Value()));
        instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(ep3CurrentState);
        isEndPoint3ProgressPossible = (ep3NextPosition.Value() != ep3TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d", ep3NextPosition.Value(),
                        ep3TargetState.Value().position.Value().Value());
    }

    // Check if both endpoints have reached their target positions
    // If both endpoints have reached their target positions, we can consider the closure motion action as complete.
    // If either endpoint has not reached its target position, we will continue the motion action
    // and set the closureTargetReached flag to false.
    // This will ensure that the closure motion action continues until both endpoints have reached their target positions.
    bool isProgressPossible = isEndPoint2ProgressPossible || isEndPoint3ProgressPossible;

    ChipLogProgress(AppServer, "Motion progress possible: %s", isProgressPossible ? "true" : "false");

    // If the closure target is not reached, we will reschedule the timer for motion action
    if (isProgressPossible)
    {
        mEp1CurrentAction            = ClosureAction::kMoveToAction;
        mEp1MotionInProgress         = true;
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs), HandleEp1ClosureActionTimer, this);
        ChipLogProgress(AppServer, "Rescheduled HandleEp1ClosureActionTimer for motion action");
        return;
    }

    // If both endpoints have reached their target positions, we can consider the closure motion action as complete.
    // Before calling HandleClosureActionComplete, we need to check if a latch action is needed.
    if (ep1CurrentState.Value().latch.HasValue() && !ep1CurrentState.Value().latch.Value().IsNull() &&
        ep1TargetState.Value().latch.HasValue() && !ep1TargetState.Value().latch.Value().IsNull())
    {
        // If currently latched (false) and target is unlatched (true), unlatch first before moving
        if (!ep1CurrentState.Value().latch.Value().Value() && ep1TargetState.Value().latch.Value().Value())
        {
            // In Real application, this would be replaced with actual unlatch logic.
            ChipLogProgress(AppServer, "Performing latch action");
            ep1CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1CurrentState);
            ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "latched action complete");
        }
    }

    // Target reached, call HandleMoveToActionComplete
    instance.HandleMoveToActionComplete();
}

bool ClosureManager::GetPanelNextPosition(const GenericDimensionStateStruct & currentState,
                                          const GenericDimensionStateStruct & targetState,
                                          DataModel::Nullable<chip::Percent100ths> & nextPosition)
{
    VerifyOrReturnValue(targetState.position.HasValue() && !targetState.position.Value().IsNull(), false,
                        ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Target position is not set"));

    VerifyOrReturnValue(currentState.position.HasValue() && !currentState.position.Value().IsNull(), false,
                        ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current position is not set"));

    chip::Percent100ths currentPosition = currentState.position.Value().Value();
    chip::Percent100ths targetPosition  = targetState.position.Value().Value();

    if (currentPosition < targetPosition)
    {
        // Increment position by 1000 units, capped at target.
        nextPosition.SetNonNull(std::min(static_cast<chip::Percent100ths>(currentPosition + kMotionPositionStep), targetPosition));
    }
    else if (currentPosition > targetPosition)
    {
        // Moving down: Decreasing the current position by a step of 1000 units,
        // ensuring it does not go below the target position.
        nextPosition.SetNonNull(std::max(static_cast<chip::Percent100ths>(currentPosition - kMotionPositionStep), targetPosition));
    }
    else
    {
        // Already at target: No further action is needed as the current position matches the target position.
        nextPosition.SetNonNull(currentPosition);
        return false; // No update needed
    }
    return true;
}

void ClosureManager::HandleCalibrateActionComplete()
{
    ChipLogProgress(AppServer, "HandleCalibrateActionComplete called");

    mClosureEndpoint1.OnCalibrateActionComplete();
    mClosurePanelEndpoint2.OnCalibrateActionComplete();
    mClosurePanelEndpoint3.OnCalibrateActionComplete();
    mIsCalibrationActionInProgress = false;
    mEp1CurrentAction                 = ClosureAction::kInvalidAction;
}

void ClosureManager::HandleStopActionComplete()
{
    ChipLogProgress(AppServer, "HandleStopActionComplete called");
    if (mIsCalibrationActionInProgress)
    {
        ChipLogDetail(AppServer, "Stopping calibration action");
        mClosureEndpoint1.OnStopCalibrateActionComplete();
        mClosurePanelEndpoint2.OnStopCalibrateActionComplete();
        mClosurePanelEndpoint3.OnStopCalibrateActionComplete();
        mIsCalibrationActionInProgress = false;
    }
    else if (mEp1MotionInProgress)
    {
        ChipLogDetail(AppServer, "Stopping move to action");
        mClosureEndpoint1.OnStopMotionActionComplete();
        mClosurePanelEndpoint2.OnStopMotionActionComplete();
        mClosurePanelEndpoint3.OnStopMotionActionComplete();
        mEp1MotionInProgress = false;
    }
    else
    {
        ChipLogDetail(AppServer, "No action in progress to stop");
    }
}

void ClosureManager::HandleMoveToActionComplete()
{
    ChipLogProgress(AppServer, "HandleMoveToActionComplete called");
    mClosureEndpoint1.OnMoveToActionComplete();
    mClosurePanelEndpoint2.OnMoveToActionComplete();
    mClosurePanelEndpoint3.OnMoveToActionComplete();
    mEp1MotionInProgress = false;
    mEp1CurrentAction            = ClosureAction::kInvalidAction;
}

void ClosureManager::HandleSetTargetActionComplete()
{
    // Add logic to handle SetTarget action completion
}

void ClosureManager::HandleStepActionComplete()
{
    // Add logic to handle Step action completion
}
