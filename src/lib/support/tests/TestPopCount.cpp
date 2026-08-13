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

#include <limits>
#include <stdint.h>

#include <pw_unit_test/framework.h>

#include <lib/support/PopCount.h>

using namespace chip;

namespace {

TEST(TestPopCount, TestZero)
{
    EXPECT_EQ(PopCount<uint8_t>(0), 0);
    EXPECT_EQ(PopCount<uint16_t>(0), 0);
    EXPECT_EQ(PopCount<uint32_t>(0), 0);
    EXPECT_EQ(PopCount<uint64_t>(0), 0);
}

TEST(TestPopCount, TestAllOnes)
{
    EXPECT_EQ(PopCount<uint8_t>(0xFF), 8);
    EXPECT_EQ(PopCount<uint16_t>(0xFFFF), 16);
    EXPECT_EQ(PopCount<uint32_t>(0xFFFFFFFF), 32);
    EXPECT_EQ(PopCount<uint64_t>(0xFFFFFFFFFFFFFFFF), 64);
}

TEST(TestPopCount, TestSingleBits)
{
    for (int i = 0; i < std::numeric_limits<unsigned int>::digits; i++)
    {
        EXPECT_EQ(PopCount(1u << i), 1);
    }
    for (int i = 0; i < std::numeric_limits<unsigned long long>::digits; i++)
    {
        EXPECT_EQ(PopCount(1ull << i), 1);
    }
}

TEST(TestPopCount, TestKnownValues)
{
    EXPECT_EQ(PopCount<uint8_t>(0xA), 2);  // 0000 1010
    EXPECT_EQ(PopCount<uint8_t>(0xAA), 4); // 1010 1010
    EXPECT_EQ(PopCount<uint16_t>(0xA), 2); // 0000 0000 0000 1010
    EXPECT_EQ(PopCount<uint16_t>(0xAAAA), 8);
    EXPECT_EQ(PopCount<uint32_t>(0xDEADBEEF), 24);
    EXPECT_EQ(PopCount<uint64_t>(0xDEADBEEFDEADBEEF), 48);
}

// Test compile-time use
static_assert(PopCount<uint8_t>(0xA) == 2);
static_assert(PopCount<uint8_t>(0xAA) == 4);
static_assert(PopCount<uint16_t>(0xA) == 2);
static_assert(PopCount<uint16_t>(0xAAAA) == 8);
static_assert(PopCount<uint32_t>(0xDEADBEEF) == 24);
static_assert(PopCount<uint64_t>(0xDEADBEEFDEADBEEF) == 48);

} // namespace
