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
#include <app/data-model-provider/MetadataLookup.h>

#include <app/data-model-provider/MetadataList.h>

namespace chip {
namespace app {
namespace DataModel {

using Protocols::InteractionModel::Status;

std::optional<ServerClusterEntry> ServerClusterFinder::Find(const ConcreteClusterPath & path)
{
    VerifyOrReturnValue(mProvider != nullptr, std::nullopt);

    if (mEndpointId != path.mEndpointId)
    {
        mEndpointId     = path.mEndpointId;
        mClusterEntries = mProvider->ServerClustersIgnoreError(path.mEndpointId);
    }

    for (auto & clusterEntry : mClusterEntries)
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
        mAttributes  = mProvider->AttributesIgnoreError(path);
    }

    for (auto & attributeEntry : mAttributes)
    {
        if (attributeEntry.attributeId == path.mAttributeId)
        {
            return attributeEntry;
        }
    }

    return std::nullopt;
}

Protocols::InteractionModel::Status ValidateClusterPath(ProviderMetadataTree * provider, const ConcreteClusterPath & path,
                                                        Protocols::InteractionModel::Status successStatus)
{
    if (ServerClusterFinder(provider).Find(path).has_value())
    {
        return successStatus;
    }

    // If we get here, the cluster identified by the path does not exist.
    auto endpoints = provider->EndpointsIgnoreError();
    for (auto & endpointEntry : endpoints)
    {
        if (endpointEntry.id == path.mEndpointId)
        {
            // endpoint is valid
            return Protocols::InteractionModel::Status::UnsupportedCluster;
        }
    }

    return Protocols::InteractionModel::Status::UnsupportedEndpoint;
}

} // namespace DataModel
} // namespace app
} // namespace chip
