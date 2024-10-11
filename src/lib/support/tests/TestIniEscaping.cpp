/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <string>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/IniEscaping.h>

using namespace chip;
using namespace chip::IniEscaping;

namespace {

struct TestCase
{
    const char * input;
    const char * expectedOutput;
};

TEST(TestIniEscaping, TestEscaping)
{
    EXPECT_EQ(EscapeKey(""), "");
    EXPECT_EQ(EscapeKey("abcd1234,!"), "abcd1234,!");
    EXPECT_EQ(EscapeKey("ab\ncd =12\\34\x7f"), "ab\\x0acd\\x20\\x3d12\\x5c34\\x7f");
    EXPECT_EQ(EscapeKey(" "), "\\x20");
    EXPECT_EQ(EscapeKey("==="), "\\x3d\\x3d\\x3d");
}

TEST(TestIniEscaping, TestUnescaping)
{
    // Test valid cases
    EXPECT_EQ(UnescapeKey(""), "");
    EXPECT_EQ(UnescapeKey("abcd1234,!"), "abcd1234,!");
    EXPECT_EQ(UnescapeKey("ab\\x0acd\\x20\\x3d12\\x5c34\\x7f"), "ab\ncd =12\\34\x7f");
    EXPECT_EQ(UnescapeKey("\\x20"), " ");
    EXPECT_EQ(UnescapeKey("\\x3d\\x3d\\x3d"), "===");
    EXPECT_EQ(UnescapeKey("\\x0d"), "\r");

    EXPECT_EQ(UnescapeKey("\\x01\\x02\\x03\\x04\\x05\\x06\\x07"), "\x01\x02\x03\x04\x05\x06\x07");
    EXPECT_EQ(UnescapeKey("\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x0e"), "\x08\x09\x0a\x0b\x0c\x0d\x0e");
    EXPECT_EQ(UnescapeKey("\\x0f\\x10\\x11\\x12\\x13\\x14\\x15"), "\x0f\x10\x11\x12\x13\x14\x15");
    EXPECT_EQ(UnescapeKey("\\x16\\x17\\x18\\x19\\x1a\\x1b\\x1c"), "\x16\x17\x18\x19\x1a\x1b\x1c");
    EXPECT_EQ(UnescapeKey("\\x1d\\x1e\\x1f\\x20\\x7f\\x3d\\x5c"), "\x1d\x1e\x1f \x7f=\\");
    EXPECT_EQ(UnescapeKey("\\x81\\x82\\xff"), "\x81\x82\xff");

    // Test invalid cases

    // letters should never be escaped
    EXPECT_NE(UnescapeKey("\\x5a\55"), "ZU");
    EXPECT_EQ(UnescapeKey("\\x5a\55"), "");

    // Capitalized hex forbidden
    EXPECT_EQ(UnescapeKey("\\x0D"), "");

    // Partial escapes forbidden
    EXPECT_EQ(UnescapeKey("1\\x0"), "");
}

TEST(TestIniEscaping, TestRoundTrip)
{
    EXPECT_EQ(UnescapeKey(EscapeKey("")), "");
    EXPECT_EQ(UnescapeKey(EscapeKey("abcd1234,!")), "abcd1234,!");
    EXPECT_EQ(UnescapeKey(EscapeKey("ab\ncd =12\\34\x7f")), "ab\ncd =12\\34\x7f");
    EXPECT_EQ(UnescapeKey(EscapeKey(" ")), " ");
    EXPECT_EQ(UnescapeKey(EscapeKey("===")), "===");
    EXPECT_EQ(UnescapeKey(EscapeKey("\r")), "\r");

    EXPECT_EQ(UnescapeKey(EscapeKey("\x01\x02\x03\x04\x05\x06\x07")), "\x01\x02\x03\x04\x05\x06\x07");
    EXPECT_EQ(UnescapeKey(EscapeKey("\x08\x09\x0a\x0b\x0c\x0d\x0e")), "\x08\x09\x0a\x0b\x0c\x0d\x0e");
    EXPECT_EQ(UnescapeKey(EscapeKey("\x0f\x10\x11\x12\x13\x14\x15")), "\x0f\x10\x11\x12\x13\x14\x15");
    EXPECT_EQ(UnescapeKey(EscapeKey("\x16\x17\x18\x19\x1a\x1b\x1c")), "\x16\x17\x18\x19\x1a\x1b\x1c");
    EXPECT_EQ(UnescapeKey(EscapeKey("\x1d\x1e\x1f \x7f=\\")), "\x1d\x1e\x1f \x7f=\\");
    EXPECT_EQ(UnescapeKey(EscapeKey("\x81\x82\xff")), "\x81\x82\xff");

    // Make sure entire range is escapable
    for (int c = 0; c <= 255; c++)
    {
        std::string s(5, static_cast<char>(c));
        EXPECT_EQ(UnescapeKey(EscapeKey(s)), s) << "c: " << c;
    }
}

} // namespace
