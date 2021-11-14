/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <limits>

namespace chip {
namespace app {

template <typename T>
struct NumericAttributeTraits
{
    // StorageType is the type used to represent this C++ type in the attribute
    // store.
    using StorageType = T;

    // The value reserved in the value space of StorageType to represent null,
    // for cases when we have a nullable value.  This value must match the value
    // excluded from the valid value range in the spec, so that we don't confuse
    // valid values with null.
    static constexpr StorageType kNullValue =
        std::is_signed<T>::value ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();

    static constexpr bool IsNullValue(StorageType value) { return value == kNullValue; }

    // Test whether a value can be represented in a "not null" value of the
    // given type, which may be a nullable value or not.  This needs to be
    // implemented for both T and StorageType if the two are distinct.
    static constexpr bool CanRepresentValue(bool isNullable, T value)
    {
        // For now, allow the null-marker value for non-nullable types.  It's
        // not what the spec says to do at the moment, but that might well
        // change, and we have quite a number of tests relying on this behavior
        // for now that we should only change once the spec really decides what
        // it's doing.
        return !isNullable || !IsNullValue(value);
    }
};

template <>
struct NumericAttributeTraits<bool>
{
    using StorageType                       = uint8_t;
    static constexpr StorageType kNullValue = 0xFF;
    static constexpr bool IsNullValue(StorageType value) { return value == kNullValue; }
    static constexpr bool CanRepresentValue(bool isNullable, StorageType value)
    {
        // This treats all nonzero values (except the null value) as true.
        return !IsNullValue(value);
    }
    static constexpr bool CanRepresentValue(bool isNullable, bool value) { return true; }
};

} // namespace app
} // namespace chip
