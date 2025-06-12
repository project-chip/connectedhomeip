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
#include "AppTask.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {
constexpr uint32_t kCountdownTimeSeconds = 10;
ClosureManager::Action_t mCurrentAction = ClosureManager::Action_t::INVALID_ACTION;

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
        SILABS_LOG("mClosureTimer timer create failed");
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
        SILABS_LOG("mClosureTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}


void ClosureManager::CancelTimer()
{
    if (osTimerStop(mClosureTimer) == osError)
    {
        SILABS_LOG("mClosureTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void ClosureManager::InitiateAction(AppEvent * event)
{
    mCurrentAction = static_cast<Action_t>(event->ClosureEvent.Action);
    switch (mCurrentAction)
        {
        case Action_t::CALIBRATE_ACTION:
            ChipLogDetail(AppServer, "Initiating calibration action");
            break;
        case Action_t::STOP_MOTION_ACTION:
            ChipLogDetail(AppServer, "Initiating stop motion action");
            break;
        case Action_t::STOP_CALIBRATE_ACTION:
            ChipLogDetail(AppServer, "Initiating stop calibration action");
            ClosureManager::GetInstance().StartTimer(kCountdownTimeSeconds * 1000);
            break;
        case Action_t::MOVE_TO_ACTION:
            ChipLogDetail(AppServer, "Initiating move to action");
            break;
        default:
            ChipLogDetail(AppServer, "Invalid action received in InitiateAction");
            return;
        }
}

void ClosureManager::HandleClosureEvent(AppEvent * event)
{
    mCurrentAction = static_cast<ClosureManager::Action_t>(event->ClosureEvent.Action);
    chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t){ ClosureManager::GetInstance().HandleClosureActionComplete(mCurrentAction); });
}

void ClosureManager::TimerEventHandler(void * timerCbArg)
{
    // The timer event handler will be called in the context of the timer task
    // once sClosureTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type              = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = mCurrentAction;
    event.Handler           = HandleClosureEvent;
    AppTask::GetAppTask().PostEvent(&event);
}

chip::Protocols::InteractionModel::Status  ClosureManager::OnCalibrateCommand()
{
  // Before calibration, we set cuurent and target state of all endpoints to null and set countdowntime.
  VerifyOrReturnValue(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set countdown time for calibration"));
  VerifyOrReturnValue(ep1.GetLogic().SetOverallState(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set overall state for calibration"));
  VerifyOrReturnValue(ep1.GetLogic().SetOverallTarget(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set overall target for calibration"));
  VerifyOrReturnValue(ep2.GetLogic().SetCurrentState(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set current state for calibration on Endpoint 2"));
  VerifyOrReturnValue(ep2.GetLogic().SetTarget(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set target for calibration on Endpoint 2"));
  VerifyOrReturnValue(ep3.GetLogic().SetCurrentState(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set current state for calibration on Endpoint 3"));
  VerifyOrReturnValue(ep3.GetLogic().SetTarget(DataModel::NullNullable) == CHIP_NO_ERROR, Status::Failure,
                      ChipLogError(AppServer, "Failed to set target for calibration on Endpoint 3"));

    
    AppEvent event;
    event.Type              = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action = CALIBRATE_ACTION;
    event.Handler           = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);
    
    isCalibrationInProgress = true;
    return Status::Success;
}

chip::Protocols::InteractionModel::Status  ClosureManager::OnStopCommand()
{
  // this function will contain the code for Stop command handling and initiate the stop action.
  return Status::Success; 
}

chip::Protocols::InteractionModel::Status ClosureManager::OnMoveToCommand(
        const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum>  position,
        const chip::Optional<bool>  latch,
        const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum>  speed)
{
  // this function will contain the code for MoveTo command handling and initiate the motion action.
  return Status::Success;
}

void ClosureManager::HandleClosureActionComplete(Action_t action)
{
        switch (action)
        {
        case Action_t::CALIBRATE_ACTION:
        {
            isCalibrationInProgress = false;
            GetInstance().ep1.OnCalibrateActionComplete();
            GetInstance().ep2.OnCalibrateActionComplete();
            GetInstance().ep3.OnCalibrateActionComplete();
            break;
        }
        case Action_t::STOP_MOTION_ACTION:
          break;
        case Action_t::STOP_CALIBRATE_ACTION:
          break;
        case Action_t::MOVE_TO_ACTION:
          break;
        default:
            ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
            break;
        }
}