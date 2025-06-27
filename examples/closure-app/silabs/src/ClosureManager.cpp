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
#include <lib/support/TimeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {
constexpr uint32_t kDefaultCountdownTimeSeconds = 10; // 10 seconds
constexpr uint32_t kCalibrateTimerMs  = 10000; // 10 seconds

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
    Action_t eventAction = static_cast<uint8_t>(event->ClosureEvent.Action);

    ClosureManager & instance = ClosureManager::GetInstance();

    // ideally, we should not receive an event for a different action while another action is ongoing.
    // But due to asynchronous processing of command and stop command, this can happen if stop is received
    // after InitaiteAction event is posted.
    // This is a safety check to ensure that we do not initiate a new action while another action is in progress.
    // If this happens, we log an error and do not proceed with initiating the action.
    if (eventAction != instance.GetCurrentAction())
    {
        ChipLogError(AppServer, "Got Event for %d in InitiateAction while current ongoing action is %d",
                     to_underlying(eventAction), to_underlying(instance.GetCurrentAction()));
        return;
    }

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
    Action_t currentAction = static_cast<uint8_t>(event->ClosureEvent.Action);

    ClosureManager & instance = ClosureManager::GetInstance();

    // ideally, we should not receive an event for a different action while another action is ongoing.
    // But due to asynchronous processing of command and stop command, this can happen if stop is received
    // after InitaiteAction event is posted.
    // This is a safety check to ensure that we do not initiate a new action while another action is in progress.
    // If this happens, we log an error and do not proceed with initiating the action.
    if (currentAction != instance.GetCurrentAction())
    {
        ChipLogError(AppServer, "Got Event for %d in InitiateAction while current ongoing action is %d",
                     to_underlying(currentAction), to_underlying(instance.GetCurrentAction()));
        return;
    }

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
            instance.HandleClosureActionComplete(instance.GetCurrentAction());
        });
        break;
    default:
        break;
    }
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    VerifyOrReturnValue(ep1.GetLogic().SetCountdownTimeFromDelegate(kDefaultCountdownTimeSeconds) == CHIP_NO_ERROR, 
            Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for calibration"));

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

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                                const chip::Optional<bool> latch,
                                const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed)
{
    // this function will contain the code for MoveTo command handling and initiate the motion action.
    return Status::Success;
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
        // This should handle the completion of a move-to action.
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
