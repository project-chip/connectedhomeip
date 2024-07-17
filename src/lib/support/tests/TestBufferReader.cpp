/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a unit test suite for CHIP BufferReader
 *
 */

#include <type_traits>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/BufferReader.h>

using namespace chip;
using namespace chip::Encoding::LittleEndian;

static const uint8_t test_buffer[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

struct TestReader : public Reader
{
    TestReader() : Reader(test_buffer, std::extent<decltype(test_buffer)>::value) {}
};

struct TestSpanReader : public Reader
{
    TestSpanReader() : Reader(ByteSpan{ test_buffer, std::extent<decltype(test_buffer)>::value }) {}
};

static void TestBufferReader_BasicImpl(Reader & reader)
{
    uint8_t first;
    uint16_t second;
    uint32_t third;
    uint64_t fourth;

    uint8_t read_buf[3]                = { 0, 0, 0 };
    const uint8_t read_buf_expected[3] = { 16, 17, 18 };

    CHIP_ERROR err =
        reader.Read8(&first).Read16(&second).Read32(&third).Read64(&fourth).ReadBytes(&read_buf[0], sizeof(read_buf)).StatusCode();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(first, 0x01);
    EXPECT_EQ(second, 0x0302);
    EXPECT_EQ(third, 0x07060504u);
    EXPECT_EQ(fourth, 0x0f0e0d0c0b0a0908u);
    EXPECT_EQ(memcmp(&read_buf[0], &read_buf_expected[0], sizeof(read_buf)), 0);
    EXPECT_EQ(reader.OctetsRead(), 18u);
    EXPECT_EQ(reader.Remaining(), 3u);
    EXPECT_TRUE(reader.HasAtLeast(2));
    EXPECT_TRUE(reader.HasAtLeast(3));
    EXPECT_FALSE(reader.HasAtLeast(4));

    uint32_t fourMore;
    err = reader.Read32(&fourMore).StatusCode();
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(TestBufferReader, TestBufferReader_Basic)
{
    TestReader reader;

    TestBufferReader_BasicImpl(reader);
}

TEST(TestBufferReader, TestBufferReader_BasicSpan)
{
    TestSpanReader reader;

    TestBufferReader_BasicImpl(reader);
}

TEST(TestBufferReader, TestBufferReader_Saturation)
{
    TestReader reader;
    uint64_t temp;
    // Read some bytes out so we can get to the end of the buffer.
    CHIP_ERROR err = reader.Read64(&temp).StatusCode();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = reader.Read64(&temp).StatusCode();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_TRUE(reader.HasAtLeast(5));
    EXPECT_FALSE(reader.HasAtLeast(6));
    uint64_t tooBig;
    err = reader.Read64(&tooBig).StatusCode();
    EXPECT_NE(err, CHIP_NO_ERROR);
    EXPECT_FALSE(reader.HasAtLeast(1));

    // Check that even though we only really read out 16 bytes, we can't read
    // out one more bytes, because our previous read failed.
    uint8_t small;
    err = reader.Read8(&small).StatusCode();
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(TestBufferReader, TestBufferReader_Skip)
{
    TestReader reader;
    uint8_t temp          = 0;
    uint16_t firstSkipLen = 2;

    // Verify Skip() advances the start pointer the correct amount.
    CHIP_ERROR err = reader.Skip(firstSkipLen).Read8(&temp).StatusCode();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(temp, test_buffer[firstSkipLen]);
    EXPECT_EQ(reader.OctetsRead(), (firstSkipLen + 1u));

    // Verify Skip() called with a length larger than available buffer space jumps to the end.
    err = reader.Skip(sizeof(test_buffer)).StatusCode();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(reader.OctetsRead(), sizeof(test_buffer));
    EXPECT_EQ(reader.Remaining(), 0u);

    // Verify no read allowed after jumping to the end.
    err = reader.Read8(&temp).StatusCode();
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(TestBufferReader, TestBufferReader_LittleEndianScalars)
{
    const uint8_t test_buf1[10] = { 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01 };

    // Unsigned 8 bits reads
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        uint8_t val1 = 0;
        uint8_t val2 = 0;
        EXPECT_TRUE(reader.Read8(&val1).Read8(&val2).IsSuccess());
        EXPECT_EQ(val1, 0xfe);
        EXPECT_EQ(val2, 0xff);
    }

    // Unsigned 16 bits reads
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        uint16_t val1 = 0;
        uint16_t val2 = 0;
        EXPECT_TRUE(reader.Read16(&val1).Read16(&val2).IsSuccess());
        EXPECT_EQ(val1, 0xfffe);
        EXPECT_EQ(val2, 0xffff);
    }

    // Unsigned 32 bits reads
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        uint32_t val1 = 0;
        uint32_t val2 = 0;
        EXPECT_TRUE(reader.Read32(&val1).Read32(&val2).IsSuccess());
        EXPECT_EQ(val1, static_cast<uint32_t>(0xfffffffeUL));
        EXPECT_EQ(val2, static_cast<uint32_t>(0xffffffffUL));
    }

    // Unsigned 32 bits reads, unaligned
    {
        uint8_t test_buf2[10] = { 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01 };
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf2 } };

        uint32_t val1 = 0;
        uint32_t val2 = 0;
        EXPECT_TRUE(reader.Skip(1).Read32(&val1).Read32(&val2).IsSuccess());
        EXPECT_EQ(reader.Remaining(), 1u);
        EXPECT_EQ(val1, static_cast<uint32_t>(0xfffffffeUL));
        EXPECT_EQ(val2, static_cast<uint32_t>(0xffffffffUL));
    }

    // Unsigned 64 bits read
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        uint64_t val = 0;
        EXPECT_TRUE(reader.Read64(&val).IsSuccess());
        EXPECT_EQ(reader.Remaining(), 2u);
        EXPECT_EQ(val, static_cast<uint64_t>(0xfffffffffffffffeULL));
    }

    // Signed 8 bits reads
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        int8_t val1 = 0;
        int8_t val2 = 0;
        EXPECT_TRUE(reader.ReadSigned8(&val1).ReadSigned8(&val2).IsSuccess());
        EXPECT_EQ(val1, -2);
        EXPECT_EQ(val2, -1);
    }

    // Signed 16 bits reads
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        int16_t val1 = 0;
        int16_t val2 = 0;
        EXPECT_TRUE(reader.ReadSigned16(&val1).ReadSigned16(&val2).IsSuccess());
        EXPECT_EQ(val1, -2);
        EXPECT_EQ(val2, -1);
    }

    // Signed 32 bits reads
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        int32_t val1 = 0;
        int32_t val2 = 0;
        EXPECT_TRUE(reader.ReadSigned32(&val1).ReadSigned32(&val2).IsSuccess());
        EXPECT_EQ(val1, -2);
        EXPECT_EQ(val2, -1);
    }

    // Signed 32 bits reads, unaligned
    {
        uint8_t test_buf2[10] = { 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01 };
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf2 } };

        int32_t val1 = 0;
        int32_t val2 = 0;
        EXPECT_TRUE(reader.Skip(1).ReadSigned32(&val1).ReadSigned32(&val2).IsSuccess());
        EXPECT_EQ(reader.Remaining(), 1u);
        EXPECT_EQ(val1, static_cast<int32_t>(-2L));
        EXPECT_EQ(val2, static_cast<int32_t>(-1L));
    }

    // Signed 64 bits read
    {
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf1 } };
        int64_t val = 0;
        EXPECT_TRUE(reader.ReadSigned64(&val).IsSuccess());
        EXPECT_EQ(reader.Remaining(), 2u);
        EXPECT_EQ(val, static_cast<int64_t>(-2LL));
    }

    // Bools
    {
        uint8_t test_buf2[5] = { 0x00, 0xff, 0x01, 0x04, 0x07 };
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf2 } };
        bool val1 = true;
        bool val2 = false;
        bool val3 = false;

        EXPECT_TRUE(reader.ReadBool(&val1).ReadBool(&val2).ReadBool(&val3).IsSuccess());
        EXPECT_EQ(reader.Remaining(), 2u);
        EXPECT_EQ(val1, false);
        EXPECT_EQ(val2, true);
        EXPECT_EQ(val3, true);
    }

    // Chars
    {
        uint8_t test_buf2[5] = { 'a', '\0', static_cast<uint8_t>('\xff'), 'b', 'c' };
        chip::Encoding::LittleEndian::Reader reader{ ByteSpan{ test_buf2 } };
        char val1 = 'z';
        char val2 = 'z';
        char val3 = 'z';

        EXPECT_TRUE(reader.ReadChar(&val1).ReadChar(&val2).ReadChar(&val3).IsSuccess());
        EXPECT_EQ(reader.Remaining(), 2u);
        EXPECT_EQ(val1, 'a');
        EXPECT_EQ(val2, '\0');
        EXPECT_EQ(val3, '\xff');
    }
}
