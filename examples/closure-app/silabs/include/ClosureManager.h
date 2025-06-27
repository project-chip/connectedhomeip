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
#include <AppEvent.h>
#include <lib/core/DataModelTypes.h>

class ClosureManager
{
public:
    enum Action_t
    {
        CALIBRATE_ACTION = 0,
        MOVE_TO_ACTION,
        STOP_MOTION_ACTION,
        STOP_CALIBRATE_ACTION,
        PANEL_STEP_ACTION,

        INVALID_ACTION
    };

    /**
     * @brief Initializes the ClosureManager and its associated resources.
     *
     * This method performs the following actions:
     * - Creates a CMSIS-OS software timer for closure operations.
     * - Initializes closure endpoints (ep1, ep2, ep3).
     * - Sets the semantic tag lists for each closure endpoint.
     */
    void Init();

    /**
     * @brief Returns the singleton instance of the ClosureManager.
     *
     * This static method provides access to the single, global instance of the ClosureManager,
     * ensuring that only one instance exists throughout the application's lifetime.
     *
     * @return Reference to the singleton ClosureManager instance.
     */
    static ClosureManager & GetInstance() { return sClosureMgr; }

    /**
     * @brief Handles the calibration command for the closure.
     *
     * This method initiates the calibration process by setting a countdown timer.
     * It posts a calibration action event to the application task and marks
     * the calibration action as in progress.
     *
     * @return chip::Protocols::InteractionModel::Status
     *         Returns Status::Success if all operations succeed, otherwise Status::Failure.
     */
    chip::Protocols::InteractionModel::Status OnCalibrateCommand();

    /**
     * @brief Handles the MoveTo command for the Closure.
     *
     * This method processes the MoveTo command, which is used to initiate a motion action
     * for a closure.
     *
     * @param position Optional target position for the closure device.
     * @param latch Optional flag indicating whether the closure should latch after moving.
     * @param speed Optional speed setting for the movement, represented as a ThreeLevelAutoEnum.
     * @return chip::Protocols::InteractionModel::Status Status of the command handling operation.
     */
    chip::Protocols::InteractionModel::Status
    OnMoveToCommand(const chip::Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> position,
                    const chip::Optional<bool> latch, const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> speed);

    /**
     * @brief Handles the Stop command for the Closure.
     *
     * This method processes the Stop command, which is used to stop an action for a closure.
     *
     * @return chip::Protocols::InteractionModel::Status
     *         Returns Status::Success if the Stop command is handled successfully,
     *         or an appropriate error status otherwise.
     */
    chip::Protocols::InteractionModel::Status OnStopCommand();

    /**
     * @brief Handles the Step command for the ClosureDimension cluster.
     *
     * This method processes and initiates step motion in a specified direction for a given number of steps,
     * optionally at a specified speed, on the provided panel endpoint.
     *
     * @param direction The direction in which to perform the step operation.
     * @param numberOfSteps The number of steps to move in the specified direction.
     * @param speed Optional speed setting for the step operation.
     * @param endpointId The endpoint on which to perform the operation.
     * @return chip::Protocols::InteractionModel::Status Status of the command execution.
     */
    chip::Protocols::InteractionModel::Status
    OnStepCommand(const chip::app::Clusters::ClosureDimension::StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                  const chip::Optional<chip::app::Clusters::Globals::ThreeLevelAutoEnum> & speed,
                  const chip::EndpointId & endpointId);

    /**
     * @brief Sets the current action being performed by the closure device.
     *
     * @param action The action to set, represented as chip::app::Clusters::ClosureControl::Action_t.
     */
    void SetCurrentAction(Action_t newAction) { mCurrentAction = newAction; }

    /**
     * @brief Retrieves the current action being performed by the closure device.
     *
     * @return The current action as defined by chip::app::Clusters::ClosureControl::Action_t.
     */
    const Action_t & GetCurrentAction() const { return mCurrentAction; }

private:
    static ClosureManager sClosureMgr;
    osTimerId_t mClosureTimer;

    bool isCalibrationInProgress = false;
    bool isStepActionInProgress  = false;

    Action_t mCurrentAction                   = Action_t::INVALID_ACTION;
    chip::EndpointId mCurrentActionEndpointId = chip::kInvalidEndpointId;

    // Define the endpoint ID for the Closure
    static constexpr chip::EndpointId kClosureEndpoint       = 1;
    static constexpr chip::EndpointId kClosurePanel1Endpoint = 2;
    static constexpr chip::EndpointId kClosurePanel2Endpoint = 3;

    chip::app::Clusters::ClosureControl::ClosureControlEndpoint ep1{ kClosureEndpoint };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint ep2{ kClosurePanel1Endpoint };
    chip::app::Clusters::ClosureDimension::ClosureDimensionEndpoint ep3{ kClosurePanel2Endpoint };

    /**
     * @brief Starts or restarts the closure function timer with the specified timeout.
     *
     * This method initiates or resets the timer associated with closure operations.
     * If the timer fails to start, an error is logged and the application error handler is invoked.
     *
     * @param aTimeoutMs Timeout duration in milliseconds for the timer.
     */
    void StartTimer(uint32_t aTimeoutMs);

    /**
     * @brief Cancels the closure timer if it is currently running.
     *
     * Attempts to stop the mClosureTimer using the osTimerStop function.
     * If stopping the timer fails, logs an error message and triggers
     * the application error handler with APP_ERROR_STOP_TIMER_FAILED.
     */
    void CancelTimer();

    /**
     * @brief Handles the completion of a closure action.
     *
     * This method is called when a closure action has completed. It notifies relevant endpoints
     * based on the type of action that was completed.
     *
     * @param action The action that has completed, used to notify relevant endpoints.
     */
    void HandleClosureActionComplete(Action_t action);

    /**
     * @brief Initiates a closure action based on the provided application event.
     *
     * This method sets the current action according to the action specified in the given AppEvent.
     * It logs the initiation of the corresponding action and, for certain actions, may start a timer.
     *
     * @param event Pointer to the AppEvent containing the action to initiate.
     */
    static void InitiateAction(AppEvent * event);

    /**
     * @brief Handles a closure action complete event.
     *
     * This method processes closure action complete event and schedules the completion of the closure action
     * to be executed asynchronously on the platform manager's work queue.
     *
     * @param event Pointer to the AppEvent containing closure event details.
     */

    static void HandleClosureActionCompleteEvent(AppEvent * event);

    /**
     * @brief Timer event handler for the ClosureManager.
     *
     * This static function is called when the closure timer expires. The handler creates an AppEvent and
     * posts the event to the application task queue. This ensures that the closure event is processed in the context of the
     * application task rather than the timer task.
     *
     * @param timerCbArg Pointer to the callback argument (unused).
     */
    static void TimerEventHandler(void * timerCbArg);

    /**
     * @brief Handles a single step action for the panel associated with the specified endpoint.
     *
     * This method processes a panel step action for the panel endpoint and updates the current position to 
     * reflect the step position and triggers timer if target is not reached.
     *
     * @param endpointId The identifier of the endpoint for which the panel step action is to be handled.
     */
    void HandlePanelStepAction(chip::EndpointId endpointId);
};
