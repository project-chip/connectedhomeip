/**
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

#include <app/clusters/water-heater-management-server/WaterHeaterManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

/**
 * Backwards-compatible wrapper around WaterHeaterManagementCluster that
 * provides the legacy Instance API used by example applications.
 *
 * Example apps that include water-heater-management-server.h can continue
 * to construct an Instance, call Init()/Shutdown(), and query HasFeature()
 * without any changes to their own code.
 */
class Instance
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) : mCluster(aEndpointId, aDelegate, aFeature) {}

    ~Instance();

    // Delete copy constructor and assignment operator.
    Instance(const Instance &)             = delete;
    Instance(Instance &&)                  = delete;
    Instance & operator=(const Instance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const { return mCluster.Cluster().HasFeature(aFeature); }

private:
    RegisteredServerCluster<WaterHeaterManagementCluster> mCluster;
    bool mRegistered = false;
};

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
