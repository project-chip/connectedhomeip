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
#include <devices/interface/DeviceInterface.h>

#include <string>

namespace chip::app {

/// A device is a entity that maintains some cluster functionality.
///
/// This implementation assumes that a device is registered on a single
/// endpoint.
class SingleEndpointDevice : public DeviceInterface
{
public:
    virtual ~SingleEndpointDevice() = default;

    EndpointId GetEndpointId() const { return mEndpointId; }

protected:
    /// The caller creating a SingleEndpointDevice MUST ensure that the underlying data for the Span of
    /// deviceTypes remains valid for the entire lifetime of the SingleEndpointDevice object instance.
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry> deviceTypes) : DeviceInterface(deviceTypes) {}

    /// Internal registration function for common device clusters and endpoint registration.
    /// Device subclasses are expected to, and must only call this as part of their own device specific Register()
    /// functions. This allows creation of common and device-specific clusters to be done together and
    /// also to complete endpoint registration.
    CHIP_ERROR SingleEndpointRegistration(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId);

    /// Internal function to unregister a single endpoint device. This will destroy the clusters part of
    /// this class, and must be called in a subclass' device-specific UnRegister() function. This allows
    /// for the destruction of the general SingleEndpointDevice clusters and device-specifc clusters from
    /// the subclass, as well as removal of the device endpoint from the provider to happen together.
    void SingleEndpointUnregistration(CodeDrivenDataModelProvider & provider);

    /// A default value of kInvalidEndpointId is used for the endpoint ID. When this is the value of the endpoint ID,
    /// it signifies that endpoint registration (and cluster creation) has not yet been completed. The endpoint
    /// ID will be set after a call to SingleEndpointRegistration() has been made.
    EndpointId mEndpointId = kInvalidEndpointId;
};

} // namespace chip::app
