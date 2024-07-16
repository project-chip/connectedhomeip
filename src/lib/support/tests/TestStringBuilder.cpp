/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/StringBuilder.h>

namespace {

using namespace chip;

TEST(TestStringBuilder, TestStringBuilder)
{

    StringBuilder<64> builder;

    EXPECT_TRUE(builder.Fit());
    EXPECT_STREQ(builder.c_str(), "");

    builder.Add("foo");
    EXPECT_TRUE(builder.Fit());
    EXPECT_STREQ(builder.c_str(), "foo");

    builder.Add("bar");
    EXPECT_TRUE(builder.Fit());
    EXPECT_STREQ(builder.c_str(), "foobar");
}

TEST(TestStringBuilder, TestIntegerAppend)
{

    StringBuilder<64> builder;

    builder.Add("nr: ").Add(1234);
    EXPECT_TRUE(builder.Fit());
    EXPECT_STREQ(builder.c_str(), "nr: 1234");

    builder.Add(", ").Add(-22);
    EXPECT_TRUE(builder.Fit());
    EXPECT_STREQ(builder.c_str(), "nr: 1234, -22");
}

TEST(TestStringBuilder, TestOverflow)
{

    {
        StringBuilder<4> builder;

        builder.Add("foo");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "foo");

        builder.Add("bar");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "foo");
    }

    {
        StringBuilder<7> builder;

        builder.Add("x: ").Add(12345);
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "x: 123");
    }
}

TEST(TestStringBuilder, TestFormat)
{
    {
        StringBuilder<100> builder;

        builder.AddFormat("Test: %d Hello %s\n", 123, "world");

        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Test: 123 Hello world\n");
    }

    {
        StringBuilder<100> builder;

        builder.AddFormat("Align: %-5s", "abc");

        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Align: abc  ");
    }

    {
        StringBuilder<100> builder;

        builder.AddFormat("Multi: %d", 1234);
        builder.AddFormat(", then 0x%04X", 0xab);

        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Multi: 1234, then 0x00AB");
    }
}

TEST(TestStringBuilder, TestFormatOverflow)
{
    {
        StringBuilder<13> builder;

        builder.AddFormat("Test: %d Hello %s\n", 123, "world");

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Test: 123 He");
    }

    {
        StringBuilder<11> builder;

        builder.AddFormat("%d %d %d %d %d", 1, 2, 3, 4, 1234);

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "1 2 3 4 12");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "1 2 3 4...");
    }

    {
        StringBuilder<11> builder;

        builder.AddFormat("%d", 1234);
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "1234");

        builder.AddFormat("%s", "abc");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "1234abc");

        builder.AddMarkerIfOverflow(); // no overflow
        EXPECT_STREQ(builder.c_str(), "1234abc");

        builder.AddFormat("%08x", 0x123456);
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "1234abc001");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "1234abc...");
    }
}

TEST(TestStringBuilder, TestOverflowMarker)
{
    {
        StringBuilder<1> builder; // useless builder, but ok

        builder.Add("abc123");

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "");
    }

    {
        StringBuilder<2> builder;

        builder.Add("abc123");

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "a");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), ".");
    }

    {
        StringBuilder<3> builder;

        builder.Add("abc123");

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "ab");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "..");
    }

    {
        StringBuilder<4> builder;

        builder.Add("abc123");

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "abc");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "...");
    }

    {
        StringBuilder<5> builder;

        builder.Add("abc123");

        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "abc1");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "a...");
    }
}

} // namespace
