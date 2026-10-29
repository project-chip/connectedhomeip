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

#include <platform/Linux/ExponentialBackOff.h>

namespace {

using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;
using chip::DeviceLayer::Internal::ExponentialBackOff;

// The Avahi DNS-SD collision retry's parameters: 1 s, doubling to 32 s,
// starting over after 64 s without a failure.
constexpr Seconds32 kInitialDelay(1);
constexpr Seconds32 kMaximumDelay(32);
constexpr Timeout kQuietPeriod = 64_s;

// Arbitrary, nonzero, so that no test depends on the clock starting at
// zero.
constexpr Timestamp kStart = 1000_s;

TEST(TestExponentialBackOff, FirstFailureGetsInitialDelay)
{
    ExponentialBackOff backOff(kInitialDelay, kMaximumDelay, kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), 1u);
}

TEST(TestExponentialBackOff, DelayDoublesUpToMaximum)
{
    ExponentialBackOff backOff(kInitialDelay, kMaximumDelay, kQuietPeriod);
    const uint32_t expected[] = { 1, 2, 4, 8, 16, 32, 32, 32 };
    Timestamp now             = kStart;

    for (const uint32_t delay : expected)
    {
        EXPECT_EQ(backOff.NextDelay(now).count(), delay);
        now += 1_s;
    }
}

TEST(TestExponentialBackOff, MaximumNeedNotBeAPowerOfTwoMultiple)
{
    ExponentialBackOff backOff(Seconds32(3), Seconds32(10), kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), 3u);
    EXPECT_EQ(backOff.NextDelay(kStart + 1_s).count(), 6u);
    EXPECT_EQ(backOff.NextDelay(kStart + 2_s).count(), 10u);
    EXPECT_EQ(backOff.NextDelay(kStart + 3_s).count(), 10u);
}

TEST(TestExponentialBackOff, DoublingDoesNotOverflow)
{
    // Doubling 2^31 s would wrap to zero in 32 bits.
    constexpr uint32_t kLarge = 1u << 31;
    ExponentialBackOff backOff(Seconds32(kLarge), Seconds32(UINT32_MAX), kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), kLarge);
    EXPECT_EQ(backOff.NextDelay(kStart + 1_s).count(), UINT32_MAX);
    EXPECT_EQ(backOff.NextDelay(kStart + 2_s).count(), UINT32_MAX);
}

TEST(TestExponentialBackOff, InitialDelayMayEqualMaximum)
{
    ExponentialBackOff backOff(Seconds32(5), Seconds32(5), kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), 5u);
    EXPECT_EQ(backOff.NextDelay(kStart + 1_s).count(), 5u);
}

TEST(TestExponentialBackOff, FailureAfterQuietPeriodStartsOver)
{
    ExponentialBackOff backOff(kInitialDelay, kMaximumDelay, kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), 1u);
    EXPECT_EQ(backOff.NextDelay(kStart + 1_s).count(), 2u);
    EXPECT_EQ(backOff.NextDelay(kStart + 1_s + kQuietPeriod + 1_ms).count(), 1u);
}

TEST(TestExponentialBackOff, FailureAtEndOfQuietPeriodStillDoubles)
{
    ExponentialBackOff backOff(kInitialDelay, kMaximumDelay, kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), 1u);
    EXPECT_EQ(backOff.NextDelay(kStart + kQuietPeriod).count(), 2u);
}

TEST(TestExponentialBackOff, QuietPeriodRunsFromLatestFailure)
{
    ExponentialBackOff backOff(kInitialDelay, kMaximumDelay, kQuietPeriod);
    Timestamp now = kStart;

    // Failures spaced 40 s apart: each within the quiet period of the
    // one before, though the run as a whole lasts longer than it.

    for (const uint32_t delay : { 1u, 2u, 4u, 8u })
    {
        EXPECT_EQ(backOff.NextDelay(now).count(), delay);
        now += 40_s;
    }
}

TEST(TestExponentialBackOff, ResetStartsOver)
{
    ExponentialBackOff backOff(kInitialDelay, kMaximumDelay, kQuietPeriod);

    EXPECT_EQ(backOff.NextDelay(kStart).count(), 1u);
    EXPECT_EQ(backOff.NextDelay(kStart + 1_s).count(), 2u);
    EXPECT_EQ(backOff.NextDelay(kStart + 2_s).count(), 4u);

    backOff.Reset();

    EXPECT_EQ(backOff.NextDelay(kStart + 3_s).count(), 1u);
    EXPECT_EQ(backOff.NextDelay(kStart + 4_s).count(), 2u);
}

} // namespace
