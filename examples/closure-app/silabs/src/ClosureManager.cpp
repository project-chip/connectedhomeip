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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {

struct ClosureTimerContext
{
    ClosureManager* instance;
    ClosureManager::Action_t action;
    chip::EndpointId endpointId;
};

constexpr uint32_t kCountdownTimeSeconds = 10;

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
    instance.SetCurrentAction(action);
    instance.mCurrentActionEndpointId = static_cast<EndpointId>(event->ClosureEvent.EndpointId);

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
        instance.StartTimer(kCountdownTimeSeconds * 100);
        break;
    case Action_t::SET_TARGET_ACTION:
        ChipLogDetail(AppServer, "Initiating set target action");
        // Timer used in sample application to simulate the set target process.
        // In a real application, this would be replaced with actual logic to set
        // the target position of the closure.
        instance.StartTimer(kCountdownTimeSeconds * 100);
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
    event.ClosureEvent.EndpointId = closureManager->mCurrentActionEndpointId;
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
    case Action_t::LATCH_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureActionComplete(instance.GetCurrentAction());
        });
        break;
    case Action_t::SET_TARGET_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandlePanelSetTargetAction(instance.mCurrentActionEndpointId);
        });
        break;
    case Action_t::PANEL_LATCH_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandleClosureActionComplete(instance.GetCurrentAction());
        });
        break;
    default:
        break;
    }
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
        instance.isCalibrationInProgress = false;
        break;
    }
    case Action_t::STOP_MOTION_ACTION:
        instance.ep1.OnStopMotionActionComplete();
        instance.ep2.OnStopMotionActionComplete();
        instance.ep3.OnStopMotionActionComplete();
        instance.isStopInProgress = false;
        break;
    case Action_t::STOP_CALIBRATE_ACTION:
        instance.ep1.OnStopCalibrateActionComplete();
        instance.ep2.OnStopCalibrateActionComplete();
        instance.ep3.OnStopCalibrateActionComplete();
        instance.isStopInProgress = false;   
        break;
    case Action_t::MOVE_TO_ACTION:
        instance.ep1.OnMoveToActionComplete();
        instance.ep2.OnMoveToActionComplete();
        instance.ep3.OnMoveToActionComplete();
        instance.isMoveToInProgress = false;
        break;
    case Action_t::LATCH_ACTION:
        // For sample app, Action completion tasks for Latch is same as MoveTo action.
        instance.ep1.OnMoveToActionComplete();
        instance.ep2.OnMoveToActionComplete();
        instance.ep3.OnMoveToActionComplete();
        instance.isMoveToInProgress = false;
        break;
    case Action_t::SET_TARGET_ACTION:
        instance.ep1.OnSetTargetActionComplete();
        if (instance.mCurrentActionEndpointId == instance.ep2.GetEndpoint())
        {
            instance.ep2.OnSetTargetActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.ep3.GetEndpoint())
        {
            instance.ep3.OnSetTargetActionComplete();
        }
        instance.isSetTargetInProgress = false;
        break;
    case Action_t::PANEL_LATCH_ACTION:
        //For sample app, Action completion tasks for Panel Latch is same as SetTarget action.
        instance.ep1.OnSetTargetActionComplete();
        if (instance.mCurrentActionEndpointId == instance.ep2.GetEndpoint())
        {
            instance.ep2.OnSetTargetActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.ep3.GetEndpoint())
        {
            instance.ep3.OnSetTargetActionComplete();
        }
        instance.isSetTargetInProgress = false;
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }
    // Reset the current action after handling the closure action
    instance.SetCurrentAction(Action_t::INVALID_ACTION);
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    VerifyOrReturnValue(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for calibration"));

    // Post an event to initiate the calibration action asynchronously.
    AppEvent event;
    event.Type                = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = CALIBRATE_ACTION;
    event.ClosureEvent.EndpointId = ep1.GetEndpoint();
    event.Handler             = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);

    isCalibrationInProgress = true;
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
  ClosureManager::Action_t action;
  if (isCalibrationInProgress ) 
    {
      ChipLogError(AppServer, "Stopping calibration action"); 
      isCalibrationInProgress = false;
      action = ClosureManager::Action_t::STOP_CALIBRATE_ACTION; 
    }
    else if (isMoveToInProgress)
    {
      ChipLogError(AppServer, "Stopping Motion action");
      isMoveToInProgress = false;
      action = ClosureManager::Action_t::STOP_MOTION_ACTION;
    } 
    else
    {
      action = ClosureManager::Action_t::INVALID_ACTION;
    }

    // Post an event to initiate the stop action asynchronously.
    AppEvent event;
    event.Type              = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = action;
    event.ClosureEvent.EndpointId = ep1.GetEndpoint();
    event.Handler           = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);

    isStopInProgress = true;
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                                const chip::Optional<bool> latch,
                                const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed)
{
    if(IsMotionActionInProgress())
    {
        ChipLogProgress(AppServer, "Closure motion action is already in progress on %d, "
                                    "Wait for motion to complete or stop motion using stop command",
                                    static_cast<int>(mCurrentActionEndpointId));
        return Status::Failure;
    }

    // Update the target state for the closure panels based on the MoveTo command.
    // This closure sample app assumes that the closure panels are represented by three endpoints:
    // - Endpoint 1: Represents the Closure Control Cluster.
    // - Endpoint 2: Represents the Closure Dimension Cluster for the first panel.
    // - Endpoint 3: Represents the Closure Dimension Cluster for the second panel.
    chip::app::Clusters::ClosureDimension::ClusterState ep2State = ep2.GetLogic().GetState();
    chip::app::Clusters::ClosureDimension::ClusterState ep3State = ep3.GetLogic().GetState();

    DataModel::Nullable<GenericCurrentStateStruct> ep2CurrentState = ep2State.currentState;
    DataModel::Nullable<GenericCurrentStateStruct> ep3CurrentState = ep3State.currentState;

    VerifyOrReturnValue(!ep2CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturnValue(!ep3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    GenericTargetStruct ep2Target = ep2State.target.IsNull() ? GenericTargetStruct() : ep2State.target.Value();
    GenericTargetStruct ep3Target = ep3State.target.IsNull() ? GenericTargetStruct() : ep3State.target.Value();

    if (position.HasValue())
    {
        ChipLogError(AppServer, "Updating target position for move to command");
       // Set the Closure panel target position for the panels based on the Closure position.
       // The position is represented as a TargetPositionEnum, which maps to specific positions for the panels.
       // The mapping values used below are for closure sample app.
        chip::Percent100ths ep2Position;
        chip::Percent100ths ep3Position;

        switch (position.Value())
        {
        case TargetPositionEnum::kCloseInFull:
            ep2Position = static_cast<chip::Percent100ths>(10000);
            ep3Position = static_cast<chip::Percent100ths>(10000);
            break;
        case TargetPositionEnum::kOpenInFull:
            ep2Position = static_cast<chip::Percent100ths>(0);
            ep3Position = static_cast<chip::Percent100ths>(0);
            break;
        case TargetPositionEnum::kPedestrian:
            ep2Position = static_cast<chip::Percent100ths>(3000);
            ep3Position = static_cast<chip::Percent100ths>(3000);
            break;
        case TargetPositionEnum::kSignature:
            ep2Position = static_cast<chip::Percent100ths>(2000);
            ep3Position = static_cast<chip::Percent100ths>(2000);
            break;
        case TargetPositionEnum::kVentilation:
            ep2Position = static_cast<chip::Percent100ths>(1000);
            ep3Position = static_cast<chip::Percent100ths>(1000);
            break;
        default:
            ChipLogProgress(AppServer, "Invalid target position received in OnMoveToCommand");
            return Status::Failure;
        }

        ep2Target.position.SetValue(ep2Position);
        ep3Target.position.SetValue(ep3Position);
    }

    if (latch.HasValue())
    {
      ChipLogError(AppServer, "Updating target latch for move to command");
        ep2Target.latch.SetValue(latch.Value());
        ep3Target.latch.SetValue(latch.Value());
    }

    if (speed.HasValue())
    {
      ChipLogError(AppServer, "Updating target speed for move to command");
        ep2Target.speed.SetValue(speed.Value());
        ep3Target.speed.SetValue(speed.Value());
    }

    VerifyOrReturnError(ep2.GetLogic().SetTarget(DataModel::MakeNullable(ep2Target)) == CHIP_NO_ERROR,  Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(ep3.GetLogic().SetTarget(DataModel::MakeNullable(ep3Target)) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set target for Endpoint 3"));
    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Post an event to initiate the move to action asynchronously.
    AppEvent event;
    event.Type              = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = MOVE_TO_ACTION;
    event.ClosureEvent.EndpointId = ep1.GetEndpoint();
    event.Handler           = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);

    isMoveToInProgress = true;
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & pos, 
                                                                             const Optional<bool> & latch, 
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             chip::EndpointId endpointId)
{
    if(IsMotionActionInProgress())
    {
        ChipLogProgress(AppServer, "Closure motion action is already in progress on %d, "
                                    "Wait for motion to complete or stop motion using stop command",
                                    static_cast<int>(mCurrentActionEndpointId));
        return Status::Failure;
    }

    VerifyOrReturnError(ep1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Update Overall Target to Null for the Closure Control on Endpoint 1
    DataModel::Nullable<GenericOverallTarget> ep1Target;
    ep1Target.SetNonNull(GenericOverallTarget{});

    VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set overall target for SetTarget command"));

    AppEvent event;
    event.Type              = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = SET_TARGET_ACTION;
    event.ClosureEvent.EndpointId = endpointId;
    event.Handler           = InitiateAction;   
    AppTask::GetAppTask().PostEvent(&event);

    isSetTargetInProgress = true;

    return Status::Success;
}

void ClosureManager::HandleClosureMotionAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    chip::app::Clusters::ClosureControl::ClusterState ep1State = instance.ep1.GetLogic().GetState();
    chip::app::Clusters::ClosureDimension::ClusterState ep2State = instance.ep2.GetLogic().GetState();
    chip::app::Clusters::ClosureDimension::ClusterState ep3State = instance.ep3.GetLogic().GetState();

    DataModel::Nullable<GenericCurrentStateStruct> currentState = DataModel::NullNullable;
    
    bool isEndPoint2TargetReached = false;
    bool isEndPoint3TargetReached = false;

    if (UpdatePanelCurrentStateToNextPosition(ep2State, currentState))
    {
        // Update the current state for Endpoint 2
        instance.ep2.GetLogic().SetCurrentState(currentState);
        isEndPoint2TargetReached = (currentState.Value().position.Value() == ep2State.target.Value().position.Value());
        ChipLogError(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", currentState.Value().position.Value(),
                                                                                          ep2State.target.Value().position.Value());
    }

    if (UpdatePanelCurrentStateToNextPosition(ep3State, currentState))
    {
        // Update the current state for Endpoint 3
        instance.ep3.GetLogic().SetCurrentState(currentState);
        isEndPoint3TargetReached = (currentState.Value().position.Value() == ep3State.target.Value().position.Value());
        ChipLogError(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d", currentState.Value().position.Value(),
                                                                                          ep3State.target.Value().position.Value());
    }

    bool closureTargetReached = isEndPoint2TargetReached || isEndPoint3TargetReached;

    ChipLogError(AppServer, "Motion progress possible: %s", closureTargetReached ? "true" : "false");

    if (!closureTargetReached)
    {
        instance.CancelTimer(); // Cancel any existing timer before starting a new action
        instance.SetCurrentAction(MOVE_TO_ACTION);
        instance.StartTimer(kCountdownTimeSeconds * 100);
        return;
    }

    if (IsClosureLatchActionNeeded(ep1State)){
        ChipLogError(AppServer, "Starting latch action timer");
        instance.SetCurrentAction(LATCH_ACTION);
        instance.StartTimer(kCountdownTimeSeconds * 200);
    } else {
      // Target reached and no latch action needed, call HandleClosureAction
      instance.HandleClosureActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
    }
}

void ClosureManager::HandlePanelSetTargetAction(EndpointId endpointId)
{
  ClosureManager & instance = ClosureManager::GetInstance();
  chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = (endpointId == instance.ep2.GetEndpoint()) ? &instance.ep2 : &instance.ep3;

  chip::app::Clusters::ClosureDimension::ClusterState epState = ep->GetLogic().GetState();
  DataModel::Nullable<GenericCurrentStateStruct> currentState = DataModel::NullNullable;
  
  bool panelTargetReached = false;

  if(UpdatePanelCurrentStateToNextPosition(epState, currentState))
  {
    ep->GetLogic().SetCurrentState(currentState);
    panelTargetReached = (currentState.Value().position.Value() == epState.target.Value().position.Value());
    ChipLogError(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", ep->GetDelegate().GetEndpoint(), currentState.Value().position.Value(),
                                                                                            epState.target.Value().position.Value());
  }

  if (!panelTargetReached)
  {
    instance.CancelTimer(); // Cancel any existing timer before starting a new action
    instance.SetCurrentAction(SET_TARGET_ACTION);
    instance.StartTimer(kCountdownTimeSeconds * 100);
    return;
  }

  if (epState.target.Value().latch != epState.currentState.Value().latch)
  {
    // If the latch state is different between target and current state, we need to start a latch action timer
    ChipLogError(AppServer, "Latch action needed for Endpoint %d", endpointId);
    instance.SetCurrentAction(PANEL_LATCH_ACTION);
    instance.StartTimer(kCountdownTimeSeconds * 200);
    return;
  }
  else
  {
    // No latch action needed, complete the action immediately
    ChipLogError(AppServer, "No latch action needed for Endpoint %d", endpointId);
    instance.HandleClosureActionComplete(SET_TARGET_ACTION);
  }
}


bool ClosureManager::UpdatePanelCurrentStateToNextPosition(
                                const chip::app::Clusters::ClosureDimension::ClusterState & epState,
                                DataModel::Nullable<GenericCurrentStateStruct> & currentState) 

{
  if (epState.target.IsNull())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Target State is null");
      return false;
  }

  if (!epState.target.Value().position.HasValue())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to  Target position is not set");
      return false;
  }

  if (epState.currentState.IsNull())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current State is null");
      return false;
  }

  if (!epState.currentState.Value().position.HasValue())
  {
      ChipLogError(AppServer, "Updating CurrentState to NextPosition failed due to Current position is not set");
      return false;
  }

  chip::Percent100ths currentPosition = epState.currentState.Value().position.Value();
  chip::Percent100ths targetPosition = epState.target.Value().position.Value();
  chip::Percent100ths nextCurrentPosition;

  if (currentPosition < targetPosition)
  {
      // Increment position by 1000 units, capped at target.
      nextCurrentPosition = std::min(static_cast<chip::Percent100ths>(currentPosition + 1000), targetPosition);
  }
  else if (currentPosition > targetPosition)
  {
      // Moving down: Decreasing the current position by a step of 1000 units, 
      // ensuring it does not go below the target position.
      nextCurrentPosition = std::max(static_cast<chip::Percent100ths>(currentPosition - 1000), targetPosition);
  }
  else
  {
      // Already at target: No further action is needed as the current position matches the target position.
      nextCurrentPosition = currentPosition;
      return false; // No update needed
  }
  
  currentState.SetNonNull().Set(
            MakeOptional(nextCurrentPosition),
            epState.currentState.Value().latch.HasValue() ? MakeOptional(epState.currentState.Value().latch.Value()) : NullOptional,
            epState.currentState.Value().speed.HasValue() ? MakeOptional(epState.currentState.Value().speed.Value()) : NullOptional
  );
  return true;
}

bool ClosureManager::IsClosureLatchActionNeeded(const chip::app::Clusters::ClosureControl::ClusterState & epState)
{
  // Latch action not needed if OverallTarget is null or latch is not set
  if (epState.mOverallTarget.IsNull() || !epState.mOverallTarget.Value().latch.HasValue())
  {
    ChipLogError(AppServer, "Latch action not needed as OverallTarget is null or latch is not set");
    return false;
  }

  // latch action needed if OverallState is null or latch is not set and OverallTarget has latch set
  if (epState.mOverallState.IsNull() || !epState.mOverallState.Value().latch.HasValue() || 
      epState.mOverallState.Value().latch.Value().IsNull())
  {
    ChipLogError(AppServer, "Latch action needed as OverallState is null or latch is not set, while OverallTarget has latch set");
    return true;
  }

  // Only return true if the latch value is different between target and state
  bool targetLatch = epState.mOverallTarget.Value().latch.Value();
  bool stateLatch = epState.mOverallState.Value().latch.Value().Value();
  ChipLogError(AppServer, "Target Latch: %s, State Latch: %s", 
               targetLatch ? "true" : "false", stateLatch ? "true" : "false");
  return targetLatch != stateLatch;
}