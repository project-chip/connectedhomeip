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

#include <app/AppConfig.h>
#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstdint>
#include <new>
#include <optional>

namespace chip {
namespace app {

/// Represents an entry in the server cluster interface registry for
/// a specific interface.
///
/// In practice this is a single-linked list element.
struct ServerClusterRegistration
{
    ServerClusterInterface * const serverClusterInterface;

    /// Managed by the registry. Null while the registration is not part of a registry.
    /// The list is terminated by `EndOfList()` rather than null, so a registration can tell
    /// whether it is registered without accessing the registry.
    ServerClusterRegistration * next;

    constexpr ServerClusterRegistration(ServerClusterInterface & interface, ServerClusterRegistration * next_item = nullptr) :
        serverClusterInterface(&interface), next(next_item)
    {}
    ServerClusterRegistration(ServerClusterRegistration && other) = default;

    // we generally do not want to allow copies as those may have different "next" entries.
    ServerClusterRegistration(const ServerClusterRegistration & other)             = delete;
    ServerClusterRegistration & operator=(const ServerClusterRegistration & other) = delete;

    /// Returns true if this registration is part of a registry.
    ///
    /// A registration must be unregistered before it is destroyed, to avoid leaving the registry with
    /// a dangling pointer. When the registration may outlive the registry (e.g. due to the unspecified
    /// order of static destruction), the call to `Unregister()` must be guarded, because the registry
    /// may already be gone. `IsRegistered()` provides that guard without accessing the registry: a
    /// registry clears all its entries when it is destroyed, so this returns false afterwards.
    bool IsRegistered() const { return next != nullptr; }

    /// The value of `next` for the last registration in a registry's list.
    /// Its `serverClusterInterface` is null.
    static constexpr const ServerClusterRegistration * EndOfList() { return &kEndOfList; }

private:
    // Used only for the end-of-list sentinel.
    constexpr ServerClusterRegistration() : serverClusterInterface(nullptr), next(nullptr) {}

    static const ServerClusterRegistration kEndOfList;
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
    [[nodiscard]] constexpr const ServerClusterRegistration & Registration() const { return registration; }

    [[nodiscard]] constexpr SERVER_CLUSTER & Cluster() { return cluster; }
    [[nodiscard]] constexpr const SERVER_CLUSTER & Cluster() const { return cluster; }

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
        if (!IsConstructed())
        {
            // Should not happen — Init/Shutdown are paired at the CodegenDataModelProvider level.
#if CHIP_CONFIG_ENABLE_SERVER_RESTART_SUPPORT
            ChipLogError(AppServer, "Destroy() called on already-destroyed cluster — this should not happen");
#else
            chipDie();
#endif
            return;
        }
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
    ~ServerClusterInterfaceRegistry();

    /// Add the given entry to the registry.
    ///
    /// Requirements:
    ///   - entry MUST NOT be part of any other registration
    ///   - LIFETIME of entry must outlive the Registry (or entry must be unregistered)
    ///
    /// There can be only a single registration for a given `endpointId/clusterId` path.
    [[nodiscard]] CHIP_ERROR Register(ServerClusterRegistration & entry);

    /// Remove an existing registration
    ///
    /// Will return CHIP_ERROR_NOT_FOUND if the given registration is not found.
    CHIP_ERROR Unregister(ServerClusterInterface *,
                          ClusterShutdownType clusterShutdownType = ClusterShutdownType::kClusterShutdown);

    /// Return the interface registered for the given cluster path or nullptr if one does not exist
    ServerClusterInterface * Get(const ConcreteClusterPath & path);

    // Set up the underlying context for all clusters that are managed by this registry.
    //
    // The values within context will be moved and used as-is.
    //
    // Returns:
    //   - CHIP_NO_ERROR on success
    //   - CHIP_ERROR_HAD_FAILURES if some cluster `Startup` calls had errors (Startup
    //     will be called for all clusters).
    CHIP_ERROR SetContext(ServerClusterContext && context);

    // Invalidates current context.
    void ClearContext();

    // Represents an iterable list of all clusters registered in this registry.
    // The list is only valid as long as the registry is not modified.
    // The list is not guaranteed to be in any particular order.
    class ServerClusterInstances
    {
    public:
        class Iterator
        {
        public:
            Iterator(ServerClusterRegistration * registration) : mRegistration(registration != nullptr ? registration : EndOfList())
            {}

            Iterator & operator++()
            {
                if (mRegistration != EndOfList())
                {
                    mRegistration = mRegistration->next;
                }
                return *this;
            }
            bool operator==(const Iterator & other) const { return mRegistration == other.mRegistration; }
            bool operator!=(const Iterator & other) const { return mRegistration != other.mRegistration; }
            // Returns nullptr for end(), because the end-of-list sentinel has a null serverClusterInterface.
            ServerClusterInterface * operator*() { return mRegistration->serverClusterInterface; }

        private:
            ServerClusterRegistration * mRegistration;
        };

        constexpr ServerClusterInstances(ServerClusterRegistration * start) : mStart(start) {}
        Iterator begin() { return { mStart }; }
        Iterator end() { return { EndOfList() }; }

    private:
        ServerClusterRegistration * mStart;
    };

    ServerClusterInstances AllServerClusterInstances();

protected:
    static constexpr ServerClusterRegistration * EndOfList()
    {
        // The list stores the sentinel in non-const `next` pointers, but never writes through it.
        return const_cast<ServerClusterRegistration *>(ServerClusterRegistration::EndOfList());
    }

    ServerClusterRegistration * mRegistrations = EndOfList();

    // A one-element cache to speed up finding a cluster within an endpoint.
    // The endpointId specifies which endpoint the cache belongs to.
    ServerClusterInterface * mCachedInterface = nullptr;

    // Managing context for this registry
    std::optional<ServerClusterContext> mContext;
};

} // namespace app
} // namespace chip
