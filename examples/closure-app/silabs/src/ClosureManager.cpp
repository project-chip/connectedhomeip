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
constexpr uint32_t kMotionCountdownTimeMs       = 1000;  // 1 second

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
    case Action_t::SET_TARGET_ACTION:
        ChipLogDetail(AppServer, "Initiating set target action");
        // Timer used in sample application to simulate the set target process.
        // In a real application, this would be replaced with actual logic to set
        // the target position of the closure.
        instance.StartTimer(kMotionCountdownTimeMs);
        break;
    case Action_t::PANEL_UNLATCH_ACTION:
        ChipLogDetail(AppServer, "Initiating panel unlatch action");
        // Timer used in sample application to simulate the panel unlatch process.
        // In a real application, this would be replaced with actual logic to unlatch
        // the closure panel
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
    case Action_t::SET_TARGET_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandlePanelSetTargetAction(instance.mCurrentActionEndpointId);
        });
        break;
    case Action_t::PANEL_UNLATCH_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandlePanelUnlatchAction(instance.mCurrentActionEndpointId);
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

        DeviceLayer::PlatformMgr().LockChipStack();
        isCalibrationInProgress = false;
        DeviceLayer::PlatformMgr().UnlockChipStack();

        break;
    }
    case Action_t::STOP_ACTION: {
        if (isCalibrationInProgress)
        {
            ChipLogDetail(AppServer, "Stopping calibration action");
            instance.mClosureEndpoint1.OnStopCalibrateActionComplete();
            instance.mClosurePanelEndpoint2.OnStopCalibrateActionComplete();
            instance.mClosurePanelEndpoint3.OnStopCalibrateActionComplete();

            DeviceLayer::PlatformMgr().LockChipStack();
            isCalibrationInProgress = false;
            DeviceLayer::PlatformMgr().UnlockChipStack();
        }
        else if (isMoveToInProgress)
        {
            ChipLogDetail(AppServer, "Stopping move to action");
            instance.mClosureEndpoint1.OnStopMotionActionComplete();
            instance.mClosurePanelEndpoint2.OnStopMotionActionComplete();
            instance.mClosurePanelEndpoint3.OnStopMotionActionComplete();

            DeviceLayer::PlatformMgr().LockChipStack();
            isMoveToInProgress = false;
            DeviceLayer::PlatformMgr().UnlockChipStack();
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
    case Action_t::SET_TARGET_ACTION:
        instance.mClosureEndpoint1.OnPanelMotionActionComplete();
        if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
        {
            instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
        {
            instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        }

        DeviceLayer::PlatformMgr().LockChipStack();
        instance.isSetTargetInProgress = false;
        DeviceLayer::PlatformMgr().UnlockChipStack();
        break;
    case Action_t::PANEL_STEP_ACTION:
        instance.mClosureEndpoint1.OnPanelMotionActionComplete();
        if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
        {
            instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        }
        else if (instance.mCurrentActionEndpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
        {
            instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        }

        DeviceLayer::PlatformMgr().LockChipStack();
        instance.isStepActionInProgress = false;
        DeviceLayer::PlatformMgr().UnlockChipStack();
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }

    DeviceLayer::PlatformMgr().LockChipStack();
    // Reset the current action and current action endpoint ID after handling the closure action
    instance.SetCurrentAction(Action_t::INVALID_ACTION);
    instance.mCurrentActionEndpointId = chip::kInvalidEndpointId;
    DeviceLayer::PlatformMgr().UnlockChipStack();
}

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kDefaultCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for calibration"));

    DeviceLayer::PlatformMgr().LockChipStack();
    SetCurrentAction(Action_t::CALIBRATE_ACTION);
    mCurrentActionEndpointId = mClosureEndpoint1.GetEndpointId();
    isCalibrationInProgress  = true;
    DeviceLayer::PlatformMgr().UnlockChipStack();

    // Post an event to initiate the calibration action asynchronously.
    // Calibration can be only initiated from Closure Endpoint 1, so we set the endpoint ID to mClosureEndpoint1.
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

    // Stop can be only initiated from Closure Endpoint 1, so we set the endpoint ID to mClosureEndpoint1.
    DeviceLayer::PlatformMgr().LockChipStack();
    SetCurrentAction(Action_t::STOP_ACTION);
    mCurrentActionEndpointId = mClosureEndpoint1.GetEndpointId();
    DeviceLayer::PlatformMgr().UnlockChipStack();

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

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & position,
                                                                             const Optional<bool> & latch,
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             const chip::EndpointId endpointId)
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

    if (isSetTargetInProgress && mCurrentActionEndpointId != endpointId)
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

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set main state while handling the SetTarget command on Endpoint 1"));

    VerifyOrReturnError(
        mClosureEndpoint1.GetLogic().SetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure,
        ChipLogError(AppServer, "Failed to set overall target while handling the SetTarget command for Endpoint %d", endpointId));

    VerifyOrReturnError(
        mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kDefaultCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
        ChipLogError(AppServer, "Failed to set countdown time while handling the SetTarget command for Endpoint %d", endpointId));

    // Post an event to initiate the unlatch action asynchronously.
    // Closure panel first performs the unlatch action if it is currently latched,
    // and then continues with the SetTarget action.
    // This is to ensure that the panel can set the target position without being latched.
    DeviceLayer::PlatformMgr().LockChipStack();
    SetCurrentAction(Action_t::PANEL_UNLATCH_ACTION);
    mCurrentActionEndpointId = endpointId;
    isSetTargetInProgress    = true;
    DeviceLayer::PlatformMgr().UnlockChipStack();

    AppEvent event;
    event.Type                    = AppEvent::kEventType_Closure;
    event.ClosureEvent.Action     = mCurrentAction;
    event.ClosureEvent.EndpointId = endpointId;
    event.Handler                 = InitiateAction;

    AppTask::GetAppTask().PostEvent(&event);

    return Status::Success;
}

