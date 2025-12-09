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

#include "ServerClusterInterface.h"
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {

/// This class specializes ServerClusterInterfaceRegistry to register ServerClusterInterface references
/// that are limited to a single endpoint. In other words, GetPaths() must return path(s) in the same
/// endpoint.
/// The assumption that every ServerClusterInterface registered is limited to a single endpoint
/// allows us to provide additional helper methods such as `ClustersOnEndpoint()` and `UnregisterAllFromEndpoint()`.
class SingleEndpointServerClusterRegistry : public ServerClusterInterfaceRegistry
{
public:
    /// represents an iterable list of clusters
    class ClustersList
    {
    public:
        class Iterator
        {
        public:
            Iterator(ServerClusterRegistration * interface, EndpointId endpoint) : mEndpointId(endpoint), mRegistration(interface)
            {
                if (mRegistration != nullptr)
                {
                    mSpan = interface->serverClusterInterface->GetPaths();
                }
                AdvanceUntilMatchingEndpoint();
            }

            Iterator & operator++()
            {
                if (!mSpan.empty())
                {
                    mSpan = mSpan.SubSpan(1);
                }
                AdvanceUntilMatchingEndpoint();
                return *this;
            }
            bool operator==(const Iterator & other) const { return mRegistration == other.mRegistration; }
            bool operator!=(const Iterator & other) const { return mRegistration != other.mRegistration; }
            ClusterId operator*() { return mSpan.begin()->mClusterId; }

        private:
            const EndpointId mEndpointId;
            ServerClusterRegistration * mRegistration;
            Span<const ConcreteClusterPath> mSpan;

            void AdvanceUntilMatchingEndpoint()
            {
                while (mRegistration != nullptr)
                {
                    if (mSpan.empty())
                    {
                        mRegistration = mRegistration->next;
                        if (mRegistration != nullptr)
                        {
                            mSpan = mRegistration->serverClusterInterface->GetPaths();
                        }
                        continue;
                    }
                    if (mSpan.begin()->mEndpointId == mEndpointId)
                    {
                        return;
                    }

                    // need to keep searching
                    mSpan = mSpan.SubSpan(1);
                }
            }
        };

        constexpr ClustersList(ServerClusterRegistration * start, EndpointId endpointId) : mEndpointId(endpointId), mStart(start) {}
        Iterator begin() { return { mStart, mEndpointId }; }
        Iterator end() { return { nullptr, mEndpointId }; }

    private:
        const EndpointId mEndpointId;
        ServerClusterRegistration * mStart;
    };

    ~SingleEndpointServerClusterRegistry() = default;

    /// Add the given entry to the registry.
    /// NOTE the requirement of entries to be part of the same endpoint.
    ///
    /// Requirements:
    ///   - entry MUST NOT be part of any other registration
    ///   - paths MUST be part of the same endpoint
    ///
    ///   - LIFETIME of entry must outlive the Registry (or entry must be unregistered)
    ///
    /// There can be only a single registration for a given `endpointId/clusterId` path.
    [[nodiscard]] CHIP_ERROR Register(ServerClusterRegistration & entry);

    /// Provides a list of clusters that are registered for the given endpoint.
    ///
    /// ClustersList points inside the internal registrations of the registry, so
    /// the list is only valid as long as the registry is not modified.
    ClustersList ClustersOnEndpoint(EndpointId endpointId);

    /// Unregister all registrations for the given endpoint.
    void UnregisterAllFromEndpoint(EndpointId endpointId, ClusterShutdownType clusterShutdownType = ClusterShutdownType::kClusterShutdown);
};

} // namespace app
} // namespace chip
