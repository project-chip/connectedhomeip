/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <type_traits>

namespace chip {

/**
 * A template function that casts a pointer of type From* to a pointer of type To*
 * and verifies that both From and To are trivially copyable structures, size of
 * To is not greater than size of From, and To doesn't have more strict alignment
 * requirements than From. That is to make sure that access to fields of To is safe.
 */
template <class ToPtr, class From>
std::enable_if_t<std::is_pointer<ToPtr>::value, ToPtr> SafePointerCast(From * from)
{
    using To = std::remove_pointer_t<ToPtr>;
    static_assert(std::is_trivially_copyable<From>(), "Casting from a non-trivially copyable type");
    static_assert(std::is_trivially_copyable<To>(), "Casting to a non-trivially copyable type");
    static_assert(sizeof(From) >= sizeof(To), "Casting to a bigger type");
    static_assert(alignof(From) >= alignof(To), "Casting to a type with more strict alignment requirements");
    return reinterpret_cast<ToPtr>(from);
}

} // namespace chip
