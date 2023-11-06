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
#include <app/util/af-enums.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {


/**
 * Interface to help manage the supported of the Microwave Oven Control Cluster.
 */
class MicrowaveOvenControlServer
{
public:
    virtual ~MicrowaveOvenControlServer() = default;

    MicrowaveOvenControlServer() {}
};


/*
* Set Operational State instance from the application layer.
*/
void SetOPInstance(EndpointId aEndpoint, OperationalState::Instance * aInstance);

/**
 * Get Operational State instance for Microwave Oven Control Cluster to know what the current state is.
 */
OperationalState::Instance * GetOPInstance(EndpointId aEndpoint);




} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip