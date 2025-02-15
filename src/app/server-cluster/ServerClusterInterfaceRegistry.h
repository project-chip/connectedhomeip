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

/// Maintains mapping clusters registered on specific endpoints.
class ServerClusterInterfaceRegistry
{
public:
    ~ServerClusterInterfaceRegistry();

    /// Associate a specific interface for the given endpoint.
    ///
    /// There MUST be a single cluster registration for any given endpoint
    /// and Register will error out if an existing cluster interface `cluster` already
    /// exists on the given endpoint for the given clusterid.
    ///
    /// Registrations need a valid endpointid and cluster MUST return a valid clusterid.
    [[nodiscard]] CHIP_ERROR Register(EndpointId endpointId, ServerClusterInterface * cluster);

    /// Remove an existing registration for a given endpoint/cluster path.
    ///
    /// Returns the previous registration if any exists (or nullptr if nothing
    /// to register)
    ServerClusterInterface * Unregister(const ConcreteClusterPath & path);

    /// Return the given registered interface or nullptr if one does not exist
    ServerClusterInterface * Get(const ConcreteClusterPath & path);

    /// Unregister all registrations for the given endpoint.
    void UnregisterAllFromEndpoint(EndpointId endpointId);

    /// Access to an application global registry of server cluster interfaces
    /// that are registered within the application.
    static ServerClusterInterfaceRegistry & Instance();

private:
    // Some pre-allocation. Minimal clusters have 2 endpoints: root/0 and
    // some other endpoint for the actual device.
    static constexpr size_t kPreallocatedEndpointClusters = 2;

    /// tracks clusters registered to a particular endpoint
    struct EndpointClusters
    {
        // The endpoint ID for this registration. kInvalidEndpointId means
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
    std::array<EndpointClusters, kPreallocatedEndpointClusters> mPreallocateEndpoints;

    // Dynamic allocated endpoint cluters, once static allocation is used up
    DynamicEndpointClusters * mDynamicEndpoints = nullptr;

    // a one-element cache to speed up finding per endpoint
    EndpointClusters * mEndpointClustersCache = nullptr;

    // a one-element cache to speed up finding clusters.
    // The endpoint id specifies which endpoint the cache belongs to.
    ClusterId mCachedClusterEndpointId        = kInvalidEndpointId;
    ServerClusterInterface * mCachedInterface = nullptr;

    /// returns nullptr if not found
    EndpointClusters * FindClusters(EndpointId endpointId);

    /// Get a new usable (either static or dynamic) endpoint cluster
    CHIP_ERROR AllocateNewEndpointClusters(EndpointId endpointId, EndpointClusters *& dest);

    // Mark every element in the given list as having no `Next`
    void DestroySingleLinkedList(ServerClusterInterface * clusters);
};

} // namespace app
} // namespace chip
