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
#include <lib/support/TimeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {

constexpr uint32_t kDefaultCountdownTimeSeconds   = 10;    // 10 seconds
constexpr uint32_t kCalibrateTimerMs              = 10000; // 10 seconds
constexpr uint32_t kMotionCountdownTimeMs         = 1000;  // 1 second for each motion.
constexpr chip::Percent100ths kMotionPositionStep = 1000;  // 10% of the total range per motion interval.

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
    Action_t eventAction = static_cast<Action_t>(event->ClosureEvent.Action);

    ClosureManager & instance = ClosureManager::GetInstance();

    // We should not receive an event for a different action while another action is ongoing.
    // But due to asynchronous processing of commands and synchronous processing of the stop command,
    // this can happen if stop is received after InitaiteAction event is posted.
    // This is a safety check to ensure that we do not initiate a new action while another action is in progress.
    // If this happens, we log an error and do not proceed with initiating the action.
    VerifyOrReturn(eventAction == instance.GetCurrentAction(),
                   ChipLogError(AppServer, "Got Event for %d in InitiateAction while current ongoing action is %d",
                                to_underlying(eventAction), to_underlying(instance.GetCurrentAction())));

    instance.CancelTimer(); // Cancel any existing timer before starting a new action

    switch (eventAction)
    {
    case Action_t::CALIBRATE_ACTION:
        ChipLogDetail(AppServer, "Initiating calibration action");
        // Timer used in sample application to simulate the calibration process.
        // In a real application, this would be replaced with actual calibration logic.
        instance.StartTimer(kCalibrateTimerMs);
        break;
    case Action_t::MOVE_TO_ACTION:
        ChipLogDetail(AppServer, "Initiating move to action");
        // For Closure sample app, Motion action is simulated with a timer with rate
        // of 10% change of position per second.
        // In a real application, this would be replaced with actual move to logic.
        instance.StartTimer(kMotionCountdownTimeMs);
        break;
    case Action_t::UNLATCH_ACTION:
        ChipLogDetail(AppServer, "Initiating unlatch action");
        // Unlatch action check is a prerequisite for the move to action.
        // In a real application, this would be replaced with actual unlatch logic.
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
    event.Type                    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action     = closureManager->GetCurrentAction();
    event.ClosureEvent.EndpointId = closureManager->mCurrentActionEndpointId;
    event.Handler                 = HandleClosureActionCompleteEvent;
    AppTask::GetAppTask().PostEvent(&event);
}

