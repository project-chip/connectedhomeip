/*
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

#include <app/clusters/wifi-network-management-server/CodegenIntegration.h>

#include <app/static-cluster-config/WiFiNetworkManagement.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
using Integration = DefaultClusterIntegration<WiFiNetworkManagementCluster,
                                              WiFiNetworkManagement::StaticApplicationConfig::kFixedClusterConfig.size()>;

Integration::Storage gStorage;
} // namespace

namespace chip::app::Clusters::WiFiNetworkManagement {
WiFiNetworkManagementCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    return Integration::FindClusterOnEndpoint(gStorage, endpointId);
}
} // namespace chip::app::Clusters::WiFiNetworkManagement

void MatterWiFiNetworkManagementClusterInitCallback(EndpointId endpointId)
{
    Integration::RegisterServer(gStorage, endpointId);
}

void MatterWiFiNetworkManagementClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType clusterShutdownType)
{
    Integration::UnregisterServer(gStorage, endpointId, clusterShutdownType);
}

void MatterWiFiNetworkManagementPluginServerInitCallback() {}
void MatterWiFiNetworkManagementPluginServerShutdownCallback() {}
