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

#include <app/clusters/wifi-network-management-server/WiFiNetworkManagementCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip::app::Clusters {

/// Compatibility wrapper for WiFiNetworkManagementCluster for ZAP-based applications.
///
/// Note that this cluster does not support automatic instantiation of the cluster
/// server even in ZAP-based applications; the application code is responsible for
/// instantiating and registering the cluster manually.
///
/// @code
/// std::optional<WiFiNetworkManagementServer> gServer;
/// void emberAfWiFiNetworkManagementClusterInitCallback(EndpointId endpoint)
/// {
///     SuccessOrDie(gServer.emplace(endpoint).Init());
/// }
/// @endcode
class WiFiNetworkManagementServer : public WiFiNetworkManagementCluster
{
public:
    using WiFiNetworkManagementCluster::WiFiNetworkManagementCluster;

    ~WiFiNetworkManagementServer();

    /// Registers this cluster with the CodegenDataModelProvider.
    CHIP_ERROR Init();

    /// Unregisters this cluster from the CodegenDataModelProvider.
    /// Unregistration happens automatically during destruction if necessary.
    CHIP_ERROR Deinit(ClusterShutdownType clusterShutdownType = ClusterShutdownType::kClusterShutdown);

private:
    ServerClusterRegistration mRegistration{ *this };
};

} // namespace chip::app::Clusters
