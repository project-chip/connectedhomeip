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
#pragma once

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

/// Allows registering and retrieving ServerClusterInterface instances for specific cluster paths.
class ServerClusterInterfaceRegistry
{
public:
    ~ServerClusterInterfaceRegistry();

    /// Associate a specific interface with the given endpoint.
    ///
    /// A `ServerClusterInterface` may only be associated on a single endpointId and
    /// there can be only a single registration for a given `endpointId/clusterId` path.
    ///
    /// This means Register WILL RETURN AN ERROR if:
    ///   - A registration on the given `endpointId/cluster->GetClusterID()` already exists
    ///   - The given `cluster` is already registered for some enpoint id
    ///
    /// Registrations need a valid endpointId and cluster MUST return a valid cluster id.
    [[nodiscard]] CHIP_ERROR Register(EndpointId endpointId, ServerClusterInterface * cluster);

    /// Remove an existing registration for a given endpoint/cluster path.
    ///
    /// Returns the previous registration if any exists (or nullptr if nothing
    /// to unregister)
    ServerClusterInterface * Unregister(const ConcreteClusterPath & path);

    /// Return the interface registered for the given cluster path or nullptr if one does not exist
    ServerClusterInterface * Get(const ConcreteClusterPath & path);

    /// Unregister all registrations for the given endpoint.
    void UnregisterAllFromEndpoint(EndpointId endpointId);

    /// Access to an application global registry of server cluster interfaces
    /// that are registered within the application.
    static ServerClusterInterfaceRegistry & Instance();

private:
    // Some pre-allocation. Minimal devices have 2 endpoints: root/0 and
    // some other endpoint for the actual device.
    static constexpr size_t kPreallocatedEndpointClusters = 2;

    /// tracks clusters registered to a particular endpoint
    struct EndpointClusters
    {
        // The endpointId for this registration. kInvalidEndpointId means
        // not allocated/used
        EndpointId endpointId = kInvalidEndpointId;

        // A single-linked list of clusters registered for the given `endpointId`
        ServerClusterInterface * firstCluster = nullptr;
    };

    struct DynamicEndpointClusters : public EndpointClusters
    {
        /// A single-linked list of endpoint clusters that is dynamically
        /// allocated.
        DynamicEndpointClusters * next;
    };

    // Serves as a pool of pre-allocated clusters to avoid HEAP
    std::array<EndpointClusters, kPreallocatedEndpointClusters> mPreallocatedEndpoints;

    // Dynamic allocated endpoint cluters, once static allocation is used up
    DynamicEndpointClusters * mDynamicEndpoints = nullptr;

    // a one-element cache to speed up finding the cluster list for an endpoint.
    EndpointClusters * mEndpointClustersCache = nullptr;

    // a one-element cache to speed up finding a cluster within an endpoint.
    // The endpointId specifies which endpoint the cache belongs to.
    ClusterId mCachedClusterEndpointId        = kInvalidEndpointId;
    ServerClusterInterface * mCachedInterface = nullptr;

    /// returns nullptr if not found
    EndpointClusters * FindClusters(EndpointId endpointId);

    /// Get a new usable (either static or dynamic) endpoint cluster
    CHIP_ERROR AllocateNewEndpointClusters(EndpointId endpointId, EndpointClusters *& dest);
};

} // namespace app
} // namespace chip
