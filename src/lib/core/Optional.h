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
#include <optional>
#include <utility>

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
class Optional
{
public:
    constexpr Optional() {}
    constexpr Optional(NullOptionalType) {}

    explicit Optional(const T & value) : mValue(value) {}

    template <class... Args>
    constexpr explicit Optional(InPlaceType, Args &&... args) : mValue(std::in_place, std::forward<Args>(args)...)
    {
    }

    constexpr Optional(const Optional & other) : mValue(other.mValue)
    {
    }

    // Converts an Optional of an implicitly convertible type
    template <class U, std::enable_if_t<!std::is_same_v<T, U> && std::is_convertible_v<const U, T>, bool> = true>
    constexpr Optional(const Optional<U> & other) : mValue(other.mValue)
    {
    }

    // Converts an Optional of a type that requires explicit conversion
    template <class U,
              std::enable_if_t<!std::is_same_v<T, U> && !std::is_convertible_v<const U, T> && std::is_constructible_v<T, const U &>,
                               bool> = true>
    constexpr explicit Optional(const Optional<U> & other) : mValue(other.mValue)
    {
    }

    constexpr Optional(Optional && other) : mValue(std::move(other.mValue))
    {
    }

    constexpr Optional & operator=(const Optional & other)
    {
        mValue = other.mValue;
        return *this;
    }

    constexpr Optional & operator=(Optional && other)
    {
        mValue = std::move(other.mValue);
        return *this;
    }

    /// Constructs the contained value in-place
    template <class... Args>
    constexpr T & Emplace(Args &&... args)
    {
        mValue.emplace(std::forward<Args>(args)...);
        return *mValue;
    }

    /** Make the optional contain a specific value */
    constexpr void SetValue(const T & value)
    {
        mValue = value;
    }

    /** Make the optional contain a specific value */
    constexpr void SetValue(T && value)
    {
        mValue = std::move(value);
    }

    /** Invalidate the value inside the optional. Optional now has no value */
    constexpr void ClearValue()
    {
        mValue = std::nullopt;
    }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    T & Value() &
    {
        VerifyOrDie(HasValue());
        return *mValue;
    }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    const T & Value() const &
    {
        VerifyOrDie(HasValue());
        return *mValue;
    }

    /** Gets the current value of the optional if the optional has a value;
        otherwise returns the provided default value. */
    const T & ValueOr(const T & defaultValue) const { return HasValue() ? Value() : defaultValue; }

    /** Checks if the optional contains a value or not */
    constexpr bool HasValue() const { return mValue.has_value(); }

    bool operator==(const Optional & other) const
    {
        return mValue == other.mValue;
    }
    bool operator!=(const Optional & other) const { return !(*this == other); }
    bool operator==(const T & other) const { return HasValue() && Value() == other; }
    bool operator!=(const T & other) const { return !(*this == other); }

    /** Convenience method to create an optional without a valid value. */
    static Optional<T> Missing() { return Optional<T>(); }

    /** Convenience method to create an optional containing the specified value. */
    template <class... Args>
    static Optional<T> Value(Args &&... args)
    {
        return Optional(InPlace, std::forward<Args>(args)...);
    }

private:
    std::optional<T> mValue;
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
