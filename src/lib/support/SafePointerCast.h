/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
