/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <lib/support/ChunkSplitter.h>

namespace {

using namespace chip;

TEST(TestChunkSplitter, TestSplitter)
{
    CharSpan out;
    size_t zero = 0;

    // empty string handling
    {
        ChunkSplitter<5> splitter(""_span);

        // next stays at nullptr
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
    }

    // single item
    {
        ChunkSplitter<6> splitter("single"_span);

        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("single"_span));

        // next stays at nullptr also after valid data
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
    }

    // multi-item
    {
        ChunkSplitter<3> splitter("onetwo3rd"_span);

        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("one"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("two"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("3rd"_span));
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
    }

    // last item smaller than chunk size
    {
        ChunkSplitter<4> splitter("fourfourii"_span);

        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("four"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("four"_span));
        EXPECT_TRUE(splitter.Next(out));
        EXPECT_TRUE(out.data_equal("ii"_span));
        EXPECT_FALSE(splitter.Next(out));
        EXPECT_EQ(out.size(), zero);
    }

    // some edge cases
}

} // namespace