void ClosureManager::HandlePanelSetTargetAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint based on the endpointId
    ClosureDimension::ClosureDimensionEndpoint * panelEp = instance.GetPanelEndpointById(endpointId);
    VerifyOrReturn(panelEp != nullptr, ChipLogError(AppServer, "Invalid instance for endpointId: %u", endpointId));

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState  = DataModel::NullNullable;

    VerifyOrReturn(panelEp->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint %d", endpointId));
    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint %d", endpointId));

    VerifyOrReturn(panelEp->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target for Endpoint %d", endpointId));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target is not set for Endpoint %d", endpointId));

    bool panelProgressPossible                            = false;
    DataModel::Nullable<chip::Percent100ths> nextPosition = DataModel::NullNullable;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(panelCurrentState.Value(), panelTargetState.Value(), nextPosition))
    {
        VerifyOrReturn(!nextPosition.IsNull(), ChipLogError(AppServer, "Next position is not set for Endpoint %d", endpointId));

        panelCurrentState.Value().position.SetValue(DataModel::MakeNullable(nextPosition.Value()));
        panelEp->GetLogic().SetCurrentState(panelCurrentState);

        panelProgressPossible = (nextPosition.Value() != panelTargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", endpointId, nextPosition.Value(),
                        panelTargetState.Value().position.Value().Value());
    }

    if (panelProgressPossible)
    {
        instance.CancelTimer(); // Cancel any existing timer before starting a new action

        DeviceLayer::PlatformMgr().LockChipStack();
        instance.SetCurrentAction(Action_t::SET_TARGET_ACTION);
        instance.mCurrentActionEndpointId = endpointId;
        DeviceLayer::PlatformMgr().UnlockChipStack();

        instance.StartTimer(kMotionCountdownTimeMs);
        return;
    }

    // If currently unlatched (false) and target is latched (true), latch after completing motion
    if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
        panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull())
    {
        if (!panelCurrentState.Value().latch.Value().Value() && panelTargetState.Value().latch.Value().Value())
        {
            DataModel::Nullable<GenericOverallCurrentState> mClosureEndpoint1OverallCurrentState = DataModel::NullNullable;
            VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(mClosureEndpoint1OverallCurrentState) ==
                               CHIP_NO_ERROR,
                           ChipLogError(AppServer, "Failed to get overall current state for Endpoint 1"));
            VerifyOrReturn(!mClosureEndpoint1OverallCurrentState.IsNull(),
                           ChipLogError(AppServer, "Overall current state is not set for Endpoint 1"));

            // In Real application, this would be replaced with actual latch logic.
            ChipLogProgress(AppServer, "Performing latch action");

            mClosureEndpoint1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            mClosureEndpoint1.GetLogic().SetOverallCurrentState(mClosureEndpoint1OverallCurrentState);

            panelCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            panelEp->GetLogic().SetCurrentState(panelCurrentState);

            ChipLogProgress(AppServer, "Latch action completed");
        }
    }

    instance.HandleClosureActionComplete(Action_t::SET_TARGET_ACTION);
}

void ClosureManager::HandlePanelUnlatchAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint based on the endpointId
    ClosureDimension::ClosureDimensionEndpoint * panelEp = instance.GetPanelEndpointById(endpointId);
    VerifyOrReturn(panelEp != nullptr, ChipLogError(AppServer, "Invalid instance for endpointId: %u", endpointId));

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
        DataModel::Nullable<GenericOverallCurrentState> mClosureEndpoint1OverallCurrentState = DataModel::NullNullable;

        VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(mClosureEndpoint1OverallCurrentState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
        VerifyOrReturn(!mClosureEndpoint1OverallCurrentState.IsNull(),
                       ChipLogError(AppServer, "Current state is not set for Endpoint 1"));

        // In Real application, this would be replaced with actual unlatch logic.
        ChipLogProgress(AppServer, "Performing unlatch action");

        mClosureEndpoint1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
        mClosureEndpoint1.GetLogic().SetOverallCurrentState(mClosureEndpoint1OverallCurrentState);

        panelCurrentState.Value().latch.SetValue(false);
        panelEp->GetLogic().SetCurrentState(panelCurrentState);

        ChipLogProgress(AppServer, "Unlatched action completed");
    }

    // Unlatch action completed, now proceed with the SetTarget action
    instance.CancelTimer(); // Cancel any existing timer before starting a Set Target action

    // Call HandlePanelSetTargetAction to continue with the SetTarget action
    instance.HandlePanelSetTargetAction(endpointId);
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

    DeviceLayer::PlatformMgr().LockChipStack();
    SetCurrentAction(PANEL_STEP_ACTION);
    mCurrentActionEndpointId = endpointId;
    isStepActionInProgress   = true;
    DeviceLayer::PlatformMgr().UnlockChipStack();

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

        DeviceLayer::PlatformMgr().LockChipStack();
        instance.SetCurrentAction(PANEL_STEP_ACTION);
        instance.mCurrentActionEndpointId = endpointId;
        DeviceLayer::PlatformMgr().UnlockChipStack();

        instance.StartTimer(kMotionCountdownTimeMs);

        return;
    }

    instance.HandleClosureActionComplete(PANEL_STEP_ACTION);
}

ClosureDimension::ClosureDimensionEndpoint * ClosureManager::GetPanelEndpointById(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
    {
        return &instance.mClosurePanelEndpoint2;
    }
    else if (endpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
    {
        return &instance.mClosurePanelEndpoint3;
    }
    else
    {
        ChipLogError(AppServer, "GetPanelEndpointById called with invalid endpointId: %u", endpointId);
        return nullptr;
    }
}
