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
#include <clusters/Descriptor/Structs.h>
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
class EndpointInterface
{
public:
    virtual ~EndpointInterface() = default;

    using SemanticTag = Clusters::Descriptor::Structs::SemanticTagStruct::Type;

    virtual CHIP_ERROR SemanticTags(ReadOnlyBufferBuilder<SemanticTag> & out) const = 0;

    virtual CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const = 0;

    virtual CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const = 0;

    /**
     * @brief Populates the provided buffer with pointers to all ServerClusterInterface instances
     *        hosted on this endpoint. The returned pointers shall be valid as long as the
     *        EndpointInterface instance is valid.
     *
     * @param[out] out The buffer to fill with ServerClusterInterface pointers.
     * @return CHIP_NO_ERROR on success or CHIP_ERROR_NO_MEMORY if the buffer is too small.
     */
};

} // namespace app
} // namespace chip
