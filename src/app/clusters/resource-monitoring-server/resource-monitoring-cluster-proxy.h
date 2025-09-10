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

#include "resource-monitoring-cluster.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

template <ClusterId DISCRIMINATOR>
class ResourceMonitoringClusterProxy : public ResourceMonitoringCluster
{
    public:
    
    ResourceMonitoringClusterProxy(const ResourceMonitoringClusterProxy &) = delete;
    ResourceMonitoringClusterProxy & operator=(const ResourceMonitoringClusterProxy &) = delete;

    ResourceMonitoringClusterProxy(ResourceMonitoringClusterProxy &&) = delete;
    ResourceMonitoringClusterProxy & operator=(ResourceMonitoringClusterProxy &&) = delete;

    ResourceMonitoringClusterProxy() = delete;

    ResourceMonitoringClusterProxy(
        EndpointId aEndpointId,
        ClusterId aClusterId,
        const BitFlags<ResourceMonitoring::Feature> enabledFeatures,
        ResourceMonitoringCluster::OptionalAttributeSet optionalAttributeSet,
        ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
        bool aResetConditionCommandSupported
    ): ResourceMonitoringCluster(
        aEndpointId,
        aClusterId,
        enabledFeatures,
        optionalAttributeSet,
        aDegradationDirection,
        aResetConditionCommandSupported
    )
    {
        static_assert(DISCRIMINATOR == HepaFilterMonitoring::Id || DISCRIMINATOR == ActivatedCarbonFilterMonitoring::Id,
                      "ResourceMonitoringClusterProxy can only be instantiated for HepaFilterMonitoring or ActivatedCarbonFilterMonitoring");
    }
};

using HepaFilterMonitoringCluster =
    ResourceMonitoringClusterProxy<HepaFilterMonitoring::Id>;
using ActivatedCarbonFilterMonitoringCluster =
    ResourceMonitoringClusterProxy<ActivatedCarbonFilterMonitoring::Id>;

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip