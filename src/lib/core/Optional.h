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

namespace chip {

/**
 * Pairs an object with a boolean value to determine if the object value
 * is actually valid or not.
 */
template <class T>
class Optional
{
public:
    Optional() : mHasValue(false) {}
    explicit Optional(const T & value) : mValue(value), mHasValue(true) {}

    constexpr Optional(const Optional & other) = default;
    constexpr Optional(Optional && other)      = default;

    /**
     * Assignment operator implementation.
     *
     * NOTE: Manually implemented instead of =default  since other::mValue may not be initialized
     * if it has no value.
     */
    Optional & operator=(const Optional & other)
    {
        if (other.HasValue())
        {
            SetValue(other.Value());
        }
        else
        {
            ClearValue();
        }
        return *this;
    }

    /** Make the optional contain a specific value */
    void SetValue(const T & value)
    {
        mValue    = value;
        mHasValue = true;
    }

    /** Invalidate the value inside the optional. Optional now has no value */
    void ClearValue() { mHasValue = false; }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    const T & Value() const
    {
        assert(HasValue());
        return mValue;
    }

    /** Gets the current value of the optional if the optional has a value;
        otherwise returns the provided default value. */
    const T & ValueOr(const T & defaultValue) const
    {
        if (HasValue())
        {
            return mValue;
        }
        return defaultValue;
    }

    /** Checks if the optional contains a value or not */
    bool HasValue() const { return mHasValue; }

    /** Comparison operator, handling missing values. */
    bool operator==(const Optional & other) const
    {
        return (mHasValue == other.mHasValue) && (!other.mHasValue || (mValue == other.mValue));
    }

    /** Comparison operator, handling missing values. */
    bool operator!=(const Optional & other) const { return !(*this == other); }

    /** Convenience method to create an optional without a valid value. */
    static Optional<T> Missing() { return Optional<T>(); }

    /** Convenience method to create an optional containing the specified value. */
    static Optional<T> Value(const T & value) { return Optional(value); }

private:
    T mValue;       ///< Value IFF optional contains a value
    bool mHasValue; ///< True IFF optional contains a value
};

} // namespace chip
