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

constexpr uint32_t kCountdownTimeSeconds = 10;
constexpr uint32_t kMotionCountdownTimeMs = 1000;

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
        break;
    case Action_t::SET_TARGET_ACTION:
        ChipLogDetail(AppServer, "Initiating set target action");
        // Timer used in sample application to simulate the set target process.
        // In a real application, this would be replaced with actual logic to set
        // the target position of the closure.
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
        // This should handle the completion of a stop motion action
        break;
    case Action_t::STOP_CALIBRATE_ACTION:
        // This should handle the completion of a stop calibration action
        break;
    case Action_t::MOVE_TO_ACTION:
        // This should handle the completion of a move-to action
        break;
    case Action_t::SET_TARGET_ACTION:
        instance.ep1.OnPanelMotionActionComplete();
        if (instance.mCurrentActionEndpointId == instance.ep2.GetEndpoint())
        {
            instance.ep2.OnPanelMotionActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.ep3.GetEndpoint())
        {
            instance.ep3.OnPanelMotionActionComplete();
        }
        instance.isSetTargetInProgress = false;
        break;
    case Action_t::PANEL_LATCH_ACTION:
        //For sample app, Action completion tasks for Panel Latch is same as SetTarget action.
        instance.ep1.OnPanelMotionActionComplete();
        if (instance.mCurrentActionEndpointId == instance.ep2.GetEndpoint())
        {
            instance.ep2.OnPanelMotionActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.ep3.GetEndpoint())
        {
            instance.ep3.OnPanelMotionActionComplete();
        }
        instance.isSetTargetInProgress = false;
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }
    // Reset the current action after handling the closure action
    instance.SetCurrentAction(Action_t::INVALID_ACTION);
    instance.mCurrentActionEndpointId = kInvalidEndpointId;
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
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                                const chip::Optional<bool> latch,
                                const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed)
{
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & position, 
                                                                             const Optional<bool> & latch, 
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             const chip::EndpointId endpointId)
{
    // Update OverallTarget of Closure based on SetTarget command.
    DataModel::Nullable<GenericOverallTarget> OverallTargetState;
    VerifyOrReturnValue(ep1.GetLogic().GetOverallTarget(OverallTargetState) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to get overall target for SetTarget command"));

    if (OverallTargetState.IsNull())
    {
        OverallTargetState.SetNonNull(GenericOverallTarget{});
    }

    if (position.HasValue())
    {
        // Set OverallTargetState position to NullOptional as panel position change cannot be represented in OverallTarget.
        OverallTargetState.Value().position = NullOptional;
    }

    if (latch.HasValue())
    {
        OverallTargetState.Value().latch.SetValue(latch.Value());
    }

    if (speed.HasValue())
    {
        OverallTargetState.Value().speed.SetValue(speed.Value());
    }

    VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(OverallTargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for SetTarget command"));

    AppEvent event;
    event.Type              = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = Action_t::SET_TARGET_ACTION;
    event.ClosureEvent.EndpointId = endpointId;
    event.Handler           = InitiateAction;

    AppTask::GetAppTask().PostEvent(&event);

    mCurrentAction = Action_t::SET_TARGET_ACTION;
    mCurrentActionEndpointId = endpointId;
    isSetTargetInProgress = true;

    return Status::Success;
}


void ClosureManager::HandlePanelSetTargetAction(EndpointId endpointId)
{
  ClosureManager & instance = ClosureManager::GetInstance();
  chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = nullptr;
  if (endpointId == instance.ep2.GetEndpoint())
  {
    ep = &instance.ep2;
  }
  else if (endpointId == instance.ep3.GetEndpoint())
  {
    ep = &instance.ep3;
  }
  else
  {
    ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId);
    return;
  }

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
    instance.SetCurrentAction(Action_t::SET_TARGET_ACTION);
    instance.StartTimer(kMotionCountdownTimeMs);
    return;
  }

  if (epState.target.Value().latch != epState.currentState.Value().latch)
  {
    // If the latch state is different between target and current state, we need to start a latch action timer
    ChipLogError(AppServer, "Latch action needed for Endpoint %d", endpointId);
    instance.SetCurrentAction(Action_t::PANEL_LATCH_ACTION);
    instance.StartTimer(kMotionCountdownTimeMs * 2);
    return;
  }
  else
  {
    // No latch action needed, complete the action immediately
    ChipLogError(AppServer, "No latch action needed for Endpoint %d", endpointId);
    instance.HandleClosureActionComplete(Action_t::SET_TARGET_ACTION);
  }
}
