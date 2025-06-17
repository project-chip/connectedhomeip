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

#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <data-model-providers/endpoint/EndpointProviderInterface.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

#include <variant>

namespace chip {
namespace app {

/**
 * @brief An implementation of EndpointProviderInterface that uses `chip::Span` to refer to its data.
 *
 * This provider is constructed using its `Builder` class. It stores `chip::Span` members that
 * point to externally managed arrays for its configuration (device types, server/client clusters,
 * semantic tags, etc.).
 *
 * @warning Lifetime of Span-Referenced Data:
 * `SpanEndpointProvider` does NOT take ownership of the data arrays referenced by its
 * internal `chip::Span` members. The caller who provides these Spans (usually via the
 * `Builder`) MUST ensure that the underlying data remains valid for the entire lifetime
 * of the `SpanEndpointProvider` instance.
 *   - For `Span<T>` (e.g., `Span<const ClusterId>`, `Span<const DeviceTypeEntry>`), the
 *     array of `T` elements must outlive the `SpanEndpointProvider`.
 *   - For `Span<T*>` (e.g., `Span<ServerClusterInterface *>`), both the array of pointers
 *     (`T*`) and the objects (`T`) pointed to by those pointers must outlive the
 *     `SpanEndpointProvider`.
 * Failure to adhere to these lifetime requirements will lead to undefined behavior.
 */
class SpanEndpointProvider : public EndpointProviderInterface
{
public:
    class Builder
    {
    public:
        explicit Builder(EndpointId id);

        Builder & SetComposition(DataModel::EndpointCompositionPattern composition);
        Builder & SetParentId(EndpointId parentId);
        Builder & SetServerClusters(Span<ServerClusterInterface *> serverClusters);
        Builder & SetClientClusters(Span<const ClusterId> clientClusters);
        Builder & SetSemanticTags(Span<const SemanticTag> semanticTags);
        Builder & SetDeviceTypes(Span<const DataModel::DeviceTypeEntry> deviceTypes);

        // Builds the SpanEndpointProvider.
        // Returns a std::variant containing either the successfully built SpanEndpointProvider
        // or a CHIP_ERROR if the build failed (e.g., due to invalid arguments).
        // Callers should check the variant's active alternative before use.
        std::variant<SpanEndpointProvider, CHIP_ERROR> build();

    private:
        EndpointId mEndpointId;
        DataModel::EndpointCompositionPattern mComposition = DataModel::EndpointCompositionPattern::kFullFamily;
        EndpointId mParentId                               = kInvalidEndpointId;
        Span<ServerClusterInterface *> mServerClusters;
        Span<const ClusterId> mClientClusters;
        Span<const SemanticTag> mSemanticTags;
        Span<const DataModel::DeviceTypeEntry> mDeviceTypes;
    };

    ~SpanEndpointProvider() override = default;

    // Delete copy and move constructors and assignment operators
    SpanEndpointProvider(const SpanEndpointProvider &)             = delete;
    SpanEndpointProvider & operator=(const SpanEndpointProvider &) = delete;
    SpanEndpointProvider(SpanEndpointProvider &&)                  = default; // Allow move
    SpanEndpointProvider & operator=(SpanEndpointProvider &&)      = default; // Allow move

    const DataModel::EndpointEntry & GetEndpointEntry() const override { return mEndpointEntry; }

    // Iteration methods
    CHIP_ERROR SemanticTags(ReadOnlyBufferBuilder<SemanticTag> & out) const override;
    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    // Getter for ServerClusterInterface, returns nullptr if the cluster is not found.
    ServerClusterInterface * GetServerCluster(ClusterId clusterId) const override;
    CHIP_ERROR ServerClusterInterfaces(ReadOnlyBufferBuilder<ServerClusterInterface *> & out) const override;

private:
    // Private constructor for Builder
    SpanEndpointProvider(EndpointId id, DataModel::EndpointCompositionPattern composition, EndpointId parentId,
                         Span<ServerClusterInterface *> serverClusters, Span<const ClusterId> clientClusters,
                         Span<const SemanticTag> semanticTags, Span<const DataModel::DeviceTypeEntry> deviceTypes);

    // Iteration methods
    // GetEndpointEntry is already public
    DataModel::EndpointEntry mEndpointEntry;
    Span<const DataModel::DeviceTypeEntry> mDeviceTypes;
    Span<const SemanticTag> mSemanticTags;
    Span<const ClusterId> mClientClusters;
    Span<ServerClusterInterface *> mServerClusters;
};

} // namespace app
} // namespace chip
