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
#include "AppConfig.h"
#include "AppTask.h"
#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>
#include <cmsis_os2.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {
constexpr uint32_t kCountdownTimeSeconds  = 10;
constexpr uint32_t kMotionCountdownTimeMs = 1000; // 1 second for each motion.
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
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp1TagList[] = {
    { .namespaceID = kNamespaceClosure,
      .tag         = kTagClosureCovering,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) },
    { .namespaceID = kNamespaceCovering,
      .tag         = kTagCoveringVenetian,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp2TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelLift,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEp3TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelTilt,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Tilt"_span)) },
};

} // namespace

ClosureManager ClosureManager::sClosureMgr;

void ClosureManager::Init()
{
    // Create cmsis os sw timer for light timer.
    mClosureTimer = osTimerNew(TimerEventHandler, // timer callback handler
                               osTimerOnce,       // no timer reload (one-shot timer)
                               (void *) this,     // pass the app task obj context
                               NULL               // No osTimerAttr_t to provide.
    );

    if (mClosureTimer == NULL)
    {
        ChipLogError(AppServer, "mClosureTimer timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }
    DeviceLayer::PlatformMgr().LockChipStack();

    // Closure endpoints initialization
    ep1.Init();
    ep2.Init();
    ep3.Init();

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp1TagList));
    SetTagList(/* endpoint= */ 2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp2TagList));
    SetTagList(/* endpoint= */ 3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEp3TagList));

    DeviceLayer::PlatformMgr().UnlockChipStack();
}

