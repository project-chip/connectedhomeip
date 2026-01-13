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

template <size_t N>
class TestStringBuilder
{
    StringBuilder<N> builder;

public:
    template <typename... Args>
    TestStringBuilder(Args &&... args) : builder(std::forward<Args>(args)...)
    {}

    template <typename F>
    void Check(F check)
    {
        check(builder);
    }
};

TEST(TestStringBuilder, TestStringBuilder)
{
    TestStringBuilder<64>().Check([](StringBuilderBase & builder) {
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "");

        builder.Add("foo");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "foo");

        builder.Add("bar");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "foobar");
    });
}

TEST(TestStringBuilder, TestIntegerAppend)
{
    TestStringBuilder<64>().Check([](StringBuilderBase & builder) {
        builder.Add("nr: ").Add(1234);
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "nr: 1234");

        builder.Add(", ").Add(-22);
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "nr: 1234, -22");
    });
}

TEST(TestStringBuilder, TestOverflow)
{
    TestStringBuilder<4>().Check([](StringBuilderBase & builder) {
        builder.Add("foo");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "foo");

        builder.Add("bar");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "foo");
    });

    TestStringBuilder<7>().Check([](StringBuilderBase & builder) {
        builder.Add("x: ").Add(12345);
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "x: 123");
    });
}

TEST(TestStringBuilder, TestFormat)
{
    TestStringBuilder<100>().Check([](StringBuilderBase & builder) {
        builder.AddFormat("Test: %d Hello %s\n", 123, "world");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Test: 123 Hello world\n");
    });

    TestStringBuilder<100>().Check([](StringBuilderBase & builder) {
        builder.AddFormat("Align: %-5s", "abc");
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Align: abc  ");
    });

    TestStringBuilder<100>().Check([](StringBuilderBase & builder) {
        builder.AddFormat("Multi: %d", 1234).AddFormat(", then 0x%04X", 0xab);
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Multi: 1234, then 0x00AB");
    });

    const char str[] = "1234567890";

    const char expected[] = "1234567890";

    auto check = [&](StringBuilderBase & builder) {
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), expected);
    };

    TestStringBuilder<11>(str, strlen(str)).Check(check);

    TestStringBuilder<11>(CharSpan::fromCharString(str)).Check(check);

    const uint8_t str2[] = "12\x01"
                           "34\x02"
                           "56\x03"
                           "7890";

    const char expected2[] = "12.34.56.7890";

    auto check2 = [&](StringBuilderBase & builder) {
        EXPECT_TRUE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), expected2);
    };

    TestStringBuilder<14>(str2, strlen(reinterpret_cast<const char *>(str2))).Check(check2);

    TestStringBuilder<14>(ByteSpan(str2, strlen(reinterpret_cast<const char *>(str2)))).Check(check2);

    const uint8_t bytes[] = { '1', '2', '\x01', '3', '4', '\x02', '5', '6', '\x00', '7', '8', '9', '0' };

    TestStringBuilder<14>(bytes, sizeof(bytes)).Check(check2);

    TestStringBuilder<14>(ByteSpan(bytes, sizeof(bytes))).Check(check2);

    const char arr[] = { '1', '2', '\0', '3', '4', '\0', '5', '6', '\0', '7', '8', '9', '0' };

    TestStringBuilder<14>(CharSpan(arr, sizeof(arr))).Check(check2);

    TestStringBuilder<14>(arr, sizeof(arr)).Check(check2);
}

