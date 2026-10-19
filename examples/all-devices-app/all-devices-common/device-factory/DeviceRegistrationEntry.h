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
struct DeviceRegistrationEntry
{
    std::unique_ptr<DeviceInterface> device;
    std::function<void()> afterDeviceRegistration;
    std::function<void()> beforeDeviceUnregistration;

    CHIP_ERROR Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {})
    {
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(device->Register(allocator, provider, composition));

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
        device->Unregister(provider);
    }
};

} // namespace chip::app
