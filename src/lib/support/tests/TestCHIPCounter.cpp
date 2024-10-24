/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <stdint.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPCounter.h>

using namespace chip;

TEST(TestCHIPCounter, TestStartWithZero)
{
    MonotonicallyIncreasingCounter<uint64_t> counter;
    EXPECT_EQ(counter.GetValue(), 0ULL);
}

TEST(TestCHIPCounter, TestInitialize)
{
    MonotonicallyIncreasingCounter<uint64_t> counter;

    EXPECT_EQ(counter.Init(4321), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 4321ULL);
}

TEST(TestCHIPCounter, TestAdvance)
{
    MonotonicallyIncreasingCounter<uint64_t> counter;

    EXPECT_EQ(counter.Init(22), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 22ULL);
    EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 23ULL);
    EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 24ULL);
}

TEST(TestCHIPCounter, TestAdvanceWithRollover)
{
    MonotonicallyIncreasingCounter<uint64_t> counter;
    EXPECT_EQ(counter.Init(UINT64_MAX), CHIP_NO_ERROR);

    EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 0ULL);
}

TEST(TestCHIPCounter, AdvanceBy)
{
    MonotonicallyIncreasingCounter<uint64_t> counter;
    constexpr uint64_t step = 9876;
    uint64_t expectedValue  = step;

    EXPECT_EQ(counter.Init(0), CHIP_NO_ERROR);
    EXPECT_EQ(counter.AdvanceBy(step), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), expectedValue);

    expectedValue += step;
    EXPECT_EQ(counter.AdvanceBy(step), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), expectedValue);

    // Force rollover
    expectedValue += UINT64_MAX;
    EXPECT_EQ(counter.AdvanceBy(UINT64_MAX), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), expectedValue);
}
