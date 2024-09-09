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

#include <app/util/attribute-storage-null-handling.h>
#include <lib/support/TypeTraits.h>

#include <limits>

namespace chip {
namespace app {

namespace detail {
template <int ByteSize, bool IsSigned>
struct WorkingTypeMapper
{
};
template <int ByteSize>
struct WorkingTypeMapper<ByteSize, true>
{
    using WorkingType = int64_t;
};
template <>
struct WorkingTypeMapper<3, true>
{
    using WorkingType = int32_t;
};
template <int ByteSize>
struct WorkingTypeMapper<ByteSize, false>
{
    using WorkingType = uint64_t;
};
template <>
struct WorkingTypeMapper<3, false>
{
    using WorkingType = uint32_t;
};
} // namespace detail

template <int ByteSize, bool IsSigned>
struct OddSizedInteger
{
    // WorkingType is the type we use to represent the value as an actual
    // integer that we can do arithmetic, greater/less-than compares, etc on.
    using WorkingType = typename detail::WorkingTypeMapper<ByteSize, IsSigned>::WorkingType;

    // StorageType is the type "at rest" in the attribute store.  It's a
    // native-endian byte buffer.
    using StorageType = uint8_t[ByteSize];
};

namespace detail {
template <int ByteSize, bool IsBigEndian>
struct IntegerByteIndexing;

template <int ByteSize>
struct IntegerByteIndexing<ByteSize, true>
{
    static constexpr int highIndex     = 0;
    static constexpr int lowIndex      = ByteSize - 1;
    static constexpr int lowerIndex    = 1;
    static constexpr int raiseIndex    = -1;
    static constexpr int pastLowIndex  = ByteSize;
    static constexpr int pastHighIndex = -1;
};

template <int ByteSize>
struct IntegerByteIndexing<ByteSize, false>
{
    static constexpr int highIndex     = ByteSize - 1;
    static constexpr int lowIndex      = 0;
    static constexpr int lowerIndex    = -1;
    static constexpr int raiseIndex    = 1;
    static constexpr int pastLowIndex  = -1;
    static constexpr int pastHighIndex = ByteSize;
};
} // namespace detail

template <int ByteSize, bool IsSigned, bool IsBigEndian>
struct NumericAttributeTraits<OddSizedInteger<ByteSize, IsSigned>, IsBigEndian> : detail::IntegerByteIndexing<ByteSize, IsBigEndian>
{
    using IntType = OddSizedInteger<ByteSize, IsSigned>;
    // StorageType is the type "at rest" in the attribute store.  It's a
    // native-endian byte buffer.
    using StorageType = typename IntType::StorageType;
    // WorkingType is the type we use to represent the value as an actual
    // integer that we can do arithmetic, greater/less-than compares, etc on.
    using WorkingType = typename IntType::WorkingType;

    using Indexing = detail::IntegerByteIndexing<ByteSize, IsBigEndian>;
    using Indexing::highIndex;
    using Indexing::lowerIndex;
    using Indexing::lowIndex;
    using Indexing::pastHighIndex;
    using Indexing::pastLowIndex;
    using Indexing::raiseIndex;

    static constexpr WorkingType StorageToWorking(StorageType storageValue)
    {
        // WorkingType can always fit all of our bit-shifting, because it has at
        // least one extra byte.
        WorkingType value = 0;
        for (int i = highIndex; i != pastLowIndex; i += lowerIndex)
        {
            value = (value << 8) | storageValue[i];
        }

        // If unsigned, we are done.  If signed, we need to make sure our high
        // bit gets treated as a sign bit, not a value bit, with our bits in 2s
        // complement.
        if (IsSigned)
        {
            constexpr WorkingType MaxPositive = (static_cast<WorkingType>(1) << (8 * ByteSize - 1)) - 1;
            if (value > MaxPositive)
            {
                value = value - (static_cast<WorkingType>(1) << (8 * ByteSize));
            }
        }

        return value;
    }

    static constexpr void WorkingToStorage(WorkingType workingValue, StorageType & storageValue)
    {
        // We can just grab the low ByteSize bytes of workingValue.
        for (int i = lowIndex; i != pastHighIndex; i += raiseIndex)
        {
            // Casting to uint8_t exactly grabs the lowest byte.
            storageValue[i] = static_cast<uint8_t>(workingValue);
            workingValue    = workingValue >> 8;
        }
    }

    static constexpr bool IsNullValue(StorageType value)
    {
        if (IsSigned)
        {
            // Check for the equivalent of the most negative integer, in 2s
            // complement notation.
            if (value[highIndex] != 0x80)
            {
                return false;
            }
            for (int i = highIndex + lowerIndex; i != pastLowIndex; i += lowerIndex)
            {
                if (value[i] != 0x00)
                {
                    return false;
                }
            }
            return true;
        }

        // Check for the equivalent of the largest unsigned integer.
        for (int i = highIndex; i != pastLowIndex; i += lowerIndex)
        {
            if (value[i] != 0xFF)
            {
                return false;
            }
        }
        return true;
    }

    static constexpr void SetNull(StorageType & value)
    {
        if (IsSigned)
        {
            value[highIndex] = 0x80;
            for (int i = highIndex + lowerIndex; i != pastLowIndex; i += lowerIndex)
            {
                value[i] = 0x00;
            }
        }
        else
        {
            for (int i = highIndex; i != pastLowIndex; i += lowerIndex)
            {
                value[i] = 0xFF;
            }
        }
    }

    static constexpr bool CanRepresentValue(bool isNullable, StorageType value) { return !isNullable || !IsNullValue(value); }

    static constexpr bool CanRepresentValue(bool isNullable, WorkingType value)
    {
        return MinValue(isNullable) <= value && value <= MaxValue(isNullable);
    }

    static CHIP_ERROR Encode(TLV::TLVWriter & writer, TLV::Tag tag, StorageType value)
    {
        return writer.Put(tag, StorageToWorking(value));
    }

    static uint8_t * ToAttributeStoreRepresentation(StorageType & value) { return value; }

    static WorkingType MinValue(bool isNullable)
    {
        if constexpr (!IsSigned)
        {
            return 0;
        }

        // Since WorkingType has at least one extra byte, the bitshift cannot overflow.
        constexpr WorkingType signedMin = -(static_cast<WorkingType>(1) << (8 * ByteSize - 1));
        if (isNullable)
        {
            // Smallest negative value is excluded for nullable signed types.
            return signedMin + 1;
        }

        return signedMin;
    }

    static WorkingType MaxValue(bool isNullable)
    {
        // Since WorkingType has at least one extra byte, none of our bitshifts
        // overflow.
        if constexpr (IsSigned)
        {
            return (static_cast<WorkingType>(1) << (8 * ByteSize - 1)) - 1;
        }

        constexpr WorkingType unsignedMax = (static_cast<WorkingType>(1) << (8 * ByteSize)) - 1;
        if (isNullable)
        {
            // Largest value is excluded for nullable unsigned types.
            return unsignedMax - 1;
        }

        return unsignedMax;
    }
};

} // namespace app
} // namespace chip
