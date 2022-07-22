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
#include <lib/support/BufferWriter.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

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

void TestSpanVersusRegular(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf_regular[5]    = { 0, 0, 0, 0, 0 };
    uint8_t buf_span[5]       = { 0, 0, 0, 0, 0 };
    uint8_t all_zeroes[5]     = { 0, 0, 0, 0, 0 };
    uint8_t final_expected[5] = { 1, 2, 3, 4, 0 };

    BufferWriter regular_writer(buf_regular, sizeof(buf_regular));
    BufferWriter span_writer(chip::MutableByteSpan{ buf_span });

    NL_TEST_ASSERT(inSuite, regular_writer.Available() == sizeof(buf_regular));
    NL_TEST_ASSERT(inSuite, span_writer.Available() == sizeof(buf_span));

    NL_TEST_ASSERT(inSuite, 0 == memcmp(buf_regular, all_zeroes, sizeof(all_zeroes)));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(buf_span, all_zeroes, sizeof(all_zeroes)));

    NL_TEST_ASSERT(inSuite, regular_writer.Put(1).Put(2).Put(3).Put(4).Fit());
    NL_TEST_ASSERT(inSuite, span_writer.Put(1).Put(2).Put(3).Put(4).Fit());

    NL_TEST_ASSERT(inSuite, 0 == memcmp(buf_regular, final_expected, sizeof(final_expected)));
    NL_TEST_ASSERT(inSuite, 0 == memcmp(buf_span, final_expected, sizeof(final_expected)));
}

void TestStringWrite(nlTestSuite * inSuite, void * inContext)
{
    {
        BWTest<BufferWriter> bb(2);
        bb.Put("hi");
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<BufferWriter> bb(1);
        bb.Put("hi");
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(2);
        bb.Put("hi");
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<LittleEndian::BufferWriter> bb(1);
        bb.Put("hi");
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(2);
        bb.Put("hi");
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<BigEndian::BufferWriter> bb(1);
        bb.Put("hi");
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
}

void TestBufferWrite(nlTestSuite * inSuite, void * inContext)
{
    {
        BWTest<BufferWriter> bb(2);
        bb.Put("hithere", 2);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<BufferWriter> bb(1);
        bb.Put("hithere", 2);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(2);
        bb.Put("hithere", 2);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<LittleEndian::BufferWriter> bb(1);
        bb.Put("hithere", 2);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(2);
        bb.Put("hithere", 2);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<BigEndian::BufferWriter> bb(1);
        bb.Put("hithere", 2);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
}

void TestPutLittleEndian(nlTestSuite * inSuite, void * inContext)
{
    {
        BWTest<LittleEndian::BufferWriter> bb(2);
        bb.Put16('h' + 'i' * 256);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }
    {
        BWTest<LittleEndian::BufferWriter> bb(4);
        bb.Put32(0x01020304);
        NL_TEST_ASSERT(inSuite, bb.expect("\x04\x03\x02\x01", 4, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(8);
        bb.Put64(0x0102030405060708);
        NL_TEST_ASSERT(inSuite, bb.expect("\x08\x07\x06\x05\x04\x03\x02\x01", 8, 0));
    }

    {
        BWTest<LittleEndian::BufferWriter> bb(3);
        bb.EndianPut(0x0102030405060708u, 3);
        NL_TEST_ASSERT(inSuite, bb.expect("\x08\x07\x06", 3, 0));
    }
}

void TestPutBigEndian(nlTestSuite * inSuite, void * inContext)
{
    {
        BWTest<BigEndian::BufferWriter> bb(2);
        bb.Put16('i' + 'h' * 256);
        NL_TEST_ASSERT(inSuite, bb.expect("hi", 2, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(4);
        bb.Put32(0x01020304);
        NL_TEST_ASSERT(inSuite, bb.expect("\x01\x02\x03\x04", 4, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(8);
        bb.Put64(0x0102030405060708);
        NL_TEST_ASSERT(inSuite, bb.expect("\x01\x02\x03\x04\x05\x06\x07\x08", 8, 0));
    }

    {
        BWTest<BigEndian::BufferWriter> bb(3);
        bb.EndianPut(0x0102030405060708u, 3);
        NL_TEST_ASSERT(inSuite, bb.expect("\x06\x07\x08", 3, 0));
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestSpanVersusRegular", TestSpanVersusRegular), //
    NL_TEST_DEF("TestStringWrite", TestStringWrite),             //
    NL_TEST_DEF("TestBufferWrite", TestBufferWrite),             //
    NL_TEST_DEF("TestPutLittleEndian", TestPutLittleEndian),     //
    NL_TEST_DEF("TestPutBigEndian", TestPutBigEndian),           //
    NL_TEST_SENTINEL()                                           //
};

} // namespace

int TestBufferWriter(void)
{
    nlTestSuite theSuite = { "BufferWriter", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBufferWriter)
