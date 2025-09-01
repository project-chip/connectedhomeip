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

LazyRegisteredServerCluster<ResourceMonitoring::HepaFilterMonitoringCluster> hepa_gServers[kResourceMonitoringMaxClusterCount];

LazyRegisteredServerCluster<ResourceMonitoring::ActivatedCarbonFilterMonitoringCluster> activatedCarbon_gServers[kResourceMonitoringMaxClusterCount];

// Find the 0-based array index corresponding to the given {endpoint id, cluster id}.
// Log an error if not found.
bool findEndpointWithLog(EndpointId endpointId, ClusterId clusterId, uint16_t & outArrayIndex)
{

    uint16_t arrayIndex =
        emberAfGetClusterServerEndpointIndex(endpointId, clusterId, kResourceMonitoringFixedClusterCount);

    if (arrayIndex >= kResourceMonitoringMaxClusterCount)
    {
        ChipLogError(AppServer, "Cound not find endpoint index for endpoint %u", endpointId);
        return false;
    }

    outArrayIndex = arrayIndex;

    return true;
}

template <typename T>
void CreateConcreteResourceMonitoringCluster(uint16_t arrayIndex, EndpointId endpointId, ClusterId clusterId, T * gServers)
{
    uint32_t rawFeatureMap{0};

    /*
    if (clusterId == HepaFilterMonitoring::Id) 
    {
        if (HepaFilterMonitoring::Attributes::FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
        {
            ChipLogError(AppServer, "Failed to get feature map for endpoint %u cluster %u", endpointId, clusterId);
            rawFeatureMap = 0;
        }
    }
    else if (clusterId == ActivatedCarbonFilterMonitoring::Id) 
    {      
        if (ActivatedCarbonFilterMonitoring::Attributes::FeatureMap::Get(endpointId, &rawFeatureMap) != Status::Success)
        {
            ChipLogError(AppServer, "Failed to get feature map for endpoint %u cluster %u", endpointId, clusterId);
            rawFeatureMap = 0;
        }
    }
    else 
    {
        ChipLogError(AppServer, "Invalid cluster id: %u for Resource Monitoring on endpoint: %u", clusterId, endpointId);
        return;
    }
    */

    gServers[arrayIndex].Create(
        endpointId,
        clusterId,
        rawFeatureMap,
        chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown,
        true /* ResetCondition command supported */);
}

} // namespace

//
// HEPA Filter Monitoring Cluster
//

void emberAfHepaFilterMonitoringClusterServerInitCallback(EndpointId endpointId )
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, HepaFilterMonitoring::Id, arrayIndex))
    {
        return;
    }

    // this call performs creation of concrete cluster object on gServers[arrayIndex]
    CreateConcreteResourceMonitoringCluster(arrayIndex, endpointId, HepaFilterMonitoring::Id, hepa_gServers);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(hepa_gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register Resource Monitoring on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfHepaFilterMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, HepaFilterMonitoring::Id, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&hepa_gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister Resource Monitoring on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    hepa_gServers[arrayIndex].Destroy();
}

void emberAfHepaFilterMonitoringClusterResetConditionCallback(chip::app::CommandHandler*, chip::app::ConcreteCommandPath const&, chip::app::Clusters::HepaFilterMonitoring::Commands::ResetCondition::DecodableType const&)
{
    // The actual command handling is done in ResourceMonitoringCluster::InvokeCommand
}

//
// Activated Carbon Filter Monitoring Cluster
//
     
void emberAfActivatedCarbonFilterMonitoringClusterServerInitCallback(EndpointId endpointId )
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, ActivatedCarbonFilterMonitoring::Id, arrayIndex))
    {
        return;
    }

    // this call performs creation of concrete cluster object on gServers[arrayIndex]
    CreateConcreteResourceMonitoringCluster(arrayIndex, endpointId, ActivatedCarbonFilterMonitoring::Id, activatedCarbon_gServers);

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(activatedCarbon_gServers[arrayIndex].Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register Resource Monitoring on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
}

void emberAfActivatedCarbonMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, ActivatedCarbonFilterMonitoring::Id, arrayIndex))
    {
        return;
    }

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&activatedCarbon_gServers[arrayIndex].Cluster());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to unregister Resource Monitoring on endpoint %u: %" CHIP_ERROR_FORMAT, endpointId, err.Format());
    }
    activatedCarbon_gServers[arrayIndex].Destroy();
}

void emberAfActivatedCarbonFilterMonitoringClusterResetConditionCallback(chip::app::CommandHandler*, chip::app::ConcreteCommandPath const&, chip::app::Clusters::ActivatedCarbonFilterMonitoring::Commands::ResetCondition::DecodableType const&){
    // The actual command handling is done in ResourceMonitoringCluster::InvokeCommand
}

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

void SetDelegate(EndpointId endpointId, ClusterId clusterId, ResourceMonitoringDelegate * delegate)
{
    uint16_t arrayIndex = 0;
    if (!findEndpointWithLog(endpointId, clusterId, arrayIndex))
    {
        return;
    }

    if (clusterId == HepaFilterMonitoring::Id) 
    {
        hepa_gServers[arrayIndex].Cluster().SetDelegate(delegate);
        return;
    } 
    else if (clusterId == ActivatedCarbonFilterMonitoring::Id) 
    {
        activatedCarbon_gServers[arrayIndex].Cluster().SetDelegate(delegate);
        return;
    }
    
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip