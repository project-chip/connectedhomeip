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

#pragma once

#include "ClosureControlEndpoint.h"
#include "ClosureDimensionEndpoint.h"

#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <protocols/interaction_model/StatusCode.h>

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
        kPanelUnLatchAction,

        kInvalidAction
    };

    /**
     * @brief Initializes the ClosureManager.
     *
     * This method sets up the necessary resources and configurations required
     * for the Closure Application to function properly.
     */
    void Init();

    static ClosureManager & GetInstance() { return sInstance; }

    /**
     * @brief Handles the "Calibrate" command for the closure manager.
     *
     * This method initiates the calibration action for the closure system. It triggers closure action timer and
     * updates the current action and endpoint ID accordingly.
     *
     * @return Status::Success if the calibration command is successfully processed.
     */
    chip::Protocols::InteractionModel::Status OnCalibrateCommand();

    /**
     * @brief Handles the "MoveTo" command for the closure manager.
     *
     * This method initiates the move to command for the closure system, allowing it to move
     * to a specified position with an optional latch and speed setting.
     *
     * @param position The target position to move to.
     * @param latch Optional parameter indicating whether to latch the closure.
     * @param speed Optional speed setting for the move action.
     * @return Status::Success if the move to command is successfully processed.
     */
    chip::Protocols::InteractionModel::Status
    OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> & position,
                    const chip::Optional<bool> & latch,
                    const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed);

    /*
     * @brief Handles the "Stop" command for the closure manager.
     *
     * This method processes the stop command for the closure system, allowing it to halt any ongoing
     * actions such as calibration or movement. It resets the current action and updates the state of
     * all endpoints accordingly.
     *
     * @return Status::Success if the stop command is successfully processed.
     */
    chip::Protocols::InteractionModel::Status OnStopCommand();

    /**
     * @brief Handles the "SetTarget" command for the closure manager.
     *
     * This method processes the set target command for the closure system, allowing it to move
     * to a specified position with an optional latch and speed setting.
     *
     * @param pos The target position to set, represented as a percentage in 100ths.
     * @param latch Optional parameter indicating whether to latch the closure.
     * @param speed Optional speed setting for the closure action.
     * @param endpointId The endpoint ID for which this command is being processed.
     * @return Status::Success if the set target command is successfully processed.
     */
    chip::Protocols::InteractionModel::Status
    OnSetTargetCommand(const chip::Optional<chip::Percent100ths> & pos, const chip::Optional<bool> & latch,
                       const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                       const chip::EndpointId endpointId);

    /*
     * @brief Handles the "Step" command for the closure manager.
     *
     * This method processes the step command for the closure system, allowing it to move
     * in a specified direction by a certain number of steps, with an optional speed setting.
     *
     * @param direction The direction in which to step (increase or decrease).
     * @param numberOfSteps The number of steps to move in the specified direction.
     * @param speed Optional speed setting for the step action.
     * @param endpointId The endpoint ID for which this command is being processed.
     * @return Status::Success if the step command is successfully processed.
     */
    chip::Protocols::InteractionModel::Status
    OnStepCommand(const chip::app::Clusters::ClosureDimension::StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                  const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                  const chip::EndpointId endpointId);

