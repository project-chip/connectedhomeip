/*
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
#include <app/codegen-data-model-provider/EmberMetadata.h>

#include <app/GlobalAttributes.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>

namespace chip {
namespace app {
namespace Ember {

using Protocols::InteractionModel::Status;

std::variant<const EmberAfCluster *,           // global attribute, data from a cluster
             const EmberAfAttributeMetadata *, // a specific attribute stored by ember
             Status                            // one of Status::Unsupported*
             >
FindAttributeMetadata(const ConcreteAttributePath & aPath)
{
    if (IsGlobalAttribute(aPath.mAttributeId))
    {
        // Global list attribute check first: during path expansion a lot of attributes
        // will actually be global attributes (so not too much of a performance hit)
        for (auto & attr : GlobalAttributesNotInMetadata)
        {
            if (attr == aPath.mAttributeId)
            {
                const EmberAfCluster * cluster = emberAfFindServerCluster(aPath.mEndpointId, aPath.mClusterId);
                if (cluster == nullptr)
                {
                    return (emberAfFindEndpointType(aPath.mEndpointId) == nullptr) ? Status::UnsupportedEndpoint
                                                                                   : Status::UnsupportedCluster;
                }

                return cluster;
            }
        }
    }
    const EmberAfAttributeMetadata * metadata =
        emberAfLocateAttributeMetadata(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);

    if (metadata == nullptr)
    {
        const EmberAfEndpointType * type = emberAfFindEndpointType(aPath.mEndpointId);
        if (type == nullptr)
        {
            return Status::UnsupportedEndpoint;
        }

        const EmberAfCluster * cluster = emberAfFindClusterInType(type, aPath.mClusterId, CLUSTER_MASK_SERVER);
        if (cluster == nullptr)
        {
            return Status::UnsupportedCluster;
        }

        // Since we know the attribute is unsupported and the endpoint/cluster are
        // OK, this is the only option left.
        return Status::UnsupportedAttribute;
    }

    return metadata;
}

} // namespace Ember
} // namespace app
} // namespace chip
