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

/**
 * @brief Traverses a variadic argument pack to locate and return the first delegate deriving from TargetDelegate.
 *
 * Checks arguments in order using `std::is_base_of_v`. If none match, triggers a compile-time assertion.
 *
 * @tparam TargetDelegate The base delegate interface type to search for.
 * @param first The first delegate argument in the pack.
 * @param rest The remaining delegate arguments.
 * @return Reference to the matching delegate object.
 */
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

/**
 * @brief Unpacks constructor arguments and matching delegate to instantiate FeatureType.
 *
 * Invoked by MakeFeature with an index sequence covering the leading non-delegate arguments.
 * Extracts the required `FeatureType::Delegate` from the delegates tuple via `FindDelegate`.
 *
 * @tparam FeatureType The feature class to construct.
 * @param args Tuple containing all forwarded arguments (constructor arguments followed by delegates tuple).
 * @param delegates Tuple containing all available delegates.
 */
template <typename FeatureType, typename Args, typename... DelegateArgs, std::size_t... Index>
constexpr auto ConstructFeature(Args && args, const std::tuple<DelegateArgs &...> & delegates, std::index_sequence<Index...>)
{
    return std::apply(
        [&](auto &... dels) {
            return FeatureType(std::get<Index>(std::forward<Args>(args))..., FindDelegate<typename FeatureType::Delegate>(dels...));
        },
        delegates);
}

/**
 * @brief Conditionally constructs a feature handler object or std::monostate.
 *
 * Intended for use in cluster member initializer lists. Takes all constructor arguments for
 * `FeatureType` followed by a trailing `std::tuple` of delegates (e.g. via `std::forward_as_tuple`).
 *
 * If `Enabled` is true, automatically forwards the constructor arguments and searches the
 * delegates tuple for `FeatureType::Delegate` to instantiate `FeatureType`.
 * If `Enabled` is false, returns `std::monostate` to avoid runtime memory or execution overhead.
 *
 * @tparam Enabled Whether the feature is enabled at compile time.
 * @tparam FeatureType The feature class to instantiate if enabled.
 * @param args Leading constructor arguments for FeatureType, followed by a trailing std::tuple of delegates.
 */
template <bool Enabled, typename FeatureType, typename... Args>
static auto MakeFeature(Args &&... args)
{
    if constexpr (Enabled)
    {
        constexpr size_t N = sizeof...(Args);
        static_assert(N >= 1, "MakeFeature requires at least a delegate tuple");
        auto argsTuple = std::forward_as_tuple(std::forward<Args>(args)...);
        return ConstructFeature<FeatureType>(argsTuple, std::get<N - 1>(argsTuple), std::make_index_sequence<N - 1>{});
    }
    else
    {
        return std::monostate{};
    }
}

/**
 * @brief Conditionally constructs an AtomicWriteSession or std::monostate.
 *
 * If `Enabled` is true, returns an active `AtomicWriteSession`. If false, returns `std::monostate`.
 *
 * @tparam Enabled Whether atomic write support is required (e.g. when presets are enabled).
 */
template <bool Enabled>
static auto MakeAtomicWriteSession(AtomicWriteSession::Delegate & delegate, TimerDelegate & timerDelegate,
                                   FabricTable * fabricTable)
{
    if constexpr (Enabled)
    {
        return AtomicWriteSession(delegate, timerDelegate, fabricTable);
    }
    else
    {
        return std::monostate{};
    }
}

/**
 * @brief Compile-time predicate checking whether any type in Args derives from Target.
 *
 * Used to evaluate feature flags (e.g. `kHasPresets`, `kHasHold`) from the delegate pack.
 *
 * @tparam Target The target delegate base class.
 * @tparam Args Parameter pack of candidate delegate types.
 */
template <typename Target, typename... Args>
inline constexpr bool kArgsHasDelegate = (std::is_base_of_v<Target, std::decay_t<Args>> || ...);

} // namespace detail
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip
