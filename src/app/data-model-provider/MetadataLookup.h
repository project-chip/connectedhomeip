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
#include <app/data-model-provider/ProviderMetadataTree.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>

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
    ReadOnlyBuffer<ServerClusterEntry> mClusterEntries;
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
    ReadOnlyBuffer<AttributeEntry> mAttributes;
};

/// Validates that the cluster identified by `path` exists within the given provider.
///
/// If the endpoint identified by the path does not exist, will return Status::UnsupportedEndpoint.
/// If the endpoint exists but does not have the cluster identified by the path, will return Status::UnsupportedCluster.
///
/// otherwise, it will return successStatus.
Protocols::InteractionModel::Status ValidateClusterPath(ProviderMetadataTree * provider, const ConcreteClusterPath & path,
                                                        Protocols::InteractionModel::Status successStatus);

/// Validates that the cluster identified by `path` exists within the given provider.
/// If the endpoint does not exist, will return Status::UnsupportedEndpoint.
/// If the endpoint exists but does not have the cluster identified by the path, will return Status::UnsupportedCluster.
///
/// Otherwise, will return successStatus.
Protocols::InteractionModel::Status ValidateClusterPath(ProviderMetadataTree * provider, const ConcreteClusterPath & path,
                                                        Protocols::InteractionModel::Status successStatus);

} // namespace DataModel
} // namespace app
} // namespace chip
