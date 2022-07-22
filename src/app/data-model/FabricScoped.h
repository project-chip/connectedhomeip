/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/data-model/BasicTypes.h>

#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

/**
 * IsFabricScoped checks whether the given type is fabric scoped.  Non-basic types (i.e. cluster objects), should provide a
 * static constexpr indicating whether the type is a fabric scoped struct, and basic types will never be fabric scoped.
 *
 * Using IsFabricScoped<X>::value without a basic type or cluster object with kIsFabricScoped will cause a compile error.  This is
 * an intended behavior to make users explicitly decide whether their cluster object is fabric-scoped or not.
 */
template <typename X>
class IsFabricScoped
{
private:
    template <typename X0, decltype(std::decay_t<X0>::kIsFabricScoped) = true>
    static constexpr bool IsFabricScopedClusterObject()
    {
        return std::decay_t<X0>::kIsFabricScoped;
    }

    template <typename X0, std::enable_if_t<IsBasicType<std::decay_t<X0>>::value, bool> = true>
    static constexpr bool IsFabricScopedClusterObject()
    {
        return false;
    }

public:
    static constexpr bool value = IsFabricScopedClusterObject<X>();
};

} // namespace DataModel
} // namespace app
} // namespace chip
