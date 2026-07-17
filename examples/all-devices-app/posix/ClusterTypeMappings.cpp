/*
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

#include "AllDevicesAppClusterImplementationRegistry.h"

#include <app/clusters/ambient-context-sensing-server/AmbientContextSensingCluster.h>
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>

// TODO: We should probably get this to be generated in the header of each
//       cluster via cluster codegen. This can be done later.
template <>
const char * GetClusterTypeName<chip::app::Clusters::OnOffCluster>()
{
    return "chip::app::Clusters::OnOffCluster";
}

template <>
const char * GetClusterTypeName<chip::app::Clusters::OccupancySensingCluster>()
{
    return "chip::app::Clusters::OccupancySensingCluster";
}

template <>
const char * GetClusterTypeName<chip::app::Clusters::AmbientContextSensingCluster>()
{
    return "chip::app::Clusters::AmbientContextSensingCluster";
}

template <>
const char * GetClusterTypeName<chip::app::Clusters::BooleanStateCluster>()
{
    return "chip::app::Clusters::BooleanStateCluster";
}

template <>
const char * GetClusterTypeName<chip::app::Clusters::BasicInformationCluster>()
{
    return "chip::app::Clusters::BasicInformationCluster";
}

chip::app::ServerClusterInterface *
AllDevicesAppClusterImplementationRegistry::GetClusterInterfaceByEndpointAndType(const char * typeName, chip::EndpointId endpoint)
{
    auto it = mClusters.find(typeName);
    if (it == mClusters.end())
    {
        return nullptr;
    }

    for (auto * cluster : it->second)
    {
        auto paths = cluster->GetPaths();
        for (const auto & path : paths)
        {
            if (path.mEndpointId == endpoint)
            {
                return cluster;
            }
        }
    }

    return nullptr;
}
