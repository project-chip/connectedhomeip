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

#include <assert.h>

#include <lib/support/Variant.h>

namespace chip {

/**
 * Pairs an object with a boolean value to determine if the object value
 * is actually valid or not.
 */
template <class T>
class Optional
{
public:
    constexpr Optional() {
        mVariant.template Set<None>();
    }
    explicit Optional(const T & value) {
        mVariant.template Set<Some>(value);
    }

    constexpr Optional(const Optional & other) = default;
    constexpr Optional(Optional && other)      = default;
    constexpr Optional & operator=(const Optional & other) = default;

    /** Make the optional contain a specific value */
    constexpr void SetValue(const T & value)
    {
        mVariant.template Set<Some>(value);
    }

    /** Invalidate the value inside the optional. Optional now has no value */
    constexpr void ClearValue() {
        mVariant.template Set<None>();
    }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    const T & Value() const
    {
        assert(HasValue());
        return mVariant.template Get<Some>().mValue;
    }

    /** Gets the current value of the optional if the optional has a value;
        otherwise returns the provided default value. */
    const T & ValueOr(const T & defaultValue) const
    {
        return HasValue() ? Value() : defaultValue;
    }

    /** Checks if the optional contains a value or not */
    constexpr bool HasValue() const {
        return !mVariant.template Is<None>();
    }

    bool operator==(const Optional & other) const { return mVariant == other.mVariant; }
    bool operator!=(const Optional & other) const { return !(*this == other); }

    /** Convenience method to create an optional without a valid value. */
    static Optional<T> Missing() { return Optional<T>(); }

    /** Convenience method to create an optional containing the specified value. */
    static Optional<T> Value(const T & value) { return Optional(value); }

private:
    struct None
    {
        static constexpr const std::size_t VariantId = 1;
        bool operator==(const None &) const { return true; }
    };

    struct Some
    {
        static constexpr const std::size_t VariantId = 2;
        Some(const T & value) : mValue(value) {}
        bool operator==(const Some & other) const { return mValue == other.mValue; }
        T mValue;
    };

    Variant<None, Some> mVariant;
};

} // namespace chip
