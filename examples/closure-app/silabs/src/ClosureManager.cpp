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
constexpr uint32_t kDefaultCountdownTimeSeconds = 10;    // 10 seconds
constexpr uint32_t kCalibrateTimerMs            = 10000; // 10 seconds

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
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEndpoint1TagList[] = {
    { .namespaceID = kNamespaceClosure,
      .tag         = kTagClosureCovering,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) },
    { .namespaceID = kNamespaceCovering,
      .tag         = kTagCoveringVenetian,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEndpoint2TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelLift,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kEndpoint3TagList[] = {
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
    mClosureEndpoint1.Init();
    mClosurePanelEndpoint2.Init();
    mClosurePanelEndpoint3.Init();

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ 1, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEndpoint1TagList));
    SetTagList(/* endpoint= */ 2, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEndpoint2TagList));
    SetTagList(/* endpoint= */ 3, Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kEndpoint3TagList));

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
        instance.mClosureEndpoint1.OnCalibrateActionComplete();
        instance.mClosurePanelEndpoint2.OnCalibrateActionComplete();
        instance.mClosurePanelEndpoint3.OnCalibrateActionComplete();
        isCalibrationInProgress = false;
        break;
    }
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
    case Action_t::MOVE_TO_ACTION:
        // This should handle the completion of a move-to action.
        break;
    case Action_t::PANEL_STEP_ACTION:
        instance.mClosureEndpoint1.OnPanelMotionActionComplete();
        if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint2.GetEndpoint())
        {
            instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint3.GetEndpoint())
        {
            instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        }
        instance.isStepActionInProgress = false;
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }
    // Reset the current action and current action endpoint ID after handling the closure action
    instance.SetCurrentAction(Action_t::INVALID_ACTION);
    instance.mCurrentActionEndpointId = chip::kInvalidEndpointId;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
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

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                        const uint16_t & numberOfSteps,
                                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                        const chip::EndpointId & endpointId)
{
    MainStateEnum mClosureEndpoint1MainState;
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().GetMainState(mClosureEndpoint1MainState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get main state for Step command on Endpoint 1"));

    // If this command is received while the MainState attribute is currently either in Disengaged, Protected, Calibrating,
    //  SetupRequired or Error, then a status code of INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(
        mClosureEndpoint1MainState != MainStateEnum::kDisengaged && mClosureEndpoint1MainState != MainStateEnum::kProtected &&
            mClosureEndpoint1MainState != MainStateEnum::kSetupRequired && mClosureEndpoint1MainState != MainStateEnum::kError &&
            mClosureEndpoint1MainState != MainStateEnum::kCalibrating,
        Status::InvalidInState,
        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(mClosureEndpoint1MainState)));

    if (isStepActionInProgress && mCurrentActionEndpointId != endpointId)
    {
        ChipLogError(AppServer, "Step action is already in progress on Endpoint %d", mCurrentActionEndpointId);
        return Status::Failure;
    }

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Update Overall Target to Null for the Closure Control on Endpoint 1
    DataModel::Nullable<GenericOverallTargetState> mClosureEndpoint1Target;

    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().GetOverallTargetState(mClosureEndpoint1Target) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get overall target for Step command"));

    if (mClosureEndpoint1Target.IsNull())
    {
        mClosureEndpoint1Target.SetNonNull(GenericOverallTargetState{});
    }

    mClosureEndpoint1Target.Value().position = NullOptional; // Reset position to Null

    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().SetOverallTargetState(mClosureEndpoint1Target) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set overall target for Step command"));

    SetCurrentAction(PANEL_STEP_ACTION);
    mCurrentActionEndpointId = endpointId;

    isStepActionInProgress = true;

    AppEvent event;
    event.Type                    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action     = PANEL_STEP_ACTION;
    event.ClosureEvent.EndpointId = endpointId;
    event.Handler                 = InitiateAction;
    AppTask::GetAppTask().PostEvent(&event);

    return Status::Success;
}

void ClosureManager::HandlePanelStepAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    ClosureDimension::ClosureDimensionEndpoint * panelEp = instance.GetPanelEndpointById(endpointId);
    VerifyOrReturn(panelEp != nullptr, ChipLogError(AppServer, "Invalid instance for endpointId: %u", endpointId));

    StepDirectionEnum stepDirection = panelEp->GetDelegate().GetStepCommandTargetDirection();

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState;

    VerifyOrReturn(panelEp->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Step action"));
    VerifyOrReturn(panelEp->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
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
        VerifyOrReturn(panelEp->GetLogic().GetStepValue(stepValue) == CHIP_NO_ERROR,
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
        panelEp->GetLogic().SetCurrentState(panelCurrentState);

        instance.CancelTimer(); // Cancel any existing timer before starting a new action
        instance.SetCurrentAction(PANEL_STEP_ACTION);
        instance.mCurrentActionEndpointId = endpointId;
        instance.StartTimer(kMotionCountdownTimeMs);

        return;
    }

    instance.HandleClosureActionComplete(PANEL_STEP_ACTION);
}

ClosureDimension::ClosureDimensionEndpoint * ClosureManager::GetPanelEndpointById(EndpointId endpointId)
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
    else
    {
        ChipLogError(AppServer, "GetPanelEndpointById called with invalid endpointId: %u", endpointId);
        return nullptr;
    }
}
