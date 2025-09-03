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
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <lib/support/TimeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {
constexpr uint32_t kDefaultCountdownTimeSeconds   = 10;   // 10 seconds
constexpr uint32_t kCalibrateCountdownTimeMs      = 3000; // 3 seconds
constexpr uint32_t kMotionCountdownTimeMs         = 1000; // 1 second for each motion.
constexpr chip::Percent100ths kMotionPositionStep = 2000; // 20% of the total range per motion interval.

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

    // Set Initial state for Closure endpoints
    VerifyOrDie(SetClosureControlInitialState(mClosureEndpoint1) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Initial state for Closure Control Endpoint set successfully");
    VerifyOrDie(SetClosurePanelInitialState(mClosurePanelEndpoint2) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Initial state for Closure Panel Endpoint 2 set successfully");
    VerifyOrDie(SetClosurePanelInitialState(mClosurePanelEndpoint3) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Initial state for Closure Panel Endpoint 3 set successfully");

    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (pTestEventDelegate != nullptr)
    {
        CHIP_ERROR err = pTestEventDelegate->AddHandler(&mClosureEndpoint1.GetDelegate());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to add handler for delegate: %s", chip::ErrorStr(err));
        }
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot add handler for delegate");
    }

    DeviceLayer::PlatformMgr().UnlockChipStack();
}

CHIP_ERROR ClosureManager::SetClosureControlInitialState(ClosureControlEndpoint & closureControlEndpoint)
{
    ChipLogProgress(AppServer, "ClosureControlEndpoint SetInitialState");
    ReturnErrorOnFailure(closureControlEndpoint.GetLogic().SetCountdownTimeFromDelegate(NullNullable));
    ReturnErrorOnFailure(closureControlEndpoint.GetLogic().SetMainState(MainStateEnum::kStopped));

    DataModel::Nullable<GenericOverallCurrentState> overallState(GenericOverallCurrentState(
        MakeOptional(DataModel::MakeNullable(CurrentPositionEnum::kFullyClosed)), MakeOptional(DataModel::MakeNullable(true)),
        MakeOptional(Globals::ThreeLevelAutoEnum::kAuto), DataModel::MakeNullable(true)));
    ReturnErrorOnFailure(closureControlEndpoint.GetLogic().SetOverallCurrentState(overallState));
    DataModel::Nullable<GenericOverallTargetState> overallTarget(
        GenericOverallTargetState(MakeOptional(DataModel::NullNullable), MakeOptional(DataModel::NullNullable),
                                  MakeOptional(Globals::ThreeLevelAutoEnum::kAuto)));
    ReturnErrorOnFailure(closureControlEndpoint.GetLogic().SetOverallTargetState(overallTarget));
    BitFlags<ClosureControl::LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureControl::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureControl::LatchControlModesBitmap::kRemoteUnlatching);
    ReturnErrorOnFailure(closureControlEndpoint.GetLogic().SetLatchControlModes(latchControlModes));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClosureManager::SetClosurePanelInitialState(ClosureDimensionEndpoint & closurePanelEndpoint)
{
    ChipLogProgress(AppServer, "ClosurePanelEndpoint SetInitialState");
    DataModel::Nullable<GenericDimensionStateStruct> currentState(
        GenericDimensionStateStruct(MakeOptional(DataModel::MakeNullable<Percent100ths>(10000)),
                                    MakeOptional(DataModel::MakeNullable(true)), MakeOptional(Globals::ThreeLevelAutoEnum::kAuto)));
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetCurrentState(currentState));

    DataModel::Nullable<GenericDimensionStateStruct> targetState(
        GenericDimensionStateStruct(MakeOptional(DataModel::NullNullable), MakeOptional(DataModel::NullNullable),
                                    MakeOptional(Globals::ThreeLevelAutoEnum::kAuto)));
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetTargetState(targetState));

    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetResolution(Percent100ths(100)));
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetStepValue(2000));
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetUnit(ClosureUnitEnum::kMillimeter));
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetUnitRange(
        ClosureDimension::Structs::UnitRangeStruct::Type{ .min = static_cast<int16_t>(0), .max = static_cast<int16_t>(10000) }));
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetOverflow(OverflowEnum::kTopInside));

    ClosureDimension::Structs::RangePercent100thsStruct::Type limitRange{ .min = static_cast<Percent100ths>(0),
                                                                          .max = static_cast<Percent100ths>(10000) };
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetLimitRange(limitRange));
    BitFlags<ClosureDimension::LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureDimension::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureDimension::LatchControlModesBitmap::kRemoteUnlatching);
    ReturnErrorOnFailure(closurePanelEndpoint.GetLogic().SetLatchControlModes(latchControlModes));

    return CHIP_NO_ERROR;
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
        instance.StartTimer(kCalibrateCountdownTimeMs);
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
        PlatformMgr().ScheduleWork([](intptr_t) { ClosureManager::GetInstance().HandleClosureMotionAction(); });
        break;
    case Action_t::UNLATCH_ACTION:
        PlatformMgr().ScheduleWork([](intptr_t) { ClosureManager::GetInstance().HandleClosureUnlatchAction(); });
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
        instance.mClosureEndpoint1.OnMoveToActionComplete();
        instance.mClosurePanelEndpoint2.OnMoveToActionComplete();
        instance.mClosurePanelEndpoint3.OnMoveToActionComplete();

        DeviceLayer::PlatformMgr().LockChipStack();
        instance.isMoveToInProgress = false;
        DeviceLayer::PlatformMgr().UnlockChipStack();

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

    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint2CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().GetCurrentState(mClosurePanelEndpoint2CurrentState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint3CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().GetCurrentState(mClosurePanelEndpoint3CurrentState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint2TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().GetTargetState(mClosurePanelEndpoint2TargetState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint3TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().GetTargetState(mClosurePanelEndpoint3TargetState) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturnError(!mClosurePanelEndpoint2CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturnError(!mClosurePanelEndpoint3CurrentState.IsNull(), Status::Failure,
                        ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    // Create target struct for the panels if the target state is not set.
    GenericDimensionStateStruct mClosurePanelEndpoint2Target =
        mClosurePanelEndpoint2TargetState.IsNull() ? GenericDimensionStateStruct() : mClosurePanelEndpoint2TargetState.Value();
    GenericDimensionStateStruct mClosurePanelEndpoint3Target =
        mClosurePanelEndpoint3TargetState.IsNull() ? GenericDimensionStateStruct() : mClosurePanelEndpoint3TargetState.Value();

    if (position.HasValue())
    {
        // Set the Closure panel target position for the panels based on the MoveTo Command position.
        // For Sample App,TargetPositionEnum is mapped to specific positions for the panels.
        Percent100ths mClosurePanelEndpoint2Position;
        Percent100ths mClosurePanelEndpoint3Position;

        switch (position.Value())
        {
        case TargetPositionEnum::kMoveToFullyClosed:
            mClosurePanelEndpoint2Position = static_cast<Percent100ths>(10000);
            mClosurePanelEndpoint3Position = static_cast<Percent100ths>(10000);
            break;
        case TargetPositionEnum::kMoveToFullyOpen:
            mClosurePanelEndpoint2Position = static_cast<Percent100ths>(0);
            mClosurePanelEndpoint3Position = static_cast<Percent100ths>(0);
            break;
        case TargetPositionEnum::kMoveToPedestrianPosition:
            mClosurePanelEndpoint2Position = static_cast<Percent100ths>(6000);
            mClosurePanelEndpoint3Position = static_cast<Percent100ths>(6000);
            break;
        case TargetPositionEnum::kMoveToSignaturePosition:
            mClosurePanelEndpoint2Position = static_cast<Percent100ths>(4000);
            mClosurePanelEndpoint3Position = static_cast<Percent100ths>(4000);
            break;
        case TargetPositionEnum::kMoveToVentilationPosition:
            mClosurePanelEndpoint2Position = static_cast<Percent100ths>(2000);
            mClosurePanelEndpoint3Position = static_cast<Percent100ths>(2000);
            break;
        default:
            ChipLogError(AppServer, "Invalid target position received in OnMoveToCommand");
            return Status::Failure;
        }

        mClosurePanelEndpoint2Target.position.SetValue(DataModel::MakeNullable(mClosurePanelEndpoint2Position));
        mClosurePanelEndpoint3Target.position.SetValue(DataModel::MakeNullable(mClosurePanelEndpoint3Position));
    }

    if (latch.HasValue())
    {
        mClosurePanelEndpoint2Target.latch.SetValue(DataModel::MakeNullable(latch.Value()));
        mClosurePanelEndpoint3Target.latch.SetValue(DataModel::MakeNullable(latch.Value()));
    }

    if (speed.HasValue())
    {
        mClosurePanelEndpoint2Target.speed.SetValue(speed.Value());
        mClosurePanelEndpoint3Target.speed.SetValue(speed.Value());
    }

    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().SetTargetState(DataModel::MakeNullable(mClosurePanelEndpoint2Target)) ==
                            CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().SetTargetState(DataModel::MakeNullable(mClosurePanelEndpoint3Target)) ==
                            CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 3"));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kDefaultCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Set the current action to UNLATCH_ACTION.
    // This is to ensure that the closure is unlatched before starting the motion action.
    // The Closure Control Cluster will handle the unlatch action before proceeding with the motion action.
    DeviceLayer::PlatformMgr().LockChipStack();
    SetCurrentAction(UNLATCH_ACTION);
    isMoveToInProgress = true;
    DeviceLayer::PlatformMgr().UnlockChipStack();

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

    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint3CurrentState;

    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint2TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint3TargetState;

    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetCurrentState(mClosurePanelEndpoint2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetCurrentState(mClosurePanelEndpoint3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetTargetState(mClosurePanelEndpoint2TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetTargetState(mClosurePanelEndpoint3TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturn(!mClosurePanelEndpoint2CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!mClosurePanelEndpoint3CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    VerifyOrReturn(!mClosurePanelEndpoint2TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 2"));
    VerifyOrReturn(!mClosurePanelEndpoint3TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 3"));

    // Once Closure is unlatched, we can proceed with the motion action for endpoints 2 and 3.
    DataModel::Nullable<Percent100ths> mClosurePanelEndpoint2NextPosition = DataModel::NullNullable;
    DataModel::Nullable<Percent100ths> mClosurePanelEndpoint3NextPosition = DataModel::NullNullable;

    bool isEndPoint2ProgressPossible = false;
    bool isEndPoint3ProgressPossible = false;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(mClosurePanelEndpoint2CurrentState.Value(), mClosurePanelEndpoint2TargetState.Value(),
                             mClosurePanelEndpoint2NextPosition))
    {
        VerifyOrReturn(!mClosurePanelEndpoint2NextPosition.IsNull(),
                       ChipLogError(AppServer, "Failed to get next position for Endpoint 2"));
        mClosurePanelEndpoint2CurrentState.Value().position.SetValue(
            DataModel::MakeNullable(mClosurePanelEndpoint2NextPosition.Value()));
        instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(mClosurePanelEndpoint2CurrentState);
        isEndPoint2ProgressPossible =
            (mClosurePanelEndpoint2NextPosition.Value() != mClosurePanelEndpoint2TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d",
                        mClosurePanelEndpoint2NextPosition.Value(),
                        mClosurePanelEndpoint2TargetState.Value().position.Value().Value());
    }

    // Get the Next Current State to be set for the endpoint 3, if target postion is not reached.
    if (GetPanelNextPosition(mClosurePanelEndpoint3CurrentState.Value(), mClosurePanelEndpoint3TargetState.Value(),
                             mClosurePanelEndpoint3NextPosition))
    {
        VerifyOrReturn(!mClosurePanelEndpoint3NextPosition.IsNull(),
                       ChipLogError(AppServer, "Failed to get next position for Endpoint 3"));
        mClosurePanelEndpoint3CurrentState.Value().position.SetValue(
            DataModel::MakeNullable(mClosurePanelEndpoint3NextPosition.Value()));
        instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(mClosurePanelEndpoint3CurrentState);
        isEndPoint3ProgressPossible =
            (mClosurePanelEndpoint3NextPosition.Value() != mClosurePanelEndpoint3TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 3 Current Position: %d, Target Position: %d",
                        mClosurePanelEndpoint3NextPosition.Value(),
                        mClosurePanelEndpoint3TargetState.Value().position.Value().Value());
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

    DataModel::Nullable<GenericOverallCurrentState> mClosureEndpoint1CurrentState;
    DataModel::Nullable<GenericOverallTargetState> mClosureEndpoint1TargetState;

    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(mClosureEndpoint1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallTargetState(mClosureEndpoint1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(!mClosureEndpoint1CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!mClosureEndpoint1TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 1"));

    // If both endpoints have reached their target positions, we can consider the closure motion action as complete.
    // Before calling HandleClosureActionComplete, we need to check if a latch action is needed.
    if (mClosureEndpoint1CurrentState.Value().latch.HasValue() && !mClosureEndpoint1CurrentState.Value().latch.Value().IsNull() &&
        mClosureEndpoint1TargetState.Value().latch.HasValue() && !mClosureEndpoint1TargetState.Value().latch.Value().IsNull())
    {
        // If currently unlatched (false) and target is latched (true), latch after moving to target position.
        if (!mClosureEndpoint1CurrentState.Value().latch.Value().Value() &&
            mClosureEndpoint1TargetState.Value().latch.Value().Value())
        {
            // In Real application, this would be replaced with actual unlatch logic.
            ChipLogProgress(AppServer, "Performing latch action");
            mClosureEndpoint1CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosureEndpoint1.GetLogic().SetOverallCurrentState(mClosureEndpoint1CurrentState);
            mClosurePanelEndpoint2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(mClosurePanelEndpoint2CurrentState);
            mClosurePanelEndpoint3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(mClosurePanelEndpoint3CurrentState);
            ChipLogProgress(AppServer, "latched action complete");
        }
    }

    // Target reached and no latch action needed, call HandleClosureAction
    instance.HandleClosureActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
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

void ClosureManager::HandleClosureUnlatchAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericOverallCurrentState> mClosureEndpoint1CurrentState;
    DataModel::Nullable<GenericOverallTargetState> mClosureEndpoint1TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> mClosurePanelEndpoint3CurrentState;

    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(mClosureEndpoint1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallTargetState(mClosureEndpoint1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetCurrentState(mClosurePanelEndpoint2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetCurrentState(mClosurePanelEndpoint3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    VerifyOrReturn(!mClosureEndpoint1CurrentState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!mClosureEndpoint1TargetState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Target state on Endpoint 1"));

    VerifyOrReturn(!mClosurePanelEndpoint2CurrentState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!mClosurePanelEndpoint3CurrentState.IsNull(),
                   ChipLogError(AppServer, "UnlatchAction failed due to Null value Current state on Endpoint 3"));

    // check if closure (endpoint 1) need unlatch before starting the motion action.
    if (mClosureEndpoint1CurrentState.Value().latch.HasValue() && !mClosureEndpoint1CurrentState.Value().latch.Value().IsNull() &&
        mClosureEndpoint1TargetState.Value().latch.HasValue() && !mClosureEndpoint1TargetState.Value().latch.Value().IsNull())
    {
        bool mClosureEndpoint1CurrentLatchValue      = mClosureEndpoint1CurrentState.Value().latch.Value().Value();
        bool mClosurePanelEndpoint2CurrentLatchValue = mClosurePanelEndpoint2CurrentState.Value().latch.HasValue() &&
            !mClosurePanelEndpoint2CurrentState.Value().latch.Value().IsNull() &&
            mClosurePanelEndpoint2CurrentState.Value().latch.Value().Value();
        bool mClosurePanelEndpoint3CurrentLatchValue = mClosurePanelEndpoint3CurrentState.Value().latch.HasValue() &&
            !mClosurePanelEndpoint3CurrentState.Value().latch.Value().IsNull() &&
            mClosurePanelEndpoint3CurrentState.Value().latch.Value().Value();

        // If currently Closure or any panel is latched (true) and target is unlatched (false), unlatch first before moving
        if ((mClosureEndpoint1CurrentLatchValue || mClosurePanelEndpoint2CurrentLatchValue ||
             mClosurePanelEndpoint3CurrentLatchValue) &&
            !mClosureEndpoint1TargetState.Value().latch.Value().Value())
        {
            // In Real application, this would be replaced with actual unlatch logic.
            ChipLogProgress(AppServer, "Performing unlatch action");
            mClosureEndpoint1CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosureEndpoint1.GetLogic().SetOverallCurrentState(mClosureEndpoint1CurrentState);
            mClosurePanelEndpoint2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(mClosurePanelEndpoint2CurrentState);
            mClosurePanelEndpoint3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(mClosurePanelEndpoint3CurrentState);
            ChipLogProgress(AppServer, "Unlatched action completed");
        }
    }

    CancelTimer(); // Cancel any existing timer before proceeding with the motion action

    // After unlatching, we can proceed with the motion action
    instance.HandleClosureMotionAction();
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
            // Underflow protection: if currentPosition <= stepValue, set to 0.
            chip::Percent100ths decreasedCurrentPosition = (currentPosition > stepValue)
                ? static_cast<chip::Percent100ths>(currentPosition - stepValue)
                : static_cast<chip::Percent100ths>(0);
            nextCurrentPosition                          = std::max(decreasedCurrentPosition, targetPosition);
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
        // Increment position by 2000 units, capped at target.
        // No overflow handling needed due to currentposition max value is 10000
        nextPosition.SetNonNull(std::min(static_cast<chip::Percent100ths>(currentPosition + kMotionPositionStep), targetPosition));
    }
    else if (currentPosition > targetPosition)
    {
        // Handling overflow for CurrentPosition
        chip::Percent100ths newCurrentPosition =
            (currentPosition > kMotionPositionStep) ? currentPosition - kMotionPositionStep : 0;
        // Moving down: Decreasing the current position by a step of 2000 units,
        // ensuring it does not go below the target position.
        nextPosition.SetNonNull(std::max(newCurrentPosition, targetPosition));
    }
    else
    {
        // Already at target: No further action is needed as the current position matches the target position.
        nextPosition.SetNonNull(currentPosition);
        return false; // No update needed
    }
    return true;
}
