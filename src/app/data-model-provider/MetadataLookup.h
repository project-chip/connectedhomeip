/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

#include <optional>

namespace chip {
namespace app {
namespace DataModel {

/// Helps search for a specific server cluster in the given
/// metadata provider.
///
/// Facilitates the very common operation of "find a cluster on a given cluster path".
class ServerClusterFinder
{
public:
    ServerClusterFinder(ProviderMetadataTree * provider) : mProvider(provider) {}

    std::optional<ServerClusterEntry> Find(const ConcreteClusterPath & path);

private:
    ProviderMetadataTree * mProvider;
    EndpointId mEndpointId = kInvalidEndpointId;
    MetadataList<ServerClusterEntry> mClusterEntries;
};

/// Helps search for a specific server attribute in the given
/// metadata provider.
///
/// Facilitates the very common operation of "find an attribute on a given attribute path".
class AttributeFinder
{
public:
    AttributeFinder(ProviderMetadataTree * provider) : mProvider(provider), mClusterPath(kInvalidEndpointId, kInvalidClusterId) {}

    std::optional<AttributeEntry> Find(const ConcreteAttributePath & path);

private:
    ProviderMetadataTree * mProvider;
    ConcreteClusterPath mClusterPath;
    MetadataList<AttributeEntry> mAttributes;
};

/// Helps search for a specific server endpoint in the given
/// metadata provider.
///
/// Facilitates the operation of "find an endpoint with a given ID".
class EndpointFinder
{
public:
    EndpointFinder(ProviderMetadataTree * provider) : mProvider(provider)
    {
        if (mProvider != nullptr)
        {
            mEndpoints = mProvider->Endpoints();
        }
    }

    std::optional<EndpointEntry> Find(EndpointId endpointId);

private:
    ProviderMetadataTree * mProvider;
    MetadataList<EndpointEntry> mEndpoints;
};

} // namespace DataModel
} // namespace app
} // namespace chip
