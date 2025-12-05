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

#include <app/clusters/descriptor/DescriptorCluster.h>
#include <app/util/basic-types.h>
#include <clusters/Descriptor/Structs.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>

#include <string>

namespace chip::app {

/// A device is an entity that maintains some cluster functionality.
class DeviceInterface : public EndpointInterface
{
public:
    virtual ~DeviceInterface() = default;

    /// Register relevant clusters on the given endpoint. This must only
    /// be called once after starting up a device for the first time. This function
    /// will create/instantiate all clusters on the device and complete endpoint registration.
    /// It should return error if there's any failure when adding the device's clusters to the provider.
    /// A parentId of kInvalidEndpointId represents that there is no parent to this device
    virtual CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                EndpointId parentId = kInvalidEndpointId) = 0;

    /// Removes a device's clusters from the given provider. This
    /// must only be called when register has succeeded before. Expected
    /// usage of this function is for when the device is no longer needed
    /// (for example, on shutown), to destory the device's clusters.
    virtual void UnRegister(CodeDrivenDataModelProvider & provider) = 0;

    // Endpoint interface implementation
    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override;
    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

protected:
    /// The caller creating a DeviceInterface MUST ensure that the underlying data for the Span of
    /// deviceTypes remains valid for the entire lifetime of the DeviceInterface object instance.
    DeviceInterface(Span<const DataModel::DeviceTypeEntry> deviceTypes) :
        mDeviceTypes(deviceTypes), mEndpointRegistration(*this, {})
    {}

    Span<const DataModel::DeviceTypeEntry> mDeviceTypes;
    EndpointInterfaceRegistration mEndpointRegistration;

    // Common clusters..
    LazyRegisteredServerCluster<Clusters::DescriptorCluster> mDescriptorCluster;
};

} // namespace chip::app
