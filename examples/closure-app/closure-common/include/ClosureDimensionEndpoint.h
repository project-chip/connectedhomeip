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
    ClosureDimensionDelegate() {}

    // Override for the DelegateBase Virtual functions
    Status HandleSetTarget(const Optional<Percent100ths> & pos, const Optional<bool> & latch,
                           const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                      const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    bool IsManualLatchingNeeded() override { return false; }
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
        mEndpoint(endpoint), mContext(mEndpoint), mDelegate(), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
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
     * @brief Handles the completion of a stop motion action for the closure dimension endpoint.
     *
     * This function is called when a motion action (such as opening or closing) has been stopped.
     * It should update the internal state of the closure dimension to reflect the completion of the stop action.
     * Implement any necessary logic to synchronize the endpoint's state with the actual hardware or system status.
     */
    void OnStopMotionActionComplete();

    /**
     * @brief Handles post-calibration logic for the closure dimension endpoint.
     *
     * This function is called when the calibration action has been stopped.
     * It should update the closure dimension state and perform any necessary
     * cleanup or state transitions required after calibration completes.
     */
    void OnStopCalibrateActionComplete();

    /**
     * @brief Handles the completion of a calibration action for the closure dimension endpoint.
     *
     * This method updates the internal logic state to reflect that calibration has completed.
     * It sets the current state to a fully closed position, marks the calibration as complete,
     * and sets the mode to automatic. The target state is reset to null.
     */
    void OnCalibrateActionComplete();

    /**
     * @brief Handles post-action updates for closure dimension state.
     *
     * This method is called after a "MoveTo" action is completed. It should be used to update
     * the internal state of the closure dimension endpoint to reflect the new position or state
     * resulting from the completed action.
     */
    void OnMoveToActionComplete();

private:
    EndpointId mEndpoint = kInvalidEndpointId;
    MatterContext mContext;
    ClosureDimensionDelegate mDelegate;
    ClusterLogic mLogic;
    Interface mInterface;
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
