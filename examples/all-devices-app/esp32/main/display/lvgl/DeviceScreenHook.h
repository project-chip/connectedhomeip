/*
 *
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

#include "DeviceScreenRegistration.h"
#include "DeviceScreenRegistry.h"

#include <type_traits>
#include <utility>

namespace chip::app {

namespace detail {

template <typename T, typename = void>
struct HasDeviceScreen : std::false_type
{
};

template <typename T>
struct HasDeviceScreen<T, std::void_t<decltype(RegisterDeviceScreen(std::declval<T &>(), std::declval<DeviceScreenRegistry &>()))>>
    : std::true_type
{
};

} // namespace detail

class DeviceScreenHook
{
public:
    template <typename TDevice>
    static void OnDeviceRegistered(TDevice & device)
    {
        if constexpr (detail::HasDeviceScreen<TDevice>::value)
        {
            RegisterDeviceScreen(device, DeviceScreenRegistry::Instance());
        }
        else
        {
            RegisterMissingDeviceScreen(device, device.GetEndpointId(), DeviceScreenRegistry::Instance());
        }
    }
};

} // namespace chip::app
