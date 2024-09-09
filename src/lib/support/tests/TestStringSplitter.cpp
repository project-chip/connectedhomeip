/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <lib/support/StringSplitter.h>

namespace {

using namespace chip;

TEST(TestStringSplitter, TestStrdupSplitter)
{
    CharSpan out;

    // empty string handling
    {
        StringSplitter splitter("", ',');

        // next stays at nullptr
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.data(), nullptr);
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.data(), nullptr);
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.data(), nullptr);
    }

    // single item
    {
        StringSplitter splitter("single", ',');

        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("single"_span));

        // next stays at nullptr also after valid data
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.data(), nullptr);
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.data(), nullptr);
    }

    // multi-item
    {
        StringSplitter splitter("one,two,three", ',');

        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("one"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("two"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("three"_span));
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.data(), nullptr);
    }

    // mixed
    {
        StringSplitter splitter("a**bc*d,e*f", '*');

        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("a"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("bc"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("d,e"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("f"_span));
        EXPECT_FALSE(splitter.Next(out));
    }

    // some edge cases
    {
        StringSplitter splitter(",", ',');
        // Note that even though "" is nullptr right away, "," becomes two empty strings
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_FALSE(splitter.Next(out));
    }
    {
        StringSplitter splitter("log,", ',');
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("log"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_FALSE(splitter.Next(out));
    }
    {
        StringSplitter splitter(",log", ',');
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("log"_span));
        EXPECT_FALSE(splitter.Next(out));
    }
    {
        StringSplitter splitter(",,,", ',');
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal(""_span));
        EXPECT_FALSE(splitter.Next(out));
    }
}

TEST(TestStringSplitter, TestNullResilience)
{
    {
        StringSplitter splitter(nullptr, ',');
        CharSpan span;
        EXPECT_FALSE(splitter.Next(span));
        EXPECT_EQ(span.data(), nullptr);
    }
}

} // namespace
