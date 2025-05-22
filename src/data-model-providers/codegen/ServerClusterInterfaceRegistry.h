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

#include <cstdint>
#include <new>

namespace chip {
namespace app {

/// Represents an entry in the server cluster interface registry for
/// a specific interface.
///
/// In practice this is a single-linked list element.
struct ServerClusterRegistration
{
    // A single-linked list of clusters registered for the given `endpointId`
    ServerClusterInterface * const serverClusterInterface;
    ServerClusterRegistration * next;

    constexpr ServerClusterRegistration(ServerClusterInterface & interface, ServerClusterRegistration * next_item = nullptr) :
        serverClusterInterface(&interface), next(next_item)
    {}
    ServerClusterRegistration(ServerClusterRegistration && other) = default;

    // we generally do not want to allow copies as those may have different "next" entries.
    ServerClusterRegistration(const ServerClusterRegistration & other)             = delete;
    ServerClusterRegistration & operator=(const ServerClusterRegistration & other) = delete;
};

/// It is very typical to join together a registration and a Server
/// This templates makes this registration somewhat easier/standardized.
template <typename SERVER_CLUSTER>
struct RegisteredServerCluster
{
    template <typename... Args>
    RegisteredServerCluster(Args &&... args) : cluster(std::forward<Args>(args)...), registration(cluster)
    {}

    [[nodiscard]] constexpr ServerClusterRegistration & Registration() { return registration; }
    [[nodiscard]] constexpr SERVER_CLUSTER & Cluster() { return cluster; }

private:
    SERVER_CLUSTER cluster;
    ServerClusterRegistration registration;
};

/// Lazy-construction of a RegisteredServerCluster to allow at-runtime lifetime management
///
/// If using this class, manamement of Create/Destroy MUST be done correctly.
template <typename SERVER_CLUSTER>
struct LazyRegisteredServerCluster
{
public:
    constexpr LazyRegisteredServerCluster() = default;
    ~LazyRegisteredServerCluster()
    {
        if (IsConstructed())
        {
            Destroy();
        }
    }

    void Destroy()
    {
        VerifyOrDie(IsConstructed());
        Registration().~ServerClusterRegistration();
        memset(mRegistration, 0, sizeof(mRegistration));

        Cluster().~SERVER_CLUSTER();
        memset(mCluster, 0, sizeof(mCluster));
    }

    template <typename... Args>
    void Create(Args &&... args)
    {
        VerifyOrDie(!IsConstructed());

        new (mCluster) SERVER_CLUSTER(std::forward<Args>(args)...);
        new (mRegistration) ServerClusterRegistration(Cluster());
    }

    [[nodiscard]] constexpr bool IsConstructed() const
    {
        // mRegistration is supposed to containt a serverClusterInterface that is NOT null
        // so we check for non-zero content. This relies that nullptr is 0
        return Registration().serverClusterInterface != nullptr;
    }

    [[nodiscard]] constexpr ServerClusterRegistration & Registration()
    {
        return *std::launder(reinterpret_cast<ServerClusterRegistration *>(mRegistration));
    }

    [[nodiscard]] constexpr const ServerClusterRegistration & Registration() const
    {
        return *std::launder(reinterpret_cast<const ServerClusterRegistration *>(mRegistration));
    }

    [[nodiscard]] constexpr SERVER_CLUSTER & Cluster() { return *std::launder(reinterpret_cast<SERVER_CLUSTER *>(mCluster)); }

    [[nodiscard]] constexpr const SERVER_CLUSTER & Cluster() const
    {
        return *std::launder(reinterpret_cast<const SERVER_CLUSTER *>(mCluster));
    }

private:
    alignas(SERVER_CLUSTER) uint8_t mCluster[sizeof(SERVER_CLUSTER)]                            = { 0 };
    alignas(ServerClusterRegistration) uint8_t mRegistration[sizeof(ServerClusterRegistration)] = { 0 };
};

/// Allows registering and retrieving ServerClusterInterface instances for specific cluster paths.
class ServerClusterInterfaceRegistry
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

    ~ServerClusterInterfaceRegistry();

    /// Add the given entry to the registry.
    /// NOTE the requirement of entries to be part of the same endpoint.
    ///
    /// Requirements:
    ///   - entry MUST NOT be part of any other registration
    ///   - paths MUST be part of the same endpoint (requirement for codegen server cluster interface implementations)
    ///
    ///   - LIFETIME of entry must outlive the Registry (or entry must be unregistered)
    ///
    /// There can be only a single registration for a given `endpointId/clusterId` path.
    [[nodiscard]] CHIP_ERROR Register(ServerClusterRegistration & entry);

    /// Remove an existing registration
    ///
    /// Will return CHIP_ERROR_NOT_FOUND if the given registration is not found.
    CHIP_ERROR Unregister(ServerClusterInterface *);

    /// Return the interface registered for the given cluster path or nullptr if one does not exist
    ServerClusterInterface * Get(const ConcreteClusterPath & path);

    /// Provides a list of clusters that are registered for the given endpoint.
    ///
    /// ClustersList points inside the internal registrations of the registry, so
    /// the list is only valid as long as the registry is not modified.
    ClustersList ClustersOnEndpoint(EndpointId endpointId);

    /// Unregister all registrations for the given endpoint.
    void UnregisterAllFromEndpoint(EndpointId endpointId);

    // Set up the underlying context for all clusters that are managed by this registry.
    //
    // The values within context will be copied and used.
    CHIP_ERROR SetContext(ServerClusterContext && context);

    // Invalidates current context.
    void ClearContext();

private:
    ServerClusterRegistration * mRegistrations = nullptr;

    // A one-element cache to speed up finding a cluster within an endpoint.
    // The endpointId specifies which endpoint the cache belongs to.
    ServerClusterInterface * mCachedInterface = nullptr;

    // Managing context for this registry
    std::optional<ServerClusterContext> mContext;
};

} // namespace app
} // namespace chip
