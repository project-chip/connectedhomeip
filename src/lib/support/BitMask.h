/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/BitFlags.h>

namespace chip {

/**
 * Stores bit masks and flags in a more typesafe manner.
 *
 * Extends BitFlags for boolean flags and also adds support for bit-mask set options.
 *
 */
template <typename FlagsEnum, typename StorageType = typename std::underlying_type_t<FlagsEnum>>
class BitMask : public BitFlags<FlagsEnum, StorageType>
{
public:
    using IntegerType = typename BitFlags<FlagsEnum, StorageType>::IntegerType;

    constexpr BitMask() : BitFlags<FlagsEnum, StorageType>() {}
    constexpr BitMask(const BitFlags<FlagsEnum, StorageType> & other) : BitFlags<FlagsEnum, StorageType>(other) {}
    constexpr BitMask(BitFlags<FlagsEnum, StorageType> && other) : BitFlags<FlagsEnum, StorageType>(std::move(other)) {}
    BitMask(const BitMask &) = default;

    explicit BitMask(FlagsEnum value) : BitFlags<FlagsEnum, StorageType>(value) {}
    explicit BitMask(IntegerType value) : BitFlags<FlagsEnum, StorageType>(value) {}

    template <typename... Args>
    constexpr BitMask(FlagsEnum flag, Args &&... args) : BitFlags<FlagsEnum, StorageType>(flag, std::forward<Args>(args)...)
    {}

    template <typename... Args>
    constexpr BitMask(IntegerType value, Args &&... args) : BitFlags<FlagsEnum, StorageType>(value, std::forward<Args>(args)...)
    {}

    BitMask & operator=(const BitMask &) = default;

    BitMask & operator=(const BitFlags<FlagsEnum, StorageType> & other)
    {
        BitFlags<FlagsEnum, StorageType>::SetRaw(other.Raw());
        return *this;
    }

    BitMask & operator=(FlagsEnum value)
    {
        BitFlags<FlagsEnum, StorageType>::SetRaw(static_cast<StorageType>(value));
        return *this;
    }

    /**
     * Set a field within the bit mask integer.
     *
     * Fields are assumed to be contiguous bits within the underlying integer, for example:
     * A mask of 0x70 == 0b01110000 means that a 3 bit value is encoded at bits [4;6].
     *
     * Calling SetField(0x70, n) is equivalent to "(value & ~0x70) | (n << 4)"
     *
     * @param mask  The mask, code assumes a continous bit mask
     * @param value The value, NOT shifted, MUST fit within the number of bits of the mask
     */
    constexpr BitMask & SetField(FlagsEnum mask, IntegerType value)
    {
        IntegerType bitMask = static_cast<IntegerType>(mask);
        IntegerType shift   = GetShiftToFirstSetBit(bitMask);

        // NOTE: value should be fully contained within the shift mask.
        //
        // assert((value & (mask >> shift)) == value);

        // Clear bits overlayed by the mask
        IntegerType updated = static_cast<IntegerType>(BitFlags<FlagsEnum, StorageType>::Raw() & ~bitMask);

        // Set the right bits
        updated = static_cast<IntegerType>(updated | (bitMask & (value << shift)));

        BitFlags<FlagsEnum, StorageType>::SetRaw(updated);

        return *this;
    }

    /**
     * Gets an underlying field that is contained within a bit subset of the integer.
     * Examples:
     *    GetField(0x70) == GetField(0b01110000) == ((n & 0x70) >> 4) == ((n >> 4) 0x07)
     *    GetField(0xC0) == GetField(0b11000000) == ((n & 0xC0) >> 6) == ((n >> 6) 0x03)
     *
     * @param mask The bit mask to be used, assumed to be a contigous bit mask
     */
    IntegerType GetField(FlagsEnum mask) const
    {
        IntegerType bitMask = static_cast<IntegerType>(mask);
        IntegerType shift   = GetShiftToFirstSetBit(bitMask);

        // Forward the right bits
        return static_cast<IntegerType>(((BitFlags<FlagsEnum, StorageType>::Raw() & bitMask) >> shift));
    }

protected:
    /// Get the shift amount to reach the first non-zero bit in a bitmask
    ///
    /// Examples:
    ///    GetShiftToFirstSetBit(0b0000001) == 0
    ///    GetShiftToFirstSetBit(0b0011100) == 2
    ///    GetShiftToFirstSetBit(0b0110000) == 4
    ///
    /// Note: This does NOT validate if the given mask is a valid mask. So:
    ///    GetShiftToFirstSetBit(0b0100010) == 1
    ///    GetShiftToFirstSetBit(0b010110100) == 2
    static constexpr IntegerType GetShiftToFirstSetBit(IntegerType bitMask)
    {
        IntegerType count = 0;
        IntegerType mask  = 0x01;

        if (bitMask == 0)
        {
            return sizeof(bitMask); // generally invalid value
        }

        while ((mask & bitMask) == 0)
        {
            mask = static_cast<IntegerType>(mask << 1);
            count++;
        }
        return count;
    }
};

} // namespace chip
