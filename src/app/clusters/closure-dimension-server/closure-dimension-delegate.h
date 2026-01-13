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

#include <app-common/zap-generated/cluster-enums.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

/** @brief
 *    Defines methods for implementing application-specific logic for the Closure Dimension Cluster.
 */

class DelegateBase
{
public:
    DelegateBase(){};
    virtual ~DelegateBase() = default;

    /**
     * @brief This function handles SetTarget command implementaion.
     *
     * @param [in] position TargetState position to be set
     * @param [in] latch TargetState Latch to be set
     * @param [in] speed TargetState speed to be set
     *
     * @return Success when succesfully handled.
     *         Error when handle SetTarget fails.
     */
    virtual Protocols::InteractionModel::Status HandleSetTarget(const Optional<Percent100ths> & position,
                                                                const Optional<bool> & latch,
                                                                const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;

    /**
     * @brief This function handles Step command implementaion.
     *
     * @param [in] direction step direction
     * @param [in] numberOfSteps total number of steps
     * @param [in] speed speed of each step
     *
     * @return Success when successfully handled.
     *         Error when handle Step fails.
     */
    virtual Protocols::InteractionModel::Status HandleStep(const StepDirectionEnum & direction, const uint16_t & numberOfSteps,
                                                           const Optional<Globals::ThreeLevelAutoEnum> & speed) = 0;
};

} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
