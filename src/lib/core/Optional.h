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

/**
 *    @file
 *      This file defines the chip::Optional class to handle values which may
 *      or may not be present.
 *
 */
#pragma once

#include <new>
#include <type_traits>
#include <utility>
#include <optional>

#include <lib/core/InPlace.h>
#include <lib/support/CodeUtils.h>


namespace chip {

/// An empty class type used to indicate optional type with uninitialized state.
struct NullOptionalType
{
    explicit NullOptionalType() = default;
};
inline constexpr NullOptionalType NullOptional{};

/**
 * Pairs an object with a boolean value to determine if the object value
 * is actually valid or not.
 */
template <class T>
class Optional : public std::optional<T>
{
public:
    using std::optional<T>::optional; // bring in all constructors

    constexpr Optional(NullOptionalType) : std::optional<T>(std::nullopt) {}

    /// Constructs the contained value in-place
    template <class... Args>
    constexpr T & Emplace(Args &&... args)
    {
        return emplace(std::forward<Args>(args)...);
    }

    /** Make the optional contain a specific value */
    constexpr void SetValue(const T & value)
    {
        *this = value;
    }

    /** Make the optional contain a specific value */
    constexpr void SetValue(T && value)
    {
        *this = std::move(value);
    }

    /** Invalidate the value inside the optional. Optional now has no value */
    constexpr void ClearValue()
    {
        *this = std::nullopt;
    }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    T & Value() &
    {
        return ((std::optional<T> *)this)->value();
    }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    const T & Value() const &
    {
        VerifyOrDie(HasValue());
        return ((std::optional<T> *)this)->value();
    }

    /** Gets the current value of the optional if the optional has a value;
        otherwise returns the provided default value. */
    const T & ValueOr(const T & defaultValue) const { return value_or(defaultValue); }

    /** Checks if the optional contains a value or not */
    constexpr bool HasValue() const {
        return ((std::optional<T> *)this)->has_value();
    }

    /** Convenience method to create an optional without a valid value. */
    static Optional<T> Missing() { return Optional<T>(); }

    /** Convenience method to create an optional containing the specified value. */
    template <class... Args>
    static Optional<T> Value(Args &&... args)
    {
        return Optional(std::in_place, std::forward<Args>(args)...);
    }
};

template <class T>
constexpr Optional<std::decay_t<T>> MakeOptional(T && value)
{
    return Optional<std::decay_t<T>>(InPlace, std::forward<T>(value));
}

template <class T, class... Args>
constexpr Optional<T> MakeOptional(Args &&... args)
{
    return Optional<T>(InPlace, std::forward<Args>(args)...);
}

} // namespace chip
