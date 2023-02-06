/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/util/attribute-storage-null-handling.h>
#include <lib/core/Optional.h>

#include <type_traits>

namespace chip {
namespace app {
namespace DataModel {

/**
 * NullNullable is an alias for NullOptional, for better readability.
 */
constexpr auto NullNullable = NullOptional;

/*
 * Dedicated type for nullable things, to differentiate them from optional
 * things.
 */
template <typename T>
struct Nullable : protected Optional<T>
{
    //
    // The following 'using' statement is needed to make visible
    // all constructors of the base class within this derived class.
    //
    using Optional<T>::Optional;

    // Pull in APIs that make sense on Nullable with the same names as on
    // Optional.
    using Optional<T>::Value;

    // Some consumers need an easy way to determine our underlying type.
    using UnderlyingType = T;

    constexpr void SetNull() { Optional<T>::ClearValue(); }
    constexpr bool IsNull() const { return !Optional<T>::HasValue(); }

    template <class... Args>
    constexpr T & SetNonNull(Args &&... args)
    {
        return Optional<T>::Emplace(std::forward<Args>(args)...);
    }

    // For integer types, being nullable involves a range restriction.
    template <
        typename U = std::decay_t<T>,
        typename std::enable_if_t<(std::is_integral<U>::value && !std::is_same<U, bool>::value) || std::is_enum<U>::value, int> = 0>
    constexpr bool HasValidValue() const
    {
        return NumericAttributeTraits<T>::CanRepresentValue(/* isNullable = */ true, Value());
    }

    // For all other types, all values are valid.
    template <typename U                     = std::decay_t<T>,
              typename std::enable_if_t<(!std::is_integral<U>::value || std::is_same<U, bool>::value) && !std::is_enum<U>::value,
                                        int> = 0>
    constexpr bool HasValidValue() const
    {
        return true;
    }

    // The only fabric-scoped objects in the spec are commands, events and structs inside lists, and none of those can be nullable.
    static constexpr bool kIsFabricScoped = false;

    bool operator==(const Nullable & other) const { return Optional<T>::operator==(other); }
    bool operator!=(const Nullable & other) const { return Optional<T>::operator!=(other); }
    bool operator==(const T & other) const { return Optional<T>::operator==(other); }
    bool operator!=(const T & other) const { return Optional<T>::operator!=(other); }
};

template <class T>
constexpr Nullable<std::decay_t<T>> MakeNullable(T && value)
{
    return Nullable<std::decay_t<T>>(InPlace, std::forward<T>(value));
}

template <class T, class... Args>
constexpr Nullable<T> MakeNullable(Args &&... args)
{
    return Nullable<T>(InPlace, std::forward<Args>(args)...);
}

} // namespace DataModel
} // namespace app
} // namespace chip
