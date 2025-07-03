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

#include <app/persistence/PascalString.h>
#include <lib/core/StringBuilderAdapters.h>

namespace {

using namespace chip;
using namespace chip::app::Storage;

TEST(TestPascalString, TestCharStringOperations)
{
    char buffer[8] = { 0 };

    {
        ShortPascalString str(buffer);

        ASSERT_TRUE(str.SetLength(0));
        ASSERT_TRUE(str.Content().data_equal(""_span));
        ASSERT_EQ(str.GetLength(), 0u);
        ASSERT_EQ(str.PascalContent().size(), 1u);
        ASSERT_EQ(str.Buffer().size(), 8u);
        ASSERT_EQ(str.Buffer().data(), buffer);

        ASSERT_TRUE(str.SetValue("foo"_span));
        ASSERT_TRUE(str.Content().data_equal("foo"_span));
        ASSERT_EQ(str.GetLength(), 3u);
        ASSERT_EQ(str.PascalContent().size(), 4u);
        ASSERT_EQ(str.Buffer().size(), 8u);
        ASSERT_EQ(str.Buffer().data(), buffer);

        // exactly 7 bytes can be stored, since one is reserved for size
        ASSERT_TRUE(str.SetValue("1234567"_span));
        ASSERT_EQ(str.GetLength(), 7u);
        ASSERT_TRUE(str.Content().data_equal("1234567"_span));

        // 8 bytes is too long
        ASSERT_FALSE(str.SetValue("toolong1"_span));
    }

    {
        LongPascalString str(buffer);

        ASSERT_TRUE(str.SetLength(0));
        ASSERT_TRUE(str.Content().data_equal(""_span));
        ASSERT_EQ(str.GetLength(), 0u);
        ASSERT_EQ(str.PascalContent().size(), 2u);

        ASSERT_TRUE(str.SetValue("foo"_span));
        ASSERT_TRUE(str.Content().data_equal("foo"_span));
        ASSERT_EQ(str.GetLength(), 3u);
        ASSERT_EQ(str.PascalContent().size(), 5u);

        // exactly 6 bytes can be stored, since two are reserved for size
        ASSERT_TRUE(str.SetValue("123456"_span));
        ASSERT_EQ(str.GetLength(), 6u);
        ASSERT_TRUE(str.Content().data_equal("123456"_span));

        // 7 bytes is too long
        ASSERT_FALSE(str.SetValue("toolong"_span));
    }
}

TEST(TestPascalString, TestByteStringOperations)
{
    uint8_t buffer[8] = { 0 };

    {
        ShortPascalBytes str(buffer);

        ASSERT_TRUE(str.SetLength(0));
        ASSERT_TRUE(str.Content().empty());
        ASSERT_EQ(str.GetLength(), 0u);
        ASSERT_EQ(str.PascalContent().size(), 1u);
        ASSERT_EQ(str.Buffer().size(), 8u);
        ASSERT_EQ(str.Buffer().data(), buffer);

        const uint8_t foo[] = { 1, 2, 3 };
        ASSERT_TRUE(str.SetValue(ByteSpan(foo)));
        ASSERT_TRUE(str.Content().data_equal(ByteSpan(foo)));
        ASSERT_EQ(str.GetLength(), 3u);
        ASSERT_EQ(str.PascalContent().size(), 4u);
        ASSERT_EQ(str.Buffer().size(), 8u);

        // exactly 7 bytes can be stored, since one is reserved for size
        const uint8_t fits[] = { 1, 2, 3, 4, 5, 6, 7 };
        ASSERT_TRUE(str.SetValue(ByteSpan(fits)));
        ASSERT_EQ(str.GetLength(), 7u);
        ASSERT_TRUE(str.Content().data_equal(ByteSpan(fits)));

        // 8 bytes is too long
        const uint8_t toolong1[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
        ASSERT_FALSE(str.SetValue(ByteSpan(toolong1)));
    }

    {
        LongPascalBytes str(buffer);

        ASSERT_TRUE(str.SetLength(0));
        ASSERT_TRUE(str.Content().empty());
        ASSERT_EQ(str.GetLength(), 0u);
        ASSERT_EQ(str.PascalContent().size(), 2u);
        ASSERT_EQ(str.Buffer().size(), 8u);
        ASSERT_EQ(str.Buffer().data(), buffer);

        const uint8_t foo[] = { 1, 2, 3 };
        ASSERT_TRUE(str.SetValue(ByteSpan(foo)));
        ASSERT_TRUE(str.Content().data_equal(ByteSpan(foo)));
        ASSERT_EQ(str.GetLength(), 3u);
        ASSERT_EQ(str.PascalContent().size(), 5u);
        ASSERT_EQ(str.Buffer().size(), 8u);

        // exactly 6 bytes can be stored, since two are reserved for size
        const uint8_t fits[] = { 1, 2, 3, 4, 5, 6 };
        ASSERT_TRUE(str.SetValue(ByteSpan(fits)));
        ASSERT_EQ(str.GetLength(), 6u);
        ASSERT_TRUE(str.Content().data_equal(ByteSpan(fits)));

        // 7 bytes is too long
        const uint8_t toolong[] = { 1, 2, 3, 4, 5, 6, 7 };
        ASSERT_FALSE(str.SetValue(ByteSpan(toolong)));
    }
}

TEST(TestPascalString, TestNullability)
{
    {
        uint8_t buffer[8] = { 0xFF, 0xFF, 1, 2, 3 };
        ASSERT_TRUE(ShortPascalBytes(buffer).IsNull());
        ASSERT_TRUE(LongPascalBytes(buffer).IsNull());
    }

    {
        char buffer[8] = { '\xFF', '\xFF', 'a', 'b', 'c' };
        ASSERT_TRUE(ShortPascalString(buffer).IsNull());
        ASSERT_TRUE(LongPascalString(buffer).IsNull());
    }

    {
        uint8_t buffer[8] = { 0 };
        ShortPascalBytes s(buffer);

        ASSERT_FALSE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 0U);

        s.SetNull();
        ASSERT_TRUE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 0U);

        const uint8_t foo[] = { 1, 2, 3 };
        ASSERT_TRUE(s.SetValue(ByteSpan(foo)));
        ASSERT_FALSE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 3U);
    }

    {
        uint8_t buffer[8] = { 0 };
        LongPascalBytes s(buffer);

        ASSERT_FALSE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 0U);

        s.SetNull();
        ASSERT_TRUE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 0U);

        const uint8_t foo[] = { 1, 2, 3 };
        ASSERT_TRUE(s.SetValue(ByteSpan(foo)));
        ASSERT_FALSE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 3U);
    }

    {
        char buffer[8] = { 0 };
        LongPascalString s(buffer);

        ASSERT_FALSE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 0U);

        s.SetNull();
        ASSERT_TRUE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 0U);

        ASSERT_TRUE(s.SetValue("test"_span));
        ASSERT_FALSE(s.IsNull());
        ASSERT_EQ(s.GetLength(), 4U);
    }
}

