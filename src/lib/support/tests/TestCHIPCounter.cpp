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

#include <gtest/gtest.h>

#include <lib/support/CHIPCounter.h>

TEST(TestCHIPCounter, TestCheckStartWithZero)
{
    chip::MonotonicallyIncreasingCounter<uint64_t> counter;
    EXPECT_EQ(counter.GetValue(), 0u);
}

TEST(TestCHIPCounter, TestCheckInitialize)
{
    chip::MonotonicallyIncreasingCounter<uint64_t> counter;

    EXPECT_EQ(counter.Init(4321), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 4321u);
}

TEST(TestCHIPCounter, TestCheckAdvance)
{
    chip::MonotonicallyIncreasingCounter<uint64_t> counter;

    EXPECT_EQ(counter.Init(22), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 22u);
    EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 23u);
    EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 24u);
}
