/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <array>
#include <optional>
#include <type_traits>

namespace chip {
namespace app {

/**
 * Base class for server-side cluster implementations.
 */
class ServerCluster : protected AttributeAccessInterface, protected CommandHandlerInterface
{
public:
    ServerCluster(EndpointId aEndpointId, ClusterId aClusterId);
    virtual ~ServerCluster() { Shutdown(); }

    EndpointId GetEndpointId() { return mEndpointId; }

    virtual CHIP_ERROR Init();
    virtual void Shutdown();

protected:
    void InvokeCommand(HandlerContext & handlerContext) override {};

private:
    EndpointId mEndpointId;
};

/**
 * An adapter class that statically allocates memory for a fixed number of cluster instances
 * and manages their lifecycle.
 *
 * @see DECLARE_CLUSTER_SERVER()
 */
template <class S, size_t N>
class EmberClusterAdapter;

class EmberClusterAdapterImpl
{
    template <class S, size_t N>
    friend class EmberClusterAdapter;

    static void Init(ServerCluster & cluster);
    static void InitFailureInstancesExceeded();
};

template <class S, size_t N>
class EmberClusterAdapter final
{
    static_assert(std::is_base_of_v<ServerCluster, S>);

public:
    constexpr EmberClusterAdapter() = default;

    void Init(EndpointId endpoint)
    {
        for (auto & instance : mInstances)
        {
            if (!instance)
            {
                EmberClusterAdapterImpl::Init(instance.emplace(endpoint));
                return;
            }
        }
        EmberClusterAdapterImpl::InitFailureInstancesExceeded();
    }

    void Shutdown(EndpointId endpoint)
    {
        for (auto & instance : mInstances)
        {
            if (instance && instance->GetEndpointId() == endpoint)
            {
                instance.reset(); // destructor calls Shutdown
                return;
            }
        }
        // ignore silently
    }

private:
    std::array<std::optional<S>, N> mInstances;
};

} // namespace app
} // namespace chip
