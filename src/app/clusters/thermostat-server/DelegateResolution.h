/**
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

#include "ThermostatClusterAtomic.h"
#include <credentials/FabricTable.h>
#include <lib/support/TimerDelegate.h>

#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace detail {

template <typename TargetDelegate, typename First, typename... Rest>
constexpr decltype(auto) FindDelegate(First && first, Rest &&... rest)
{
    if constexpr (std::is_base_of_v<TargetDelegate, std::decay_t<First>>)
    {
        return std::forward<First>(first);
    }
    else if constexpr (sizeof...(Rest) > 0)
    {
        return FindDelegate<TargetDelegate>(std::forward<Rest>(rest)...);
    }
    else
    {
        static_assert(std::is_base_of_v<TargetDelegate, std::decay_t<First>>,
                      "Exhausted list of delegates without finding target delegate");
    }
}

template <bool Enabled, typename FeatureType, typename... DelegateArgs, typename... ExtraArgs>
static auto MakeFeature(const std::tuple<DelegateArgs &...> & delegates, ExtraArgs &&... extraArgs)
{
    if constexpr (Enabled)
    {
        return std::apply(
            [&](auto &... dels) {
                return FeatureType(std::forward<ExtraArgs>(extraArgs)..., FindDelegate<typename FeatureType::Delegate>(dels...));
            },
            delegates);
    }
    else
    {
        return std::monostate{};
    }
}

template <bool Enabled>
static auto MakeAtomicWriteSession(AtomicWriteSession::Delegate & delegate, TimerDelegate & timerDelegate,
                                   FabricTable * fabricTable)
{
    if constexpr (Enabled)
    {
        return AtomicWriteSession(delegate, timerDelegate, *fabricTable);
    }
    else
    {
        return std::monostate{};
    }
}

template <typename Target, typename... Args>
inline constexpr bool kArgsHasDelegate = (std::is_base_of_v<Target, std::decay_t<Args>> || ...);

} // namespace detail
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
