/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

struct Instance
{
    /**
     * Creates an Instance object which wraps a resource monitoring cluster object and attaches the provided delegate.
     *
     * This overloaded constructor facilitates keeping existing code in (for instance)
     * `examples/placeholder/linux/resource-monitoring-delegates.cpp`. This constructor forwards to the primary constructor overload
     * and then calls SetDelegate() with the supplied delegate. The caller is responsible for ensuring that the delegate remains
     * valid for the lifetime of the cluster instance.
     *
     * @param delegate                         Pointer to a ResourceMonitoringDelegate that will be associated with this instance.
     *                                        The caller must ensure the delegate outlives the cluster instance.
     * @param aEndpointId                      The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId                       The ID of the ResourceMonitoring aliased cluster to be instantiated.
     * @param aFeatureMap                      The feature map of the cluster (packed as a uint32_t).
     * @param aDegradationDirection            The degradation direction of the cluster.
     * @param resetConditionCommandSupported   Whether the ResetCondition command is supported by the cluster.
     */
    Instance(chip::app::Clusters::ResourceMonitoring::Delegate * delegate, chip::EndpointId endpointId, chip::ClusterId clusterId,
             uint32_t featureMap, chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum degradationDirection,
             bool resetConditionCommandSupported);

    virtual ~Instance();

    CHIP_ERROR Init() { return mCluster.Cluster().Init(); };

    // Attribute setters
    Protocols::InteractionModel::Status UpdateCondition(uint8_t newCondition)
    {
        return mCluster.Cluster().UpdateCondition(newCondition);
    }
    Protocols::InteractionModel::Status
    UpdateChangeIndication(chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum newChangeIndication)
    {
        return mCluster.Cluster().UpdateChangeIndication(newChangeIndication);
    }
    Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool newInPlaceIndicator)
    {
        return mCluster.Cluster().UpdateInPlaceIndicator(newInPlaceIndicator);
    }
    Protocols::InteractionModel::Status UpdateLastChangedTime(DataModel::Nullable<uint32_t> newLastChangedTime)
    {
        return mCluster.Cluster().UpdateLastChangedTime(newLastChangedTime);
    }

    // Attribute getters
    uint8_t GetCondition() const { return mCluster.Cluster().GetCondition(); }
    ChangeIndicationEnum GetChangeIndication() const { return mCluster.Cluster().GetChangeIndication(); }
    DegradationDirectionEnum GetDegradationDirection() const { return mCluster.Cluster().GetDegradationDirection(); }
    bool GetInPlaceIndicator() const { return mCluster.Cluster().GetInPlaceIndicator(); }
    DataModel::Nullable<uint32_t> GetLastChangedTime() const { return mCluster.Cluster().GetLastChangedTime(); }

    chip::app::RegisteredServerCluster<chip::app::Clusters::ResourceMonitoring::ResourceMonitoringCluster> mCluster;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
