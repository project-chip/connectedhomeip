/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "Device.h"
#include <memory>

using namespace chip::app::Clusters;

namespace chip::app {

CHIP_ERROR Device::BaseRegistration(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mEndpointId = endpoint;

    mDescriptorCluster.Create(endpoint, DescriptorCluster::OptionalAttributesSet(0), Span<const SemanticTag>());
    ReturnErrorOnFailure(provider.AddCluster(mDescriptorCluster.Registration()));

    mEndpointRegistration.endpointEntry = DataModel::EndpointEntry{
        .id                 = endpoint, //
        .parentId           = parentId, //
        .compositionPattern = DataModel::EndpointCompositionPattern::kFullFamily,
    };
    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const
{
    ReturnErrorOnFailure(out.EnsureAppendCapacity(1));

    if (mDescriptorCluster.IsConstructed())
    {
        ReturnErrorOnFailure(out.Append(DataModel::DeviceTypeEntry{
            .deviceTypeId       = mDeviceType.deviceType,
            .deviceTypeRevision = static_cast<uint8_t>(mDeviceType.revision),
        }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::SemanticTags(ReadOnlyBufferBuilder<SemanticTag> & out) const
{
    // no semantic tags
    return CHIP_NO_ERROR;
}

CHIP_ERROR Device::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    // no bindings
    return CHIP_NO_ERROR;
}

} // namespace chip::app
