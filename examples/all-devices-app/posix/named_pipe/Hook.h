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

#include <posix/named_pipe/Dispatcher.h>
#include <type_traits>

#include <device/types/ambient-context-sensor/NamedPipeTranslators.h>
#include <device/types/boolean-state-sensor/NamedPipeTranslators.h>
#include <device/types/dimmable-light/NamedPipeTranslators.h>
#include <device/types/dimmable-plug-in-unit/NamedPipeTranslators.h>
#include <device/types/electrical-sensor/NamedPipeTranslators.h>
#include <device/types/mounted-dimmable-load-control/NamedPipeTranslators.h>
#include <device/types/mounted-on-off-control/NamedPipeTranslators.h>
#include <device/types/occupancy-sensor/NamedPipeTranslators.h>
#include <device/types/on-off-light/NamedPipeTranslators.h>
#include <device/types/on-off-plug-in-unit/NamedPipeTranslators.h>
#include <device/types/root-node/NamedPipeTranslators.h>

namespace chip::app::NamedPipe {

namespace detail {

template <typename T, typename = void>
struct HasNamedPipeTranslators : std::false_type
{
};

template <typename T>
struct HasNamedPipeTranslators<
    T, std::void_t<decltype(RegisterNamedPipeTranslators(std::declval<T &>(), std::declval<Dispatcher &>()))>> : std::true_type
{
};

} // namespace detail

class Hook
{
public:
    template <typename TDevice>
    static void OnDeviceRegistered(TDevice & device)
    {
        if constexpr (detail::HasNamedPipeTranslators<TDevice>::value)
        {
            RegisterNamedPipeTranslators(device, Dispatcher::Instance());
        }
    }
};

} // namespace chip::app::NamedPipe
