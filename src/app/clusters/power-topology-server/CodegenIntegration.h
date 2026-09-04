/*
 *
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

#include <app/clusters/power-topology-server/DefaultPowerTopologyCircuitNodeStorage.h>
#include <app/clusters/power-topology-server/PowerTopologyCluster.h>
#include <app/clusters/power-topology-server/PowerTopologyDelegate.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

class Instance
{
public:
    /// Uses DefaultCircuitNodeStorage for the ElectricalCircuitNodes attribute, so an application
    /// enabling the ElectricalCircuit feature gets persistence with no extra work. An application
    /// that cannot allocate, or that wants its own persistence, should use the overload below.
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature, FabricTable * aFabricTable = nullptr) :
        mCluster(PowerTopologyCluster::Config{
            .endpointId         = aEndpointId,
            .delegate           = aDelegate,
            .features           = aFeature,
            .fabricTable        = aFabricTable,
            .circuitNodeStorage = &mDefaultCircuitNodeStorage,
        })
    {}

    /// Uses application-provided storage for ElectricalCircuitNodes. `aCircuitNodeStorage` must
    /// outlive this Instance.
    Instance(EndpointId aEndpointId, Delegate & aDelegate, BitMask<Feature> aFeature, CircuitNodeStorage & aCircuitNodeStorage,
             FabricTable * aFabricTable = nullptr) :
        mCluster(PowerTopologyCluster::Config{
            .endpointId         = aEndpointId,
            .delegate           = aDelegate,
            .features           = aFeature,
            .fabricTable        = aFabricTable,
            .circuitNodeStorage = &aCircuitNodeStorage,
        })
    {}

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

private:
    // Only used by the first constructor; harmless (and unallocated) otherwise, since
    // DefaultCircuitNodeStorage allocates nothing until Init() and the cluster only calls Init()
    // when the ElectricalCircuit feature is enabled.
    DefaultCircuitNodeStorage mDefaultCircuitNodeStorage;
    RegisteredServerCluster<PowerTopologyCluster> mCluster;
};

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
