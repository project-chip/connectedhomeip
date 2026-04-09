/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/support/PopCount.h>

using namespace chip;

namespace {

TEST(TestPopCount, TestZero)
{
    EXPECT_EQ(PopCount(0u), 0);
    EXPECT_EQ(PopCount(static_cast<unsigned char>(0)), 0);
    EXPECT_EQ(PopCount(static_cast<unsigned short>(0)), 0);
    EXPECT_EQ(PopCount(0ul), 0);
    EXPECT_EQ(PopCount(0ull), 0);
}

TEST(TestPopCount, TestAllOnes)
{
    EXPECT_EQ(PopCount(static_cast<unsigned char>(0xFF)), 8);
    EXPECT_EQ(PopCount(static_cast<unsigned short>(0xFFFF)), 16);
    EXPECT_EQ(PopCount(0xFFFFFFFFu), 32);
    EXPECT_EQ(PopCount(0xFFFFFFFFFFFFFFFFull), 64);
}

TEST(TestPopCount, TestSingleBits)
{
    for (int i = 0; i < 32; i++)
    {
        EXPECT_EQ(PopCount(1u << i), 1);
    }
    for (int i = 0; i < 64; i++)
    {
        EXPECT_EQ(PopCount(1ull << i), 1);
    }
}

TEST(TestPopCount, TestKnownValues)
{
    EXPECT_EQ(PopCount(0xAu), 2);  // 1010
    EXPECT_EQ(PopCount(0xFu), 4);  // 1111
    EXPECT_EQ(PopCount(0x55u), 4); // 01010101
    EXPECT_EQ(PopCount(0xAAu), 4); // 10101010
    EXPECT_EQ(PopCount(0xDEADBEEFu), 24);
}

} // namespace
