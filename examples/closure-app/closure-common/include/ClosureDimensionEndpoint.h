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

#include <app/clusters/closure-dimension-server/closure-dimension-cluster-logic.h>
#include <app/clusters/closure-dimension-server/closure-dimension-cluster-objects.h>
#include <app/clusters/closure-dimension-server/closure-dimension-delegate.h>
#include <app/clusters/closure-dimension-server/closure-dimension-matter-context.h>
#include <app/clusters/closure-dimension-server/closure-dimension-server.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

using Protocols::InteractionModel::Status;

/**
 * @class ClosureDimensionDelegate
 * @brief A delegate class that handles Closure Dimension commands at the application level.
 *
 * This class is responsible for processing Closure Dimension commands such as Stop, MoveTo, and Calibrate
 * according to specific business logic. It is designed to be used as a delegate for the Closure Dimension cluster.
 *
 */
class ClosureDimensionDelegate : public DelegateBase
{
public:
    ClosureDimensionDelegate(EndpointId endpoint) : mEndpoint(endpoint) {}

    // Override for the DelegateBase Virtual functions
    Status HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
                           const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                      const Optional<Globals::ThreeLevelAutoEnum> & speed) override;

    /**
     * @brief Retrieves the endpoint for this instance.
     *
     * @return The endpoint (EndpointId) for this instance.
     */
    EndpointId GetEndpoint() const { return mEndpoint; }

    /**
     * @brief Function to get the present target direction for the step command.
     */
    StepDirectionEnum GetStepCommandTargetDirection() const { return mStepCommandTargetDirection; }

    /**
     * @brief Function to save the present target direction of the step command.
     */
    void SetStepCommandTargetDirection(StepDirectionEnum direction) { mStepCommandTargetDirection = direction; }

private:
    EndpointId mEndpoint                          = kInvalidEndpointId;
    StepDirectionEnum mStepCommandTargetDirection = StepDirectionEnum::kUnknownEnumValue;
};

/**
 * @class ClosureDimensionEndpoint
 * @brief Represents a Closure Dimension cluster endpoint.
 *
 * This class encapsulates the logic and interfaces required to manage a Closure Dimension cluster endpoint.
 * It integrates the delegate, context, logic, and interface components for the endpoint.
 *
 * @param mEndpoint The endpoint ID associated with this Closure Dimension endpoint.
 * @param mContext The Matter context for the endpoint.
 * @param mDelegate The delegate instance for handling commands.
 * @param mLogic The cluster logic associated with the endpoint.
 * @param mInterface The interface for interacting with the cluster.
 */
class ClosureDimensionEndpoint
{
public:
    ClosureDimensionEndpoint(EndpointId endpoint) :
        mEndpoint(endpoint), mContext(mEndpoint), mDelegate(mEndpoint), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
    {}

    /**
     * @brief Initializes the ClosureDimensionEndpoint instance.
     *
     * @return CHIP_ERROR indicating the result of the initialization.
     */
    CHIP_ERROR Init();

    /**
     * @brief Retrieves the delegate associated with this Closure Dimension endpoint.
     *
     * @return Reference to the ClosureDimensionDelegate instance.
     */
    ClosureDimensionDelegate & GetDelegate() { return mDelegate; }

    /**
     * @brief Returns a reference to the associated ClusterLogic instance.
     *
     * @return ClusterLogic& Reference to the internal ClusterLogic object.
     */
    ClusterLogic & GetLogic() { return mLogic; }

    /**
     * @brief Handles the completion of a stop motion action.
     *
     * This function is called when a motion action has been stopped.
     * It updates the internal state of the closure dimension endpoint to reflect the
     * stopping of the motion action.
     */
    void OnStopMotionActionComplete();

    /**
     * @brief Handles the completion of the stop calibration action.
     *
     * This function is called when the calibration action has been stopped.
     * It updates the internal state of the closure dimension endpoint to reflect the
     * stopping of the calibration action.
     */
    void OnStopCalibrateActionComplete();

    /**
     * @brief Handles the completion of a calibration action.
     *
     * This method is called when the calibration process is finished.
     * It should update the internal state of the closure dimension endpoint to reflect
     * the completion of the calibration action, resets the countdown timer and generates
     * a motion completed event.
     */
    void OnCalibrateActionComplete();

    /**
     * @brief Handles the completion of a motion action for closure control.
     *
     * This function is called when a move-to action has finished executing.
     * It should update the internal state of the closure dimension endpoint to reflect
     * the completion of the move-to action, resets the countdown timer and generates
     * a motion completed event.
     */
    void OnMoveToActionComplete();

    /**
     * @brief Handles the completion of a panel motion action for closure Panel endpoint.
     *
     * This function is called when a panel motion action has been completed.
     * It updates the internal state of the closure panel endpoint to reflect
     * the completion of the panel motion action.
     */
    void OnPanelMotionActionComplete();

    /**
     * @brief Retrieves the endpoint ID associated with this Closure Dimension endpoint.
     *
     * @return The EndpointId of this Closure Dimension endpoint.
     */
    EndpointId GetEndpointId() const { return mEndpoint; }

private:
    EndpointId mEndpoint = kInvalidEndpointId;
    MatterContext mContext;
    ClosureDimensionDelegate mDelegate;
    ClusterLogic mLogic;
    Interface mInterface;

    /**
     * @brief Updates the current state of the closure dimension endpoint from the target state.
     *
     * This function retrieves the target state and updates the current state accordingly.
     * It ensures that the current state reflects the latest target position, latch status, and speed.
     */
    void UpdateCurrentStateFromTargetState();
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
