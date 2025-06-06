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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip {
namespace app {

/**
 * @brief Defines the interface for an object that can provide information about a Matter endpoint.
 *
 * This interface is used describe the structure and capabilities of an endpoint, including its device
 * types, client clusters, server clusters, and semantic tags.
 *
 * Implementations of this interface are responsible for providing instances of ServerClusterInterface
 * for each server cluster they expose.
 */
class EndpointProviderInterface
{
public:
    virtual ~EndpointProviderInterface() = default;

    using SemanticTag = chip::app::Clusters::Descriptor::Structs::SemanticTagStruct::Type;

    /**
     * @brief Retrieves the basic information about the endpoint.
     *
     * @return A constant reference to the DataModel::EndpointEntry struct containing
     *         the endpoint ID, parent endpoint ID, and composition pattern.
     */
    virtual const DataModel::EndpointEntry & GetEndpointEntry() const = 0;

    /**
     * @brief Populates the provided ReadOnlyBufferBuilder with the semantic tags for this endpoint.
     *
     * @param[out] out The ReadOnlyBufferBuilder to fill with SemanticTagStruct instances.
     * @return CHIP_NO_ERROR on success, or an error code if the operation fails (e.g., buffer too small).
     */
    virtual CHIP_ERROR SemanticTags(ReadOnlyBufferBuilder<SemanticTag> & out) const = 0;

    /**
     * @brief Populates the provided ReadOnlyBufferBuilder with the device types for this endpoint.
     *
     * @param[out] out The ReadOnlyBufferBuilder to fill with DataModel::DeviceTypeEntry instances.
     * @return CHIP_NO_ERROR on success, or an error code if the operation fails (e.g., buffer too small).
     */
    virtual CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const = 0;

    /**
     * @brief Populates the provided ReadOnlyBufferBuilder with the client cluster IDs for this endpoint.
     *
     * @param[out] out The ReadOnlyBufferBuilder to fill with ClusterId instances.
     * @return CHIP_NO_ERROR on success, or an error code if the operation fails (e.g., buffer too small).
     */
    virtual CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const = 0;

    /**
     * @brief Retrieves a pointer to the ServerClusterInterface for the given cluster ID.
     *
     * @param clusterId The ID of the server cluster to retrieve.
     * @return A pointer to the ServerClusterInterface if found, otherwise nullptr.
     */
    virtual ServerClusterInterface * GetServerCluster(ClusterId clusterId) const = 0;

    /**
     * @brief Populates the provided ReadOnlyBufferBuilder with pointers to all ServerClusterInterface instances
     *        hosted on this endpoint.
     *
     * @param[out] out The ReadOnlyBufferBuilder to fill with ServerClusterInterface pointers.
     * @return CHIP_NO_ERROR on success, or an error code if the operation fails (e.g., buffer too small).
     */
    virtual CHIP_ERROR ServerClusterInterfaces(ReadOnlyBufferBuilder<ServerClusterInterface *> & out) const = 0;
};

} // namespace app
} // namespace chip
