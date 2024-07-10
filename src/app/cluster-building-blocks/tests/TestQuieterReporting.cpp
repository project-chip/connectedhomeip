/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <app/cluster-building-blocks/QuieterReporting.h>

#include <limits.h>

#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>

#include <gtest/gtest.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

class FakeClock
{
public:
    FakeClock() = default;

    Timestamp Advance(Milliseconds64 numMillis)
    {
        mCurrentTimestamp += numMillis;
        return mCurrentTimestamp;
    }

    void SetMonotonic(Timestamp now) { mCurrentTimestamp = now; }

    Timestamp now() const { return mCurrentTimestamp; }

private:
    Timestamp mCurrentTimestamp{};
};

TEST(TestQuieterReporting, ChangeToFromZeroPolicyWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(10) };
    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_FALSE(attribute.WasJustMarkedDirty());

    EXPECT_FALSE(attribute.value().IsNull());
    EXPECT_EQ(attribute.policy(), QuieterReportingPolicyFlags{});

    auto now = fakeClock.now();

    attribute.policy().Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
    EXPECT_TRUE(attribute.policy().HasOnly(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero));

    // 10 --> 11, expect not marked dirty yet.
    attribute.SetValue(11, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // 11 --> 0, expect marked dirty.
    attribute.SetValue(0, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 0);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // 0 --> 11, expect marked dirty.
    attribute.SetValue(11, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // 11 --> 12, expect not marked dirty.
    attribute.SetValue(12, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // Reset policy, expect 12 --> 0 does not mark dirty due to no longer having the policy that causes it.
    attribute.policy().ClearAll();
    EXPECT_FALSE(attribute.policy().HasAny());

    attribute.SetValue(0, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 0);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());
}

TEST(TestQuieterReporting, ChangeOnIncrementPolicyWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(10) };

    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_FALSE(attribute.WasJustMarkedDirty());
    ASSERT_EQ(*attribute.value(), 10);

    auto now = fakeClock.now();

    attribute.policy().Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement);
    EXPECT_TRUE(attribute.policy().HasOnly(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement));

    // 10 --> 9, expect not marked dirty yet.
    attribute.SetValue(9, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 9);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // 9 --> 10, expect marked dirty.
    attribute.SetValue(10, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // 10 --> 11, expect marked dirty.
    attribute.SetValue(11, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // 11 --> 11, expect marked not dirty.
    attribute.SetValue(11, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // 11 --> null, expect marked dirty (null change always marks dirty)
    attribute.SetValue(NullNullable, now);
    EXPECT_TRUE(attribute.value().IsNull());
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // null --> null, not dirty (no change)
    attribute.SetValue(NullNullable, now);
    EXPECT_TRUE(attribute.value().IsNull());
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // null --> 11, expect marked dirty (null change always marks dirty).
    attribute.SetValue(11, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // Reset policy, expect 11 --> 12 does not mark dirty due to no longer having the policy that causes it.
    attribute.policy().ClearAll();
    EXPECT_FALSE(attribute.policy().HasAny());

    attribute.SetValue(12, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());
}

TEST(TestQuieterReporting, ChangeOnDecrementPolicyWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(9) };

    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_FALSE(attribute.WasJustMarkedDirty());
    ASSERT_EQ(*attribute.value(), 9);

    auto now = fakeClock.now();

    attribute.policy().Set(QuieterReportingPolicyEnum::kMarkDirtyOnDecrement);
    EXPECT_TRUE(attribute.policy().HasOnly(QuieterReportingPolicyEnum::kMarkDirtyOnDecrement));

    // 9 --> 10, expect not marked dirty yet.
    attribute.SetValue(10, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // 10 --> 9, expect marked dirty.
    attribute.SetValue(9, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 9);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // 9 --> 8, expect marked dirty.
    attribute.SetValue(8, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 8);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // Second call in a row always false.
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // 8 --> 8, expect not marked dirty.
    attribute.SetValue(8, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 8);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // 8 --> null, expect marked dirty (null change always marks dirty)
    attribute.SetValue(NullNullable, now);
    EXPECT_TRUE(attribute.value().IsNull());
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // null --> null, not dirty (no change)
    attribute.SetValue(NullNullable, now);
    EXPECT_TRUE(attribute.value().IsNull());
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // null --> 11, expect marked dirty (null change always marks dirty).
    attribute.SetValue(11, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // Reset policy, expect 11 --> 10 does not mark dirty due to no longer having the policy that causes it.
    attribute.policy().ClearAll();
    EXPECT_FALSE(attribute.policy().HasAny());

    attribute.SetValue(10, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());
}

TEST(TestQuieterReporting, SufficientChangePredicateWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(9) };

    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_FALSE(attribute.WasJustMarkedDirty());
    ASSERT_EQ(*attribute.value(), 9);

    auto now = fakeClock.now();

    attribute.SetValue(10, now);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    attribute.ForceDirty(now);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    auto predicate = attribute.GetPredicateForSufficientTimeSinceLastDirty(1000_ms);

    now = fakeClock.Advance(100_ms);

    // Last dirty value is 10. This won't mark dirty again due to predicate mismatch.
    attribute.SetValue(11, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    now = fakeClock.Advance(900_ms);
    // Last dirty value is 10 still. This will mark dirty because both enough time has passed
    // and value is different from the last dirty value.
    attribute.SetValue(11, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // Last dirty value is 11. Since there has not been a value change, no amount of time will
    // mark dirty.
    now = fakeClock.Advance(1000_ms);
    attribute.SetValue(11, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    now = fakeClock.Advance(1000_ms);
    attribute.SetValue(11, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // Change the value to a value that marks dirty.
    now = fakeClock.Advance(1_ms);
    attribute.SetValue(12, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // Wait a small delay and change again. Will not mark dirty due to too little time.
    now = fakeClock.Advance(1_ms);
    attribute.SetValue(13, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 13);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    now = fakeClock.Advance(1_ms);
    attribute.SetValue(14, now, predicate);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 14);
    EXPECT_FALSE(attribute.WasJustMarkedDirty());

    // Change to a value that marks dirty no matter what (e.g. null). Should be dirty even
    // before delay.
    now = fakeClock.Advance(1_ms);
    attribute.SetValue(NullNullable, now, predicate);
    EXPECT_TRUE(attribute.value().IsNull());
    EXPECT_TRUE(attribute.WasJustMarkedDirty());

    // Null --> Null should not lead to dirty.
    now = fakeClock.Advance(1000_ms);
    attribute.SetValue(NullNullable, now, predicate);
    EXPECT_TRUE(attribute.value().IsNull());
    EXPECT_FALSE(attribute.WasJustMarkedDirty());
}
