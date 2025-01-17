/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/data-model-provider/MetadataSearch.h>

namespace chip {
namespace app {
namespace DataModel {

std::optional<ServerClusterEntry> ServerClusterFinder::Find(const ConcreteClusterPath & path)
{
    VerifyOrReturnValue(mProvider != nullptr, std::nullopt);

    if (mEndpointId != path.mEndpointId)
    {
        mEndpointId     = path.mEndpointId;
        mClusterEntries = mProvider->ServerClusters(path.mEndpointId);
    }

    auto serverClustersSpan = mClusterEntries.GetSpanValidForLifetime();

    for (auto & clusterEntry : serverClustersSpan)
    {
        if (clusterEntry.clusterId == path.mClusterId)
        {
            return clusterEntry;
        }
    }

    return std::nullopt;
}

std::optional<AttributeEntry> AttributeFinder::Find(const ConcreteAttributePath & path)
{
    VerifyOrReturnValue(mProvider != nullptr, std::nullopt);

    if (mClusterPath != path)
    {
        mClusterPath = path;
        mAttributes  = mProvider->Attributes(path);
    }

    auto attributesSpan = mAttributes.GetSpanValidForLifetime();
    for (auto & attributeEntry : attributesSpan)
    {
        if (attributeEntry.attributeId == path.mAttributeId)
        {
            return attributeEntry;
        }
    }

    return std::nullopt;
}

std::optional<EndpointEntry> EndpointFinder::Find(EndpointId endpointId)
{
    auto endpointsSpan = mEndpoints.GetSpanValidForLifetime();
    for (auto & endpointEntry : endpointsSpan)
    {
        if (endpointEntry.id == endpointId)
        {
            return endpointEntry;
        }
    }

    return std::nullopt;
}

} // namespace DataModel
} // namespace app
} // namespace chip