void ClosureManager::HandleClosureActionCompleteEvent(AppEvent * event)
{
    Action_t currentAction = static_cast<Action_t>(event->ClosureEvent.Action);

    ClosureManager & instance = ClosureManager::GetInstance();

    // We should not receive an event for a different action while another action is ongoing.
    // But due to asynchronous processing of commands and synchronous processing of the stop command,
    // this can happen if stop is received after InitaiteAction event is posted.
    // This is a safety check to ensure that we do not initiate a new action while another action is in progress.
    // If this happens, we log an error and do not proceed with initiating the action.
    VerifyOrReturn(currentAction == instance.GetCurrentAction(),
                   ChipLogError(AppServer, "Got Event for %d in InitiateAction while current ongoing action is %d",
                                to_underlying(currentAction), to_underlying(instance.GetCurrentAction())));

    switch (currentAction)
    {
    case Action_t::CALIBRATE_ACTION:
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
    case Action_t::UNLATCH_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureUnlatchAction();
        });
        break;
    default:
        break;
    }
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    VerifyOrReturnValue(ep1.GetLogic().SetCountdownTimeFromDelegate(kDefaultCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for calibration"));

    SetCurrentAction(Action_t::CALIBRATE_ACTION);
    mCurrentActionEndpointId = ep1.GetEndpointId();

    isCalibrationInProgress = true;

    // Post an event to initiate the calibration action asynchronously.
    // Calibration can be only initiated from Closure Endpoint 1, so we set the endpoint ID to ep1.
    AppEvent event;
    event.Type                    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action     = GetCurrentAction();
    event.ClosureEvent.EndpointId = mCurrentActionEndpointId;
    event.Handler                 = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);
    SetCurrentAction(CALIBRATE_ACTION);

    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    // As Stop should be handled immediately, we will handle it synchronously.
    // For simulation purposes, we will just log the stop action and contnue to handle the stop action completion.
    // In a real application, this would be replaced with actual logic to stop the closure action.
    ChipLogDetail(AppServer, "Handling Stop command for closure action");

    CancelTimer();

    SetCurrentAction(Action_t::STOP_ACTION);
    mCurrentActionEndpointId = ep1.GetEndpointId();

    HandleClosureActionComplete(Action_t::STOP_ACTION);

    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> position,
                                                                          const Optional<bool> latch,
                                                                          const Optional<Globals::ThreeLevelAutoEnum> speed)
{

    // Update the target state for the closure panels based on the MoveTo command.
    // This closure sample app assumes that the closure panels are represented by two endpoints:
    // - Endpoint 2: Represents the Closure Dimension Cluster for the first panel.
    // - Endpoint 3: Represents the Closure Dimension Cluster for the second panel.

    // For sample app, MoveTo command to Fullopen , will set target position of both panels to 0
    //  MoveTo command to Fullclose will set target position of both panels to 10000
    //  We simulate harware action by using timer for 1 sec and updating the current state of the panels after the timer expires.
    //  till we reach the target position.

    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    VerifyOrReturnError(ep2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;
    VerifyOrReturnError(ep3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    VerifyOrReturnError(ep2.GetLogic().GetTargetState(ep2TargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;
    VerifyOrReturnError(ep3.GetLogic().GetTargetState(ep3TargetState) == CHIP_NO_ERROR, Status::Failure,
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
        Percent100ths ep2Position;
        Percent100ths ep3Position;

        switch (position.Value())
        {
        case TargetPositionEnum::kMoveToFullyClosed:
            ep2Position = static_cast<Percent100ths>(10000);
            ep3Position = static_cast<Percent100ths>(10000);
            break;
        case TargetPositionEnum::kMoveToFullyOpen:
            ep2Position = static_cast<Percent100ths>(0);
            ep3Position = static_cast<Percent100ths>(0);
            break;
        case TargetPositionEnum::kMoveToPedestrianPosition:
            ep2Position = static_cast<Percent100ths>(3000);
            ep3Position = static_cast<Percent100ths>(3000);
            break;
        case TargetPositionEnum::kMoveToSignaturePosition:
            ep2Position = static_cast<Percent100ths>(2000);
            ep3Position = static_cast<Percent100ths>(2000);
            break;
        case TargetPositionEnum::kMoveToVentilationPosition:
            ep2Position = static_cast<Percent100ths>(1000);
            ep3Position = static_cast<Percent100ths>(1000);
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

    VerifyOrReturnError(ep2.GetLogic().SetTargetState(DataModel::MakeNullable(ep2Target)) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(ep3.GetLogic().SetTargetState(DataModel::MakeNullable(ep3Target)) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Endpoint 3"));

    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Set the current action to UNLATCH_ACTION.
    // This is to ensure that the closure is unlatched before starting the motion action.
    // The Closure Control Cluster will handle the unlatch action before proceeding with the motion action.
    SetCurrentAction(UNLATCH_ACTION);
    isMoveToInProgress = true;

    // Post an event to initiate the move to action asynchronously.
    // MoveTo Command can only be initiated from Closure Control Endpoint (Endpoint 1).
    AppEvent event;
    event.Type                = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = mCurrentAction;
    event.Handler             = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);

    return Status::Success;
}

void ClosureManager::HandleClosureMotionAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;

    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;

    VerifyOrReturn(ep2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(ep3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    VerifyOrReturn(ep2.GetLogic().GetTargetState(ep2TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    VerifyOrReturn(ep3.GetLogic().GetTargetState(ep3TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturn(!ep2CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!ep3CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    VerifyOrReturn(!ep2TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 2"));
    VerifyOrReturn(!ep3TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 3"));

    // Once Closure is unlatched, we can proceed with the motion action for endpoints 2 and 3.
    DataModel::Nullable<Percent100ths> ep2NextPosition = DataModel::NullNullable;
    DataModel::Nullable<Percent100ths> ep3NextPosition = DataModel::NullNullable;

    bool isEndPoint2ProgressPossible = false;
    bool isEndPoint3ProgressPossible = false;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(ep2CurrentState.Value(), ep2TargetState.Value(), ep2NextPosition))
    {
        VerifyOrReturn(!ep2NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 2"));
        ep2CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep2NextPosition.Value()));
        instance.ep2.GetLogic().SetCurrentState(ep2CurrentState);
        isEndPoint2ProgressPossible = (ep2NextPosition.Value() != ep2TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", ep2NextPosition.Value(),
                        ep2TargetState.Value().position.Value().Value());
    }

    // Get the Next Current State to be set for the endpoint 3, if target postion is not reached.
    if (GetPanelNextPosition(ep3CurrentState.Value(), ep3TargetState.Value(), ep3NextPosition))
    {
        VerifyOrReturn(!ep3NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 3"));
        ep3CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep3NextPosition.Value()));
        instance.ep3.GetLogic().SetCurrentState(ep3CurrentState);
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
        instance.CancelTimer(); // Cancel any existing timer before starting a new action
        instance.SetCurrentAction(MOVE_TO_ACTION);
        instance.StartTimer(kMotionCountdownTimeMs);
        return;
    }

    DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState;
    DataModel::Nullable<GenericOverallTargetState> ep1TargetState;

    VerifyOrReturn(ep1.GetLogic().GetOverallCurrentState(ep1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(ep1.GetLogic().GetOverallTargetState(ep1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(!ep1CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!ep1TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 1"));

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
            ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.ep2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.ep3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "latched action complete");
        }
    }

    // Target reached and no latch action needed, call HandleClosureAction
    instance.HandleClosureActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
}

void ClosureManager::HandleClosureUnlatchAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState;
    DataModel::Nullable<GenericOverallTargetState> ep1TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;

    VerifyOrReturn(ep1.GetLogic().GetOverallCurrentState(ep1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(ep1.GetLogic().GetOverallTargetState(ep1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(ep2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(ep3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    VerifyOrReturn(!ep1CurrentState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!ep1TargetState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Target state on Endpoint 1"));

    VerifyOrReturn(!ep2CurrentState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!ep3CurrentState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Current state on Endpoint 3"));

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
            ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.ep2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.ep3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "Unlatched action completed");
        }
    }

    CancelTimer(); // Cancel any existing timer before proceeding with the motion action

    // After unlatching, we can proceed with the motion action
    instance.HandleClosureMotionAction();
}

void ClosureManager::HandleClosureActionComplete(Action_t action)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    switch (action)
    {
    case Action_t::CALIBRATE_ACTION: {
        instance.ep1.OnCalibrateActionComplete();
        instance.ep2.OnCalibrateActionComplete();
        instance.ep3.OnCalibrateActionComplete();
        isCalibrationInProgress = false;
        break;
    }
    case Action_t::MOVE_TO_ACTION:
        instance.ep1.OnMoveToActionComplete();
        instance.ep2.OnMoveToActionComplete();
        instance.ep3.OnMoveToActionComplete();
        instance.isMoveToInProgress = false;
        break;
    case Action_t::UNLATCH_ACTION:
        // HandleClosureActionComplete should not be called for UNLATCH_ACTION,
        // as unlatch action is prerequisite for MOVE_TO_ACTION.
        ChipLogError(AppServer, "HandleClosureActionComplete should not be called for UNLATCH_ACTION");
        break;
    case Action_t::STOP_ACTION: {
        if (isCalibrationInProgress)
        {
            ChipLogDetail(AppServer, "Stopping calibration action");
            instance.ep1.OnStopCalibrateActionComplete();
            instance.ep2.OnStopCalibrateActionComplete();
            instance.ep3.OnStopCalibrateActionComplete();
            isCalibrationInProgress = false;
        }
        else if (isMoveToInProgress)
        {
            ChipLogDetail(AppServer, "Stopping move to action");
            instance.ep1.OnStopMotionActionComplete();
            instance.ep2.OnStopMotionActionComplete();
            instance.ep3.OnStopMotionActionComplete();
            isMoveToInProgress = false;
        }
        else
        {
            ChipLogDetail(AppServer, "No action in progress to stop");
        }
        break;
    }
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }
    // Reset the current action and current action endpoint ID after handling the closure action
    instance.SetCurrentAction(Action_t::INVALID_ACTION);
    instance.mCurrentActionEndpointId = chip::kInvalidEndpointId;
}

bool ClosureManager::GetPanelNextPosition(const GenericDimensionStateStruct & currentState,
                                          const GenericDimensionStateStruct & targetState,
                                          DataModel::Nullable<Percent100ths> & nextPosition)
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
