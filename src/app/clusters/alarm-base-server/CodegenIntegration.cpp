/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/alarm-base-server/CodegenIntegration.h>
#include <app/clusters/alarm-base-server/alarm-base-cluster-objects.h>

#ifdef MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT
#include <app/clusters/dishwasher-alarm-server/CodegenIntegration.h>
#endif

#ifdef MATTER_DM_REFRIGERATOR_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT
#include <app/clusters/refrigerator-alarm-server/CodegenIntegration.h>
#endif

namespace chip::app::Clusters::AlarmBase {

AlarmBaseCluster * FindClusterOnEndpoint(EndpointId endpointId, ClusterId clusterId)
{
    if (clusterId == DishwasherAlarm::Id)
    {
#ifdef MATTER_DM_DISHWASHER_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT
        return Clusters::DishwasherAlarm::FindClusterOnEndpoint(endpointId);
#else
        (void) endpointId;
        return nullptr;
#endif
    }
    if (clusterId == RefrigeratorAlarm::Id)
    {
#ifdef MATTER_DM_REFRIGERATOR_ALARM_CLUSTER_SERVER_ENDPOINT_COUNT
        return Clusters::RefrigeratorAlarm::FindClusterOnEndpoint(endpointId);
#else
        (void) endpointId;
        return nullptr;
#endif
    }
    return nullptr;
}

} // namespace chip::app::Clusters::AlarmBase
