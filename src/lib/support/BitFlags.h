/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file defines functions for manipulating Boolean flags in
 *      a bitfield.
 *
 */

#pragma once

#include <stdint.h>

#include <type_traits>
#include <utility>

namespace chip {

/**
 * Stores bit flags in a type safe manner.
 *
 * @tparam FlagsEnum is an `enum` or (preferably) `enum class` type.
 * @tparam StorageType is the underlying storage type (like uint16_t, uint32_t etc.)
 *         and defaults to the underlying storage type of `FlagsEnum`.
 */
template <typename FlagsEnum, typename StorageType = typename std::underlying_type_t<FlagsEnum>>
class BitFlags
{
public:
    static_assert(sizeof(StorageType) >= sizeof(FlagsEnum), "All flags should fit in the storage type");
    using IntegerType = StorageType;

    constexpr BitFlags() : mValue(0) {}
    BitFlags(const BitFlags & other) = default;
    BitFlags & operator=(const BitFlags &) = default;

    explicit BitFlags(FlagsEnum value) : mValue(static_cast<IntegerType>(value)) {}
    explicit BitFlags(IntegerType value) : mValue(value) {}

    template <typename... Args>
    BitFlags(FlagsEnum flag, Args &&... args) : mValue(Or(flag, std::forward<Args>(args)...))
    {}

    template <typename... Args>
    BitFlags(const BitFlags<FlagsEnum> & flags, Args &&... args) : mValue(Or(flags, std::forward<Args>(args)...))
    {}

    template <typename... Args>
    BitFlags(IntegerType value, Args &&... args) : mValue(value | Or(std::forward<Args>(args)...))
    {}

    /**
     * Set flag(s).
     *
     * @param other     Flag(s) to set. Any flags not set in @a other are unaffected.
     */
    BitFlags & Set(const BitFlags & other)
    {
        mValue |= other.mValue;
        return *this;
    }

    /**
     * Set flag(s).
     *
     * @param flag      Typed flag(s) to set. Any flags not in @a v are unaffected.
     */
    constexpr BitFlags & Set(FlagsEnum flag)
    {
        mValue |= static_cast<IntegerType>(flag);
        return *this;
    }

    /**
     * Set or clear flag(s).
     *
     * @param flag      Typed flag(s) to set or clear. Any flags not in @a flag are unaffected.
     * @param isSet     If true, set the flag; if false, clear it.
     */
    constexpr BitFlags & Set(FlagsEnum flag, bool isSet) { return isSet ? Set(flag) : Clear(flag); }

    /**
     * Clear flag(s).
     *
     * @param other     Typed flag(s) to clear. Any flags not in @a other are unaffected.
     */
    BitFlags & Clear(const BitFlags & other)
    {
        mValue &= ~other.mValue;
        return *this;
    }

    /**
     * Clear flag(s).
     *
     * @param flag  Typed flag(s) to clear. Any flags not in @a flag are unaffected.
     */
    constexpr BitFlags & Clear(FlagsEnum flag)
    {
        mValue &= static_cast<IntegerType>(~static_cast<IntegerType>(flag));
        return *this;
    }

    /**
     * Clear all flags.
     */
    BitFlags & ClearAll()
    {
        mValue = 0;
        return *this;
    }

    /**
     * Check whether flag(s) are set.
     *
     * @param flag      Flag(s) to test.
     * @returns         True if any flag in @a flag is set, otherwise false.
     */
    constexpr bool Has(FlagsEnum flag) const { return (mValue & static_cast<IntegerType>(flag)) != 0; }

    /**
     * Check that no flags outside the arguments are set.
     *
     * @param args      Flags to test. Arguments can be BitFlags<FlagsEnum>, BitFlags<FlagsEnum>, or FlagsEnum.
     * @returns         True if no flag is set other than those passed.
     *                  False if any flag is set other than those passed.
     *
     * @note            Flags passed need not be set; this test only requires that no *other* flags be set.
     */
    template <typename... Args>
    bool HasOnly(Args &&... args) const
    {
        return (mValue & Or(std::forward<Args>(args)...)) == mValue;
    }

    /**
     * Check that all given flags are set.
     *
     * @param args      Flags to test. Arguments can be BitFlags<FlagsEnum>, BitFlags<FlagsEnum>, or FlagsEnum.
     * @returns         True if all given flags are set.
     *                  False if any given flag is not set.
     */
    template <typename... Args>
    bool HasAll(Args &&... args) const
    {
        const IntegerType all = Or(std::forward<Args>(args)...);
        return (mValue & all) == all;
    }

    /**
     * Check that at least one of the given flags is set.
     *
     * @param args      Flags to test. Arguments can be BitFlags<FlagsEnum>, BitFlags<FlagsEnum>, or FlagsEnum.
     * @returns         True if all given flags are set.
     *                  False if any given flag is not set.
     */
    template <typename... Args>
    bool HasAny(Args &&... args) const
    {
        return (mValue & Or(std::forward<Args>(args)...)) != 0;
    }

    /**
     * Check that at least one flag is set.
     *
     * @returns         True if any flag is set, false otherwise.
     */
    bool HasAny() const { return mValue != 0; }

    /**
     * Find the logical intersection of flags.
     *
     * @param lhs       Some flags.
     * @param rhs       Some flags.
     * @returns         Flags set in both @a lhs and @a rhs.
     *
     * @note: A multi-argument `BitFlags` constructor serves the function of `operator|`.
     */
    friend BitFlags<FlagsEnum> operator&(BitFlags<FlagsEnum> lhs, const BitFlags<FlagsEnum> & rhs)
    {
        return BitFlags<FlagsEnum>(lhs.mValue & rhs.mValue);
    }

    /**
     * Get the flags as the type FlagsEnum.
     *
     * @note            This allows easily storing flags as a base FlagsEnum in a POD type,
     *                  and enables equality comparisons.
     */
    operator FlagsEnum() const { return static_cast<FlagsEnum>(mValue); }

    /**
     * Set and/or clear all flags with a value of the underlying storage type.
     *
     * @param value     New storage value.
     */
    BitFlags & SetRaw(IntegerType value)
    {
        mValue = value;
        return *this;
    }

    /**
     * Get the flags as the underlying integer type.
     *
     * @note            This is intended to be used only to store flags into a raw binary record.
     */
    IntegerType Raw() const { return mValue; }

    /**
     * Get the address of the flags as a pointer to the underlying integer type.
     *
     * @note            This is intended to be used only to read flags from a raw binary record.
     */
    StorageType * RawStorage() { return &mValue; }

private:
    // Find the union of BitFlags and/or FlagsEnum values.
    template <typename... Args>
    static constexpr IntegerType Or(FlagsEnum flag, Args &&... args)
    {
        return static_cast<IntegerType>(flag) | Or(std::forward<Args>(args)...);
    }
    template <typename... Args>
    static constexpr IntegerType Or(const BitFlags<FlagsEnum> & flags, Args &&... args)
    {
        return flags.mValue | Or(std::forward<Args>(args)...);
    }
    static constexpr IntegerType Or(FlagsEnum value) { return static_cast<IntegerType>(value); }
    static constexpr IntegerType Or(const BitFlags<FlagsEnum> & flags) { return flags.Raw(); }

    StorageType mValue = 0;
};

} // namespace chip
