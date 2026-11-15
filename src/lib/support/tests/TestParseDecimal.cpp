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

#include <lib/support/ParseDecimal.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <cstdint>
#include <limits>

namespace {

using namespace chip;

TEST(TestParseDecimal, ParsesUnsigned)
{
    uint8_t u8 = 0;
    EXPECT_TRUE(ParseDecimal("0", u8));
    EXPECT_EQ(u8, 0u);
    EXPECT_TRUE(ParseDecimal("255", u8));
    EXPECT_EQ(u8, 255u);
    EXPECT_TRUE(ParseDecimal("007", u8));
    EXPECT_EQ(u8, 7u);

    uint64_t u64 = 0;
    EXPECT_TRUE(ParseDecimal("18446744073709551615", u64));
    EXPECT_EQ(u64, std::numeric_limits<uint64_t>::max());
}

TEST(TestParseDecimal, ParsesSigned)
{
    int8_t i8 = 0;
    EXPECT_TRUE(ParseDecimal("-128", i8));
    EXPECT_EQ(i8, -128);
    EXPECT_TRUE(ParseDecimal("127", i8));
    EXPECT_EQ(i8, 127);
}

TEST(TestParseDecimal, RejectsOutOfRange)
{
    uint8_t u8 = 42;
    EXPECT_FALSE(ParseDecimal("256", u8));
    EXPECT_FALSE(ParseDecimal("-1", u8));
    EXPECT_EQ(u8, 42u);

    int8_t i8 = 42;
    EXPECT_FALSE(ParseDecimal("128", i8));
    EXPECT_FALSE(ParseDecimal("-129", i8));
    EXPECT_EQ(i8, 42);
}

TEST(TestParseDecimal, RejectsMalformed)
{
    uint32_t value = 42;
    EXPECT_FALSE(ParseDecimal("", value));
    EXPECT_FALSE(ParseDecimal("+1", value));
    EXPECT_FALSE(ParseDecimal(" 1", value));
    EXPECT_FALSE(ParseDecimal("1 ", value));
    EXPECT_FALSE(ParseDecimal("12x", value));
    EXPECT_FALSE(ParseDecimal("0x10", value));
    EXPECT_FALSE(ParseDecimal("-", value));
    EXPECT_EQ(value, 42u);
}

TEST(TestParseDecimal, HandlesNullAndEmpty)
{
    uint32_t value = 42;
    EXPECT_FALSE(ParseDecimal(static_cast<const char *>(nullptr), value));
    EXPECT_FALSE(ParseDecimal(CharSpan(), value));
    EXPECT_EQ(value, 42u);
}

TEST(TestParseDecimal, ParsesWholeSpanOnly)
{
    // The span covers "12" of "123"; the character after the span is not part of the input.
    const char digits[] = "123";
    uint32_t value      = 0;
    EXPECT_TRUE(ParseDecimal(CharSpan(digits, 2), value));
    EXPECT_EQ(value, 12u);
}

} // namespace
