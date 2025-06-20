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

#include <app/storage/PascalString.h>

namespace {

using namespace chip;
using namespace chip::app::Storage;

TEST(TestPascalString, TestCharStringOperations)
{
    char buffer[8] = {0};

    {
        ShortPascalString str(buffer);

        ASSERT_TRUE(str.SetLength(0));
        ASSERT_TRUE(str.Content().data_equal(""_span));
        ASSERT_EQ(str.GetLength(), 0u);
        ASSERT_EQ(str.PascalContent().size(), 1u);

        ASSERT_TRUE(str.SetValue("foo"_span));
        ASSERT_TRUE(str.Content().data_equal("foo"_span));
        ASSERT_EQ(str.GetLength(), 3u);
        ASSERT_EQ(str.PascalContent().size(), 4u);

        // exactly 8 bytes, however one is reserved for size
        ASSERT_FALSE(str.SetValue("toolong1"_span));
    }

    {
        LongPascalString str(buffer);

        ASSERT_TRUE(str.SetLength(0));
        ASSERT_TRUE(str.Content().data_equal(""_span));
        ASSERT_EQ(str.GetLength(), 0u);
        ASSERT_EQ(str.PascalContent().size(), 1u);

        ASSERT_TRUE(str.SetValue("foo"_span));
        ASSERT_TRUE(str.Content().data_equal("foo"_span));
        ASSERT_EQ(str.GetLength(), 3u);
        ASSERT_EQ(str.PascalContent().size(), 5u);

        // exactly 6 bytes, however two are reserved for size
        ASSERT_FALSE(str.SetValue("toolong"_span));
    }
}

} // namespace
