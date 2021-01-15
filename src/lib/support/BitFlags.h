/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <utility>

namespace chip {

/**
 * Stores bit flags in a type safe manner.
 *
 * @tparam StorageType is the underlying storage type (like uint16_t, uint32_t etc.)
 * @tparam FlagsEnum is the typesafe flags setting
 */
template <typename StorageType, typename FlagsEnum>
class BitFlags
{
public:
    static_assert(sizeof(StorageType) >= sizeof(FlagsEnum), "All flags should fit in the storage type");

    BitFlags() {}
    BitFlags(const BitFlags &) = default;
    BitFlags & operator=(const BitFlags &) = default;

    explicit BitFlags(FlagsEnum v) { Set(v); }

    explicit BitFlags(StorageType value) { SetRaw(value); }

    BitFlags & Set(FlagsEnum v)
    {
        mValue = static_cast<StorageType>(mValue | static_cast<StorageType>(v));
        return *this;
    }

    BitFlags & Clear(FlagsEnum v)
    {
        mValue &= static_cast<StorageType>(~static_cast<StorageType>(v));
        return *this;
    }

    BitFlags & Set(FlagsEnum v, bool isSet) { return isSet ? Set(v) : Clear(v); }

    bool Has(FlagsEnum v) const { return (mValue & static_cast<StorageType>(v)) != 0; }

    bool Has(StorageType other) const { return (mValue & other) == other; }

    BitFlags & Set(const BitFlags & other)
    {
        mValue |= other.mValue;
        return *this;
    }

    StorageType Raw() const { return mValue; }
    BitFlags & SetRaw(StorageType value)
    {
        mValue = value;
        return *this;
    }

    /** Check that no flags outside the arguments are set.*/
    template <typename... Args>
    bool HasOnly(Args &&... args) const
    {
        return IsZeroAfterClearing(mValue, std::forward<Args>(args)...);
    }

private:
    StorageType mValue = 0;

    template <typename... Args>
    static bool IsZeroAfterClearing(StorageType value, FlagsEnum flagToClear, Args &&... args)
    {
        value &= static_cast<StorageType>(~static_cast<StorageType>(flagToClear));
        return IsZeroAfterClearing(value, std::forward<Args>(args)...);
    }

    static bool IsZeroAfterClearing(StorageType value) { return value == 0; }
};

/**
 * @deprecated Use typesafe BitFlags class instead.
 */
template <typename FlagsT, typename FlagT>
inline bool GetFlag(const FlagsT & inFlags, const FlagT inFlag)
{
    return (inFlags & static_cast<FlagsT>(inFlag)) != 0;
}

/**
 * @deprecated Use typesafe BitFlags class instead.
 */
template <typename FlagsT, typename FlagT>
inline void ClearFlag(FlagsT & inFlags, const FlagT inFlag)
{
    inFlags &= static_cast<FlagsT>(~static_cast<FlagsT>(inFlag));
}

/**
 * @deprecated Use typesafe BitFlags class instead.
 */
template <typename FlagsT, typename FlagT>
inline void SetFlag(FlagsT & inFlags, const FlagT inFlag)
{
    inFlags = static_cast<FlagsT>(inFlags | static_cast<FlagsT>(inFlag));
}

/**
 * @deprecated Use typesafe BitFlags class instead.
 */
template <typename FlagsT, typename FlagT>
inline void SetFlag(FlagsT & inFlags, const FlagT inFlag, const bool inValue)
{
    if (inValue)
    {
        SetFlag(inFlags, inFlag);
    }
    else
    {
        ClearFlag(inFlags, inFlag);
    }
}

} // namespace chip
