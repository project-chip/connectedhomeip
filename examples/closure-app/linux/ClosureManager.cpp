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

#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters::ClosureDimension;

namespace {
// Define a constant for the countdown time
constexpr uint32_t kCountdownTimeSeconds          = 10;
constexpr uint32_t kCalibrateCountdownTimeMs      = 3000; // 3 seconds for calibrate motion
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
const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosureEndpoint1TagList[] = {
    { .namespaceID = kNamespaceClosure,
      .tag         = kTagClosureCovering,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Closure.Covering"_span)) },
    { .namespaceID = kNamespaceCovering,
      .tag         = kTagCoveringVenetian,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("Covering.Venetian"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosurePanelEndpoint2TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelLift,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Lift"_span)) },
};

const Clusters::Descriptor::Structs::SemanticTagStruct::Type kClosurePanelEndpoint3TagList[] = {
    { .namespaceID = kNamespaceClosurePanel,
      .tag         = kTagClosurePanelTilt,
      .label       = chip::MakeOptional(DataModel::Nullable<chip::CharSpan>("ClosurePanel.Tilt"_span)) },
};

} // namespace

// Definition of the static instance of ClosureManager
ClosureManager ClosureManager::sInstance;

void ClosureManager::Init()
{
    VerifyOrDie(mClosureEndpoint1.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Control Endpoint initialized successfully");

    VerifyOrDie(mClosurePanelEndpoint2.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Panel Endpoint 2 initialized successfully");

    VerifyOrDie(mClosurePanelEndpoint3.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Panel Endpoint 3 initialized successfully");

    // Set Taglist for Closure endpoints
    SetTagList(/* endpoint= */ kClosureEndpoint1,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosureEndpoint1TagList));
    SetTagList(/* endpoint= */ kClosurePanelEndpoint2,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosurePanelEndpoint2TagList));
    SetTagList(/* endpoint= */ kClosurePanelEndpoint3,
               Span<const Clusters::Descriptor::Structs::SemanticTagStruct::Type>(kClosurePanelEndpoint3TagList));

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
}

void ClosureManager::Shutdown()
{
    TestEventTriggerDelegate * pTestEventDelegate = Server::GetInstance().GetTestEventTriggerDelegate();

    if (pTestEventDelegate != nullptr)
    {
        pTestEventDelegate->RemoveHandler(&mClosureEndpoint1.GetDelegate());
    }
    else
    {
        ChipLogError(AppServer, "TestEventTriggerDelegate is null, cannot remove handler for delegate");
    }
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

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    // Cancel any existing timers for closure actions over all endpoints
    DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);

    mEp1CurrentAction              = ClosureAction::kCalibrateAction;
    mIsCalibrationActionInProgress = true;

    // For sample application, we are using a timer to simulate the hardware calibration action.
    // In a real application, this would be replaced with actual calibration logic and call HandleClosureActionComplete.
    VerifyOrReturnValue(DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kCalibrateCountdownTimeMs),
                                                              HandleEp1ClosureActionTimer, nullptr) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to start closure action timer"));

    ChipLogProgress(AppServer, "ClosureManager: Calibration action started for endpoint 1");
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    // Cancel any existing timers for closure action
    DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
    mEp1CurrentAction = ClosureAction::kStopAction;
    HandleStopActionComplete();
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                const Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed)
{
    // Cancel any existing timer for closure action
    (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp1ClosureActionTimer, this);

    // Update the target state for the closure panels based on the MoveTo command.
    // This closure sample app assumes that the closure panels are represented by two endpoints:
    // - Endpoint 2: Represents the Closure Dimension Cluster for the first panel.
    // - Endpoint 3: Represents the Closure Dimension Cluster for the second panel.

    // For sample app, MoveTo command to Fullopen , will set target position of both panels to 0
    //  MoveTo command to Fullclose will set target position of both panels to 10000
    //  We simulate harware action by using timer for 1 sec and updating the current state of the panels after the timer expires.
    //  till we reach the target position.

    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));

    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().GetTargetState(ep2TargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().GetTargetState(ep3TargetState) == CHIP_NO_ERROR, Status::Failure,
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
        chip::Percent100ths ep2Position;
        chip::Percent100ths ep3Position;

        switch (position.Value())
        {
        case TargetPositionEnum::kMoveToFullyClosed:
            ep2Position = static_cast<chip::Percent100ths>(10000);
            ep3Position = static_cast<chip::Percent100ths>(10000);
            break;
        case TargetPositionEnum::kMoveToFullyOpen:
            ep2Position = static_cast<chip::Percent100ths>(0);
            ep3Position = static_cast<chip::Percent100ths>(0);
            break;
        case TargetPositionEnum::kMoveToPedestrianPosition:
            ep2Position = static_cast<chip::Percent100ths>(6000);
            ep3Position = static_cast<chip::Percent100ths>(6000);
            break;
        case TargetPositionEnum::kMoveToSignaturePosition:
            ep2Position = static_cast<chip::Percent100ths>(4000);
            ep3Position = static_cast<chip::Percent100ths>(4000);
            break;
        case TargetPositionEnum::kMoveToVentilationPosition:
            ep2Position = static_cast<chip::Percent100ths>(2000);
            ep3Position = static_cast<chip::Percent100ths>(2000);
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

    VerifyOrReturnError(mClosurePanelEndpoint2.GetLogic().SetTargetState(DataModel::MakeNullable(ep2Target)) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 2"));
    VerifyOrReturnError(mClosurePanelEndpoint3.GetLogic().SetTargetState(DataModel::MakeNullable(ep3Target)) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set target for Endpoint 3"));
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    mEp1CurrentAction    = ClosureAction::kMoveToAction;
    mEp1MotionInProgress = true;
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs), HandleEp1ClosureActionTimer, this);
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & position,
                                                                             const Optional<bool> & latch,
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             const chip::EndpointId endpointId)
{
    MainStateEnum ep1MainState;
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().GetMainState(ep1MainState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get main state for Step command on Endpoint 1"));

    // If this command is received while the MainState attribute is currently either in Disengaged, Protected, Calibrating,
    //  SetupRequired or Error, then a status code of INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

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

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetOverallTargetState(overallTargetState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for SetTarget command for Endpoint %d", endpointId));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(kCountdownTimeSeconds) == CHIP_NO_ERROR,
                        Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for SetTarget command on Endpoint %d", endpointId));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set main state for SetTarget command on Endpoint 1"));

    // Post an event to initiate the unlatch action asynchronously.
    // Closure panel first performs the unlatch action if it is currently latched,
    // and then continues with the SetTarget action.
    // This is to ensure that the panel can move to the target position without being latched.
    if (endpointId == kClosurePanelEndpoint2)
    {
        mEp2CurrentAction    = ClosureManager::ClosureAction::kSetTargetAction;
        mEp2MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp2ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for SetTarget command on Endpoint 2");
    }
    else if (endpointId == kClosurePanelEndpoint3)
    {
        mEp3CurrentAction    = ClosureManager::ClosureAction::kSetTargetAction;
        mEp3MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp3ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for SetTarget command on Endpoint 3");
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return Status::Failure;
    }

    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                        const uint16_t & numberOfSteps,
                                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                        const chip::EndpointId endpointId)
{
    MainStateEnum ep1MainState;
    VerifyOrReturnError(mClosureEndpoint1.GetLogic().GetMainState(ep1MainState) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get main state for Step command on Endpoint 1"));

    // If this command is received while the MainState attribute is currently either in Disengaged, Protected, Calibrating,
    //  SetupRequired or Error, then a status code of INVALID_IN_STATE shall be returned.
    VerifyOrReturnError(ep1MainState != MainStateEnum::kDisengaged && ep1MainState != MainStateEnum::kProtected &&
                            ep1MainState != MainStateEnum::kSetupRequired && ep1MainState != MainStateEnum::kError &&
                            ep1MainState != MainStateEnum::kCalibrating,
                        Status::InvalidInState,
                        ChipLogError(AppServer, "Step command not allowed in current state: %d", static_cast<int>(ep1MainState)));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetMainState(MainStateEnum::kMoving) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    VerifyOrReturnError(mClosureEndpoint1.GetLogic().SetCountdownTimeFromDelegate(10) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set countdown time for move to command on Endpoint 1"));

    // Update Overall Target to Null for the Closure Control on Endpoint 1
    DataModel::Nullable<GenericOverallTargetState> ep1Target;

    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().GetOverallTargetState(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to get overall target for Step command"));

    if (ep1Target.IsNull())
    {
        ep1Target.SetNonNull(GenericOverallTargetState{});
    }

    ep1Target.Value().position = NullOptional; // Reset position to Null

    VerifyOrReturnValue(mClosureEndpoint1.GetLogic().SetOverallTargetState(ep1Target) == CHIP_NO_ERROR, Status::Failure,
                        ChipLogError(AppServer, "Failed to set overall target for Step command"));

    if (endpointId == kClosurePanelEndpoint2)
    {
        mEp2CurrentAction    = ClosureManager::ClosureAction::kStepAction;
        mEp2MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp2ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for Step command on Endpoint 2");
    }
    else if (endpointId == kClosurePanelEndpoint3)
    {
        mEp3CurrentAction    = ClosureManager::ClosureAction::kStepAction;
        mEp3MotionInProgress = true;
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
        (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                     HandleEp3ClosureActionTimer, this);
        ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for Step command on Endpoint 3");
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return Status::Failure;
    }
    return Status::Success;
}

void ClosureManager::HandleEp1ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp1ClosureActionTimer called with current action: %d",
                 static_cast<int>(instance.mEp1CurrentAction));

    switch (instance.mEp1CurrentAction)
    {
    case ClosureAction::kCalibrateAction:
        instance.HandleCalibrateActionComplete();
        break;
    case ClosureAction::kStopAction:
        // Add logic to handle Stop action completion
        break;
    case ClosureAction::kMoveToAction:
        instance.HandleClosureMotionAction();
        break;
    case ClosureAction::kLatchAction:
        // Add logic to handle Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp1ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleEp2ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp2ClosureActionTimer called with current action: %d",
                 static_cast<int>(instance.mEp2CurrentAction));

    switch (instance.mEp2CurrentAction)
    {
    case ClosureAction::kSetTargetAction:
        instance.HandlePanelSetTargetAction(kClosurePanelEndpoint2);
        break;
    case ClosureAction::kStepAction:
        instance.HandlePanelStepAction(kClosurePanelEndpoint2);
        break;
    case ClosureAction::kPanelUnLatchAction:
        instance.HandlePanelUnlatchAction(kClosurePanelEndpoint2);
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp2ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleEp3ClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();
    ChipLogError(AppServer, "HandleEp3ClosureActionTimer called with current action: %d",
                 static_cast<int>(instance.mEp3CurrentAction));

    switch (instance.mEp3CurrentAction)
    {
    case ClosureAction::kSetTargetAction:
        instance.HandlePanelSetTargetAction(kClosurePanelEndpoint3);
        break;
    case ClosureAction::kStepAction:
        instance.HandlePanelStepAction(kClosurePanelEndpoint3);
        break;
    case ClosureAction::kPanelLatchAction:
        instance.HandlePanelUnlatchAction(kClosurePanelEndpoint3);
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleEp3ClosureActionTimer");
        break;
    }
}

void ClosureManager::HandlePanelUnlatchAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint instance based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * panelEp = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(panelEp != nullptr,
                   ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));

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

        VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(ep1OverallCurrentState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
        VerifyOrReturn(!ep1OverallCurrentState.IsNull(), ChipLogError(AppServer, "Current state is not set for Endpoint 1"));

        // In Real application, this would be replaced with actual unlatch logic.
        ChipLogProgress(AppServer, "Performing unlatch action");

        ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
        mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1OverallCurrentState);

        panelCurrentState.Value().latch.SetValue(false);
        panelEp->GetLogic().SetCurrentState(panelCurrentState);

        ChipLogProgress(AppServer, "Unlatched action completed");
    }

    // Cancel any active timers before starting the panel movement
    if (endpointId == kClosurePanelEndpoint2)
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
    }
    else if (endpointId == kClosurePanelEndpoint3)
    {
        (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return;
    }

    instance.HandlePanelSetTargetAction(endpointId);
}

void ClosureManager::HandlePanelSetTargetAction(EndpointId endpointId)
{
    // Get the endpoint instance based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(ep != nullptr,
                   ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));

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
        // Start the timer for the respective endpoint to continue with the SetTarget action
        if (endpointId == kClosurePanelEndpoint2)
        {
            mEp2CurrentAction = ClosureManager::ClosureAction::kSetTargetAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp2ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for SetTarget Action command on Endpoint 2");
        }
        else if (endpointId == kClosurePanelEndpoint3)
        {
            mEp3CurrentAction = ClosureManager::ClosureAction::kSetTargetAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp3ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for SetTarget Action command on Endpoint 3");
        }
        else
        {
            ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        }
        return;
    }

    // If currently unlatched (false) and target is latched (true), latch after completing motion
    if (panelCurrentState.Value().latch.HasValue() && !panelCurrentState.Value().latch.Value().IsNull() &&
        panelTargetState.Value().latch.HasValue() && !panelTargetState.Value().latch.Value().IsNull())
    {
        if (!panelCurrentState.Value().latch.Value().Value() && panelTargetState.Value().latch.Value().Value())
        {
            DataModel::Nullable<GenericOverallCurrentState> ep1OverallCurrentState = DataModel::NullNullable;
            VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(ep1OverallCurrentState) == CHIP_NO_ERROR,
                           ChipLogError(AppServer, "Failed to get overall current state for Endpoint 1"));
            VerifyOrReturn(!ep1OverallCurrentState.IsNull(),
                           ChipLogError(AppServer, "Overall current state is not set for Endpoint 1"));

            // In Real application, this would be replaced with actual latch logic.
            ChipLogProgress(AppServer, "Performing latch action");

            ep1OverallCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1OverallCurrentState);

            panelCurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            ep->GetLogic().SetCurrentState(panelCurrentState);

            ChipLogProgress(AppServer, "Latch action completed");
        }
    }
    // If the target position is reached, call the HandlePanelSetTargetActionComplete method to complete the action
    HandlePanelSetTargetActionComplete(endpointId);
}

