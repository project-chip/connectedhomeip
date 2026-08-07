/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/alarm-base-server/CodegenIntegrationHelpers.h>
#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters::AlarmBase {

uint32_t GetClusterRevision(ClusterId clusterId)
{
    for (const auto & entry : kAliasedClusters)
    {
        if (entry.id == clusterId)
        {
            return entry.revision;
        }
    }
    return 0;
}

bool EndpointHasCommand(EndpointId endpointId, ClusterId clusterId, CommandId commandId)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(endpointId, clusterId);
    VerifyOrReturnValue(cluster != nullptr, false);
    VerifyOrReturnValue(cluster->acceptedCommandList != nullptr, false);

    for (const CommandId * cmd = cluster->acceptedCommandList; *cmd != kInvalidCommandId; cmd++)
    {
        if (*cmd == commandId)
        {
            return true;
        }
    }
    return false;
}

} // namespace chip::app::Clusters::AlarmBase
