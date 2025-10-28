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
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster.h>
#include <app/static-cluster-config/ActivatedCarbonFilterMonitoring.h>
#include <app/static-cluster-config/HepaFilterMonitoring.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/ClusterIntegration.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <array>
#include <cstdint>
#include <forward_list>
#include <memory>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace Protocols::InteractionModel;

namespace {
struct ListEntry
{
    template <typename... Args>
    ListEntry(Args &&... args) : cluster(std::forward<Args>(args)...), registration(cluster)
    {}

    ResourceMonitoringCluster cluster;
    ServerClusterRegistration registration;
    ListEntry * next{ nullptr };
};

ListEntry * gActiveClusters{ nullptr };

// Removes the first node with a matching pair (endpointId, clusterId) from the list and returns the new head of the list.
ListEntry * removeListEntryFromList(ListEntry * head, EndpointId endpointId, ClusterId clusterId)
{
    ListEntry * current  = head;
    ListEntry * previous = nullptr;

    while (current)
    {
        if (current->cluster.GetEndpointId() == endpointId && current->cluster.GetClusterId() == clusterId)
        {
            CodegenDataModelProvider::Instance().Registry().Unregister(&(current->cluster));
            ListEntry * temp = current;
            if (previous)
            {
                previous->next = current->next;
            }
            else
            {
                head = current->next;
            }
            current = current->next;
            delete temp;
        }
        else
        {
            previous = current;
            current  = current->next;
        }
    }
    return head;
}

} // namespace

// Common helper for cluster initialization
void InitResourceMonitoringCluster(EndpointId endpointId, ClusterId clusterId, BitFlags<ResourceMonitoring::Feature> featureFlags,
                                   const ResourceMonitoring::ResourceMonitoringCluster::OptionalAttributeSet & optionalAttributeSet,
                                   chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum degradationDirection,
                                   bool resetConditionSupported)
{
    ListEntry * previous = nullptr;
    ListEntry * current  = gActiveClusters;

    while (current != nullptr)
    {
        previous = current;
        current  = current->next;
    }

    ListEntry * newNode =
        new ListEntry{ endpointId, clusterId, featureFlags, optionalAttributeSet, degradationDirection, resetConditionSupported };

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(newNode->registration);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, endpointId,
                     ChipLogValueMEI(clusterId), err.Format());
    }

    if (previous)
    {
        previous->next = newNode;
    }
    else
    {
        gActiveClusters = newNode; // List was empty
    }
}

//
// HEPA Filter Monitoring Cluster
//

void MatterHepaFilterMonitoringClusterInitCallback(EndpointId endpointId)
{
    uint32_t optionalAttributeBits = 0;
    ResourceMonitoring::ResourceMonitoringCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
    uint32_t featureMap = CodegenClusterIntegration::LoadFeatureMap(endpointId, HepaFilterMonitoring::Id);

    InitResourceMonitoringCluster(endpointId, HepaFilterMonitoring::Id, BitFlags<ResourceMonitoring::Feature>{ featureMap },
                                  optionalAttributeSet, chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown,
                                  true // reset condition command supported
    );
}

void MatterHepaFilterMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    gActiveClusters = removeListEntryFromList(gActiveClusters, endpointId, HepaFilterMonitoring::Id);
}

//
// Activated Carbon Filter Monitoring Cluster
//

void MatterActivatedCarbonFilterMonitoringClusterInitCallback(EndpointId endpointId)
{
    uint32_t optionalAttributeBits = 0;
    ResourceMonitoring::ResourceMonitoringCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);
    uint32_t featureMap = CodegenClusterIntegration::LoadFeatureMap(endpointId, ActivatedCarbonFilterMonitoring::Id);

    InitResourceMonitoringCluster(endpointId, ActivatedCarbonFilterMonitoring::Id,
                                  BitFlags<ResourceMonitoring::Feature>{ featureMap }, optionalAttributeSet,
                                  chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown,
                                  true // reset condition command supported
    );
}

void MatterActivatedCarbonFilterMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    gActiveClusters = removeListEntryFromList(gActiveClusters, endpointId, ActivatedCarbonFilterMonitoring::Id);
}

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

ResourceMonitoringCluster * GetClusterInstance(EndpointId endpointId, ClusterId clusterId)
{
    ListEntry * current = gActiveClusters;

    while (current)
    {
        if (current->cluster.GetEndpointId() == endpointId && current->cluster.GetClusterId() == clusterId)
        {
            return &(current->cluster);
        }
        current = current->next;
    }

    return nullptr;
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
