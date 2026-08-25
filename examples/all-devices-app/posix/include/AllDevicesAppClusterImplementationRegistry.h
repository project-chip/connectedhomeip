/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <map>
#include <string>
#include <vector>

namespace chip {
namespace app {
class ServerClusterInterface;
} // namespace app
} // namespace chip

template <typename ClusterType>
const char * GetClusterTypeName();

/**
 * This class is responsible for holding pointers to all cluster instances
 * registered by the application, so that particular concrete implementations
 * can be recovered for a given endpoint for debug interfacing. This replaces
 * what a specific app would have locally stored in some module during init
 * by a generic mechanism that scales with dynamic endpoint additions.
 *
 * The implementation is type-safe and relies on the `GetClusterTypeName` template
 * to identify a cluster implementation for registration by type.
 *
 * Since the SDK is compiled without RTTI (`-fno-rtti`), the registry cannot dynamically
 * resolve inheritance hierarchies or query runtime types.
 *
 * This means you must register the exact type you intend to retrieve:
 * 1. If you want to retrieve the cluster using a base class interface (e.g., `OnOffCluster`),
 *    you must register it as the base class type (by explicitly specifying the template parameter).
 * 2. If you want to retrieve the cluster using its derived class type (e.g., `OnOffLightingCluster`),
 *    you must register it as that derived class type.
 *
 * Example Usage:
 * @code
 *   // 1. Register a cluster instance during startup:
 *   registry.RegisterClusterInstance<chip::app::Clusters::OnOffCluster>(&lightDevice->OnOffCluster());
 *
 *   // 2. Retrieve the cluster instance by endpoint ID later:
 *   auto * cluster = registry.GetClusterByEndpoint<chip::app::Clusters::OnOffCluster>(endpointId);
 *   if (cluster != nullptr)
 *   {
 *       cluster->SetOnOff(true);
 *   }
 * @endcode
 */
class AllDevicesAppClusterImplementationRegistry
{
public:
    AllDevicesAppClusterImplementationRegistry() = default;

    template <typename ClusterType>
    void RegisterClusterInstance(ClusterType * instance)
    {
        const char * name = GetClusterTypeName<ClusterType>();
        mClusters[name].push_back(instance);
    }

    template <typename ClusterType>
    ClusterType * GetClusterByEndpoint(chip::EndpointId endpoint)
    {
        const char * name = GetClusterTypeName<ClusterType>();
        auto * cluster    = GetClusterInterfaceByEndpointAndType(name, endpoint);
        return static_cast<ClusterType *>(cluster);
    }

private:
    chip::app::ServerClusterInterface * GetClusterInterfaceByEndpointAndType(const char * typeName, chip::EndpointId endpoint);

    std::map<std::string, std::vector<chip::app::ServerClusterInterface *>> mClusters;
};
