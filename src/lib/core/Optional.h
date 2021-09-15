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

#include <lib/core/CHIPCore.h>
#include <lib/core/InPlace.h>
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
    constexpr Optional() : mHasValue(false) {}
    ~Optional()
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
    }

    explicit Optional(const T & value) : mHasValue(true) { new (&mValue.mData) T(value); }

    template <class... Args>
    constexpr explicit Optional(InPlaceType, Args &&... args) : mHasValue(true)
    {
        new (&mValue.mData) T(std::forward<Args>(args)...);
    }

    constexpr Optional(const Optional & other) : mHasValue(other.mHasValue)
    {
        if (mHasValue)
        {
            new (&mValue.mData) T(other.mValue.mData);
        }
    }

    constexpr Optional(Optional && other) : mHasValue(other.mHasValue)
    {
        if (mHasValue)
        {
            new (&mValue.mData) T(std::move(other.mValue.mData));
            other.mValue.mData.~T();
            other.mHasValue = false;
        }
    }

    constexpr Optional & operator=(const Optional & other)
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
        mHasValue = other.mHasValue;
        if (mHasValue)
        {
            new (&mValue.mData) T(other.mValue.mData);
        }
        return *this;
    }

    constexpr Optional & operator=(Optional && other)
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
        mHasValue = other.mHasValue;
        if (mHasValue)
        {
            new (&mValue.mData) T(std::move(other.mValue.mData));
            other.mValue.mData.~T();
            other.mHasValue = false;
        }
        return *this;
    }

    /// Constructs the contained value in-place
    template <class... Args>
    constexpr T & Emplace(Args &&... args)
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
        mHasValue = true;
        new (&mValue.mData) T(std::forward<Args>(args)...);
        return mValue.mData;
    }

    /** Make the optional contain a specific value */
    constexpr void SetValue(const T & value)
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
        mHasValue = true;
        new (&mValue.mData) T(value);
    }

    /** Make the optional contain a specific value */
    constexpr void SetValue(T && value)
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
        mHasValue = true;
        new (&mValue.mData) T(std::move(value));
    }

    /** Invalidate the value inside the optional. Optional now has no value */
    constexpr void ClearValue()
    {
        if (mHasValue)
        {
            mValue.mData.~T();
        }
        mHasValue = false;
    }

    /** Gets the current value of the optional. Valid IFF `HasValue`. */
    const T & Value() const
    {
        VerifyOrDie(HasValue());
        return mValue.mData;
    }

    /** Gets the current value of the optional if the optional has a value;
        otherwise returns the provided default value. */
    const T & ValueOr(const T & defaultValue) const { return HasValue() ? Value() : defaultValue; }

    /** Checks if the optional contains a value or not */
    constexpr bool HasValue() const { return mHasValue; }

    bool operator==(const Optional & other) const
    {
        return (mHasValue == other.mHasValue) && (!other.mHasValue || (mValue.mData == other.mValue.mData));
    }
    bool operator!=(const Optional & other) const { return !(*this == other); }

    /** Convenience method to create an optional without a valid value. */
    static Optional<T> Missing() { return Optional<T>(); }

    /** Convenience method to create an optional containing the specified value. */
    template <class... Args>
    static Optional<T> Value(Args &&... args)
    {
        return Optional(InPlace, std::forward<Args>(args)...);
    }

private:
    bool mHasValue;
    union Value
    {
        Value() {}
        ~Value() {}
        T mData;
    } mValue;
};

template <class T, class... Args>
constexpr Optional<T> MakeOptional(Args &&... args)
{
    return Optional<T>(InPlace, std::forward<Args>(args)...);
}

} // namespace chip
