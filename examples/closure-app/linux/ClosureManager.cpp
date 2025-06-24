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
    VerifyOrDie(mClosureEndpoint.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Control Endpoint initialized successfully");

    VerifyOrDie(mClosurePanelEndpoint1.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Panel Endpoint 1 initialized successfully");

    VerifyOrDie(mClosurePanelEndpoint2.Init() == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Closure Panel Endpoint 2 initialized successfully");

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
        CHIP_ERROR err = pTestEventDelegate->AddHandler(&mClosureEndpoint.GetDelegate());
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
                                                              HandleClosureActionTimer, this) == CHIP_NO_ERROR,
                        Status::Failure, ChipLogError(AppServer, "Failed to start closure action timer"));
    mCurrentAction                 = ClosureManager::Action_t::CALIBRATE_ACTION;
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
    ClosureManager * manager = reinterpret_cast<ClosureManager *>(aAppState);

    VerifyOrReturn(manager != nullptr, ChipLogError(AppServer, "HandleClosureActionTimer called with null manager"));

    ChipLogProgress(AppServer, "HandleClosureActionTimer called for action: %d", static_cast<int>(manager->mCurrentAction));

    switch (manager->mCurrentAction)
    {
    case ClosureManager::Action_t::CALIBRATE_ACTION:
        manager->HandleClosureActionComplete(ClosureManager::Action_t::CALIBRATE_ACTION);
        break;
    case ClosureManager::Action_t::STOP_ACTION:
        manager->HandleClosureActionComplete(ClosureManager::Action_t::STOP_ACTION);
        break;
    case ClosureManager::Action_t::MOVE_TO_ACTION:
        manager->HandleClosureActionComplete(ClosureManager::Action_t::MOVE_TO_ACTION);
        break;
    case ClosureManager::Action_t::LATCH_ACTION:
        // Add logic to handle Latch action completion
        break;
    case ClosureManager::Action_t::SET_TARGET_ACTION:
        manager->HandleClosureActionComplete(ClosureManager::Action_t::SET_TARGET_ACTION);
        break;
    case ClosureManager::Action_t::STEP_ACTION:
        manager->HandleClosureActionComplete(ClosureManager::Action_t::STEP_ACTION);
        break;
    case ClosureManager::Action_t::PANEL_LATCH_ACTION:
        // Add logic to handle Panel Latch action completion
        break;
    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureActionTimer");
        break;
    }
}

void ClosureManager::HandleClosureActionComplete(ClosureManager::Action_t action)
{
    ChipLogProgress(AppServer, "HandleClosureActionComplete called for action: %d", static_cast<int>(action));
    switch (action)
    {
    case ClosureManager::Action_t::CALIBRATE_ACTION: {
        mClosureEndpoint.OnCalibrateActionComplete();
        mClosurePanelEndpoint1.OnCalibrateActionComplete();
        mClosurePanelEndpoint2.OnCalibrateActionComplete();
        mIsCalibrationActionInProgress = false;
        break;
    }

    case ClosureManager::Action_t::STOP_ACTION: {
        if (mIsCalibrationActionInProgress)
        {
            mClosureEndpoint.OnStopCalibrateActionComplete();
            mClosurePanelEndpoint1.OnStopCalibrateActionComplete();
            mClosurePanelEndpoint2.OnStopCalibrateActionComplete();
            mIsCalibrationActionInProgress = false;
        }
        else if (mIsMoveToActionInProgress)
        {
            mClosureEndpoint.OnStopMotionActionComplete();
            mClosurePanelEndpoint1.OnStopMotionActionComplete();
            mClosurePanelEndpoint2.OnStopMotionActionComplete();
            mIsMoveToActionInProgress = false;
        }
        else if (mIsSetTargetActionInProgress)
        {
            // Add logic to handle stopping SetTarget action
            mIsSetTargetActionInProgress = false;
        }
        else if (mIsStepActionInProgress)
        {
            // Add logic to handle stopping Step action
            mIsStepActionInProgress = false;
        }
        break;
    }

    case ClosureManager::Action_t::MOVE_TO_ACTION: {
        mClosureEndpoint.OnMoveToActionComplete();
        mClosurePanelEndpoint1.OnMoveToActionComplete();
        mClosurePanelEndpoint2.OnMoveToActionComplete();
        mIsMoveToActionInProgress = false;
        break;
    }

    case ClosureManager::Action_t::SET_TARGET_ACTION:
        // Add logic to handle SetTarget action completion
        mIsSetTargetActionInProgress = false;
        break;

    case ClosureManager::Action_t::STEP_ACTION:
        // Add logic to handle Step action completion
        mIsStepActionInProgress = false;
        break;

    default:
        ChipLogError(AppServer, "Invalid action received in HandleClosureAction");
        break;
    }
    mCurrentAction     = ClosureManager::Action_t::INVALID_ACTION;
    mCurrentEndpointId = chip::kInvalidEndpointId;
}