private:
    /**
     * @brief Singleton instance of the ClosureManager.
     *
     * Used to provide global access to the ClosureManager throughout the application.
     */
    static ClosureManager sInstance;

    // Endpoint ID for the main closure device
    static constexpr chip::EndpointId kClosureEndpoint1 = 1;
    // Endpoint ID for the first closure panel
    static constexpr chip::EndpointId kClosurePanelEndpoint2 = 2;
    // Endpoint ID for the second closure panel
    static constexpr chip::EndpointId kClosurePanelEndpoint3 = 3;

    chip::app::Clusters::ClosureControl::ClosureControlEndpoint mClosureEndpoint1{ kClosureEndpoint1 };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint mClosurePanelEndpoint2{ kClosurePanelEndpoint2 };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint mClosurePanelEndpoint3{ kClosurePanelEndpoint3 };

    /**
     * @brief Stores the current endpoint ID being managed or operated on.
     *
     * Initialized to an invalid endpoint ID and updated as needed during operations.
     */
    chip::EndpointId mCurrentEndpointId = chip::kInvalidEndpointId;

    /**
     * @brief Tracks the current action being performed by the ClosureManager.
     *
     * Initialized to an invalid action and updated as needed during operations.
     */
    ClosureAction mCurrentAction = ClosureAction::kInvalidAction;

    /**
     * @brief Timer callback handler for closure actions.
     *
     * This static method is called when the timer for a closure action expires.
     * It is responsible for handling the completion or progression of closure-related actions.
     *
     * @param layer Pointer to the system layer that triggered the timer.
     * @param aAppState Application-specific state or context passed to the timer.
     */
    static void HandleClosureActionTimer(chip::System::Layer * layer, void * aAppState);

    /**
     * @brief Handles the motion action for the closure system.
     *
     * This method is called when a move-to action has been initiated,
     * allowing for any necessary updates or state changes.
     */
    void HandleClosureMotionAction();

    /**
     * @brief Calculates the next position for a panel based on the closure panel state.
     *
     * This function determines the next position by incrementing or decrementing current position of the panel
     * by a fixed step (1000 units) towards the target position, ensuring it does not overshoot the target.
     *
     * @param[in]  currentState   The current state of the panel, containing the current position.
     * @param[in]  targetState    The target state of the panel, containing the desired position.
     * @param[out] nextPosition   A reference to a Nullable object that will be updated with the next current position.
     *
     * @return true if the next position was updated and movement is required; false if no update is needed
     *         or if either the current or target position is not set.
     */
    bool GetPanelNextPosition(const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & currentState,
                              const chip::app::Clusters::ClosureDimension::GenericDimensionStateStruct & targetState,
                              chip::app::DataModel::Nullable<chip::Percent100ths> & nextPosition);

    /**
     * @brief Handles the step action for a panel endpoint.
     *
     * This method updates the current position of the panel endpoint based on the step action
     * and checks if the target position is reached. If so, it performs the latch action
     * if required.
     *
     * @param endpointId The identifier of the endpoint for which the panel step action should be handled.
     */
    void HandlePanelStepAction(chip::EndpointId endpointId);

    /**
     * @brief Handles the SetTarget motion action for a panel endpoint.
     *
     * This method Performs the update the current positions of panel endpoint to next position
     * and when target position is reached, it performs the latch action if required.
     *
     * @param endpointId The identifier of the endpoint for which the panel target action should be handled.
     */
    void HandlePanelSetTargetAction(chip::EndpointId endpointId);

    /**
     * @brief Handles the unlatch action for a closure panel.
     *
     * This method performs the unlatch action if required for the specified closure panel endpoint.
     * It updates the current state of the panel and sets the target state accordingly and then calls
     * HandlePanelSetTargetAction to move the panel to the target position.
     *
     * @param endpointId The identifier of the endpoint for which the unlatch action should be handled.
     */
    void HandlePanelUnlatchAction(chip::EndpointId endpointId);

    /**
     * @brief Handles the completion of a Calibrate action.
     *
     * This method is called when a calibrate action has finished executing.
     * It is responsible for handling necessary cleanup or endpoint state updates.
     *
     * @param action The action that has been completed.
     */
    void HandleCalibrateActionComplete();

    /**
     * @brief Handles the completion of a MoveTo action.
     *
     * This method is called when a move-to action has finished executing.
     * It is responsible for handling necessary cleanup or endpoint state updates.
     *
     * @param action The action that has been completed.
     */
    void HandleMoveToActionComplete();

    /**
     * @brief Handles the completion of a Stop action.
     *
     * This method is called when a stop action has finished executing.
     * It is responsible for handling necessary cleanup or endpoint state updates.
     *
     * @param action The action that has been completed.
     */
    void HandleStopActionComplete();
    
    CHIP_ERROR SetInitialState();
    /**
     * @brief Handles the completion of a SetTarget action.
     *
     * This method is called when a set target action has finished executing.
     * It is responsible for handling necessary cleanup or endpoint state updates.
     *
     * @param action The action that has been completed.
     */
    void HandlePanelSetTargetActionComplete();

    /**
     * @brief Handles the completion of a Step action.
     *
     * This method is called when a step action has finished executing.
     * It is responsible for handling necessary cleanup or endpoint state updates.
     *
     * @param action The action that has been completed.
     */
    void HandlePanelStepActionComplete();

    bool mIsCalibrationActionInProgress       = false;
    bool mIsMoveToActionInProgress            = false;
    bool mIsSetTargetActionInProgress         = false;
    bool mIsStepActionInProgress              = false;
    chip::EndpointId mCurrentActionEndpointId = chip::kInvalidEndpointId;
