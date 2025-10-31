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

#include <app/clusters/descriptor/descriptor-cluster.h>
#include <app/util/basic-types.h>
#include <clusters/Descriptor/Structs.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <devices/base-device/Device.h>

#include <string>

namespace chip::app {

/// A device is a entity that maintains some cluster functionality.
///
/// Current implementation assumes that a device is registered on a single
/// endpoint.
class SingleEndpointDevice : public BaseDevice
{
public:
    virtual ~SingleEndpointDevice() = default;

    EndpointId GetEndpointId() const { return mEndpointId; }

    /// Register relevant clusters on the given endpoint
    virtual CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                EndpointId parentId = kInvalidEndpointId) = 0;

    /// Remove clusters from the given provider.
    ///
    /// Will only be called if register has succeeded before
    virtual void UnRegister(CodeDrivenDataModelProvider & provider) = 0;

protected:
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes) : BaseDevice(deviceTypes) {}

    /// Internal registration function for common device clusters and endpoint registration.
    /// Subclasses are expected to call this
    CHIP_ERROR SingleEndpointRegistration(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId);

    EndpointId mEndpointId = kInvalidEndpointId;

    // Common clusters..
    LazyRegisteredServerCluster<Clusters::DescriptorCluster> mDescriptorCluster;
};

} // namespace chip::app