TEST(TestPascalString, TestIsValid)
{
    {
        uint8_t buff[] = { 3, 0, 0 };
        ASSERT_FALSE(ShortPascalBytes::IsValid(ByteSpan(buff)));
        buff[0] = 254;
        ASSERT_FALSE(ShortPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 255; // null string
        ASSERT_TRUE(ShortPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 2;
        ASSERT_TRUE(ShortPascalBytes::IsValid(ByteSpan(buff)));
        buff[0] = 1;
        ASSERT_TRUE(ShortPascalBytes::IsValid(ByteSpan(buff)));
    }

    {
        uint8_t buff[] = { 4, 0, 0, 0, 0 };
        ASSERT_FALSE(LongPascalBytes::IsValid(ByteSpan(buff)));
        buff[0] = 254;
        ASSERT_FALSE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 255;
        ASSERT_FALSE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[1] = 255;
        ASSERT_TRUE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 0;
        ASSERT_FALSE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[1] = 0;
        ASSERT_TRUE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 1;
        ASSERT_TRUE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 2;
        ASSERT_TRUE(LongPascalBytes::IsValid(ByteSpan(buff)));

        buff[0] = 3;
        ASSERT_TRUE(LongPascalBytes::IsValid(ByteSpan(buff)));
    }
}

} // namespace
