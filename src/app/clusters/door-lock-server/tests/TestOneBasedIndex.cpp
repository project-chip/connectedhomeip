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

#include <pw_unit_test/framework.h>

#include <app/clusters/door-lock-server/OneBasedIndex.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <limits>
#include <type_traits>

namespace {

using namespace chip::app::Clusters::DoorLock;

// The whole point of the type: no implicit conversion from or to integers, and
// no mixing of index domains.
static_assert(!std::is_convertible_v<uint16_t, UserIndex>);
static_assert(!std::is_convertible_v<int, UserIndex>);
static_assert(!std::is_convertible_v<UserIndex, uint16_t>);
static_assert(!std::is_constructible_v<UserIndex, uint16_t>);
static_assert(!std::is_default_constructible_v<UserIndex>);
static_assert(!std::is_convertible_v<UserIndex, CredentialIndex>);
static_assert(!std::is_convertible_v<CredentialIndex, ScheduleIndex>);
static_assert(!std::is_same_v<UserIndex, CredentialIndex>);

static_assert(std::is_same_v<UserIndex::StorageType, uint16_t>);
static_assert(std::is_same_v<CredentialIndex::StorageType, uint16_t>);
static_assert(std::is_same_v<ScheduleIndex::StorageType, uint8_t>);

// Usable in constant expressions.
static_assert(UserIndex::FromOneBased(1).ZeroBased() == 0);
static_assert(UserIndex::FromZeroBased(0).OneBased() == 1);

TEST(TestOneBasedIndex, OneBasedAndZeroBasedRoundTrip)
{
    constexpr UserIndex first = UserIndex::FromOneBased(1);
    EXPECT_EQ(first.OneBased(), 1u);
    EXPECT_EQ(first.ZeroBased(), 0u);

    constexpr UserIndex fromStorage = UserIndex::FromZeroBased(41);
    EXPECT_EQ(fromStorage.OneBased(), 42u);
    EXPECT_EQ(fromStorage.ZeroBased(), 41u);

    EXPECT_EQ(UserIndex::FromZeroBased(UserIndex::FromOneBased(7).ZeroBased()), UserIndex::FromOneBased(7));
}

TEST(TestOneBasedIndex, ZeroIsInvalid)
{
    constexpr UserIndex none = UserIndex::FromOneBased(0);
    EXPECT_FALSE(none.IsValid());
    EXPECT_EQ(none.OneBased(), 0u);
    EXPECT_FALSE(none.IsWithin(10));

    EXPECT_TRUE(UserIndex::FromOneBased(1).IsValid());
    EXPECT_TRUE(UserIndex::FromZeroBased(0).IsValid());
}

TEST(TestOneBasedIndex, IsWithinIsInclusiveOfCount)
{
    constexpr uint16_t kCount = 10;

    EXPECT_FALSE(UserIndex::FromOneBased(0).IsWithin(kCount));
    EXPECT_TRUE(UserIndex::FromOneBased(1).IsWithin(kCount));
    EXPECT_TRUE(UserIndex::FromOneBased(kCount).IsWithin(kCount));
    EXPECT_FALSE(UserIndex::FromOneBased(kCount + 1).IsWithin(kCount));

    // A count of 0 slots admits nothing.
    EXPECT_FALSE(UserIndex::FromOneBased(1).IsWithin(0));
}

TEST(TestOneBasedIndex, EqualityComparesTheIndexValue)
{
    EXPECT_EQ(UserIndex::FromOneBased(3), UserIndex::FromZeroBased(2));
    EXPECT_NE(UserIndex::FromOneBased(3), UserIndex::FromOneBased(2));
    EXPECT_TRUE(UserIndex::FromOneBased(0) == UserIndex::FromOneBased(0));
}

TEST(TestOneBasedIndex, CopyAndAssign)
{
    UserIndex a = UserIndex::FromOneBased(5);
    UserIndex b = a;
    EXPECT_EQ(a, b);

    b = UserIndex::FromOneBased(6);
    EXPECT_NE(a, b);
    EXPECT_EQ(b.OneBased(), 6u);
}

TEST(TestOneBasedIndex, ScheduleIndexUsesEightBitStorage)
{
    constexpr uint8_t kMax = std::numeric_limits<uint8_t>::max();

    constexpr ScheduleIndex last = ScheduleIndex::FromOneBased(kMax);
    EXPECT_TRUE(last.IsValid());
    EXPECT_EQ(last.ZeroBased(), static_cast<uint8_t>(kMax - 1));
    EXPECT_TRUE(last.IsWithin(kMax));

    EXPECT_EQ(ScheduleIndex::FromZeroBased(static_cast<uint8_t>(kMax - 1)), last);
}

TEST(TestOneBasedIndex, ProgrammingPinCredentialIsIndexZero)
{
    // The Programming PIN is the one credential addressed with spec index 0;
    // the type reports it as not valid for 1-based slot lookups.
    constexpr CredentialIndex programmingPin = CredentialIndex::FromOneBased(0);
    EXPECT_FALSE(programmingPin.IsValid());
    EXPECT_EQ(programmingPin.OneBased(), 0u);
}

} // namespace
