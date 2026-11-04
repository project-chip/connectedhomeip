/*
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <device/api/Interface.h>
#include <functional>
#include <memory>

namespace chip::app {

/// Bundles an allocated device with its post-registration and pre-unregistration callbacks.
template <typename InterfaceT, typename = std::enable_if_t<std::is_base_of_v<DeviceInterface, InterfaceT>>>
struct DeviceRegistrationEntryOfConcreteType
{
    std::unique_ptr<InterfaceT> device;
    std::function<void()> afterDeviceRegistration;
    std::function<void()> beforeDeviceUnregistration;
    Span<const EndpointComposition::SemanticTag> defaultTags = {};

    DeviceRegistrationEntryOfConcreteType() = default;

    DeviceRegistrationEntryOfConcreteType(std::unique_ptr<InterfaceT> dev, std::function<void()> afterRegistration,
                                          std::function<void()> beforeUnregistration,
                                          Span<const EndpointComposition::SemanticTag> tags = {}) :
        device(std::move(dev)),
        afterDeviceRegistration(std::move(afterRegistration)), beforeDeviceUnregistration(std::move(beforeUnregistration)),
        defaultTags(tags)
    {}

    template <typename OtherInterfaceT, typename = std::enable_if_t<std::is_base_of_v<InterfaceT, OtherInterfaceT>>>
    explicit DeviceRegistrationEntryOfConcreteType(DeviceRegistrationEntryOfConcreteType<OtherInterfaceT> && other) :
        DeviceRegistrationEntryOfConcreteType(std::move(other.device), std::move(other.afterDeviceRegistration),
                                              std::move(other.beforeDeviceUnregistration), other.defaultTags)
    {}

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {})
    {
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);
        if (composition.tagList.empty() && !defaultTags.empty())
        {
            composition.tagList = defaultTags;
        }
        ReturnErrorOnFailure(static_cast<DeviceInterface *>(device.get())->Register(allocator, provider, composition));

        if (afterDeviceRegistration)
        {
            afterDeviceRegistration();
        }

        return CHIP_NO_ERROR;
    }

    void Unregister(CodeDrivenDataModelProvider & provider)
    {
        VerifyOrReturn(device != nullptr);

        if (beforeDeviceUnregistration)
        {
            beforeDeviceUnregistration();
        }
        static_cast<DeviceInterface *>(device.get())->Unregister(provider);
    }
};

using DeviceRegistrationEntry = DeviceRegistrationEntryOfConcreteType<DeviceInterface>;

} // namespace chip::app
