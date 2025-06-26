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
constexpr uint32_t kCountdownTimeSeconds  = 10;
constexpr ElapsedS kMotionCountdownTimeMs = 1000;

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
    case Action_t::PANEL_STEP_ACTION:
        ChipLogDetail(AppServer, "Initiating step action");
        // Timer used in sample application to simulate the step action process.
        // In a real application, this would be replaced with actual logic to perform
        // a step action on the closure.
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
    case Action_t::PANEL_STEP_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandlePanelStepAction(instance.mCurrentActionEndpointId);
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
        // This should handle the completion of a stop motion action.
        break;
    case Action_t::STOP_CALIBRATE_ACTION:
        // This should handle the completion of a stop calibration action.
        break;
    case Action_t::MOVE_TO_ACTION:
        // This should handle the completion of a move to action.
        break;
    case Action_t::PANEL_STEP_ACTION:
        instance.ep1.OnStepActionComplete();
        if (instance.mCurrentActionEndpointId == instance.ep2.GetEndpoint())
        {
            instance.ep2.OnStepActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.ep3.GetEndpoint())
        {
            instance.ep3.OnStepActionComplete();
        }
        instance.isStepActionInProgress = false;
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
    event.Type                    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action     = CALIBRATE_ACTION;
    event.ClosureEvent.EndpointId = ep1.GetEndpoint();
    event.Handler                 = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);
    SetCurrentAction(CALIBRATE_ACTION);
    mCurrentActionEndpointId = ep1.GetEndpoint();

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
    // this function will contain the code for MoveTo command handling and initiate the motion action.
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                        const uint16_t & numberOfSteps,
                                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                        const chip::EndpointId & endpointId)
{
    if (isStepActionInProgress && mCurrentActionEndpointId != endpointId)
    {
        ChipLogError(AppServer, "Step action is already in progress on Endpoint %d", mCurrentActionEndpointId);
        return Status::Failure;
    }

    VerifyOrReturnError(ep1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Update Overall Target to Null for the Closure Control on Endpoint 1
    DataModel::Nullable<GenericOverallTarget> ep1Target;

    VerifyOrReturnValue(ep1.GetLogic().GetOverallTarget(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get overall target for Step command"));

    if (ep1Target.IsNull())
    {
        ep1Target.SetNonNull(GenericOverallTarget{});
    }

    ep1Target.Value().position = NullOptional; // Reset position to Null

    VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for Step command"));

    AppEvent event;
    event.Type                    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action     = PANEL_STEP_ACTION;
    event.ClosureEvent.EndpointId = endpointId;
    event.Handler                 = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);

    SetCurrentAction(PANEL_STEP_ACTION);
    mCurrentActionEndpointId = endpointId;

    isStepActionInProgress = true;

    return Status::Success;
}

void ClosureManager::HandlePanelStepAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep =
        (endpointId == instance.ep2.GetDelegate().GetEndpoint()) ? &instance.ep2 : &instance.ep3;

    chip::app::Clusters::ClosureDimension::ClusterState epState = ep->GetLogic().GetState();
    StepDirectionEnum stepDirection                             = ep->GetDelegate().GetStepCommandTargetDirection();
    GenericCurrentStateStruct currentState;

    VerifyOrReturn(!epState.currentState.IsNull() && epState.currentState.Value().position.HasValue(),
                   ChipLogError(AppServer, "Current state is null, Step action Failed"));
    VerifyOrReturn(!epState.target.IsNull() && epState.target.Value().position.HasValue(),
                   ChipLogError(AppServer, "Target state is null, Step action Failed"));

    chip::Percent100ths currentPosition = epState.currentState.Value().position.Value();
    chip::Percent100ths targetPosition  = epState.target.Value().position.Value();
    ChipLogProgress(AppServer, "Current Position: %d, Target Position: %d", currentPosition, targetPosition);
    bool panelTargetReached = (currentPosition == targetPosition);
    ChipLogProgress(AppServer, "Panel Target Reached: %s", panelTargetReached ? "true" : "false");
    if (!panelTargetReached)
    {
        chip::Percent100ths nextCurrentPosition;

        // Compute the next position
        if (stepDirection == StepDirectionEnum::kIncrease)
        {
            nextCurrentPosition = std::min(static_cast<chip::Percent100ths>(currentPosition + epState.stepValue), targetPosition);
        }
        else
        {
            nextCurrentPosition = std::max(static_cast<chip::Percent100ths>(currentPosition - epState.stepValue), targetPosition);
        }

        // Set the new state once
        currentState.Set(MakeOptional(nextCurrentPosition),
                         epState.currentState.Value().latch.HasValue() ? MakeOptional(epState.currentState.Value().latch.Value())
                                                                       : NullOptional,
                         epState.currentState.Value().speed.HasValue() ? MakeOptional(epState.currentState.Value().speed.Value())
                                                                       : NullOptional);

        ep->GetLogic().SetCurrentState(DataModel::MakeNullable(currentState));

        instance.CancelTimer(); // Cancel any existing timer before starting a new action
        instance.SetCurrentAction(PANEL_STEP_ACTION);
        instance.mCurrentActionEndpointId = endpointId;
        instance.StartTimer(kMotionCountdownTimeMs);

        return;
    }

    instance.HandleClosureActionComplete(PANEL_STEP_ACTION);
}
