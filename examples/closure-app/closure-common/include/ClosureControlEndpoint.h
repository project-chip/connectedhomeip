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

#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
#include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <app/clusters/closure-control-server/closure-control-cluster-matter-context.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
 
#include <lib/core/CHIPError.h>
#include <unordered_set>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <app-common/zap-generated/cluster-objects.h>
 
namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/**
 * @class ClosureControlDelegate
 * @brief A delegate class that handles Closure Control commands at the application level.
 * 
 * This class is responsible for processing Closure Control commands such as Stop, MoveTo, and Calibrate
 * according to specific business logic. It is designed to be used as a delegate for the Closure Control cluster.
 * 
 * @note This implementation is a "PrintOnly" delegate, which may primarily log or print command handling actions.
 * 
 * @param mEndpoint The endpoint ID associated with this delegate.
 */
class ClosureControlDelegate : public DelegateBase
{
public:  
    ClosureControlDelegate(EndpointId endpoint) : mEndpoint(endpoint) {}

    virtual ~ClosureControlDelegate() = default;
    
    // Override for the DelegateBase Virtual functions
    
    Protocols::InteractionModel::Status HandleStopCommand() override;
    Protocols::InteractionModel::Status HandleMoveToCommand(const Optional<TargetPositionEnum> & tag, const Optional<bool> & latch,
                                                            const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    Protocols::InteractionModel::Status HandleCalibrateCommand() override;
    
    CHIP_ERROR GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError) override;
    
    bool IsManualLatchingNeeded() override;
    bool IsReadyToMove() override;
    ElapsedS GetCalibrationCountdownTime() override;
    ElapsedS GetMovingCountdownTime() override;
    ElapsedS GetWaitingForMotionCountdownTime() override;
    
    // Delegate specific functions and variables

    void SetLogic(ClusterLogic * logic) { mLogic = logic; }

    ClusterLogic * GetLogic() const { return mLogic; }

    DataModel::Nullable<ElapsedS> GetRemainingTime();
    
    uint32_t mMovingTime                          = 0;
    uint32_t mCalibratingTime                     = 0;
    uint32_t mWaitingTime                         = 0;
    DataModel::Nullable<ElapsedS> mCountDownTime  = DataModel::NullNullable;

    /**
     * @brief Handles the countdown timer expiration event
     */
    void HandleCountdownTimeExpired();
    
    /**
     * @brief Checks if closure has completed prestage or not.
     * 
     * @return true, if prestage is completed
     *         false, if prestage is not completed
     */
    bool IsPreStageComplete();

    /**
     * @brief Handles the motion request of Closure
     * @param [in] latchNeeded - true if latch is needed
     * @param [in] NewTarget - true if target is changed
     * @return Protocols::InteractionModel::Status - success or failure
     */
    Protocols::InteractionModel::Status HandleMotion();

private:
    EndpointId mEndpoint = kInvalidEndpointId;
    ClusterLogic * mLogic;
    
    
    /**
     * @brief Function to map TargetPositionEnum to Positioning Enum
     * 
     * @param [in] targetPosition, TargetPositionEnum which need to mapped
     * 
     * @return PositioningEnum
     */
    PositioningEnum GetStatePositionFromTarget(TargetPositionEnum targetPosition);
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
        mEndpoint(endpoint), mContext(mEndpoint), mDelegate(mEndpoint), mLogic(mDelegate, mContext), mInterface(mEndpoint, mLogic)
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

private:
    EndpointId mEndpoint = kInvalidEndpointId;
    MatterContext mContext; 
    ClosureControlDelegate mDelegate; 
    ClusterLogic mLogic; 
    Interface mInterface; 
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip