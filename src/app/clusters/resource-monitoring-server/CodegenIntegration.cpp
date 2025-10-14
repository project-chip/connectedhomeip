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
#include <app/static-cluster-config/HepaFilterMonitoring.h>
#include <app/static-cluster-config/ActivatedCarbonFilterMonitoring.h>
#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <data-model-providers/codegen/ClusterIntegration.h>

#include <array>
#include <cstdint>
#include <memory>
#include <forward_list>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace Protocols::InteractionModel;

namespace {
struct ActiveCluster {
    template <typename... Args>
    ActiveCluster(Args&& ...args) : cluster(std::forward<Args>(args)...), registration(cluster)
    {}

    ClusterId clusterId;
    ResourceMonitoringCluster cluster;
    ServerClusterRegistration registration;
    ActiveCluster * next{nullptr};
};

ActiveCluster* gActiveClusters{nullptr};

// Removes the first node with a matching clusterId from the list.
// Returns the new head of the list.

ActiveCluster* removeActiveClusterFromList(ActiveCluster* head, ClusterId clusterId) {
    // Handle the case where the head needs to be removed
    while (head && head->clusterId == clusterId) {
        ActiveCluster * temp = head;

        CodegenDataModelProvider::Instance().Registry().Unregister(&(temp->cluster));
        temp->registration.~ServerClusterRegistration();
        temp->cluster.~ResourceMonitoringCluster();

        head = head->next;
        delete temp;
    }
    // No head left
    if (!head) return nullptr;

    // Remove subsequent nodes
    ActiveCluster * current = head;
    while (current->next) {
        if (current->next->clusterId == clusterId) {
            ActiveCluster * temp = current->next;

            CodegenDataModelProvider::Instance().Registry().Unregister(&(temp->cluster));
            temp->registration.~ServerClusterRegistration();
            temp->cluster.~ResourceMonitoringCluster();

            current->next = temp->next;
            delete temp;
        } else {
            current = current->next;
        }
    }
    return head;
}


} // namespace

//
// HEPA Filter Monitoring Cluster
//

void MatterHepaFilterMonitoringClusterInitCallback(EndpointId endpointId)
{
    uint32_t optionalAttributeBits = 0;

    ResourceMonitoring::ResourceMonitoringCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

    ActiveCluster * current = gActiveClusters;

    while(current != nullptr) {
        current = current->next;
    }

    uint32_t featureMap = CodegenClusterIntegration::LoadFeatureMap(endpointId, HepaFilterMonitoring::Id);

    current = new ActiveCluster{
        endpointId, 
        HepaFilterMonitoring::Id, 
        BitFlags<ResourceMonitoring::Feature>{featureMap}, 
        optionalAttributeSet, 
        chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown, 
        true // reset condition command supported
    };

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(current->registration);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, endpointId,
                     ChipLogValueMEI(HepaFilterMonitoring::Id), err.Format());
    }

}



void MatterHepaFilterMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    gActiveClusters = removeActiveClusterFromList(gActiveClusters, HepaFilterMonitoring::Id);
}

//
// Activated Carbon Filter Monitoring Cluster
//
     
void MatterActivatedCarbonFilterMonitoringClusterInitCallback(EndpointId endpointId )
{
    uint32_t optionalAttributeBits = 0;

    ResourceMonitoring::ResourceMonitoringCluster::OptionalAttributeSet optionalAttributeSet(optionalAttributeBits);

    ActiveCluster * current = gActiveClusters;

    while(current != nullptr) {
        current = current->next;
    }

    uint32_t featureMap = CodegenClusterIntegration::LoadFeatureMap(endpointId, HepaFilterMonitoring::Id);

    current = new ActiveCluster{
        endpointId, 
        ActivatedCarbonFilterMonitoring::Id, 
        BitFlags<ResourceMonitoring::Feature>{featureMap}, 
        optionalAttributeSet, 
        chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum::kDown, 
        true
    };

    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(current->registration);
            

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register cluster %u/" ChipLogFormatMEI ":   %" CHIP_ERROR_FORMAT, endpointId,
                     ChipLogValueMEI(ActivatedCarbonFilterMonitoring::Id), err.Format());
    }
}

void MatterActivatedCarbonFilterMonitoringClusterShutdownCallback(EndpointId endpointId)
{
    gActiveClusters = removeActiveClusterFromList(gActiveClusters, ActivatedCarbonFilterMonitoring::Id);
}

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

void SetDelegate(EndpointId endpointId, ClusterId clusterId, ResourceMonitoringDelegate * delegate)
{

    ActiveCluster * current = gActiveClusters;

    while(current && current->clusterId != clusterId) {
        current = current->next;
    }

    if (current) {
        current->cluster.SetDelegate(delegate);
    }
    
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip