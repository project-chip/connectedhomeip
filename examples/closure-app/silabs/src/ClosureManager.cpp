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
    case Action_t::PANEL_UNLATCH_ACTION:
        ChipLogDetail(AppServer, "Initiating panel unlatch action");
        // Timer used in sample application to simulate the panel unlatch process.
        // In a real application, this would be replaced with actual logic to unlatch
        // the closure panel
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
    case Action_t::PANEL_UNLATCH_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) {
            ClosureManager & instance = ClosureManager::GetInstance();
            instance.HandlePanelUnlatchAction(instance.mCurrentActionEndpointId);
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

    SetCurrentAction(Action_t::CALIBRATE_ACTION);
    mCurrentActionEndpointId = ep1.GetEndpoint();

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
    MainStateEnum ep1MainState;
    VerifyOrReturnError(ep1.GetLogic().GetMainState(ep1MainState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get main state for Step command on Endpoint 1"));

    // If this command is received while the MainState attribute is currently either in Disengaged, Protected, Calibrating,
    //  SetupRequired or Error, then a status code of INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

    if (isSetTargetInProgress && mCurrentActionEndpointId != endpointId)
    {
        ChipLogError(AppServer, "SetTarget action is already in progress on Endpoint %d", endpointId);
        return Status::Failure;
    }

    // Update OverallTarget of Closure based on SetTarget command.
    DataModel::Nullable<GenericOverallTargetState> overallTargetState;
    VerifyOrReturnError(ep1.GetLogic().GetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure,
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

    VerifyOrReturnError(ep1.GetLogic().SetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for SetTarget command for Endpoint %d", endpointId));

    VerifyOrReturnError(ep1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint %d", endpointId));

    VerifyOrReturnError(ep1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set main state for move to command on Endpoint 1"));

    // Post an event to initiate the unlatch action asynchronously.
    // Closure panel first performs the unlatch action if it is currently latched,
    // and then continues with the SetTarget action.
    // This is to ensure that the panel can move to the target position without being latched.
    SetCurrentAction(Action_t::PANEL_UNLATCH_ACTION);
    mCurrentActionEndpointId = endpointId;
    isSetTargetInProgress    = true;

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

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState = DataModel::NullNullable;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState  = DataModel::NullNullable;

    VerifyOrReturn(ep->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint %d", endpointId));
    VerifyOrReturn(!panelCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint %d", endpointId));

    VerifyOrReturn(ep->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target for Endpoint %d", endpointId));
    VerifyOrReturn(!panelTargetState.IsNull(), ChipLogError(AppServer, "Target is not set for Endpoint %d", endpointId));

    bool panelProgressPossible                            = false;
    DataModel::Nullable<chip::Percent100ths> nextPosition = DataModel::NullNullable;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(panelCurrentState.Value(), panelTargetState.Value(), nextPosition))
    {
        VerifyOrReturn(!nextPosition.IsNull(), ChipLogError(AppServer, "Next position is not set for Endpoint %d", endpointId));

        panelCurrentState.Value().position.SetValue(DataModel::MakeNullable(nextPosition.Value()));
        ep->GetLogic().SetCurrentState(panelCurrentState);

        panelProgressPossible = (nextPosition.Value() != panelTargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint %d Current Position: %d, Target Position: %d", endpointId, nextPosition.Value(),
                        panelTargetState.Value().position.Value().Value());
    }

    if (panelProgressPossible)
    {
        instance.CancelTimer(); // Cancel any existing timer before starting a new action
        instance.SetCurrentAction(Action_t::SET_TARGET_ACTION);
        instance.mCurrentActionEndpointId = endpointId;
        instance.StartTimer(kMotionCountdownTimeMs);
        return;
    }

    // If currently unlatched (false) and target is latched (true), latch after completing motion
    if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
        panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull())
    {
        if (!panelCurrentState.Value().latch.Value().Value() && panelTargetState.Value().latch.Value().Value())
        {
            DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState = DataModel::NullNullable;
            VerifyOrReturn(ep1.GetLogic().GetOverallCurrentState(ep1OverallCurrentState) == CHIP_NO_ERROR,
                           ChipLogError(AppServer, "Failed to get overall current state for Endpoint 1"));
            VerifyOrReturn(!ep1OverallCurrentState.IsNull(),
                           ChipLogError(AppServer, "Overall current state is not set for Endpoint 1"));

            // In Real application, this would be replaced with actual latch logic.
            ChipLogProgress(AppServer, "Performing latch action");

            ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            ep1.GetLogic().SetOverallCurrentState(ep1OverallCurrentState);

            panelCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            ep->GetLogic().SetCurrentState(panelCurrentState);

            ChipLogProgress(AppServer, "Latch action completed");
        }
    }

    instance.HandleClosureActionComplete(Action_t::SET_TARGET_ACTION);
}

void ClosureManager::HandlePanelUnlatchAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * panelEp = nullptr;
    if (endpointId == instance.ep2.GetEndpoint())
    {
        panelEp = &instance.ep2;
    }
    else if (endpointId == instance.ep3.GetEndpoint())
    {
        panelEp = &instance.ep3;
    }
    else
    {
        ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId);
        return;
    }

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
        DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState = DataModel::NullNullable;

        VerifyOrReturn(ep1.GetLogic().GetOverallCurrentState(ep1OverallCurrentState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
        VerifyOrReturn(!ep1OverallCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint 1"));

        // In Real application, this would be replaced with actual unlatch logic.
        ChipLogProgress(AppServer, "Performing unlatch action");

        ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
        ep1.GetLogic().SetOverallCurrentState(ep1OverallCurrentState);

        panelCurrentState.Value().latch.SetValue(false);
        panelEp->GetLogic().SetCurrentState(panelCurrentState);

        ChipLogProgress(AppServer, "Unlatched action completed");
    }

    // Unlatch action completed, now proceed with the SetTarget action
    instance.CancelTimer(); // Cancel any existing timer before starting a Set Target action

    // Call HandlePanelSetTargetAction to continue with the SetTarget action
    instance.HandlePanelSetTargetAction(endpointId);
}
