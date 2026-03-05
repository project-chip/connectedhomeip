/*
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/clusters/actions-server/ActionsCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

// Legacy wrapper for backwards compatibility with existing applications.
class ActionsServer
{
public:
    /**
     * Creates an actions server instance. This is a backwards compatibility wrapper around ActionsCluster.
     * @param endpointId The endpoint on which this cluster exists.
     * @param delegate A reference to the delegate to be used by this server.
     */
    ActionsServer(EndpointId endpointId, Actions::Delegate & delegate);
    ~ActionsServer();

    /**
     * Register the actions cluster instance with the codegen data model provider.
     * @return Returns an error if registration fails.
     */
    CHIP_ERROR Init();

    /**
     * Unregister the actions cluster instance from the data model provider.
     */
    void Shutdown();

    // Legacy Notifiers - these proxy directly to the new cluster
    void ActionListModified(EndpointId aEndpoint);
    void EndpointListModified(EndpointId aEndpoint);

    /**
     * @return The endpoint ID.
     */
    EndpointId GetEndpointId() { return mCluster.Cluster().GetPaths()[0].mEndpointId; }

    // The Code Driven ActionsCluster instance
    chip::app::RegisteredServerCluster<Actions::ActionsCluster> mCluster;
};

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
