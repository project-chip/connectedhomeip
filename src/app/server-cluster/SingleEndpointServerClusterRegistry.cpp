/*
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
#include <app/server-cluster/SingleEndpointServerClusterRegistry.h>

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <optional>

namespace chip {
namespace app {

CHIP_ERROR SingleEndpointServerClusterRegistry::Register(ServerClusterRegistration & entry)
{
    Span<const ConcreteClusterPath> paths = entry.serverClusterInterface->GetPaths();
    VerifyOrReturnError(!paths.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    // Codegen registry requirements (so that we can support endpoint unregistration): every
    // path must belong to the same endpoint id.
    for (const ConcreteClusterPath & path : paths)
    {
        VerifyOrReturnError(path.mEndpointId == paths[0].mEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    }

    return ServerClusterInterfaceRegistry::Register(entry);
}

SingleEndpointServerClusterRegistry::ClustersList SingleEndpointServerClusterRegistry::ClustersOnEndpoint(EndpointId endpointId)
{
    return { mRegistrations, endpointId };
}

void SingleEndpointServerClusterRegistry::UnregisterAllFromEndpoint(EndpointId endpointId)
{
    ServerClusterRegistration * prev    = nullptr;
    ServerClusterRegistration * current = mRegistrations;
    while (current != nullptr)
    {
        // Requirements for Paths:
        //   - GetPaths() MUST be non-empty
        //   - GetPaths() MUST belong to the same endpoint
        // Loop below relies on that: if the endpoint matches, it can be removed
        auto paths = current->serverClusterInterface->GetPaths();
        if (paths.empty() || paths.front().mEndpointId == endpointId)
        {
            if (mCachedInterface == current->serverClusterInterface)
            {
                mCachedInterface = nullptr;
            }
            if (prev == nullptr)
            {
                mRegistrations = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            ServerClusterRegistration * actual_next = current->next;

            current->next = nullptr; // Make sure current does not look like part of a list.
            if (mContext.has_value())
            {
                current->serverClusterInterface->Shutdown();
            }

            current = actual_next;
        }
        else
        {
            prev    = current;
            current = current->next;
        }
    }
}

} // namespace app
} // namespace chip