TEST(TestStringBuilder, TestFormatOverflow)
{
    TestStringBuilder<13>().Check([](StringBuilderBase & builder) {
        builder.AddFormat("Test: %d Hello %s\n", 123, "world");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "Test: 123 He");
    });

    TestStringBuilder<11>().Check([](StringBuilderBase & builder) {
        builder.AddFormat("%d %d %d %d %d", 1, 2, 3, 4, 1234);
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "1 2 3 4 12");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "1 2 3 4...");
    });

    TestStringBuilder<11>().Check([](StringBuilderBase & builder) {
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
    });

    const char str[] = "1234567890a";

    const char expected[] = "1234567890";

    auto check = [&](StringBuilderBase & builder) {
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), expected);
    };

    TestStringBuilder<11>(str, strlen(str), false).Check(check);

    TestStringBuilder<11>(CharSpan::fromCharString(str), false).Check(check);

    const uint8_t str2[] = "12\x01"
                           "34\x02"
                           "56\x03"
                           "7890a";

    const char expected2[] = "12.34.56.7890";

    auto check2 = [&](StringBuilderBase & builder) {
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), expected2);
    };

    TestStringBuilder<14>(str2, strlen(reinterpret_cast<const char *>(str2)), false).Check(check2);

    TestStringBuilder<14>(ByteSpan(str2, strlen(reinterpret_cast<const char *>(str2))), false).Check(check2);

    const uint8_t bytes[] = { '1', '2', '\x01', '3', '4', '\x02', '5', '6', '\x00', '7', '8', '9', '0', 'a' };

    TestStringBuilder<14>(bytes, sizeof(bytes), false).Check(check2);

    TestStringBuilder<14>(ByteSpan(bytes, sizeof(bytes)), false).Check(check2);

    const char arr[] = { '1', '2', '\0', '3', '4', '\0', '5', '6', '\0', '7', '8', '9', '0', 'a' };

    TestStringBuilder<14>(CharSpan(arr, sizeof(arr)), false).Check(check2);

    TestStringBuilder<14>(arr, sizeof(arr), false).Check(check2);
}

TEST(TestStringBuilder, TestOverflowMarker)
{
    TestStringBuilder<1>().Check([](StringBuilderBase & builder) {
        builder.Add("abc123");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "");
    });

    TestStringBuilder<2>().Check([](StringBuilderBase & builder) {
        builder.Add("abc123");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "a");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), ".");
    });

    TestStringBuilder<3>().Check([](StringBuilderBase & builder) {
        builder.Add("abc123");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "ab");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "..");
    });

    TestStringBuilder<4>().Check([](StringBuilderBase & builder) {
        builder.Add("abc123");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "abc");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "...");
    });

    TestStringBuilder<5>().Check([](StringBuilderBase & builder) {
        builder.Add("abc123");
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), "abc1");

        builder.AddMarkerIfOverflow();
        EXPECT_STREQ(builder.c_str(), "a...");
    });

    const char str[] = "1234567890a";

    const char expected[] = "1234567...";

    auto check = [&](StringBuilderBase & builder) {
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), expected);
    };

    TestStringBuilder<11>(str, strlen(str)).Check(check);

    TestStringBuilder<11>(CharSpan::fromCharString(str)).Check(check);

    const uint8_t str2[] = "12\x01"
                           "34\x02"
                           "56\x03"
                           "7890a";

    const char expected2[] = "12.34.56.7...";

    auto check2 = [&](StringBuilderBase & builder) {
        EXPECT_FALSE(builder.Fit());
        EXPECT_STREQ(builder.c_str(), expected2);
    };

    TestStringBuilder<14>(str2, strlen(reinterpret_cast<const char *>(str2))).Check(check2);

    TestStringBuilder<14>(ByteSpan(str2, strlen(reinterpret_cast<const char *>(str2)))).Check(check2);

    const uint8_t bytes[] = { '1', '2', '\x01', '3', '4', '\x02', '5', '6', '\x00', '7', '8', '9', '0', 'a' };

    TestStringBuilder<14>(bytes, sizeof(bytes)).Check(check2);

    TestStringBuilder<14>(ByteSpan(bytes, sizeof(bytes))).Check(check2);

    const char arr[] = { '1', '2', '\0', '3', '4', '\0', '5', '6', '\0', '7', '8', '9', '0', 'a' };

    TestStringBuilder<14>(CharSpan(arr, sizeof(arr))).Check(check2);

    TestStringBuilder<14>(arr, sizeof(arr)).Check(check2);
}

} // namespace
