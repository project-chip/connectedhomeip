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

#include "NetworkCommissioningCluster.h"

#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

/// Automates integration of a NetworkCommissioningCluster with CodegenDataModelProvider.
///
/// This class provides a very thin wrapper around the NetworkCommissioningCluster and its
/// registration within the CodegenDataModelProvider.
///
class Instance
{
public:
    using WiFiDriver     = DeviceLayer::NetworkCommissioning::WiFiDriver;
    using ThreadDriver   = DeviceLayer::NetworkCommissioning::ThreadDriver;
    using EthernetDriver = DeviceLayer::NetworkCommissioning::EthernetDriver;

    /// Calls Init on the cluster and registers the cluster within the CodegenDataModelProvider Registry
    CHIP_ERROR Init();

    /// Calls Shutdown on the cluster and unregisters the cluster from the CodegenDataModelProvider Registry
    void Shutdown();

    Instance(EndpointId aEndpointId, WiFiDriver * apDelegate) : mCluster(aEndpointId, apDelegate) {}
    Instance(EndpointId aEndpointId, ThreadDriver * apDelegate) : mCluster(aEndpointId, apDelegate) {}
    Instance(EndpointId aEndpointId, EthernetDriver * apDelegate) : mCluster(aEndpointId, apDelegate) {}

private:
    RegisteredServerCluster<NetworkCommissioningCluster> mCluster;
};

// The InstanceAndDriver class encapsulates the creation and management of a transport driver instance (Wi-Fi, Thread, or Ethernet)
// together with a NetworkCommissioningCluster instance.
// It provides a unified interface to initialize, configure, and operate both components,
// ensuring they are properly linked for network commissioning operations. This class simplifies the integration process by handling
// the instantiation and lifecycle of both the transport driver and the cluster as a single unit.
//
// Until all platform driver instances are migrated to use this class, Instance constructors remain public to maintain both
// instantiation methods. Once the transition is complete, the constructors will be moved to the private section.
template <typename TransportDriver>
class InstanceAndDriver : public Instance
{
public:
    InstanceAndDriver(EndpointId aEndpointId) : Instance(aEndpointId, &mDriver) {}

    TransportDriver & GetDriver() { return mDriver; }

private:
    TransportDriver mDriver;
};
} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
