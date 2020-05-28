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
    Optional & operator=(const Optional & other) = default;

    void SetValue(const T & value)
    {
        mValue    = value;
        mHasValue = true;
    }

    void ClearValue(void) { mHasValue = false; }

    const T & Value(void) const
    {
        assert(HasValue());
        return mValue;
    }
    bool HasValue() const { return mHasValue; }

    bool operator==(const Optional & other) const
    {
        return (mHasValue == other.mHasValue) && (!mHasValue || (mValue == other.mValue));
    }

    static Optional<T> Missing(void) { return Optional<T>(); }
    static Optional<T> Value(const T & value) { return Optional(value); }

private:
    T mValue;
    bool mHasValue;
};

} // namespace chip
