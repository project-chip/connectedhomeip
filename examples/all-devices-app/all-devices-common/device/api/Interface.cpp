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

#include <device/api/Interface.h>

using namespace chip::app::Clusters;

namespace chip::app {

CHIP_ERROR DeviceInterface::DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const
{
    VerifyOrReturnValue(mDescriptorCluster.IsConstructed(), CHIP_NO_ERROR);
    ReturnErrorOnFailure(out.ReferenceExisting(mDeviceTypes));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInterface::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    // no bindings
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceInterface::RegisterDescriptor(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                               EndpointComposition composition)
{
    VerifyOrReturnError(endpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: Allow mDescriptorCluster to be initialized with custom optional attribute sets.
    mDescriptorCluster.Create(endpoint, DescriptorCluster::OptionalAttributesSet(0), composition.tagList);
    CHIP_ERROR err = provider.AddCluster(mDescriptorCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        mDescriptorCluster.Destroy();
        return err;
    }

    mEndpointRegistration.endpointEntry = DataModel::EndpointEntry{
        .id                 = endpoint,
        .parentId           = composition.parentId,
        .compositionPattern = composition.pattern,
    };
    return CHIP_NO_ERROR;
}

void DeviceInterface::UnregisterDescriptor(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    LogErrorOnFailure(provider.RemoveEndpoint(endpoint, ClusterShutdownType::kClusterShutdown));
    if (mDescriptorCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mDescriptorCluster.Cluster()));
        mDescriptorCluster.Destroy();
    }
}

} // namespace chip::app
