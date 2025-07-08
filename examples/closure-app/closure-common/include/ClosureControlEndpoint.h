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

#include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
#include <app/clusters/closure-control-server/closure-control-cluster-matter-context.h>
#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <app/clusters/closure-control-server/closure-control-server.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/TestEventTriggerDelegate.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @class ClosureControlDelegate
 * @brief Delegate class for handling closure control commands and Test event triggers.
 *
 * Inherits from DelegateBase and TestEventTriggerHandler to provide implementations
 * for closure control operations such as Stop, MoveTo, Calibration, and
 * error retrieval, and Test event triggering.
 */
class ClosureControlDelegate : public DelegateBase, public TestEventTriggerHandler
{
public:
    ClosureControlDelegate() {}

    // Override for the DelegateBase Virtual functions
    Protocols::InteractionModel::Status HandleStopCommand() override;
    Protocols::InteractionModel::Status HandleMoveToCommand(const Optional<TargetPositionEnum> & position,
                                                            const Optional<bool> & latch,
                                                            const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    Protocols::InteractionModel::Status HandleCalibrateCommand() override;

    bool IsReadyToMove() override;
    bool IsManualLatchingNeeded() override;
    ElapsedS GetCalibrationCountdownTime() override;
    ElapsedS GetMovingCountdownTime() override;
    ElapsedS GetWaitingForMotionCountdownTime() override;

    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

    // Delegate specific functions and variables

    void SetLogic(ClusterLogic * logic) { mLogic = logic; }

    ClusterLogic * GetLogic() const { return mLogic; }

private:
    ClusterLogic * mLogic;
};

/**
 * @class ClosureControlEndpoint
 * @brief Represents a Closure Control cluster endpoint.
 *
 * This class encapsulates the logic and interfaces required to manage a Closure Control cluster endpoint.
 * It integrates the delegate, context, logic, and interface components for the endpoint.
 *
 * @param mEndpoint The endpoint ID associated with this Closure Control endpoint.
 * @param mContext The Matter context for the endpoint.
 * @param mDelegate The delegate instance for handling commands.
 * @param mLogic The cluster logic associated with the endpoint.
 * @param mInterface The interface for interacting with the cluster.
 */
class ClosureControlEndpoint
{
public:
    ClosureControlEndpoint(EndpointId endpoint) :
        mEndpoint(endpoint), mContext(mEndpoint), mDelegate(), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
    {
        mDelegate.SetLogic(&mLogic);
    }

    /**
     * @brief Initializes the ClosureControlEndpoint instance.
     *
     * @return CHIP_ERROR indicating the result of the initialization.
     */
    CHIP_ERROR Init();

    /**
     * @brief Retrieves the delegate associated with this Closure Control endpoint.
     *
     * @return Reference to the ClosureControlDelegate instance.
     */
    ClosureControlDelegate & GetDelegate() { return mDelegate; }

    /**
     * @brief Returns a reference to the ClusterLogic instance associated with this object.
     *
     * @return ClusterLogic& Reference to the internal ClusterLogic object.
     */
    ClusterLogic & GetLogic() { return mLogic; }

    /**
     * @brief Handles the completion of a stop motion action.
     *
     * This function is called when a motion action has been stopped.
     * It should update the internal state of the closure control endpoint to reflect the
     * stopping of the motion action.
     */
    void OnStopMotionActionComplete();

    /**
     * @brief Handles the completion of the stop calibration action.
     *
     * This function is called when the calibration action has been stopped.
     * It should update the internal state of the closure control endpoint to reflect the
     * stopping of the calibration action.
     */
    void OnStopCalibrateActionComplete();

    /**
     * @brief Handles the completion of a calibration action.
     *
     * This method is called when the calibration process is finished.
     * It should update the internal state of the closure control endpoint to reflect the
     * completion of the calibration action, resets the countdown timer and generates
     * a motion completed event.
     */
    void OnCalibrateActionComplete();

    /**
     * @brief Handles the completion of a motion action for closure control.
     *
     * This function is called when a move-to action has finished executing.
     * It should update the internal state of the closure control endpoint to reflect the
     * completion of the move-to action, resets the countdown timer and generates
     * a motion completed event.
     */
    void OnMoveToActionComplete();

    /**
     * @brief Handles the completion of a panel motion action for closure endpoint.
     *
     * This function is called when a panel motion action has been completed.
     * It updates the internal state of the closure endpoint to reflect
     * the completion of the panel motion action.
     */
    void OnPanelMotionActionComplete();

    /**
     * @brief Retrieves the endpoint ID associated with this Closure Control endpoint.
     *
     * @return The EndpointId of this Closure Control endpoint.
     */
    EndpointId GetEndpointId() const { return mEndpoint; }

private:
    EndpointId mEndpoint = kInvalidEndpointId;
    MatterContext mContext;
    ClosureControlDelegate mDelegate;
    ClusterLogic mLogic;
    Interface mInterface;

    /**
     * @brief Updates the current state of the closure control endpoint from the target state.
     *
     * This function retrieves the target state and updates the current state accordingly.
     * It ensures that the current state reflects the latest target position, latch status, and speed.
     */
    void UpdateCurrentStateFromTargetState();

    /**
     * @brief Maps a TargetPositionEnum value to the corresponding CurrentPositionEnum value.
     *
     * @param value The TargetPositionEnum value to be mapped.
     * @return CurrentPositionEnum The corresponding CurrentPositionEnum value.
     */
    CurrentPositionEnum MapTargetPositionToCurrentPositioning(TargetPositionEnum value);
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
