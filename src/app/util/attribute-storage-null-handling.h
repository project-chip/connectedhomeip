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

#include <lib/core/CHIPTLV.h>
#include <lib/support/TypeTraits.h>

#include <limits>

namespace chip {
namespace app {

template <typename T,
          bool IsBigEndian =
// BIGENDIAN_CPU to match how the attribute store works, because that's
// what where our data buffer is eventually ending up or coming from.
#if BIGENDIAN_CPU
              true
#else  // BIGENDIAN_CPU
              false
#endif // BIGENDIAN_CPU
          >
struct NumericAttributeTraits
{
    // StorageType is the type used to represent this C++ type in the attribute
    // store.
    using StorageType = T;

    // WorkingType is the type used to represent this C++ type when we are
    // actually working with it as a value.
    using WorkingType = T;

    // Convert a working value to a storage value.  This uses an outparam
    // instead of a return value because some specializations have complicated
    // StorageTypes that can't be returned by value.  This function can assume
    // that WorkingType passed a CanRepresentValue check.
    static constexpr void WorkingToStorage(WorkingType workingValue, StorageType & storageValue) { storageValue = workingValue; }

    // Convert a storage value to a working value.  Some specializations do more
    // interesting things here.
    static constexpr WorkingType StorageToWorking(StorageType storageValue) { return storageValue; }

private:
    // We need to make sure we never look like we are assigning NaN to an
    // integer, even in a not-reached branch.  Without "if constexpr", the best
    // we can do is these functions using enable_if.
    template <typename U = T, typename std::enable_if_t<std::is_floating_point<U>::value, int> = 0>
    static constexpr StorageType GetNullValue()
    {
        return std::numeric_limits<T>::quiet_NaN();
    }

    template <typename U = T, typename std::enable_if_t<std::is_integral<U>::value, int> = 0>
    static constexpr StorageType GetNullValue()
    {
        return std::is_signed<T>::value ? std::numeric_limits<T>::min() : std::numeric_limits<T>::max();
    }

    template <typename U = T, typename std::enable_if_t<std::is_enum<U>::value, int> = 0>
    static constexpr StorageType GetNullValue()
    {
        static_assert(!std::is_signed<std::underlying_type_t<T>>::value, "Enums must be unsigned");
        return static_cast<StorageType>(std::numeric_limits<std::underlying_type_t<T>>::max());
    }

public:
    // The value reserved in the value space of StorageType to represent null,
    // for cases when we have a nullable value.  This value must match the value
    // excluded from the valid value range in the spec, so that we don't confuse
    // valid values with null.
    static constexpr StorageType kNullValue = NumericAttributeTraits::GetNullValue();

    template <typename U = T, typename std::enable_if_t<!std::is_floating_point<U>::value, int> = 0>
    static constexpr bool IsNullValue(StorageType value)
    {
        return value == kNullValue;
    }

    template <typename U = T, typename std::enable_if_t<std::is_floating_point<U>::value, int> = 0>
    static constexpr bool IsNullValue(StorageType value)
    {
        // Trying to include math.h (to use isnan()) fails on EFR32, both when
        // included as "cmath" and when included as "math.h".  For lack of
        // isnan(), just fall back on the NaN != NaN thing.
        return value != value;
    }

    static constexpr void SetNull(StorageType & value) { value = kNullValue; }

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

    static CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, StorageType value)
    {
        return writer.Put(tag, static_cast<T>(value));
    }

    // Utility that lets consumers treat a StorageType instance as a uint8_t*
    // for writing to the attribute store.
    static uint8_t * ToAttributeStoreRepresentation(StorageType & value) { return reinterpret_cast<uint8_t *>(&value); }
};

template <>
struct NumericAttributeTraits<bool>
{
    using StorageType = uint8_t;
    using WorkingType = bool;

    static constexpr void WorkingToStorage(WorkingType workingValue, StorageType & storageValue) { storageValue = workingValue; }

    static constexpr WorkingType StorageToWorking(StorageType storageValue) { return storageValue; }

    static constexpr bool IsNullValue(StorageType value) { return value == kNullValue; }
    static constexpr void SetNull(StorageType & value) { value = kNullValue; }
    static constexpr bool CanRepresentValue(bool isNullable, StorageType value)
    {
        // This treats all nonzero values (except the null value) as true.
        return !IsNullValue(value);
    }
    static constexpr bool CanRepresentValue(bool isNullable, bool value) { return true; }

    static CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, StorageType value)
    {
        return writer.Put(tag, static_cast<bool>(value));
    }

    static uint8_t * ToAttributeStoreRepresentation(StorageType & value) { return reinterpret_cast<uint8_t *>(&value); }

private:
    static constexpr StorageType kNullValue = 0xFF;
};

} // namespace app
} // namespace chip
