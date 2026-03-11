/**
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/clusters/closure-control-server/ClosureControlClusterObjects.h>
#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <clusters/ClosureControl/Enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/** @brief Defines methods for implementing application-specific logic for the Closure Control Cluster.
 */
class ClosureControlClusterDelegate
{
public:
    ClosureControlClusterDelegate()          = default;
    virtual ~ClosureControlClusterDelegate() = default;

    /**
     * @brief This function handles Stop command implementaion.
     *
     * @return Success when closure succesfully handles stop.
     *         Error when stop fails.
     */
    virtual Protocols::InteractionModel::Status HandleStopCommand() = 0;

    /**
     * @brief This function handles MoveTo command implementaion.
     *
     * @param [in] position Target position to be set
     * @param [in] latch Target Latch to be set
     * @param [in] speed Target speed to be set
     *
     * @return Success when closure succesfully handles motion.
     *         Error when motion fails.
     */
    virtual Protocols::InteractionModel::Status HandleMoveToCommand(const Optional<TargetPositionEnum> & position,
                                                                    const Optional<bool> & latch,
                                                                    const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;

    /**
     * @brief This function handles Calibrate command implementaion.
     *
     * @return Success when closure succesfully handles calibration.
     *         Error when calibration fails.
     */
    virtual Protocols::InteractionModel::Status HandleCalibrateCommand() = 0;

    /**
     * @brief Checks whether the closure can move (as opposed to still needing pre-motion stages to complete).
     *
     * @return true if closure is ready to move
     *         false if closure is not ready to move
     */
    virtual bool IsReadyToMove() = 0;

    /**
     * @brief Get the countdown time required by the closure for calibration.
     *
     * @return Time required for calibration action.
     */
    virtual ElapsedS GetCalibrationCountdownTime() = 0;

    /**
     * @brief Get the countdown time required by the closure for Motion.
     *
     * @return Time required for Motion action.
     */
    virtual ElapsedS GetMovingCountdownTime() = 0;

    /**
     * @brief Get the countdown time required by the closure for pre-stage before start of motion.
     *
     * @return Time required for Motion action.
     */
    virtual ElapsedS GetWaitingForMotionCountdownTime() = 0;

    // These methods are called when the corresponding attributes are updated by the server, whether
    // via InvokeCommand, or direct server APIs. Default implementations return true;
    // override only the callbacks you need.
    virtual bool OnCountdownTimeChanged(DataModel::Nullable<ElapsedS> newCountdownTime) { return true; }
    virtual bool OnMainStateChanged(MainStateEnum newState) { return true; }
    virtual bool OnCurrentErrorListChanged(DataModel::List<const ClosureErrorEnum> newCurrentErrorList) { return true; }
    virtual bool OnOverallCurrentStateChanged(DataModel::Nullable<GenericOverallCurrentState> newState) { return true; }
    virtual bool OnOverallTargetStateChanged(DataModel::Nullable<GenericOverallTargetState> newState) { return true; }
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
