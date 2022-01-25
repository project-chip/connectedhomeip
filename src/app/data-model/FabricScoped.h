/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/TypeTraits.h>
#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

/*
 * Check whether a cluster object struct is fabric scoped.
 *   A fabric scoped struct contains a field of "FabricIndex" type, however, we cannot tell the difference between that field and
 * other uint8_t fields. Thus we add a GetFabricIndex member function for getting the fabric id. Here, IsFabricScoped checks the
 * presence of GetFabricIndex function. This template can be used with std::enable_if.
 */
template <typename T>
class IsFabricScoped
{
private:
    template <typename Tp>
    static auto TestHasFabricIndex(int) -> TemplatedTrueType<decltype(&Tp::GetFabricIndex)>;

    template <typename Tp>
    static auto TestHasFabricIndex(long) -> std::false_type;

public:
    static constexpr bool value = decltype(TestHasFabricIndex<std::decay_t<T>>(0))::value;
};

} // namespace DataModel
} // namespace app
} // namespace chip
