/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/support/verhoeff/Verhoeff.h>

#include <string>

namespace {

TEST(TestVerhoeff, TestVerhoeff)
{

    // Valid cases
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("236"), '3');
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("0"), '4');
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("11111"), '5');

    EXPECT_TRUE(Verhoeff10::ValidateCheckChar("123451"));
    EXPECT_TRUE(Verhoeff10::ValidateCheckChar("0987652"));
    EXPECT_TRUE(Verhoeff10::ValidateCheckChar("150"));

    // Invalid cases
    EXPECT_NE(Verhoeff10::ComputeCheckChar("236"), '8');
    EXPECT_NE(Verhoeff10::ComputeCheckChar("0"), '1');
    EXPECT_NE(Verhoeff10::ComputeCheckChar("11111"), '7');

    EXPECT_FALSE(Verhoeff10::ValidateCheckChar("123456"));
    EXPECT_FALSE(Verhoeff10::ValidateCheckChar("0987651"));
    EXPECT_FALSE(Verhoeff10::ValidateCheckChar("157"));

    // Transposition
    EXPECT_NE(Verhoeff10::ComputeCheckChar("12345"), Verhoeff10::ComputeCheckChar("13245"));
    EXPECT_NE(Verhoeff10::ComputeCheckChar("1122334455"), Verhoeff10::ComputeCheckChar("1122334545"));
    EXPECT_NE(Verhoeff10::ComputeCheckChar("1234567890"), Verhoeff10::ComputeCheckChar("1234567809"));

    // Non adjacent transpose
    EXPECT_NE(Verhoeff10::ComputeCheckChar("12345"), Verhoeff10::ComputeCheckChar("14325"));
    EXPECT_NE(Verhoeff10::ComputeCheckChar("876543"), Verhoeff10::ComputeCheckChar("678543"));

    // Long numbers
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("4356678912349008"), '7');
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("78324562830019274123748"), '4');

    // Invalid character
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("123F4567"), 0);
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("0A"), 0);
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("I"), 0);
    EXPECT_EQ(Verhoeff10::ComputeCheckChar("23.4"), 0);

    // Empty string
    EXPECT_EQ(Verhoeff10::ComputeCheckChar(""), '0');
}

} // namespace
