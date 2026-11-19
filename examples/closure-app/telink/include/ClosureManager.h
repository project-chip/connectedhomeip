/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/*
 * @class ClosureManager
 * @brief Manages the initialization and operations related to closure and
 *        closure panel endpoints in the application.
 *
 * @note This class is part of the closure application example
 */

#pragma once

#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"

class ClosureManager
{
public:
    enum class ClosureAction : uint8_t
    {
        kCalibrateAction = 0,
        kMoveToAction,
        kLatchAction,
        kStopAction,
        kSetTargetAction,
        kStepAction,
        kPanelLatchAction,
        kPanelUnLatchAction, // New action for unlatching the panel before SetTarget

        kInvalidAction
    };

    static ClosureManager & GetInstance() { return sInstance; }

    void Init();
    void Shutdown();

    chip::Protocols::InteractionModel::Status OnCalibrateCommand();
    CHIP_ERROR SetClosureControlInitialState(chip::app::Clusters::ClosureControl::ClosureControlEndpoint & closureControlEndpoint);
    CHIP_ERROR SetClosurePanelInitialState(chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint & closurePanelEndpoint);
    chip::Protocols::InteractionModel::Status
    OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> & position,
                    const chip::Optional<bool> & latch,
                    const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed);
    chip::Protocols::InteractionModel::Status OnStopCommand();
    chip::Protocols::InteractionModel::Status
    OnSetTargetCommand(const chip::Optional<chip::Percent100ths> & pos, const chip::Optional<bool> & latch,
                       const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                       const chip::EndpointId endpointId);
    chip::Protocols::InteractionModel::Status
    OnStepCommand(const chip::app::Clusters::ClosureDimension::StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                  const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                  const chip::EndpointId endpointId);

private:
    ClosureManager()                                   = default;
    ~ClosureManager()                                  = default;
    ClosureManager(ClosureManager const &)             = delete;
    ClosureManager & operator=(ClosureManager const &) = delete;

    static ClosureManager sInstance;

    static constexpr chip::EndpointId kClosureEndpoint1      = 1;
    static constexpr chip::EndpointId kClosurePanelEndpoint2 = 2;
    static constexpr chip::EndpointId kClosurePanelEndpoint3 = 3;

    chip::app::Clusters::ClosureControl::ClosureControlEndpoint mClosureEndpoint1{ kClosureEndpoint1 };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint mClosurePanelEndpoint2{ kClosurePanelEndpoint2 };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint mClosurePanelEndpoint3{ kClosurePanelEndpoint3 };

    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint * GetCurrentPanelInstance(chip::EndpointId endpointId);

    ClosureAction mEp1CurrentAction = ClosureAction::kInvalidAction;
    ClosureAction mEp2CurrentAction = ClosureAction::kInvalidAction;
    ClosureAction mEp3CurrentAction = ClosureAction::kInvalidAction;

    static void HandleEp1ClosureActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleEp2ClosureActionTimer(chip::System::Layer * layer, void * aAppState);
    static void HandleEp3ClosureActionTimer(chip::System::Layer * layer, void * aAppState);

    void HandlePanelStepAction(chip::EndpointId endpointId);
    void HandlePanelSetTargetAction(chip::EndpointId endpointId);
    void HandlePanelUnlatchAction(chip::EndpointId endpointId);
    void HandleCalibrateActionComplete();
    void HandleMoveToActionComplete();
    void HandleStopActionComplete();
    void HandlePanelSetTargetActionComplete(chip::EndpointId endpointId, bool positionUpdated = true);
    void HandlePanelStepActionComplete(chip::EndpointId endpointId);
    void HandleClosureMotionAction();
    bool GetPanelNextPosition(const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & currentState,
                              const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & targetState,
                              chip::app::DataModel::Nullable<chip::Percent100ths> & nextPosition);

    bool mIsCalibrationActionInProgress = false;
    bool mEp1MotionInProgress           = false;
    bool mEp2MotionInProgress           = false;
    bool mEp3MotionInProgress           = false;
};