void ClosureManager::StartTimer(uint32_t aTimeoutMs)
{
    // Starts or restarts the function timer
    if (osTimerStart(mClosureTimer, pdMS_TO_TICKS(aTimeoutMs)) != osOK)
    {
        ChipLogError(AppServer, "mClosureTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void ClosureManager::CancelTimer()
{
    if (osTimerStop(mClosureTimer) == osError)
    {
        ChipLogError(AppServer, "mClosureTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void ClosureManager::InitiateAction(AppEvent * event)
{
    Action_t action = static_cast<Action_t>(event->ClosureEvent.Action);

    ClosureManager & instance = ClosureManager::GetInstance();

    instance.CancelTimer(); // Cancel any existing timer before starting a new action

    switch (action)
    {
    case Action_t::CALIBRATE_ACTION:
        ChipLogDetail(AppServer, "Initiating calibration action");
        // Timer used in sample application to simulate the calibration process.
        // In a real application, this would be replaced with actual calibration logic.
        instance.StartTimer(kCountdownTimeSeconds * 1000);
        break;
    case Action_t::STOP_MOTION_ACTION:
        ChipLogDetail(AppServer, "Initiating stop motion action");
        break;
    case Action_t::STOP_CALIBRATE_ACTION:
        ChipLogDetail(AppServer, "Initiating stop calibration action");
        break;
    case Action_t::MOVE_TO_ACTION:
        ChipLogDetail(AppServer, "Initiating move to action");
        // For Closure sample app, Motion action is simulated with a timer with rate
        // of 10% change of position per second.
        // In a real application, this would be replaced with actual move to logic.
        instance.StartTimer(kMotionCountdownTimeMs);
        break;
    default:
        ChipLogDetail(AppServer, "Invalid action received in InitiateAction");
        return;
    }
}

void ClosureManager::TimerEventHandler(void * timerCbArg)
{
    ClosureManager * closureManager = static_cast<ClosureManager *>(timerCbArg);

    // The timer event handler will be called in the context of the timer task
    // once sClosureTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type                = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = closureManager->GetCurrentAction();
    event.Handler             = HandleClosureActionCompleteEvent;
    AppTask::GetAppTask().PostEvent(&event);
}

void ClosureManager::HandleClosureActionCompleteEvent(AppEvent * event)
{
    Action_t currentAction = static_cast<ClosureManager::Action_t>(event->ClosureEvent.Action);

    switch (currentAction)
    {
    case Action_t::CALIBRATE_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureActionComplete(instance.GetCurrentAction());
        });
        break;
    case Action_t::STOP_MOTION_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureActionComplete(instance.GetCurrentAction());
        });
        break;
    case Action_t::STOP_CALIBRATE_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureActionComplete(instance.GetCurrentAction());
        });
        break;
    case Action_t::MOVE_TO_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureMotionAction();
        });
        break;
    default:
        break;
    }
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    VerifyOrReturnValue(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for calibration"));

    // Post an event to initiate the calibration action asynchronously.
    AppEvent event;
    event.Type                = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = CALIBRATE_ACTION;
    event.Handler             = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);
    SetCurrentAction(CALIBRATE_ACTION);

    isCalibrationInProgress = true;
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    // this function will contain the code for Stop command handling and initiate the stop action.
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                                const chip::Optional<bool> latch,
                                const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed)
{

    // Update the target state for the closure panels based on the MoveTo command.
    // This closure sample app assumes that the closure panels are represented by two endpoints:
    // - Endpoint 2: Represents the Closure Dimension Cluster for the first panel.
    // - Endpoint 3: Represents the Closure Dimension Cluster for the second panel.

    // For sample app, MoveTo command to Fullopen , will set target position of both panels to 0
    //  MoveTo command to Fullclose will set target position of both panels to 10000
    //  We simulate harware action by using timer for 1 sec and updating the current state of the panels after the timer expires.
    //  till we reach the target position.

    DataModel::Nullable<GenericCurrentStateStruct> ep2CurrentState;
    VerifyOrReturnError(ep2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    DataModel::Nullable<GenericCurrentStateStruct> ep3CurrentState;
    VerifyOrReturnError(ep3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    DataModel::Nullable<GenericTargetStruct> ep2TargetState;
    VerifyOrReturnError(ep2.GetLogic().GetTarget(ep2TargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    DataModel::Nullable<GenericTargetStruct> ep3TargetState;
    VerifyOrReturnError(ep3.GetLogic().GetTarget(ep3TargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturnError(!ep2CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturnError(!ep3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    // Create target struct for the panels if the target state is not set.
    GenericTargetStruct ep2Target = ep2TargetState.IsNull() ? GenericTargetStruct() : ep2TargetState.Value();
    GenericTargetStruct ep3Target = ep3TargetState.IsNull() ? GenericTargetStruct() : ep3TargetState.Value();

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

        ep2Target.position.SetValue(ep2Position);
        ep3Target.position.SetValue(ep3Position);
    }

    if (latch.HasValue())
    {
        ep2Target.latch.SetValue(latch.Value());
        ep3Target.latch.SetValue(latch.Value());
    }

    if (speed.HasValue())
    {
        ep2Target.speed.SetValue(speed.Value());
        ep3Target.speed.SetValue(speed.Value());
    }

    VerifyOrReturnError(ep2.GetLogic().SetTarget(DataModel::MakeNullable(ep2Target)) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(ep3.GetLogic().SetTarget(DataModel::MakeNullable(ep3Target)) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Endpoint 3"));

    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    SetCurrentAction(MOVE_TO_ACTION);
    isMoveToInProgress = true;

    // Post an event to initiate the move to action asynchronously.
    // MoveTo Command can only be initiated from Closure Control Endpoint (Endpoint 1).
    AppEvent event;
    event.Type                = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = MOVE_TO_ACTION;
    event.Handler             = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);


    return Status::Success;
}

void ClosureManager::HandleClosureMotionAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState;
    DataModel::Nullable<GenericCurrentStateStruct> ep2CurrentState;
    DataModel::Nullable<GenericCurrentStateStruct> ep3CurrentState;

    DataModel::Nullable<GenericOverallTargetState> ep1TargetState;
    DataModel::Nullable<GenericTargetStruct> ep2TargetState;
    DataModel::Nullable<GenericTargetStruct> ep3TargetState;

    VerifyOrReturn(ep1.GetLogic().GetOverallCurrentState(ep1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(ep1.GetLogic().GetOverallTargetState(ep1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(ep2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(ep3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    VerifyOrReturn(ep2.GetLogic().GetTarget(ep2TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    VerifyOrReturn(ep3.GetLogic().GetTarget(ep3TargetState) == CHIP_NO_ERROR,
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
            instance.ep1.GetLogic().SetOverallCurrentState(ep1CurrentState);
            ep2CurrentState.Value().latch.SetValue(false);
            instance.ep2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(false);
            instance.ep3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "Unlatched action completed");
        }
    }

    // Once Closure is unlatched, we can proceed with the motion action for endpoints 2 and 3.
    Optional<chip::Percent100ths> ep2NextPosition = NullOptional;
    Optional<chip::Percent100ths> ep3NextPosition = NullOptional;

    bool isEndPoint2ProgressPossible = false;
    bool isEndPoint3ProgressPossible = false;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(ep2CurrentState.Value(), ep2TargetState.Value(), ep2NextPosition))
    {
        VerifyOrReturn(ep2NextPosition.HasValue(),
                       ChipLogError(AppServer, "Failed to get next position for Endpoint 2"));
        ep2CurrentState.Value().position.SetValue(ep2NextPosition.Value());
        instance.ep2.GetLogic().SetCurrentState(ep2CurrentState);
        isEndPoint2ProgressPossible = (ep2NextPosition.Value() != ep2TargetState.Value().position.Value());
        ChipLogProgress(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d",
                        ep2NextPosition.Value(), ep2TargetState.Value().position.Value());
    }

    // Get the Next Current State to be set for the endpoint 3, if target postion is not reached.
    if (GetPanelNextPosition(ep3CurrentState.Value(), ep3TargetState.Value(), ep3NextPosition))
    {
        VerifyOrReturn(ep3NextPosition.HasValue(),
                       ChipLogError(AppServer, "Failed to get next position for Endpoint 3"));
        ep3CurrentState.Value().position.SetValue(ep3NextPosition.Value());
        instance.ep3.GetLogic().SetCurrentState(ep3CurrentState);
        isEndPoint3ProgressPossible = (ep3NextPosition.Value() != ep3TargetState.Value().position.Value());
        ChipLogProgress(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d",
                        ep3NextPosition.Value(), ep3TargetState.Value().position.Value());
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
        instance.CancelTimer(); // Cancel any existing timer before starting a new action
        instance.SetCurrentAction(MOVE_TO_ACTION);
        instance.StartTimer(kMotionCountdownTimeMs);
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
            instance.ep1.GetLogic().SetOverallCurrentState(ep1CurrentState);
            ep2CurrentState.Value().latch.SetValue(true);
            instance.ep2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(true);
            instance.ep3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "latched action complete");
        }
    }

    // Target reached and no latch action needed, call HandleClosureAction
    instance.HandleClosureActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
}

void ClosureManager::HandleClosureActionComplete(Action_t action)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    switch (action)
    {
    case Action_t::CALIBRATE_ACTION: {
        GetInstance().ep1.OnCalibrateActionComplete();
        GetInstance().ep2.OnCalibrateActionComplete();
        GetInstance().ep3.OnCalibrateActionComplete();
        instance.isCalibrationInProgress = false;
        break;
    }
    case Action_t::STOP_MOTION_ACTION:
        // This should handle the completion of a stop motion action.
        break;
    case Action_t::STOP_CALIBRATE_ACTION:
        // This should handle the completion of a stop calibration action.
        break;
    case Action_t::MOVE_TO_ACTION:
        instance.ep1.OnMoveToActionComplete();
        instance.ep2.OnMoveToActionComplete();
        instance.ep3.OnMoveToActionComplete();
        instance.isMoveToInProgress = false;
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }
    // Reset the current action after handling the closure action
    GetInstance().SetCurrentAction(Action_t::INVALID_ACTION);
}

bool ClosureManager::GetPanelNextPosition(const GenericCurrentStateStruct & currentState,
                                           const GenericTargetStruct & targetState,
                                           Optional<chip::Percent100ths> & nextPosition)
{
    VerifyOrReturnValue(targetState.position.HasValue(), false,
                        ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Target position is not set"));

    VerifyOrReturnValue(currentState.position.HasValue(), false,
                        ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current position is not set"));

    chip::Percent100ths currentPosition = currentState.position.Value();
    chip::Percent100ths targetPosition  = targetState.position.Value();

    if (currentPosition < targetPosition)
    {
        // Increment position by 1000 units, capped at target.
        nextPosition.SetValue(std::min(static_cast<chip::Percent100ths>(currentPosition + kMotionPositionStep), targetPosition));
    }
    else if (currentPosition > targetPosition)
    {
        // Moving down: Decreasing the current position by a step of 1000 units,
        // ensuring it does not go below the target position.
        nextPosition.SetValue(std::max(static_cast<chip::Percent100ths>(currentPosition - kMotionPositionStep), targetPosition));
    }
    else
    {
        // Already at target: No further action is needed as the current position matches the target position.
        nextPosition.SetValue(currentPosition);
        return false; // No update needed
    }
    return true;
}
