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
#include "SpanEndpointProvider.h"

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

// Builder implementation
SpanEndpointProvider::Builder::Builder(EndpointId id) : mEndpointId(id) {}

SpanEndpointProvider::Builder & SpanEndpointProvider::Builder::SetComposition(DataModel::EndpointCompositionPattern composition)
{
    mComposition = composition;
    return *this;
}

SpanEndpointProvider::Builder & SpanEndpointProvider::Builder::SetParentId(EndpointId parentId)
{
    mParentId = parentId;
    return *this;
}

SpanEndpointProvider::Builder & SpanEndpointProvider::Builder::SetServerClusters(Span<ServerClusterInterface *> serverClusters)
{
    mServerClusters = serverClusters;
    return *this;
}

SpanEndpointProvider::Builder & SpanEndpointProvider::Builder::SetClientClusters(Span<const ClusterId> clientClusters)
{
    mClientClusters = clientClusters;
    return *this;
}

SpanEndpointProvider::Builder & SpanEndpointProvider::Builder::SetSemanticTags(Span<const SemanticTag> semanticTags)
{
    mSemanticTags = semanticTags;
    return *this;
}

SpanEndpointProvider::Builder & SpanEndpointProvider::Builder::SetDeviceTypes(Span<const DataModel::DeviceTypeEntry> deviceTypes)
{
    mDeviceTypes = deviceTypes;
    return *this;
}

std::pair<SpanEndpointProvider, CHIP_ERROR> SpanEndpointProvider::Builder::build()
{
    if (mEndpointId == kInvalidEndpointId)
    {
        return { SpanEndpointProvider(), CHIP_ERROR_INVALID_ARGUMENT };
    }

    for (auto * cluster : mServerClusters)
    {
        if (cluster == nullptr || cluster->GetPaths().empty())
        {
            ChipLogError(DataManagement, "Builder: Attempted to build with an invalid server cluster entry.");
            return { SpanEndpointProvider(), CHIP_ERROR_INVALID_ARGUMENT };
        }
    }

    return { SpanEndpointProvider(mEndpointId, mComposition, mParentId, mServerClusters, mClientClusters, mSemanticTags,
                                  mDeviceTypes),
             CHIP_NO_ERROR };
}

// SpanEndpointProvider implementation
SpanEndpointProvider::SpanEndpointProvider() :
    mEndpointEntry({ kInvalidEndpointId, kInvalidEndpointId, DataModel::EndpointCompositionPattern::kFullFamily })
// Other members are default-initialzed (empty spans)
{}

CHIP_ERROR
SpanEndpointProvider::SemanticTags(
    ReadOnlyBufferBuilder<chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type> & out) const
{
    return out.ReferenceExisting(mSemanticTags);
}

CHIP_ERROR SpanEndpointProvider::DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const
{
    return out.ReferenceExisting(mDeviceTypes);
}

CHIP_ERROR SpanEndpointProvider::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(mClientClusters);
}

ServerClusterInterface * SpanEndpointProvider::GetServerCluster(ClusterId clusterId) const
{
    for (auto * serverCluster : mServerClusters)
    {
        // Check serverCluster is not null before dereferencing
        if (serverCluster != nullptr && !serverCluster->GetPaths().empty() &&
            serverCluster->GetPaths().front().mClusterId == clusterId)
        {
            return serverCluster;
        }
    }
    return nullptr;
}

CHIP_ERROR SpanEndpointProvider::ServerClusterInterfaces(ReadOnlyBufferBuilder<ServerClusterInterface *> & out) const
{
    return out.ReferenceExisting(mServerClusters);
}

// Private constructor for Builder
SpanEndpointProvider::SpanEndpointProvider(EndpointId id, DataModel::EndpointCompositionPattern composition, EndpointId parentId,
                                           Span<ServerClusterInterface *> serverClusters, Span<const ClusterId> clientClusters,
                                           Span<const SemanticTag> semanticTags,
                                           Span<const DataModel::DeviceTypeEntry> deviceTypes) :
    mEndpointEntry({ id, parentId, composition }),
    mDeviceTypes(deviceTypes), mSemanticTags(semanticTags), mClientClusters(clientClusters), mServerClusters(serverClusters)
{}

} // namespace app
} // namespace chip
