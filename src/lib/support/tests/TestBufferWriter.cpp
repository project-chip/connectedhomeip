/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <lib/support/BufferWriter.h>

namespace {

template <class Base>
class BWTest : public Base
{
public:
    static constexpr size_t kLen    = 64;
    static constexpr uint8_t kGuard = 0xfe;

    uint8_t mBuf[kLen];
    size_t mLen;

    BWTest(size_t len) : Base(mBuf + 1, len), mLen(len) { memset(mBuf, kGuard, kLen); }

    bool expect(const void * val, size_t needed, size_t available)
    {
        // check guards
        for (size_t i = mLen + 1; i < sizeof(mBuf); i++)
        {
            if (mBuf[i] != kGuard)
            {
                printf("Guards failure at index %d\n", static_cast<int>(i));
                return false;
            }
        }
        if (mBuf[0] != kGuard)
        {
            printf("Guards failure at index 0: buffer underflow\n");
            return false;
        }

        size_t written = 0xcafebabe;
        bool fit       = Base::Fit(written);
        if (written == 0xcafebabe)
        {
            printf("Fit(written) didn't set written\n");
            return false;
        }
        if ((fit && (mLen < needed || written != needed)) || (!fit && (mLen >= needed || written != mLen)))
        {
            printf("Fit(written) is wrong: mLen == %u, needed == %u, written == %u, Fit() == %s\n", static_cast<unsigned int>(mLen),
                   static_cast<unsigned int>(needed), static_cast<unsigned int>(written), fit ? "true" : "false");
            return false;
        }

        // check everything else
        if (memcmp(mBuf + 1, val, needed < mLen ? needed : mLen) != 0)
        {
            printf("Memory comparison failed.\n");
            return false;
        }

        return Base::Available() == available && Base::Needed() == needed;
    }
};

using namespace chip::Encoding;

TEST(TestBufferWriter, TestSpanVersusRegular)
{
    uint8_t buf_regular[5]    = { 0, 0, 0, 0, 0 };
    uint8_t buf_span[5]       = { 0, 0, 0, 0, 0 };
    uint8_t all_zeroes[5]     = { 0, 0, 0, 0, 0 };
    uint8_t final_expected[5] = { 1, 2, 3, 4, 0 };

    BufferWriter regular_writer(buf_regular, sizeof(buf_regular));
    BufferWriter span_writer(chip::MutableByteSpan{ buf_span });

    EXPECT_EQ(regular_writer.Available(), sizeof(buf_regular));
    EXPECT_EQ(span_writer.Available(), sizeof(buf_span));

    EXPECT_EQ(0, memcmp(buf_regular, all_zeroes, sizeof(all_zeroes)));
    EXPECT_EQ(0, memcmp(buf_span, all_zeroes, sizeof(all_zeroes)));

    EXPECT_TRUE(regular_writer.Put(1).Put(2).Put(3).Put(4).Fit());
    EXPECT_TRUE(span_writer.Put(1).Put(2).Put(3).Put(4).Fit());

    EXPECT_EQ(0, memcmp(buf_regular, final_expected, sizeof(final_expected)));
    EXPECT_EQ(0, memcmp(buf_span, final_expected, sizeof(final_expected)));
}

TEST(TestBufferWriter, TestStringWrite)
{
    {
        BWTest<BufferWriter> bb(2);
        bb.Put("hi");
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<BufferWriter> bb(1);
        bb.Put("hi");
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(2);
        bb.Put("hi");
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<LittleEndian::BufferWriter> bb(1);
        bb.Put("hi");
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(2);
        bb.Put("hi");
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<BigEndian::BufferWriter> bb(1);
        bb.Put("hi");
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
}

TEST(TestBufferWriter, TestBufferWrite)
{
    {
        BWTest<BufferWriter> bb(2);
        bb.Put("hithere", 2);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<BufferWriter> bb(1);
        bb.Put("hithere", 2);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(2);
        bb.Put("hithere", 2);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<LittleEndian::BufferWriter> bb(1);
        bb.Put("hithere", 2);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(2);
        bb.Put("hithere", 2);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<BigEndian::BufferWriter> bb(1);
        bb.Put("hithere", 2);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
}

TEST(TestBufferWriter, TestPutLittleEndian)
{
    {
        BWTest<LittleEndian::BufferWriter> bb(2);
        bb.Put16('h' + 'i' * 256);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }
    {
        BWTest<LittleEndian::BufferWriter> bb(4);
        bb.Put32(0x01020304);
        EXPECT_TRUE(bb.expect("\x04\x03\x02\x01", 4, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(8);
        bb.Put64(0x0102030405060708);
        EXPECT_TRUE(bb.expect("\x08\x07\x06\x05\x04\x03\x02\x01", 8, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(3);
        bb.EndianPut(0x0102030405060708u, 3);
        EXPECT_TRUE(bb.expect("\x08\x07\x06", 3, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(4);
        bb.PutSigned8(static_cast<int8_t>(-6));
        EXPECT_TRUE(bb.expect("\xfa", 1, 3));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(4);
        bb.PutSigned16(static_cast<int16_t>(-2));
        EXPECT_TRUE(bb.expect("\xfe\xff", 2, 2));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(4);
        bb.PutSigned32(static_cast<int32_t>(-2));
        EXPECT_TRUE(bb.expect("\xfe\xff\xff\xff", 4, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(8);
        bb.PutSigned64(static_cast<int64_t>(-2));
        EXPECT_TRUE(bb.expect("\xfe\xff\xff\xff\xff\xff\xff\xff", 8, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(7);
        bb.PutSigned64(static_cast<int64_t>(-2));
        EXPECT_TRUE(bb.expect("\xfe\xff\xff\xff\xff\xff\xff", 8, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(9);
        bb.PutSigned64(static_cast<int64_t>(9223372036854775807LL));
        EXPECT_TRUE(bb.expect("\xff\xff\xff\xff\xff\xff\xff\x7f", 8, 1));
    }
}

TEST(TestBufferWriter, TestPutBigEndian)
{
    {
        BWTest<BigEndian::BufferWriter> bb(2);
        bb.Put16('i' + 'h' * 256);
        EXPECT_TRUE(bb.expect("hi", 2, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(4);
        bb.Put32(0x01020304);
        EXPECT_TRUE(bb.expect("\x01\x02\x03\x04", 4, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(8);
        bb.Put64(0x0102030405060708);
        EXPECT_TRUE(bb.expect("\x01\x02\x03\x04\x05\x06\x07\x08", 8, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(3);
        bb.EndianPut(0x0102030405060708u, 3);
        EXPECT_TRUE(bb.expect("\x06\x07\x08", 3, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(4);
        bb.PutSigned8(static_cast<int8_t>(-6));
        EXPECT_TRUE(bb.expect("\xfa", 1, 3));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(4);
        bb.PutSigned16(static_cast<int16_t>(-2));
        EXPECT_TRUE(bb.expect("\xff\xfe", 2, 2));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(4);
        bb.PutSigned32(static_cast<int32_t>(-2));
        EXPECT_TRUE(bb.expect("\xff\xff\xff\xfe", 4, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(8);
        bb.PutSigned64(static_cast<int64_t>(-2));
        EXPECT_TRUE(bb.expect("\xff\xff\xff\xff\xff\xff\xff\xfe", 8, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(7);
        bb.PutSigned64(static_cast<int64_t>(-2));
        EXPECT_TRUE(bb.expect("\xff\xff\xff\xff\xff\xff\xff", 8, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(9);
        bb.PutSigned64(static_cast<int64_t>(9223372036854775807LL));
        EXPECT_TRUE(bb.expect("\x7f\xff\xff\xff\xff\xff\xff\xff", 8, 1));
    }
}
} // namespace