void ClosureManager::HandlePanelStepAction(EndpointId endpointId)
{
    ClosureManager & instance = ClosureManager::GetInstance();

    // Get the endpoint instance based on the endpointId
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * ep = GetCurrentPanelInstance(endpointId);
    VerifyOrReturn(ep != nullptr,
                   ChipLogError(AppServer, "HandlePanelSetTargetAction called with invalid endpointId: %u", endpointId));
    StepDirectionEnum stepDirection = ep->GetDelegate().GetStepCommandTargetDirection();

    DataModel::Nullable<GenericDimensionStateStruct> panelCurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> panelTargetState;

    VerifyOrReturn(ep->GetLogic().GetCurrentState(panelCurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Step action"));
    VerifyOrReturn(ep->GetLogic().GetTargetState(panelTargetState) == CHIP_NO_ERROR,
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
        VerifyOrReturn(ep->GetLogic().GetStepValue(stepValue) == CHIP_NO_ERROR,
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
        ep->GetLogic().SetCurrentState(panelCurrentState);

        // Cancel any existing timer before starting a new action
        if (endpointId == kClosurePanelEndpoint2)
        {
            instance.mEp2CurrentAction = ClosureManager::ClosureAction::kStepAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp2ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp2ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp2ClosureActionTimer for Step Action command on Endpoint 2");
        }
        else if (endpointId == kClosurePanelEndpoint3)
        {
            instance.mEp3CurrentAction = ClosureManager::ClosureAction::kStepAction;
            (void) DeviceLayer::SystemLayer().CancelTimer(HandleEp3ClosureActionTimer, this);
            (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs),
                                                         HandleEp3ClosureActionTimer, this);
            ChipLogError(AppServer, "Triggered HandleEp3ClosureActionTimer for Step Action command on Endpoint 3");
        }
        else
        {
            ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        }
        return;
    }

    // If the target position is reached, we can complete the action
    HandlePanelStepActionComplete(endpointId);
}

void ClosureManager::HandleClosureMotionAction()
{
    ClosureManager & instance = ClosureManager::GetInstance();

    DataModel::Nullable<GenericOverallCurrentState> ep1CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep2CurrentState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3CurrentState;

    DataModel::Nullable<GenericOverallTargetState> ep1TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep2TargetState;
    DataModel::Nullable<GenericDimensionStateStruct> ep3TargetState;

    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallCurrentState(ep1CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 1"));
    VerifyOrReturn(mClosureEndpoint1.GetLogic().GetOverallTargetState(ep1TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 1"));

    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetCurrentState(ep2CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetCurrentState(ep3CurrentState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get current state for Endpoint 3"));
    VerifyOrReturn(mClosurePanelEndpoint2.GetLogic().GetTargetState(ep2TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 2"));
    VerifyOrReturn(mClosurePanelEndpoint3.GetLogic().GetTargetState(ep3TargetState) == CHIP_NO_ERROR,
                   ChipLogError(AppServer, "Failed to get target state for Endpoint 3"));

    VerifyOrReturn(!ep1CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 1"));
    VerifyOrReturn(!ep2CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 2"));
    VerifyOrReturn(!ep3CurrentState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Current state on Endpoint 3"));

    VerifyOrReturn(!ep1TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 1"));
    VerifyOrReturn(!ep2TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 2"));
    VerifyOrReturn(!ep3TargetState.IsNull(),
                   ChipLogError(AppServer, "MoveToCommand failed due to Null value Target state on Endpoint 3"));

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
            instance.mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1CurrentState);
            ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(false));
            instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "Unlatched action completed");
        }
    }

    // Once Closure is unlatched, we can proceed with the motion action for endpoints 2 and 3.
    DataModel::Nullable<chip::Percent100ths> ep2NextPosition = DataModel::NullNullable;
    DataModel::Nullable<chip::Percent100ths> ep3NextPosition = DataModel::NullNullable;

    bool isEndPoint2ProgressPossible = false;
    bool isEndPoint3ProgressPossible = false;

    // Get the Next Current State to be set for the endpoint 2, if target postion is not reached.
    if (GetPanelNextPosition(ep2CurrentState.Value(), ep2TargetState.Value(), ep2NextPosition))
    {
        VerifyOrReturn(!ep2NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 2"));
        ep2CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep2NextPosition.Value()));
        instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(ep2CurrentState);
        isEndPoint2ProgressPossible = (ep2NextPosition.Value() != ep2TargetState.Value().position.Value().Value());
        ChipLogProgress(AppServer, "EndPoint 2 Current Position: %d, Target Position: %d", ep2NextPosition.Value(),
                        ep2TargetState.Value().position.Value().Value());
    }

    // Get the Next Current State to be set for the endpoint 3, if target postion is not reached.
    if (GetPanelNextPosition(ep3CurrentState.Value(), ep3TargetState.Value(), ep3NextPosition))
    {
        VerifyOrReturn(!ep3NextPosition.IsNull(), ChipLogError(AppServer, "Failed to get next position for Endpoint 3"));
        ep3CurrentState.Value().position.SetValue(DataModel::MakeNullable(ep3NextPosition.Value()));
        instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(ep3CurrentState);
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
        mEp1CurrentAction    = ClosureAction::kMoveToAction;
        mEp1MotionInProgress = true;
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(kMotionCountdownTimeMs), HandleEp1ClosureActionTimer,
                                              this);
        ChipLogProgress(AppServer, "Rescheduled HandleEp1ClosureActionTimer for motion action");
        return;
    }

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
            instance.mClosureEndpoint1.GetLogic().SetOverallCurrentState(ep1CurrentState);
            ep2CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosurePanelEndpoint2.GetLogic().SetCurrentState(ep2CurrentState);
            ep3CurrentState.Value().latch.SetValue(DataModel::MakeNullable(true));
            instance.mClosurePanelEndpoint3.GetLogic().SetCurrentState(ep3CurrentState);
            ChipLogProgress(AppServer, "latched action complete");
        }
    }

    // Target reached, call HandleMoveToActionComplete
    instance.HandleMoveToActionComplete();
}

bool ClosureManager::GetPanelNextPosition(const GenericDimensionStateStruct & currentState,
                                          const GenericDimensionStateStruct & targetState,
                                          DataModel::Nullable<chip::Percent100ths> & nextPosition)
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

ClosureDimensionEndpoint * ClosureManager::GetCurrentPanelInstance(EndpointId endpointId)
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
    return nullptr;
}

void ClosureManager::HandleCalibrateActionComplete()
{
    ChipLogProgress(AppServer, "HandleCalibrateActionComplete called");

    mClosureEndpoint1.OnCalibrateActionComplete();
    mClosurePanelEndpoint2.OnCalibrateActionComplete();
    mClosurePanelEndpoint3.OnCalibrateActionComplete();
    mIsCalibrationActionInProgress = false;
    mEp1CurrentAction              = ClosureAction::kInvalidAction;
}

void ClosureManager::HandleStopActionComplete()
{
    ChipLogProgress(AppServer, "HandleStopActionComplete called");
    if (mIsCalibrationActionInProgress)
    {
        ChipLogDetail(AppServer, "Stopping calibration action");
        mClosureEndpoint1.OnStopCalibrateActionComplete();
        mClosurePanelEndpoint2.OnStopCalibrateActionComplete();
        mClosurePanelEndpoint3.OnStopCalibrateActionComplete();
        mIsCalibrationActionInProgress = false;
    }
    else if (mEp1MotionInProgress)
    {
        ChipLogDetail(AppServer, "Stopping move to action");
        mClosureEndpoint1.OnStopMotionActionComplete();
        mClosurePanelEndpoint2.OnStopMotionActionComplete();
        mClosurePanelEndpoint3.OnStopMotionActionComplete();
        mEp1MotionInProgress = false;
    }
    else
    {
        ChipLogDetail(AppServer, "No action in progress to stop");
    }
}

void ClosureManager::HandleMoveToActionComplete()
{
    ChipLogProgress(AppServer, "HandleMoveToActionComplete called");
    mClosureEndpoint1.OnMoveToActionComplete();
    mClosurePanelEndpoint2.OnMoveToActionComplete();
    mClosurePanelEndpoint3.OnMoveToActionComplete();
    mEp1MotionInProgress = false;
    mEp1CurrentAction    = ClosureAction::kInvalidAction;
}

void ClosureManager::HandlePanelSetTargetActionComplete(chip::EndpointId endpointId)
{
    ChipLogProgress(AppServer, "HandleSetTargetActionComplete called");

    ClosureManager & instance = ClosureManager::GetInstance();
    instance.mClosureEndpoint1.OnPanelMotionActionComplete();

    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
    {
        instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        instance.mEp2CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp2MotionInProgress = false;
    }
    else if (endpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
    {
        instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        instance.mEp3CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp3MotionInProgress = false;
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for SetTarget command: %d", endpointId);
        return;
    }

    ChipLogProgress(AppServer, "SetTarget action completed for Endpoint %d", endpointId);
}

void ClosureManager::HandlePanelStepActionComplete(chip::EndpointId endpointId)
{
    ChipLogProgress(AppServer, "HandleStepActionComplete called");

    ClosureManager & instance = ClosureManager::GetInstance();
    instance.mClosureEndpoint1.OnPanelMotionActionComplete();
    if (endpointId == instance.mClosurePanelEndpoint2.GetEndpointId())
    {
        instance.mClosurePanelEndpoint2.OnPanelMotionActionComplete();
        instance.mEp2CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp2MotionInProgress = false;
    }
    else if (endpointId == instance.mClosurePanelEndpoint3.GetEndpointId())
    {
        instance.mClosurePanelEndpoint3.OnPanelMotionActionComplete();
        instance.mEp3CurrentAction    = ClosureAction::kInvalidAction;
        instance.mEp3MotionInProgress = false;
    }
    else
    {
        ChipLogError(AppServer, "Invalid endpoint ID for Step command: %d", endpointId);
        return;
    }

    ChipLogProgress(AppServer, "Step action completed for Endpoint %d", endpointId);
}
