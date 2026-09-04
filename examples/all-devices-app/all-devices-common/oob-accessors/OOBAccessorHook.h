/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <oob-accessors/OOBAccessorRegistry.h>
#include <type_traits>
#include <utility>

#include <device/types/ambient-context-sensor/OOBAccessors.h>
#include <device/types/boolean-state-sensor/OOBAccessors.h>
#include <device/types/dimmable-light/OOBAccessors.h>
#include <device/types/dimmable-plug-in-unit/OOBAccessors.h>
#include <device/types/electrical-sensor/OOBAccessors.h>
#include <device/types/mounted-dimmable-load-control/OOBAccessors.h>
#include <device/types/mounted-on-off-control/OOBAccessors.h>
#include <device/types/occupancy-sensor/OOBAccessors.h>
#include <device/types/on-off-light/OOBAccessors.h>
#include <device/types/on-off-plug-in-unit/OOBAccessors.h>
#include <device/types/robotic-vacuum-cleaner/OOBAccessors.h>
#include <device/types/root-node/OOBAccessors.h>

namespace chip::app {

namespace detail {

template <typename T, typename = void>
struct HasOOBAccessors : std::false_type
{
};

template <typename T>
struct HasOOBAccessors<T, std::void_t<decltype(RegisterOOBAccessors(std::declval<T &>(), std::declval<OOBAccessorRegistry &>()))>>
    : std::true_type
{
};

} // namespace detail

class OOBAccessorHook
{
public:
    template <typename TDevice>
    static void OnDeviceRegistered(TDevice & device)
    {
        if constexpr (detail::HasOOBAccessors<TDevice>::value)
        {
            RegisterOOBAccessors(device, OOBAccessorRegistry::Instance());
        }
    }
};

} // namespace chip::app
