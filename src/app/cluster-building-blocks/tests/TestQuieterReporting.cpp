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
#include <lib/core/StringBuilderAdapters.h>
#include <system/SystemClock.h>

#include <pw_unit_test/framework.h>

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
    EXPECT_FALSE(attribute.value().IsNull());
    EXPECT_EQ(attribute.policy(), QuieterReportingPolicyFlags{});

    auto now = fakeClock.now();

    attribute.policy().Set(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero);
    EXPECT_TRUE(attribute.policy().HasOnly(QuieterReportingPolicyEnum::kMarkDirtyOnChangeToFromZero));

    // 10 --> 11, expect not marked dirty yet.
    EXPECT_EQ(attribute.SetValue(11, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // 11 --> 0, expect marked dirty.
    EXPECT_EQ(attribute.SetValue(0, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 0);

    // 0 --> 0, expect NOT marked dirty.
    EXPECT_EQ(attribute.SetValue(0, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 0);

    // 0 --> 11, expect marked dirty.
    EXPECT_EQ(attribute.SetValue(11, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // 11 --> 12, expect not marked dirty.
    EXPECT_EQ(attribute.SetValue(12, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);

    // Reset policy, expect 12 --> 0 does not mark dirty due to no longer having the policy that causes it.
    attribute.policy().ClearAll();
    EXPECT_FALSE(attribute.policy().HasAny());

    EXPECT_EQ(attribute.SetValue(0, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 0);
}

TEST(TestQuieterReporting, ChangeOnIncrementPolicyWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(10) };

    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_EQ(attribute.value().Value(), 10);

    auto now = fakeClock.now();

    attribute.policy().Set(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement);
    EXPECT_TRUE(attribute.policy().HasOnly(QuieterReportingPolicyEnum::kMarkDirtyOnIncrement));

    // 10 --> 9, expect not marked dirty yet.
    EXPECT_EQ(attribute.SetValue(9, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 9);

    // 9 --> 10, expect marked dirty.
    EXPECT_EQ(attribute.SetValue(10, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);

    // 10 --> 11, expect marked dirty.
    EXPECT_EQ(attribute.SetValue(11, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // 11 --> 11, expect marked not dirty.
    EXPECT_EQ(attribute.SetValue(11, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // 11 --> null, expect marked dirty (null change always marks dirty)
    EXPECT_EQ(attribute.SetValue(NullNullable, now), AttributeDirtyState::kMustReport);
    EXPECT_TRUE(attribute.value().IsNull());

    // null --> null, not dirty (no change)
    EXPECT_EQ(attribute.SetValue(NullNullable, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_TRUE(attribute.value().IsNull());

    // null --> 11, expect marked dirty (null change always marks dirty).
    EXPECT_EQ(attribute.SetValue(11, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // Reset policy, expect 11 --> 12 does not mark dirty due to no longer having the policy that causes it.
    attribute.policy().ClearAll();
    EXPECT_FALSE(attribute.policy().HasAny());

    EXPECT_EQ(attribute.SetValue(12, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);
}

TEST(TestQuieterReporting, ChangeOnDecrementPolicyWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(9) };

    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_EQ(attribute.value().Value(), 9);

    auto now = fakeClock.now();

    attribute.policy().Set(QuieterReportingPolicyEnum::kMarkDirtyOnDecrement);
    EXPECT_TRUE(attribute.policy().HasOnly(QuieterReportingPolicyEnum::kMarkDirtyOnDecrement));

    // 9 --> 10, expect not marked dirty yet.
    EXPECT_EQ(attribute.SetValue(10, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);

    // 10 --> 9, expect marked dirty.
    EXPECT_EQ(attribute.SetValue(9, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 9);

    // 9 --> 8, expect marked dirty.
    EXPECT_EQ(attribute.SetValue(8, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 8);

    // Second call in a row always false.

    // 8 --> 8, expect not marked dirty.
    EXPECT_EQ(attribute.SetValue(8, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 8);

    // 8 --> null, expect marked dirty (null change always marks dirty)
    EXPECT_EQ(attribute.SetValue(NullNullable, now), AttributeDirtyState::kMustReport);
    EXPECT_TRUE(attribute.value().IsNull());

    // null --> null, not dirty (no change)
    EXPECT_EQ(attribute.SetValue(NullNullable, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_TRUE(attribute.value().IsNull());

    // null --> 11, expect marked dirty (null change always marks dirty).
    EXPECT_EQ(attribute.SetValue(11, now), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // Reset policy, expect 11 --> 10 does not mark dirty due to no longer having the policy that causes it.
    attribute.policy().ClearAll();
    EXPECT_FALSE(attribute.policy().HasAny());

    EXPECT_EQ(attribute.SetValue(10, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);
}

TEST(TestQuieterReporting, SufficientChangePredicateWorks)
{
    FakeClock fakeClock;
    fakeClock.SetMonotonic(100_ms);

    QuieterReportingAttribute<int> attribute{ MakeNullable<int>(9) };

    // Always start not dirty (because first sub priming always just read value anyway).
    ASSERT_EQ(attribute.value().Value(), 9);

    auto now = fakeClock.now();

    EXPECT_EQ(attribute.SetValue(10, now), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 10);

    auto predicate = attribute.GetPredicateForSufficientTimeSinceLastDirty(1000_ms);

    now = fakeClock.Advance(100_ms);

    // Last dirty value is 10. This won't mark dirty again due to predicate mismatch.
    EXPECT_EQ(attribute.SetValue(11, now, predicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    now = fakeClock.Advance(900_ms);
    // Last dirty value is 10 still. This will mark dirty because both enough time has passed
    // and value is different from the last dirty value.
    EXPECT_EQ(attribute.SetValue(11, now, predicate), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // Last dirty value is 11. Since there has not been a value change, no amount of time will
    // mark dirty.
    now = fakeClock.Advance(1000_ms);
    EXPECT_EQ(attribute.SetValue(11, now, predicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    now = fakeClock.Advance(1000_ms);
    EXPECT_EQ(attribute.SetValue(11, now, predicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 11);

    // Change the value to a value that marks dirty.
    now = fakeClock.Advance(1_ms);
    EXPECT_EQ(attribute.SetValue(12, now, predicate), AttributeDirtyState::kMustReport);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);

    // Wait a small delay and change again. Will not mark dirty due to too little time.
    now = fakeClock.Advance(1_ms);
    EXPECT_EQ(attribute.SetValue(13, now, predicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 13);

    now = fakeClock.Advance(1_ms);
    EXPECT_EQ(attribute.SetValue(14, now, predicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 14);

    // Forcing dirty can NOT done with a force-true predicate.
    decltype(attribute)::SufficientChangePredicate forceTruePredicate{
        [](const decltype(attribute)::SufficientChangePredicateCandidate &) -> bool { return true; }
    };
    now = fakeClock.Advance(1_ms);
    EXPECT_EQ(attribute.SetValue(12, now, forceTruePredicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_EQ(attribute.value().ValueOr(INT_MAX), 12);

    // Change to a value that marks dirty no matter what (e.g. null). Should be dirty even
    // before delay.
    now = fakeClock.Advance(1_ms);
    EXPECT_EQ(attribute.SetValue(NullNullable, now, predicate), AttributeDirtyState::kMustReport);
    EXPECT_TRUE(attribute.value().IsNull());

    // Null --> Null should not lead to dirty.
    now = fakeClock.Advance(1000_ms);
    EXPECT_EQ(attribute.SetValue(NullNullable, now, predicate), AttributeDirtyState::kNoReportNeeded);
    EXPECT_TRUE(attribute.value().IsNull());
}
