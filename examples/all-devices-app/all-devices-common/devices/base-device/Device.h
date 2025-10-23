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

#pragma once

#include <app/util/basic-types.h>
#include <app/clusters/descriptor/descriptor-cluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <clusters/Descriptor/Structs.h>

#include <string>

namespace chip::app {

/// A device is a entity that maintains some cluster functionality.
///
/// Current implementation assumes that a device is registered on a single
/// endpoint.
class Device : public EndpointInterface
{
public:
    using DeviceType = Clusters::Descriptor::Structs::DeviceTypeStruct::Type;
    Device(const DeviceType & deviceType) : mDeviceType(deviceType), mEndpointRegistration(*this, {}) {}
    virtual ~Device() = default;

    EndpointId GetEndpointId() const { return mEndpointId; }

    /// Register relevant clusters on the given endpoint
    virtual CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                EndpointId parentId = kInvalidEndpointId) = 0;

    /// Remove clusters from the given provider.
    ///
    /// Will only be called if register has succeeded before
    virtual void UnRegister(CodeDrivenDataModelProvider & provider) = 0;

    // Endpoint interface implementation
    CHIP_ERROR SemanticTags(ReadOnlyBufferBuilder<SemanticTag> & out) const override;
    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

protected:
    /// Internal registration function for common device clusters and endpoint registration.
    /// Subclasses are expected to call this
    CHIP_ERROR BaseRegistration(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId);

    EndpointId mEndpointId = kInvalidEndpointId;
    const DeviceType mDeviceType;
    EndpointInterfaceRegistration mEndpointRegistration;

    // Common clusters..
    LazyRegisteredServerCluster<Clusters::DescriptorCluster> mDescriptorCluster;
};

} // namespace chip::app
