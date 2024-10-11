/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/BitMask.h>

#include <algorithm>
#include <cstring>
#include <initializer_list>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>

using namespace chip;

namespace {

enum class TestEnum : uint16_t
{
    kZero = 0x0000, // not a valid mask or bit flag, here anyway

    kBit_0      = 0x0001,
    kBits_1_2   = 0x0006,
    kBits_4_7   = 0x00F0,
    kBits_High4 = 0xF000,
    kBits_High8 = 0xFF00,
};

TEST(TestBitMask, TestBitMaskOperations)
{
    BitMask<TestEnum> mask;

    EXPECT_EQ(mask.Raw(), 0);

    mask.SetField(TestEnum::kBits_1_2, 2);
    EXPECT_EQ(mask.Raw(), 0x0004);

    mask.SetRaw(0);
    mask.SetField(TestEnum::kBits_4_7, 0x0B);
    EXPECT_EQ(mask.Raw(), 0x00B0);

    mask.SetRaw(0);

    for (uint16_t i = 0; i < 0x10; i++)
    {
        mask.SetField(TestEnum::kBits_High4, i);
        EXPECT_EQ(mask.Raw(), (i << 12));
    }

    mask.SetField(TestEnum::kBits_High8, 0x23);
    EXPECT_EQ(mask.Raw(), 0x2300);

    mask.SetField(TestEnum::kBits_High4, 0xA);
    EXPECT_EQ(mask.Raw(), 0xA300);
}

TEST(TestBitMask, TestBitFieldLogic)
{
    BitMask<TestEnum> mask;

    // some general logic that still applies for bit fields just in case
    EXPECT_FALSE(mask.HasAny(TestEnum::kBits_High4));
    EXPECT_FALSE(mask.HasAny(TestEnum::kBits_High8));

    // setting something non-zero in the upper 4 bits sets "something" in both
    // upper and 4 and 8 bits
    mask.SetField(TestEnum::kBits_High4, 0x01);
    EXPECT_TRUE(mask.HasAny(TestEnum::kBits_High4));
    EXPECT_TRUE(mask.HasAny(TestEnum::kBits_High8));

    // sets something visible in high 8 bits, but not high 4 bits
    mask.SetField(TestEnum::kBits_High8, 0x01);
    EXPECT_FALSE(mask.HasAny(TestEnum::kBits_High4));
    EXPECT_TRUE(mask.HasAny(TestEnum::kBits_High8));
}

TEST(TestBitMask, TestBitMaskInvalid)
{
    BitMask<TestEnum> mask;

    // This generally tests for no infinite loops. Nothing to set here
    mask.SetField(TestEnum::kZero, 0x01);
    EXPECT_EQ(mask.Raw(), 0);

    mask.SetRaw(0x1234);
    mask.SetField(TestEnum::kZero, 0x01);
    EXPECT_EQ(mask.Raw(), 0x1234);
}

TEST(TestBitMask, TestClear)
{
    BitMask<TestEnum> mask1;
    BitMask<TestEnum> mask2;

    mask1.Set(TestEnum::kBit_0);
    mask1.Set(TestEnum::kBits_1_2);
    mask1.Set(TestEnum::kBits_High8);

    mask2.Set(TestEnum::kBits_1_2);
    mask1.Clear(mask2);

    EXPECT_EQ(mask1.Raw(), 0xFF01);
}

} // namespace
