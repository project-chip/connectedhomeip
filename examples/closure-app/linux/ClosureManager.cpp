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

chip::Protocols::InteractionModel::Status ClosureManager::OnCalibrateCommand()
{
    // Cancel any existing timer for closure action
    DeviceLayer::SystemLayer().CancelTimer(HandleClosureActionTimer, this);

    // For sample application, we are using a timer to simulate the hardware calibration action.
    // In a real application, this would be replaced with actual calibration logic and call HandleClosureActionComplete.
    VerifyOrReturnValue(DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kCountdownTimeSeconds),
                                                              HandleClosureActionTimer, nullptr) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to start closure action timer"));
    mCurrentAction                 = ClosureAction::kCalibrateAction;
    mCurrentEndpointId             = kClosureEndpoint1;
    mIsCalibrationActionInProgress = true;

    ChipLogProgress(AppServer, "ClosureManager: Calibration action started for endpoint %d", mCurrentEndpointId);

    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStopCommand()
{
    // Add logic to handle the Stop command
    return Status::Success;
}

chip::Protocols::InteractionModel::Status
ClosureManager::OnMoveToCommand(const Optional<TargetPositionEnum> & position, const Optional<bool> & latch,
                                const Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed)
{
    // Add logic to handle the MoveTo command
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnSetTargetCommand(const Optional<Percent100ths> & pos,
                                                                             const Optional<bool> & latch,
                                                                             const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                             const chip::EndpointId endpointId)
{
    // Add logic to handle the SetTarget command
    return Status::Success;
}

chip::Protocols::InteractionModel::Status ClosureManager::OnStepCommand(const StepDirectionEnum & direction,
                                                                        const uint16_t & numberOfSteps,
                                                                        const Optional<Globals::ThreeLevelAutoEnum> & speed,
                                                                        const chip::EndpointId endpointId)
{
    // Add logic to handle the Step command
    return Status::Success;
}

void ClosureManager::HandleClosureActionTimer(System::Layer * layer, void * aAppState)
{
    // Mark aAppState as unused to avoid compiler warnings
    // Will be used in closure dimension cluster Commands
    (void) aAppState;

    ClosureManager & instance = ClosureManager::GetInstance();

    switch (instance.mCurrentAction)
    {
    case ClosureAction::kCalibrateAction:
        instance.HandleCalibrateActionComplete();
        break;
    case ClosureAction::kStopAction:
        // Add logic to handle Stop action completion
        break;
    case ClosureAction::kMoveToAction:
        // Add logic to handle MoveTo action completion
        break;
    case ClosureAction::kLatchAction:
        // Add logic to handle Latch action completion
        break;
    case ClosureAction::kSetTargetAction:
        // Add logic to handle SetTarget action completion
        break;
    case ClosureAction::kStepAction:
        // Add logic to handle Step action completion
        break;
    case ClosureAction::kPanelLatchAction:
        // Add logic to handle Panel Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleCalibrateActionComplete()
{
    ChipLogProgress(AppServer, "HandleCalibrateActionComplete called");

    mClosureEndpoint1.OnCalibrateActionComplete();
    mClosurePanelEndpoint2.OnCalibrateActionComplete();
    mClosurePanelEndpoint3.OnCalibrateActionComplete();
    mIsCalibrationActionInProgress = false;
    mCurrentAction                 = ClosureAction::kInvalidAction;
    mCurrentEndpointId             = chip::kInvalidEndpointId;
}

void ClosureManager::HandleStopActionComplete()
{
    // Add logic to handle Stop action completion
}

void ClosureManager::HandleMoveToActionComplete()
{
    // Add logic to handle MoveTo action completion
}

void ClosureManager::HandleSetTargetActionComplete()
{
    // Add logic to handle SetTarget action completion
}

void ClosureManager::HandleStepActionComplete()
{
    // Add logic to handle Step action completion
}
