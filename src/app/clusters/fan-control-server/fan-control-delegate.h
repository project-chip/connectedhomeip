/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandResponseHelper.h>
#include <app/util/af.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

/** @brief
 *    Defines methods for implementing application-specific logic for the FanControl Cluster.
 */
class Delegate
{
public:
    /**
     * @brief
     *   This method handles the step command. This will happen as fast as possible.
     *
     *   @param[in]  direction            direction in which to step
     *   @param[in]  wrap                 whether the step command wraps or not
     *   @param[in]  wrap                 whether the step command wraps or not
     *
     *   @return Success On success.
     *   @return Other Value indicating it failed to execute the command.
     */
    virtual Protocols::InteractionModel::Status HandleStep(StepDirectionEnum direction, bool wrap, bool off) = 0;

    Delegate(EndpointId endpoint) : mEndpoint(endpoint) {}

    virtual ~Delegate() = default;

protected:
    EndpointId mEndpoint = 0;
};

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
