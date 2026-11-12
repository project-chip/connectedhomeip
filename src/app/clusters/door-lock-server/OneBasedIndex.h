/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <cstdint>
#include <type_traits>

namespace chip {
namespace app {
namespace Clusters {
namespace DoorLock {

/**
 * A 1-based Door Lock index (user, credential or schedule slot) as used on the
 * wire by the Door Lock cluster, where index 0 means "none" or "invalid" and
 * valid slots are numbered 1..N.
 *
 * The type exists so that 1-based spec indices and 0-based storage offsets
 * cannot be mixed by accident: there is no implicit conversion from or to the
 * underlying integer, and the base is spelled out at every construction and
 * every read (`FromOneBased` / `OneBased`, `FromZeroBased` / `ZeroBased`).
 *
 * `Tag` is only used to make indices of different domains distinct types, so a
 * `UserIndex` cannot be passed where a `CredentialIndex` is expected.
 */
template <typename Tag, typename Storage>
class OneBasedIndex
{
    static_assert(std::is_unsigned_v<Storage>, "OneBasedIndex storage must be an unsigned integer type");

public:
    using StorageType = Storage;

    OneBasedIndex(const OneBasedIndex &)             = default;
    OneBasedIndex & operator=(const OneBasedIndex &) = default;

    /// Wrap a value that is already 1-based (e.g. received from a command).
    static constexpr OneBasedIndex FromOneBased(Storage oneBased) { return OneBasedIndex(oneBased); }

    /// Wrap a 0-based storage offset. `zeroBased` must be < max(Storage).
    static constexpr OneBasedIndex FromZeroBased(Storage zeroBased) { return OneBasedIndex(static_cast<Storage>(zeroBased + 1)); }

    /// The 1-based value (0 when the index is invalid / "none").
    constexpr Storage OneBased() const { return mOneBased; }

    /// The 0-based storage offset. Only defined when `IsValid()`.
    constexpr Storage ZeroBased() const { return static_cast<Storage>(mOneBased - 1); }

    /// True for 1..max(Storage); false for the spec "invalid" index 0.
    constexpr bool IsValid() const { return mOneBased != 0; }

    /// True when the index addresses one of `count` slots numbered 1..count.
    constexpr bool IsWithin(Storage count) const { return IsValid() && mOneBased <= count; }

    constexpr bool operator==(const OneBasedIndex & other) const { return mOneBased == other.mOneBased; }
    constexpr bool operator!=(const OneBasedIndex & other) const { return mOneBased != other.mOneBased; }

private:
    constexpr explicit OneBasedIndex(Storage oneBased) : mOneBased(oneBased) {}

    Storage mOneBased;
};

/// Index into the user database. Valid range: 1..NumberOfTotalUsersSupported.
using UserIndex = OneBasedIndex<struct UserIndexTag, uint16_t>;

/// Index into the credential database for a given credential type. Valid range:
/// 1..NumberOf<Type>CredentialsSupported, except kProgrammingPIN which uses 0
/// (`IsValid()` is false for that one credential and `ZeroBased()` must not be
/// called on it).
using CredentialIndex = OneBasedIndex<struct CredentialIndexTag, uint16_t>;

/// Index into the week day, year day or holiday schedule database. Valid range:
/// 1..NumberOf<Kind>SchedulesSupported.
using ScheduleIndex = OneBasedIndex<struct ScheduleIndexTag, uint8_t>;

} // namespace DoorLock
} // namespace Clusters
} // namespace app
} // namespace chip
