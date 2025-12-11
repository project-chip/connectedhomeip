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

#include <devices/interface/SingleEndpointDevice.h>

using namespace chip::app::Clusters;

namespace chip::app {

CHIP_ERROR SingleEndpointDevice::SingleEndpointRegistration(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                            EndpointId parentId)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    mEndpointId = endpoint;

    // TODO: This needs to be updated to be more customizable and allow the cluster to be created with
    //  optional attributes or semantic tags being set.
    mDescriptorCluster.Create(endpoint, DescriptorCluster::OptionalAttributesSet(0), Span<const SemanticTag>());
    ReturnErrorOnFailure(provider.AddCluster(mDescriptorCluster.Registration()));

    mEndpointRegistration.endpointEntry = DataModel::EndpointEntry{
        .id                 = endpoint,
        .parentId           = parentId,
        .compositionPattern = DataModel::EndpointCompositionPattern::kFullFamily,
    };
    return CHIP_NO_ERROR;
}

void SingleEndpointDevice::SingleEndpointUnregistration(CodeDrivenDataModelProvider & provider)
{
    LogErrorOnFailure(provider.RemoveEndpoint(mEndpointId, ClusterShutdownType::kClusterShutdown));
    if (mDescriptorCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mDescriptorCluster.Cluster(), ClusterShutdownType::kClusterShutdown));
        mDescriptorCluster.Destroy();
    }
}

} // namespace chip::app
