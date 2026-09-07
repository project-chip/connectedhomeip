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
#include <data-model-providers/codedriven/endpoint/EndpointInterface.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {

/**
 * @brief An implementation of EndpointInterface that uses `chip::Span` to refer to its data.
 *
 * This provider is constructed using its `Builder` class. It stores `chip::Span` members that
 * point to externally managed arrays for its configuration (device types, server/client clusters, etc.).
 *
 * @warning Lifetime of Span-Referenced Data:
 * `SpanEndpoint` does NOT take ownership of the data arrays referenced by its
 * internal `chip::Span` members. The caller who provides these Spans (usually via the
 * `Builder`) MUST ensure that the underlying data remains valid for the entire lifetime
 * of the `SpanEndpoint` instance.
 *   - For `Span<T>` (e.g., `Span<const ClusterId>`, `Span<const DeviceTypeEntry>`), the
 *     array of `T` elements must outlive the `SpanEndpoint`.
 *   - For `Span<T*>` (e.g., `Span<ServerClusterInterface *>`), both the array of pointers
 *     (`T*`) and the objects (`T`) pointed to by those pointers must outlive the
 *     `SpanEndpoint`.
 * Failure to adhere to these lifetime requirements will lead to undefined behavior.
 */
class SpanEndpoint : public EndpointInterface
{
public:
    /**
     * @brief Builder class for constructing a SpanEndpoint.
     *
     * This class provides a way to set the client clusters, semantic tags,
     * and device types of the SpanEndpoint.
     */
    class Builder
    {
    public:
        explicit Builder() = default;

        Builder & SetClientClusters(Span<const ClusterId> clientClusters);
        Builder & SetDeviceTypes(Span<const DataModel::DeviceTypeEntry> deviceTypes);

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
        Builder & SetEndpointUniqueId(CharSpan endpointUniqueId);
#endif
        SpanEndpoint Build();

    private:
        Span<const ClusterId> mClientClusters;
        Span<const DataModel::DeviceTypeEntry> mDeviceTypes;

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
        CharSpan mEndpointUniqueId;
#endif
    };

    ~SpanEndpoint() override = default;

    // Delete copy constructor and assignment operator. SpanEndpoint holds non-owning data (Spans).
    // This helps prevent accidental copies that could lead multiple objects pointing to the same external data.
    SpanEndpoint(const SpanEndpoint &)             = delete;
    SpanEndpoint & operator=(const SpanEndpoint &) = delete;

    // Allow move semantics for SpanEndpoint.
    SpanEndpoint(SpanEndpoint &&)             = default;
    SpanEndpoint & operator=(SpanEndpoint &&) = default;

    // Iteration methods
    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    CharSpan EndpointUniqueID() const override;
#endif

private:
    // Private constructor for Builder
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    SpanEndpoint(const Span<const ClusterId> & clientClusters, const Span<const DataModel::DeviceTypeEntry> & deviceTypes,
                 const CharSpan & uniqueEndpointId);
#else
    SpanEndpoint(const Span<const ClusterId> & clientClusters, const Span<const DataModel::DeviceTypeEntry> & deviceTypes);
#endif

    // Iteration methods
    Span<const DataModel::DeviceTypeEntry> mDeviceTypes;
    Span<const ClusterId> mClientClusters;

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    CharSpan mEndpointUniqueId;
#endif
};

} // namespace app
} // namespace chip
