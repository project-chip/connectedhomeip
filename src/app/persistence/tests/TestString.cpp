/*
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

#include <app/persistence/String.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Storage;

TEST(TestString, TestSetContent)
{
    String<10> testString;

    // Expect empty initialization
    EXPECT_TRUE(testString.Content().data_equal(""_span));
    EXPECT_STREQ(testString.c_str(), "");

    // basic test
    EXPECT_TRUE(testString.SetContent("hello"_span));
    EXPECT_TRUE(testString.Content().data_equal("hello"_span));
    EXPECT_STREQ(testString.c_str(), "hello");

    // Exactly fitting
    EXPECT_TRUE(testString.SetContent("0123456789"_span));
    EXPECT_TRUE(testString.Content().data_equal("0123456789"_span));
    EXPECT_STREQ(testString.c_str(), "0123456789");

    // Too large
    EXPECT_FALSE(testString.SetContent("0123456789a"_span));
    EXPECT_TRUE(testString.Content().empty());
    EXPECT_STREQ(testString.c_str(), "");

    // empty content
    EXPECT_TRUE(testString.SetContent(""_span));
    EXPECT_TRUE(testString.Content().empty());
    EXPECT_STREQ(testString.c_str(), "");

    // C-str works after truncation
    EXPECT_TRUE(testString.SetContent("0123456789"_span));
    EXPECT_TRUE(testString.SetContent("01"_span));
    EXPECT_TRUE(testString.Content().data_equal("01"_span));
    EXPECT_STREQ(testString.c_str(), "01");
}

TEST(TestString, TestCStr)
{
    String<5> testString;

    EXPECT_TRUE(testString.SetContent("abc"_span));
    EXPECT_STREQ(testString.c_str(), "abc");

    // c_str() should work even if we do not have spare characters
    EXPECT_TRUE(testString.SetContent("abcde"_span));
    EXPECT_STREQ(testString.c_str(), "abcde");
}

} // namespace
