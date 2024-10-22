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
#include <app/data-model-provider/MetadataTypes.h>

namespace chip {
namespace app {
namespace DataModel {

const AttributeEntry AttributeEntry::kInvalid{ .path = ConcreteAttributePath(kInvalidEndpointId, kInvalidClusterId,
                                                                             kInvalidAttributeId) };

const CommandEntry CommandEntry::kInvalid{ .path = ConcreteCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId) };

const ClusterEntry ClusterEntry::kInvalid{
    .path = ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId),
    .info = ClusterInfo(0 /* version */), // version of invalid cluster entry does not matter
};

// A default implementation if just first/next exist
bool ProviderMetadataTree::EndpointExists(EndpointId endpoint)
{
    for (EndpointId id = FirstEndpoint(); id != kInvalidEndpointId; id = NextEndpoint(id))
    {
        if (id == endpoint)
        {
            return true;
        }
    }
    return false;
}

} // namespace DataModel
} // namespace app
} // namespace chip
