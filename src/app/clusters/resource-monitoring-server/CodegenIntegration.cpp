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
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-proxy.h>
#include <app/static-cluster-config/HepaFilterMonitoring.h>
#include <app/static-cluster-config/ActivatedCarbonFilterMonitoring.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/ClusterIntegration.h>


#include <array>
#include <cstdint>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Protocols::InteractionModel;

namespace {

/// Choosing HepaFilterMonitoring as the fixed cluster for this example.    
static constexpr size_t kResourceMonitoringFixedClusterCount =
    HepaFilterMonitoring::StaticApplicationConfig::kFixedClusterConfig.size();
static constexpr size_t kResourceMonitoringMaxClusterCount = kResourceMonitoringFixedClusterCount + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

LazyRegisteredServerCluster<ResourceMonitoring::ResourceMonitoringCluster> gServers[kResourceMonitoringMaxClusterCount];


template <ClusterId CLUSTER_ID>
class IntegrationDelegate : public CodegenClusterIntegration::Delegate
{
    static_assert(CLUSTER_ID == HepaFilterMonitoring::Id || CLUSTER_ID == ActivatedCarbonFilterMonitoring::Id,
                  "IntegrationDelegate can only be instantiated for HepaFilterMonitoring or ActivatedCarbonFilterMonitoring");
public:
    ServerClusterRegistration & CreateRegistration(EndpointId endpointId, unsigned emberEndpointIndex,
                                                   uint32_t optionalAttributeBits, uint32_t featureMap) override
    {
        // Create OptionalAttributeSet from optionalAttributeBits
        ResourceMonitoring::ResourceMonitoringCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

        // Create the cluster with all required parameters
        gServers[emberEndpointIndex].Create(
            endpointId,
            CLUSTER_ID,
            BitFlags<ResourceMonitoring::Feature>(featureMap),
            optionalAttributeSet,
            chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown,
            true // ResetCondition command supported
        );

        return gServers[emberEndpointIndex].Registration();
    }

    ServerClusterInterface & FindRegistration(unsigned emberEndpointIndex) override
    {
        return gServers[emberEndpointIndex].Cluster();
    }
    void ReleaseRegistration(unsigned emberEndpointIndex) override { gServers[emberEndpointIndex].Destroy(); }
};



} // namespace

//
// HEPA Filter Monitoring Cluster
//

void emberAfHepaFilterMonitoringClusterServerInitCallback(EndpointId endpointId)
{

    IntegrationDelegate<HepaFilterMonitoring::Id> integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = HepaFilterMonitoring::Id,
            .fixedClusterServerEndpointCount = kResourceMonitoringFixedClusterCount,
            .maxEndpointCount                = kResourceMonitoringMaxClusterCount,
            .fetchFeatureMap                 = true,
            .fetchOptionalAttributes         = true,
        },
        integrationDelegate);
}

void emberAfHepaFilterMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate<HepaFilterMonitoring::Id> integrationDelegate;

    CodegenClusterIntegration::UnregisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = HepaFilterMonitoring::Id,
            .fixedClusterServerEndpointCount = kResourceMonitoringFixedClusterCount,
            .maxEndpointCount                = kResourceMonitoringMaxClusterCount,
        },
        integrationDelegate);
}

//
// Activated Carbon Filter Monitoring Cluster
//
     
void emberAfActivatedCarbonFilterMonitoringClusterServerInitCallback(EndpointId endpointId )
{
    IntegrationDelegate<ActivatedCarbonFilterMonitoring::Id> integrationDelegate;

    CodegenClusterIntegration::RegisterServer(
        {
            .endpointId                      = endpointId,
            .clusterId                       = ActivatedCarbonFilterMonitoring::Id,
            .fixedClusterServerEndpointCount = kResourceMonitoringFixedClusterCount,
            .maxEndpointCount                = kResourceMonitoringMaxClusterCount,
            .fetchFeatureMap                 = true,
            .fetchOptionalAttributes         = true,
        },
        integrationDelegate);
}

void emberAfActivatedCarbonMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    IntegrationDelegate<ActivatedCarbonFilterMonitoring::Id> integrationDelegate;
    
    CodegenClusterIntegration::UnregisterServer(
    {
        .endpointId                      = endpointId,
        .clusterId                       = HepaFilterMonitoring::Id,
        .fixedClusterServerEndpointCount = kResourceMonitoringFixedClusterCount,
        .maxEndpointCount                = kResourceMonitoringMaxClusterCount,
    },
    integrationDelegate);
}

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

void SetDelegate(EndpointId endpointId, ClusterId clusterId, ResourceMonitoringDelegate * delegate)
{
    // uint16_t arrayIndex = 0;
    // if (!findEndpointWithLog(endpointId, clusterId, arrayIndex))
    // {
    //     return;
    // }

    // if (clusterId == HepaFilterMonitoring::Id) 
    // {
    //     hepa_gServers[arrayIndex].Cluster().SetDelegate(delegate);
    //     return;
    // } 
    // else if (clusterId == ActivatedCarbonFilterMonitoring::Id) 
    // {
    //     activatedCarbon_gServers[arrayIndex].Cluster().SetDelegate(delegate);
    //     return;
    // }
    
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip