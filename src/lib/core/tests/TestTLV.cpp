/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      This file implements unit tests for the CHIP TLV implementation.
 *
 */

#include <nlbyteorder.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVCircularBuffer.h>
#include <lib/core/TLVData.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestUtils.h>
#include <lib/support/logging/Constants.h>

#include <system/TLVPacketBufferBackingStore.h>

#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::TLV;

enum
{
    TestProfile_1 = 0xAABBCCDD,
    TestProfile_2 = 0x11223344,
    TestProfile_3 = 0x11223355
};

// clang-format off
static const char sLargeString [] =
    "START..."
    "!123456789ABCDEF@123456789ABCDEF#123456789ABCDEF$123456789ABCDEF%123456789ABCDEF^123456789ABCDEF&123456789ABCDEF*123456789ABCDEF"
    "01234567(9ABCDEF01234567)9ABCDEF01234567-9ABCDEF01234567=9ABCDEF01234567[9ABCDEF01234567]9ABCDEF01234567;9ABCDEF01234567'9ABCDEF"
    "...END";
// clang-format on

void TestAndOpenContainer(TLVReader & reader, TLVType type, Tag tag, TLVReader & containerReader)
{
    EXPECT_EQ(reader.GetType(), type);
    EXPECT_EQ(reader.GetTag(), tag);
    EXPECT_EQ(reader.GetLength(), 0u);

    CHIP_ERROR err = reader.OpenContainer(containerReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(containerReader.GetContainerType(), type);
}

template <class T>
void TestAndEnterContainer(T & t, TLVType type, Tag tag, TLVType & outerContainerType)
{
    EXPECT_EQ(t.GetType(), type);
    EXPECT_EQ(t.GetTag(), tag);
    EXPECT_EQ(t.GetLength(), 0u);

    TLVType expectedContainerType = t.GetContainerType();

    CHIP_ERROR err = t.EnterContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(outerContainerType, expectedContainerType);
    EXPECT_EQ(t.GetContainerType(), type);
}

template <class T>
void TestNext(T & t)
{
    CHIP_ERROR err = t.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestSkip(TLVReader & reader)
{
    CHIP_ERROR err = reader.Skip();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void TestMove(TLVUpdater & updater)
{
    CHIP_ERROR err = updater.Move();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

template <class T>
void TestEnd(T & t)
{
    CHIP_ERROR err;

    err = t.Next();
    EXPECT_EQ(err, CHIP_END_OF_TLV);
}

void TestEndAndCloseContainer(TLVReader & reader, TLVReader & containerReader)
{
    CHIP_ERROR err;

    TestEnd<TLVReader>(containerReader);

    err = reader.CloseContainer(containerReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

template <class T>
void TestEndAndExitContainer(T & t, TLVType outerContainerType)
{
    CHIP_ERROR err;

    TestEnd<T>(t);

    err = t.ExitContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(t.GetContainerType(), outerContainerType);
}

#define TEST_GET(s, type, tag, expectedVal, expectedErr)                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        EXPECT_EQ(s.GetType(), type);                                                                                              \
        EXPECT_EQ(s.GetTag(), tag);                                                                                                \
        EXPECT_EQ(s.GetLength(), 0u);                                                                                              \
                                                                                                                                   \
        decltype(expectedVal) __val;                                                                                               \
        CHIP_ERROR __err = s.Get(__val);                                                                                           \
        EXPECT_EQ(__err, expectedErr);                                                                                             \
        if (__err == CHIP_NO_ERROR)                                                                                                \
        {                                                                                                                          \
            EXPECT_EQ(__val, expectedVal);                                                                                         \
        }                                                                                                                          \
    } while (false)

#define TEST_GET_NOERROR(s, type, tag, expectedVal) TEST_GET(s, type, tag, expectedVal, CHIP_NO_ERROR)

void ForEachElement(TLVReader & reader, void * context, void (*cb)(TLVReader & reader, void * context))
{
    CHIP_ERROR err;

    while (true)
    {
        err = reader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            return;
        }
        EXPECT_EQ(err, CHIP_NO_ERROR);

        if (cb != nullptr)
        {
            cb(reader, context);
        }

        if (TLVTypeIsContainer(reader.GetType()))
        {
            TLVType outerContainerType;

            err = reader.EnterContainer(outerContainerType);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            ForEachElement(reader, context, cb);

            err = reader.ExitContainer(outerContainerType);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }
    }
}

/**
 * context
 */

struct TestTLVContext
{
    int mEvictionCount     = 0;
    uint32_t mEvictedBytes = 0;
};

class TestTLV : public ::testing::Test
{
public:
    static TestTLVContext ctx;
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TestTLVContext TestTLV::ctx;

void TestNull(TLVReader & reader, Tag tag)
{
    EXPECT_EQ(reader.GetType(), kTLVType_Null);
    EXPECT_EQ(reader.GetTag(), tag);
    EXPECT_EQ(reader.GetLength(), 0u);
}

void TestString(TLVReader & reader, Tag tag, const char * expectedVal)
{
    EXPECT_EQ(reader.GetType(), kTLVType_UTF8String);
    EXPECT_EQ(reader.GetTag(), tag);

    size_t expectedLen = strlen(expectedVal);
    EXPECT_EQ(reader.GetLength(), expectedLen);

    chip::Platform::ScopedMemoryBuffer<char> valBuffer;
    char * val = static_cast<char *>(valBuffer.Alloc(expectedLen + 1).Get());

    CHIP_ERROR err = reader.GetString(val, static_cast<uint32_t>(expectedLen) + 1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(memcmp(val, expectedVal, expectedLen + 1), 0);
}

void TestDupString(TLVReader & reader, Tag tag, const char * expectedVal)
{
    EXPECT_EQ(reader.GetType(), kTLVType_UTF8String);
    EXPECT_EQ(reader.GetTag(), tag);

    size_t expectedLen = strlen(expectedVal);
    EXPECT_EQ(reader.GetLength(), expectedLen);

    char * val     = nullptr;
    CHIP_ERROR err = reader.DupString(val);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(memcmp(val, expectedVal, expectedLen + 1), 0);
    chip::Platform::MemoryFree(val);
}

void TestDupBytes(TLVReader & reader, Tag tag, const uint8_t * expectedVal, uint32_t expectedLen)
{
    EXPECT_EQ(reader.GetType(), kTLVType_UTF8String);
    EXPECT_EQ(reader.GetTag(), tag);

    EXPECT_EQ(reader.GetLength(), expectedLen);

    uint8_t * val  = nullptr;
    CHIP_ERROR err = reader.DupBytes(val, expectedLen);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(memcmp(val, expectedVal, expectedLen), 0);
    chip::Platform::MemoryFree(val);
}

void TestBufferContents(const System::PacketBufferHandle & buffer, const uint8_t * expectedVal, size_t expectedLen)
{
    System::PacketBufferHandle buf = buffer.Retain();
    while (!buf.IsNull())
    {
        size_t len = buf->DataLength();
        EXPECT_LE(len, expectedLen);

        EXPECT_EQ(memcmp(buf->Start(), expectedVal, len), 0);

        expectedVal += len;
        expectedLen -= len;

        buf.Advance();
    }

    EXPECT_EQ(expectedLen, 0u);
}

// clang-format off
static const uint8_t Encoding1[] =
{
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00, 0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00, 0x88, 0x02,
    0x00, 0x36, 0x00, 0x00, 0x2A, 0x00, 0xEF, 0x02, 0xF0, 0x67, 0xFD, 0xFF, 0x07, 0x00, 0x90, 0x2F,
    0x50, 0x09, 0x00, 0x00, 0x00, 0x15, 0x18, 0x17, 0xD4, 0xBB, 0xAA, 0xDD, 0xCC, 0x11, 0x00, 0xB4,
    0xA0, 0xBB, 0x0D, 0x00, 0x14, 0xB5, 0x00, 0x28, 0x6B, 0xEE, 0x6D, 0x70, 0x11, 0x01, 0x00, 0x0E,
    0x01, 0x53, 0x54, 0x41, 0x52, 0x54, 0x2E, 0x2E, 0x2E, 0x21, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x40, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x23, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x24, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x25, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x5E, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x26, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x2A, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x28, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x29, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x2D, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x3D, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x5B, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x5D, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x3B, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x27, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x2E, 0x2E, 0x2E, 0x45, 0x4E, 0x44, 0x18,
    0x18, 0x18, 0xCC, 0xBB, 0xAA, 0xDD, 0xCC, 0x05, 0x00, 0x0E, 0x54, 0x68, 0x69, 0x73, 0x20, 0x69,
    0x73, 0x20, 0x61, 0x20, 0x74, 0x65, 0x73, 0x74, 0x8A, 0xFF, 0xFF, 0x33, 0x33, 0x8F, 0x41, 0xAB,
    0x00, 0x00, 0x01, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0xE6, 0x31, 0x40, 0x18
};
// clang-format on

// clang-format off
static const uint8_t Encoding1_DataMacro [] =
{
    CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 1)),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 2), true),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(2), false),
        CHIP_TLV_ARRAY(CHIP_TLV_TAG_CONTEXT_SPECIFIC(0)),
            CHIP_TLV_INT8(CHIP_TLV_TAG_ANONYMOUS, 42),
            CHIP_TLV_INT8(CHIP_TLV_TAG_ANONYMOUS, -17),
            CHIP_TLV_INT32(CHIP_TLV_TAG_ANONYMOUS, -170000),
            CHIP_TLV_UINT64(CHIP_TLV_TAG_ANONYMOUS, 40000000000ULL),
            CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
            CHIP_TLV_END_OF_CONTAINER,
            CHIP_TLV_LIST(CHIP_TLV_TAG_ANONYMOUS),
                CHIP_TLV_NULL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 17)),
                CHIP_TLV_NULL(CHIP_TLV_TAG_IMPLICIT_PROFILE_4Bytes(900000)),
                CHIP_TLV_NULL(CHIP_TLV_TAG_ANONYMOUS),
                CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_IMPLICIT_PROFILE_4Bytes(4000000000ULL)),
                    CHIP_TLV_UTF8_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_4Bytes(70000), sizeof(sLargeString) - 1,
                    'S', 'T', 'A', 'R', 'T', '.', '.', '.',
                    '!', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '@',
                    '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '#', '1',
                    '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '$', '1', '2',
                    '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '%', '1', '2', '3',
                    '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '^', '1', '2', '3', '4',
                    '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '&', '1', '2', '3', '4', '5',
                    '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '*', '1', '2', '3', '4', '5', '6',
                    '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
                    '0', '1', '2', '3', '4', '5', '6', '7', '(', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0',
                    '1', '2', '3', '4', '5', '6', '7', ')', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1',
                    '2', '3', '4', '5', '6', '7', '-', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2',
                    '3', '4', '5', '6', '7', '=', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3',
                    '4', '5', '6', '7', '[', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4',
                    '5', '6', '7', ']', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5',
                    '6', '7', ';', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5', '6',
                    '7', '\'', '9', 'A', 'B', 'C', 'D', 'E', 'F',
                    '.', '.', '.', 'E', 'N', 'D'),
                CHIP_TLV_END_OF_CONTAINER,
            CHIP_TLV_END_OF_CONTAINER,
        CHIP_TLV_END_OF_CONTAINER,
        CHIP_TLV_UTF8_STRING_1ByteLength(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 5), sizeof("This is a test") - 1,
            'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't'),
        CHIP_TLV_FLOAT32(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(65535),
            0x33, 0x33, 0x8f, 0x41), // (float)17.9
        CHIP_TLV_FLOAT64(CHIP_TLV_TAG_IMPLICIT_PROFILE_4Bytes(65536),
            0x66, 0x66, 0x66, 0x66, 0x66, 0xE6, 0x31, 0x40), // (double)17.9
    CHIP_TLV_END_OF_CONTAINER
};
// clang-format on

static CHIP_ERROR WriteIntMinMax(TLVWriter & writer)
{
    CHIP_ERROR err;

    err = writer.Put(AnonymousTag(), static_cast<int8_t>(INT8_MIN));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int8_t>(INT8_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int16_t>(INT16_MIN));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int16_t>(INT16_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int32_t>(INT32_MIN));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int32_t>(INT32_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int64_t>(INT64_MIN));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<int64_t>(INT64_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<uint8_t>(UINT8_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<uint16_t>(UINT16_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<uint32_t>(UINT32_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(AnonymousTag(), static_cast<uint64_t>(UINT64_MAX));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    return err;
}

static void CheckIntMinMax(TLVReader & reader)
{
    // Writer did Put(AnonymousTag(), static_cast<int8_t>(INT8_MIN))
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(INT8_MIN));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(INT8_MIN));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(INT8_MIN));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT8_MIN));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int8_t>(INT8_MAX))
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(INT8_MAX));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(INT8_MAX));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(INT8_MAX));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT8_MAX));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int16_t>(INT16_MIN))
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(INT16_MIN));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(INT16_MIN));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT16_MIN));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int16_t>(INT16_MAX))
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(INT16_MAX));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(INT16_MAX));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT16_MAX));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int32_t>(INT32_MIN))
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(INT32_MIN));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT32_MIN));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int32_t>(INT32_MAX))
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(INT32_MAX));
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT32_MAX));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int64_t>(INT64_MIN))
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT64_MIN));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<int64_t>(INT64_MAX))
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(INT64_MAX));

    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<uint8_t>(UINT8_MAX))
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint8_t>(UINT8_MAX));
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint16_t>(UINT8_MAX));
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint32_t>(UINT8_MAX));
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint64_t>(UINT8_MAX));

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<uint16_t>(UINT16_MAX))
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint16_t>(UINT16_MAX));
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint32_t>(UINT16_MAX));
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint64_t>(UINT16_MAX));

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<uint32_t>(UINT32_MAX))
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint32_t>(UINT32_MAX));
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint64_t>(UINT32_MAX));

    TestNext<TLVReader>(reader);

    // Writer did Put(AnonymousTag(), static_cast<uint64_t>(UINT64_MAX))
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int8_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int16_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int32_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int64_t>(0), CHIP_ERROR_WRONG_TLV_TYPE);

    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint8_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint16_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint32_t>(0), CHIP_ERROR_INVALID_INTEGER_VALUE);
    TEST_GET_NOERROR(reader, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint64_t>(UINT64_MAX));
}

void WriteEncoding1(TLVWriter & writer)
{
    CHIP_ERROR err;
    TLVWriter writer2;

    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.PutBoolean(ProfileTag(TestProfile_2, 2), false);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        TLVWriter writer3;

        err = writer2.OpenContainer(ContextTag(0), kTLVType_Array, writer3);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // TODO(#1306): expand coverage of inttype encoding tests.
        err = writer3.Put(AnonymousTag(), static_cast<int32_t>(42));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int32_t>(-17));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int32_t>(-170000));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<uint64_t>(40000000000ULL));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        {
            TLVWriter writer4;

            err = writer3.OpenContainer(AnonymousTag(), kTLVType_Structure, writer4);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer3.CloseContainer(writer4);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        {
            TLVWriter writer5;

            err = writer3.OpenContainer(AnonymousTag(), kTLVType_List, writer5);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer5.PutNull(ProfileTag(TestProfile_1, 17));
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer5.PutNull(ProfileTag(TestProfile_2, 900000));
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer5.PutNull(AnonymousTag());
            EXPECT_EQ(err, CHIP_NO_ERROR);

            {
                TLVType outerContainerType;

                err = writer5.StartContainer(ProfileTag(TestProfile_2, 4000000000ULL), kTLVType_Structure, outerContainerType);
                EXPECT_EQ(err, CHIP_NO_ERROR);

                err = writer5.PutString(CommonTag(70000), sLargeString);
                EXPECT_EQ(err, CHIP_NO_ERROR);

                err = writer5.EndContainer(outerContainerType);
                EXPECT_EQ(err, CHIP_NO_ERROR);
            }

            err = writer3.CloseContainer(writer5);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        err = writer2.CloseContainer(writer3);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer2.PutString(ProfileTag(TestProfile_1, 5), "This is a test");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.Put(ProfileTag(TestProfile_2, 65535), static_cast<float>(17.9));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.Put(ProfileTag(TestProfile_2, 65536), 17.9);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void WriteEmptyEncoding(TLVWriter & writer)
{
    CHIP_ERROR err;
    TLVWriter writer2;

    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        TLVWriter writer3;

        err = writer2.OpenContainer(ProfileTag(TestProfile_1, 256), kTLVType_Array, writer3);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.CloseContainer(writer3);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ReadEncoding1(TLVReader & reader)
{
    TestNext<TLVReader>(reader);

    {
        TLVReader reader2;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(reader2);

        {
            TLVReader reader3;

            TestAndOpenContainer(reader2, kTLVType_Array, ContextTag(0), reader3);

            TestNext<TLVReader>(reader3);

            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(42));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(42));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(42));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(42));
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint8_t>(42), CHIP_ERROR_WRONG_TLV_TYPE);
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint16_t>(42), CHIP_ERROR_WRONG_TLV_TYPE);
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(42), CHIP_ERROR_WRONG_TLV_TYPE);
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(42), CHIP_ERROR_WRONG_TLV_TYPE);

            TestNext<TLVReader>(reader3);

            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(-17));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(-17));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(-17));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(-17));
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(-17), CHIP_ERROR_WRONG_TLV_TYPE);

            TestNext<TLVReader>(reader3);

            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(-170000));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(-170000));

            TestNext<TLVReader>(reader3);

            TEST_GET(reader3, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int64_t>(40000000000ULL),
                     CHIP_ERROR_WRONG_TLV_TYPE);
            TEST_GET_NOERROR(reader3, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint64_t>(40000000000ULL));

            TestNext<TLVReader>(reader3);

            {
                TLVReader reader4;

                TestAndOpenContainer(reader3, kTLVType_Structure, AnonymousTag(), reader4);

                TestEndAndCloseContainer(reader3, reader4);
            }

            TestNext<TLVReader>(reader3);

            {
                TLVReader reader5;

                TestAndOpenContainer(reader3, kTLVType_List, AnonymousTag(), reader5);

                TestNext<TLVReader>(reader5);

                TestNull(reader5, ProfileTag(TestProfile_1, 17));

                TestNext<TLVReader>(reader5);

                TestNull(reader5, ProfileTag(TestProfile_2, 900000));

                TestNext<TLVReader>(reader5);

                TestNull(reader5, AnonymousTag());

                TestNext<TLVReader>(reader5);

                {
                    TLVType outerContainerType;

                    TestAndEnterContainer<TLVReader>(reader5, kTLVType_Structure, ProfileTag(TestProfile_2, 4000000000ULL),
                                                     outerContainerType);

                    TestNext<TLVReader>(reader5);

                    TestString(reader5, CommonTag(70000), sLargeString);

                    TestEndAndExitContainer<TLVReader>(reader5, outerContainerType);
                }

                TestEndAndCloseContainer(reader3, reader5);
            }

            TestEndAndCloseContainer(reader2, reader3);
        }

        TestNext<TLVReader>(reader2);

        TestString(reader2, ProfileTag(TestProfile_1, 5), "This is a test");

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), 17.9f);
        TEST_GET_NOERROR(reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), static_cast<double>(17.9f));

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65536), 17.9);

        TestEndAndCloseContainer(reader, reader2);
    }

    TestEnd<TLVReader>(reader);
}

void WriteEncoding2(TLVWriter & writer)
{
    CHIP_ERROR err;

    { // Container 1
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    { // Container 2
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void WriteEncoding3(TLVWriter & writer)
{
    CHIP_ERROR err;

    { // Container 1
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

void ReadEncoding3(TLVReader & reader)
{
    TLVReader reader2;

    TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

    TestNext<TLVReader>(reader2);

    TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

    TestEndAndCloseContainer(reader, reader2);
}
// clang-format off
static const uint8_t Encoding5_DataMacro [] =
{
    CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 1)),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 2), true),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(2), false),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 2), false),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(2), true),

        CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 1)),
            CHIP_TLV_BOOL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 2), true),
            CHIP_TLV_BOOL(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(2), false),
        CHIP_TLV_END_OF_CONTAINER,

        CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(1)),
            CHIP_TLV_BOOL(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(2), false),
            CHIP_TLV_BOOL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 2), true),
        CHIP_TLV_END_OF_CONTAINER,
    CHIP_TLV_END_OF_CONTAINER,

    CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(1)),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_IMPLICIT_PROFILE_2Bytes(2), false),
        CHIP_TLV_BOOL(CHIP_TLV_TAG_FULLY_QUALIFIED_6Bytes(TestProfile_1, 2), true),
    CHIP_TLV_END_OF_CONTAINER,
};
// clang-format on

void WriteEncoding5(TLVWriter & writer)
{
    CHIP_ERROR err;

    { // Container 1
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        { // Inner Container 1
            TLVWriter writer2;

            err = writer1.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer1.CloseContainer(writer2);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        { // Inner Container 2
            TLVWriter writer2;

            err = writer1.OpenContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, writer2);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            err = writer1.CloseContainer(writer2);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        err = writer.CloseContainer(writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    { // Container 2
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

/**
 * AppendEncoding2()
 *
 * This function appends two boolean types and two container types to the first
 * container.
 *
 * The boolean types are-
 * <TestProfile_1, 2, false>
 * <TestProfile_2, 2, true>
 *
 * The two new container types are-
 * <TestProfile_1, 1, kTLVType_Structure, <TestProfile_1, 2, true> <TestProfile_2, 2, false> >,
 * <TestProfile_2, 1, kTLVType_Structure, <TestProfile_2, 2, false> <TestProfile_1, 2, true> >
 */
void AppendEncoding2(uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    TestNext<TLVUpdater>(updater);

    {
        TLVType outerContainerType;

        TestAndEnterContainer<TLVUpdater>(updater, kTLVType_Structure, ProfileTag(TestProfile_1, 1), outerContainerType);

        TestNext<TLVUpdater>(updater);

        // Move the element without modification
        TestMove(updater);

        TestNext<TLVUpdater>(updater);

        // Read and copy the element with/without modification
        TEST_GET_NOERROR(updater, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // TestEnd and add data at the end of the container
        TestEnd<TLVUpdater>(updater);

        // Put new values in the encoding using the updater
        // Add <TestProfile_1, 2, false>
        err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add <TestProfile_2, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add a new container
        {
            TLVType outerContainerType1;

            err = updater.StartContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, outerContainerType1);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            // Add <TestProfile_1, 2, true>
            err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            // Add <TestProfile_1, 2, true>
            err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            // Close the container
            err = updater.EndContainer(outerContainerType1);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        // Add another new container
        {
            TLVType outerContainerType1;

            err = updater.StartContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, outerContainerType1);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            // Add <TestProfile_2, 2, false>
            err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            // Add <TestProfile_1, 2, true>
            err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            // Close the container
            err = updater.EndContainer(outerContainerType1);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        TestEndAndExitContainer<TLVUpdater>(updater, outerContainerType);
    }

    TestNext<TLVUpdater>(updater);

    // Move the container unmodified
    TestMove(updater);

    TestEnd<TLVUpdater>(updater);

    err = updater.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

/**
 * FindAppendEncoding2()
 *
 * This function appends two boolean types and two container types to the first
 * container. It is very similar to AppendEncoding2() above except for the fact
 * that it uses TLVUtilities::Find() to find the element of interest before
 * appending new data.
 *
 * The boolean types are-
 * <TestProfile_1, 2, false>
 * <TestProfile_2, 2, true>
 *
 * The two new container types are-
 * <TestProfile_1, 1, kTLVType_Structure, <TestProfile_1, 2, true> <TestProfile_2, 2, false> >,
 * <TestProfile_2, 1, kTLVType_Structure, <TestProfile_2, 2, false> <TestProfile_1, 2, true> >
 */
void FindAppendEncoding2(uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen, bool findContainer)
{
    CHIP_ERROR err;

    TLVReader reader;
    TLVUpdater updater;

    // Initialize a reader
    reader.Init(buf, dataLen);
    reader.ImplicitProfileId = TestProfile_2;

    if (findContainer)
    {
        // Find the container
        TLVReader tagReader;
        TLVType outerContainerType;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_1, 1), tagReader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = tagReader.EnterContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        do
        {
            err = tagReader.Next();
        } while (err != CHIP_END_OF_TLV);

        TestEnd<TLVReader>(tagReader);

        // Init a TLVUpdater using the TLVReader
        err = updater.Init(tagReader, maxLen - dataLen);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
    else
    {
        // Find
        TLVReader tagReader;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 2), tagReader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Test Find(recurse = true)
        TLVReader tagReader2;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 2), tagReader2, true);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        //
        // Test Find(recurse = false)
        TLVReader tagReader3;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 2), tagReader3, false);
        EXPECT_EQ(err, CHIP_ERROR_TLV_TAG_NOT_FOUND);

        // Init a TLVUpdater using the TLVReader
        err = updater.Init(tagReader, maxLen - dataLen);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        TestNext<TLVUpdater>(updater);

        // Move the element without modification
        TestMove(updater);
    }

    // Put new values in the encoding using the updater
    // Add <TestProfile_1, 2, false>
    err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), false);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add <TestProfile_2, 2, true>
    err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Add a new container
    {
        TLVType outerContainerType1;

        err = updater.StartContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, outerContainerType1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Close the container
        err = updater.EndContainer(outerContainerType1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Add another new container
    {
        TLVType outerContainerType1;

        err = updater.StartContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, outerContainerType1);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add <TestProfile_2, 2, false>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Close the container
        err = updater.EndContainer(outerContainerType1);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Move everything else unmodified
    updater.MoveUntilEnd();

    TestEnd<TLVUpdater>(updater);

    err = updater.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void AppendEncoding3(uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    TestNext<TLVUpdater>(updater);

    {
        TLVType outerContainerType;

        TestAndEnterContainer<TLVUpdater>(updater, kTLVType_Structure, ProfileTag(TestProfile_1, 1), outerContainerType);

        TestNext<TLVUpdater>(updater);

        // Move the element without modification
        TestMove(updater);

        // Put new value in the encoding using the updater
        // Add <TestProfile_2, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        TestEndAndExitContainer<TLVUpdater>(updater, outerContainerType);
    }

    TestEnd<TLVUpdater>(updater);

    err = updater.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void AppendEncoding4(uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    // Add a new container
    {
        TLVType outerContainerType;

        err = updater.StartContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Close the container
        err = updater.EndContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = updater.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void DeleteEncoding5(uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    TestNext<TLVUpdater>(updater);

    {
        TLVType outerContainerType;

        TestAndEnterContainer<TLVUpdater>(updater, kTLVType_Structure, ProfileTag(TestProfile_1, 1), outerContainerType);

        TestNext<TLVUpdater>(updater);

        TestMove(updater);

        TestNext<TLVUpdater>(updater);

        TestMove(updater);

        TestNext<TLVUpdater>(updater);

        // Get the value to inspect and skip writing it
        TEST_GET_NOERROR(updater, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

        TestNext<TLVUpdater>(updater);

        // Skip the next boolean type and don't copy by doing nothing

        TestNext<TLVUpdater>(updater);

        // Read ahead into the next container and decide whether to skip or
        // not based on elements in the container
        {
            TLVReader reader;
            TLVType containerType;

            updater.GetReader(reader);

            TestAndEnterContainer<TLVReader>(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), containerType);

            TestNext<TLVReader>(reader);

            // If the container's first element has the tag <TestProfile_1, 2>
            // skip the whole container, and if NOT copy the container
            if (reader.GetTag() != ProfileTag(TestProfile_1, 2))
                TestMove(updater);
        }

        TestNext<TLVUpdater>(updater);

        // Skip the next container and don't copy by doing nothing

        TestEndAndExitContainer<TLVUpdater>(updater, outerContainerType);
    }

    // Move everything else unmodified
    updater.MoveUntilEnd();

    TestEnd<TLVUpdater>(updater);

    err = updater.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void ReadAppendedEncoding2(TLVReader & reader)
{
    TestNext<TLVReader>(reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), true);

        TestNext<TLVReader>(reader1);

        {
            TLVReader reader2;

            TestAndOpenContainer(reader1, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

            TestNext<TLVReader>(reader2);

            TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

            TestNext<TLVReader>(reader2);

            TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

            TestEndAndCloseContainer(reader1, reader2);
        }

        TestNext<TLVReader>(reader1);

        {
            TLVReader reader2;

            TestAndOpenContainer(reader1, kTLVType_Structure, ProfileTag(TestProfile_2, 1), reader2);

            TestNext<TLVReader>(reader2);

            TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

            TestNext<TLVReader>(reader2);

            TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

            TestEndAndCloseContainer(reader1, reader2);
        }

        TestEndAndCloseContainer(reader, reader1);
    }

    TestNext<TLVReader>(reader);

    { // Container 2
        TLVReader reader1;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_2, 1), reader1);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestEndAndCloseContainer(reader, reader1);
    }

    TestEnd<TLVReader>(reader);
}

void ReadAppendedEncoding3(TLVReader & reader)
{
    TestNext<TLVReader>(reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), true);

        TestEndAndCloseContainer(reader, reader1);
    }

    TestEnd<TLVReader>(reader);
}

void ReadAppendedEncoding4(TLVReader & reader)
{
    TestNext<TLVReader>(reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestEndAndCloseContainer(reader, reader1);
    }

    TestEnd<TLVReader>(reader);
}

void ReadDeletedEncoding5(TLVReader & reader)
{
    TestNext<TLVReader>(reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestEndAndCloseContainer(reader, reader1);
    }

    TestNext<TLVReader>(reader);

    { // Container 2
        TLVReader reader1;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_2, 1), reader1);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(reader1);

        TEST_GET_NOERROR(reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestEndAndCloseContainer(reader, reader1);
    }

    TestEnd<TLVReader>(reader);
}

/**
 *  Test Simple Write and Reader
 */
TEST_F(TestTLV, CheckSimpleWriteRead)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;
    uint32_t remainingFreedLen;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    remainingFreedLen = writer.GetRemainingFreeLength();
    EXPECT_EQ(sizeof(buf), remainingFreedLen);

    WriteEncoding1(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

#ifdef DUMP_ENCODING
    for (uint32_t i = 0; i < encodedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(encodedLen, sizeof(Encoding1));
    EXPECT_EQ(memcmp(buf, Encoding1, encodedLen), 0);

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadEncoding1(reader);
}

TEST_F(TestTLV, TestIntMinMax)
{
    CHIP_ERROR err;

    uint8_t buf[2048];
    TLVWriter writer, writer1;
    TLVReader reader, reader1;

    writer.Init(buf, sizeof(buf));
    writer.ImplicitProfileId = TestProfile_3;

    err = writer.OpenContainer(ProfileTag(TestProfile_3, 1), kTLVType_Array, writer1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    WriteIntMinMax(writer1);

    err = writer.CloseContainer(writer1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reader.Init(buf, sizeof(buf));
    reader.ImplicitProfileId = TestProfile_3;

    TestNext<TLVReader>(reader);

    TestAndOpenContainer(reader, kTLVType_Array, ProfileTag(TestProfile_3, 1), reader1);

    TestNext<TLVReader>(reader1);

    CheckIntMinMax(reader1);

    TestEndAndCloseContainer(reader, reader1);
}

/**
 *  Log the specified message in the form of @a aFormat.
 *
 *  @param[in]     aFormat   A pointer to a NULL-terminated C string with
 *                           C Standard Library-style format specifiers
 *                           containing the log message to be formatted and
 *                           logged.
 *  @param[in]     ...       An argument list whose elements should correspond
 *                           to the format specifiers in @a aFormat.
 *
 */
void ENFORCE_FORMAT(1, 2) SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

/**
 *  Test Pretty Printer
 */
TEST_F(TestTLV, CheckPrettyPrinter)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding1(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

    EXPECT_EQ(encodedLen, sizeof(Encoding1));
    EXPECT_EQ(memcmp(buf, Encoding1, encodedLen), 0);

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

static char gStringDumpWriterBuf[128]        = { 0 };
static size_t gStringDumpWriterLengthWritten = 0;

/**
 *  Log the specified message in the form of @a aFormat.
 *
 *  @param[in]     aFormat   A pointer to a NULL-terminated C string with
 *                           C Standard Library-style format specifiers
 *                           containing the log message to be formatted and
 *                           logged.
 *  @param[in]     ...       An argument list whose elements should correspond
 *                           to the format specifiers in @a aFormat.
 *
 */
void ENFORCE_FORMAT(1, 2) StringDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    gStringDumpWriterLengthWritten +=
        static_cast<size_t>(vsprintf(&gStringDumpWriterBuf[gStringDumpWriterLengthWritten], aFormat, args));

    va_end(args);
}

/**
 *  Test Octet String Pretty Printer
 */
TEST_F(TestTLV, CheckOctetStringPrettyPrinter)
{
    const uint8_t testOctetString[] = { 0x62, 0xFA, 0x82, 0x33, 0x59, 0xAC, 0xFA, 0xA9 };
    const char expectedPrint[] =
        "0x04, tag[Common Profile (2 Bytes)]: 0x0::0x0::0x0, type: Octet String (0x10), length: 8, value: hex:62FA823359ACFAA9\n";
    uint8_t encodedBuf[128] = { 0 };

    TLVWriter writer;
    writer.Init(encodedBuf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBytes(CommonTag(0), testOctetString, sizeof(testOctetString)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    TLVReader reader;
    reader.Init(encodedBuf, writer.GetLengthWritten());

    chip::TLV::Debug::Dump(reader, StringDumpWriter);

    EXPECT_STREQ(expectedPrint, gStringDumpWriterBuf);
}

/**
 *  Test Data Macros
 */
TEST_F(TestTLV, CheckDataMacro)
{
    EXPECT_EQ(sizeof(Encoding1_DataMacro), sizeof(Encoding1));
    EXPECT_EQ(memcmp(Encoding1, Encoding1_DataMacro, sizeof(Encoding1)), 0);

    uint8_t buf[2048];
    TLVWriter writer;
    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;
    WriteEncoding5(writer);
    uint32_t encodedLen = writer.GetLengthWritten();

    EXPECT_EQ(sizeof(Encoding5_DataMacro), encodedLen);
    EXPECT_EQ(memcmp(buf, Encoding5_DataMacro, encodedLen), 0);
}

static CHIP_ERROR NullIterateHandler(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    (void) aReader;
    (void) aDepth;
    (void) aContext;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR FindContainerWithElement(const TLVReader & aReader, size_t aDepth, void * aContext)
{
    TLVReader reader;
    TLVReader result;
    Tag * tag      = static_cast<Tag *>(aContext);
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVType containerType;

    reader.Init(aReader);

    if (TLVTypeIsContainer(reader.GetType()))
    {
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        err = chip::TLV::Utilities::Find(reader, *tag, result, false);

        // Map a successful find (CHIP_NO_ERROR) onto a signal that the element has been found.
        if (err == CHIP_NO_ERROR)
        {
            err = CHIP_ERROR_SENTINEL;
        }
        // Map a failed find attempt to NO_ERROR
        else if (err == CHIP_ERROR_TLV_TAG_NOT_FOUND)
        {
            err = CHIP_NO_ERROR;
        }
    }
    return err;
}

/**
 *  Test CHIP TLV Utilities
 */
TEST_F(TestTLV, CheckTLVUtilities)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader, reader1;
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding1(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

    EXPECT_EQ(encodedLen, sizeof(Encoding1));
    EXPECT_EQ(memcmp(buf, Encoding1, encodedLen), 0);

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;

    reader1.Init(reader);
    err = reader1.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Find a tag
    TLVReader tagReader;
    err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 65536), tagReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Find with reader positioned "on" the element of interest
    err = chip::TLV::Utilities::Find(reader1, ProfileTag(TestProfile_1, 1), tagReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Find a tag that's not present
    err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 1024), tagReader);
    EXPECT_EQ(err, CHIP_ERROR_TLV_TAG_NOT_FOUND);

    // Find with a predicate
    {
        uint8_t buf1[74];

        writer.Init(buf1);
        writer.ImplicitProfileId = TestProfile_2;

        WriteEncoding2(writer);

        // Initialize a reader
        reader1.Init(buf1, writer.GetLengthWritten());
        reader1.ImplicitProfileId = TestProfile_2;

        // position the reader on the first element
        reader1.Next();
        Tag tag = ProfileTag(TestProfile_1, 1);
        err     = chip::TLV::Utilities::Find(reader1, FindContainerWithElement, &tag, tagReader, false);
        EXPECT_EQ(err, CHIP_ERROR_TLV_TAG_NOT_FOUND);

        tag = ProfileTag(TestProfile_2, 2);
        err = chip::TLV::Utilities::Find(reader1, FindContainerWithElement, &tag, tagReader, false);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(tagReader.GetType(), kTLVType_Structure);
        EXPECT_EQ(tagReader.GetTag(), ProfileTag(TestProfile_1, 1));

        // Position the reader on the second element
        reader1.Next();
        err = chip::TLV::Utilities::Find(reader1, FindContainerWithElement, &tag, tagReader, false);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(tagReader.GetType(), kTLVType_Structure);
        EXPECT_EQ(tagReader.GetTag(), ProfileTag(TestProfile_2, 1));
    }

    // Count
    size_t count;
    const size_t expectedCount = 18;
    reader1.Init(reader);
    reader1.Next();

    err = chip::TLV::Utilities::Count(reader, count);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(count, expectedCount);

    // Count with reader already positioned "on" the first element in the encoding
    err = chip::TLV::Utilities::Count(reader1, count);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(count, expectedCount);

    // Iterate
    err = chip::TLV::Utilities::Iterate(reader, NullIterateHandler, nullptr);
    EXPECT_EQ(err, CHIP_END_OF_TLV);
}

/**
 *  Test CHIP TLV Empty Find
 */
TEST_F(TestTLV, CheckTLVEmptyFind)
{
    uint8_t buf[30];
    TLVWriter writer;
    TLVReader reader;
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEmptyEncoding(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;

    // Find the empty container
    TLVReader tagReader;
    err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_1, 256), tagReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

// clang-format off
uint8_t Encoding2[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x88, 0x02, 0x00,
    0x18,
    // Container 2
    0x95, 0x01, 0x00,
        0x88, 0x02, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
    0x18
};

uint8_t AppendedEncoding2[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x88, 0x02, 0x00,
        0xC8, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x89, 0x02, 0x00,
        0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
            0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
            0x88, 0x02, 0x00,
        0x18,
        0x95, 0x01, 0x00,
            0x88, 0x02, 0x00,
            0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x18,
    0x18,
    // Container 2
    0x95, 0x01, 0x00,
        0x88, 0x02, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
    0x18
};
// clang-format on

void WriteAppendReadTest0()
{
    uint8_t buf[74];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding2(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

#ifdef DUMP_ENCODING
    printf("Initial encoding:\n");
    for (uint32_t i = 0; i < encodedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(encodedLen, sizeof(Encoding2));
    EXPECT_EQ(memcmp(buf, Encoding2, encodedLen), 0);

    // Append new data into encoding
    AppendEncoding2(buf, encodedLen, sizeof(buf), updatedLen);

#ifdef DUMP_ENCODING
    printf("Updated encoding:\n");
    for (uint32_t i = 0; i < updatedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(updatedLen, sizeof(AppendedEncoding2));
    EXPECT_EQ(memcmp(buf, AppendedEncoding2, updatedLen), 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding2(reader);
}

void WriteFindAppendReadTest(bool findContainer)
{
    uint8_t buf[74];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding2(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

#ifdef DUMP_ENCODING
    printf("Initial encoding:\n");
    for (uint32_t i = 0; i < encodedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(encodedLen, sizeof(Encoding2));
    EXPECT_EQ(memcmp(buf, Encoding2, encodedLen), 0);

    // Append new data into encoding
    FindAppendEncoding2(buf, encodedLen, sizeof(buf), updatedLen, findContainer);

#ifdef DUMP_ENCODING
    printf("Updated encoding:\n");
    for (uint32_t i = 0; i < updatedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(updatedLen, sizeof(AppendedEncoding2));
    EXPECT_EQ(memcmp(buf, AppendedEncoding2, updatedLen), 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding2(reader);
}

// clang-format off
uint8_t Encoding3[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0x88, 0x02, 0x00,
    0x18,
};

uint8_t AppendedEncoding3[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0x88, 0x02, 0x00,
        0x89, 0x02, 0x00,
    0x18
};
// clang-format on

void WriteAppendReadTest1()
{
    uint8_t buf[14];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding3(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

#ifdef DUMP_ENCODING
    printf("Initial encoding:\n");
    for (uint32_t i = 0; i < encodedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(encodedLen, sizeof(Encoding3));
    EXPECT_EQ(memcmp(buf, Encoding3, encodedLen), 0);

    // Append new data into encoding
    AppendEncoding3(buf, encodedLen, sizeof(buf), updatedLen);

#ifdef DUMP_ENCODING
    printf("Updated encoding:\n");
    for (uint32_t i = 0; i < updatedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(updatedLen, sizeof(AppendedEncoding3));
    EXPECT_EQ(memcmp(buf, AppendedEncoding3, updatedLen), 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding3(reader);
}

// clang-format off
uint8_t AppendedEncoding4[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0x88, 0x02, 0x00,
    0x18,
};
// clang-format on

void AppendReadTest()
{
    uint8_t buf[11];
    uint32_t updatedLen;

    memset(buf, 0, sizeof(buf));

#ifdef DUMP_ENCODING
    printf("Initial encoding:\n");
    for (uint32_t i = 0; i < sizeof(buf); i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    // Append new data to encoding
    AppendEncoding4(buf, 0, sizeof(buf), updatedLen);

#ifdef DUMP_ENCODING
    printf("Updated encoding:\n");
    for (uint32_t i = 0; i < updatedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(updatedLen, sizeof(AppendedEncoding4));
    EXPECT_EQ(memcmp(buf, AppendedEncoding4, updatedLen), 0);

    TLVReader reader;
    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding4(reader);
}

// clang-format off
uint8_t Encoding5[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x88, 0x02, 0x00,
        0xC8, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x89, 0x02, 0x00,
        0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
            0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
            0x88, 0x02, 0x00,
        0x18,
        0x95, 0x01, 0x00,
            0x88, 0x02, 0x00,
            0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x18,
    0x18,
    // Container 2
    0x95, 0x01, 0x00,
        0x88, 0x02, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
    0x18
};

uint8_t DeletedEncoding5[] =
{
    // Container 1
    0xD5, 0xBB, 0xAA, 0xDD, 0xCC, 0x01, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
        0x88, 0x02, 0x00,
    0x18,
    // Container 2
    0x95, 0x01, 0x00,
        0x88, 0x02, 0x00,
        0xC9, 0xBB, 0xAA, 0xDD, 0xCC, 0x02, 0x00,
    0x18
};
// clang-format on

void WriteDeleteReadTest()
{
    uint8_t buf[74];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding5(writer);

    uint32_t encodedLen = writer.GetLengthWritten();

#ifdef DUMP_ENCODING
    for (uint32_t i = 0; i < encodedLen; i++)
    {
        if (i != 0 && i % 16 == 0)
            printf("\n");
        printf("0x%02X, ", buf[i]);
    }
    printf("\n");
#endif

    EXPECT_EQ(encodedLen, sizeof(Encoding5));
    EXPECT_EQ(memcmp(buf, Encoding5, encodedLen), 0);

    // Delete some elements from the encoding
    DeleteEncoding5(buf, encodedLen, sizeof(buf), updatedLen);

    EXPECT_EQ(updatedLen, sizeof(DeletedEncoding5));
    EXPECT_EQ(memcmp(buf, DeletedEncoding5, updatedLen), 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadDeletedEncoding5(reader);
}

/**
 *  Test Packet Buffer
 */
TEST_F(TestTLV, CheckPacketBuffer)
{
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(sizeof(Encoding1), 0);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferTLVReader reader;

    writer.Init(buf.Retain());
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding1(writer);

    TestBufferContents(buf, Encoding1, sizeof(Encoding1));

    reader.Init(buf.Retain());
    reader.ImplicitProfileId = TestProfile_2;

    ReadEncoding1(reader);

    reader.Init(buf.Retain());
    reader.ImplicitProfileId = TestProfile_2;

    ReadEncoding1(reader);
}

CHIP_ERROR CountEvictedMembers(TLVCircularBuffer & inBuffer, void * inAppData, TLVReader & inReader)
{
    TestTLVContext * context = static_cast<TestTLVContext *>(inAppData);
    CHIP_ERROR err;

    // "Process" the first element in the reader
    err = inReader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = inReader.Skip();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    context->mEvictionCount++;
    context->mEvictedBytes += inReader.GetLengthRead();

    return CHIP_NO_ERROR;
}

TEST_F(TestTLV, CheckCircularTLVBufferSimple)
{
    // Write 40 bytes as 4 separate events into a 30 byte buffer.  On
    // completion of the test, the buffer should contain 2 elements
    // and 2 elements should have been evicted in the last call to
    // WriteEncoding.

    uint8_t backingStore[30];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TestTLVContext * context = &TestTLV::ctx;
    TLVCircularBuffer buffer(backingStore, 30);
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = &TestTLV::ctx;

    writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    EXPECT_EQ(context->mEvictionCount, 2);
    EXPECT_EQ(context->mEvictedBytes, 18u);
    EXPECT_EQ(buffer.DataLength(), 22u);
    EXPECT_EQ((buffer.DataLength() + context->mEvictedBytes), writer.GetLengthWritten());

    // At this point the buffer should contain 2 instances of Encoding3.
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);

    ReadEncoding3(reader);

    TestNext<TLVReader>(reader);

    ReadEncoding3(reader);

    // Check that the reader is out of data
    TestEnd<TLVReader>(reader);
}

TEST_F(TestTLV, CheckCircularTLVBufferStartMidway)
{
    // Write 40 bytes as 4 separate events into a 30 byte buffer.  On
    // completion of the test, the buffer should contain 2 elements
    // and 2 elements should have been evicted in the last call to
    // WriteEncoding.

    uint8_t backingStore[30];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TestTLVContext * context = &TestTLV::ctx;
    TLVCircularBuffer buffer(backingStore, 30, &(backingStore[15]));
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = &TestTLV::ctx;

    writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    EXPECT_EQ(context->mEvictionCount, 2);
    EXPECT_EQ(context->mEvictedBytes, 18u);
    EXPECT_EQ(buffer.DataLength(), 22u);
    EXPECT_EQ((buffer.DataLength() + context->mEvictedBytes), writer.GetLengthWritten());

    // At this point the buffer should contain 2 instances of Encoding3.
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);

    ReadEncoding3(reader);

    TestNext<TLVReader>(reader);

    ReadEncoding3(reader);

    // Check that the reader is out of data
    TestEnd<TLVReader>(reader);
}

TEST_F(TestTLV, CheckCircularTLVBufferEvictStraddlingEvent)
{
    // Write 95 bytes to the buffer as 9 different TLV elements: 1
    // 7-byte element and 8 11-byte elements.
    // On completion of the test, the buffer should contain 2 elements
    // and 7 elements should have been evicted in the last call to
    // WriteEncoding.

    TestTLVContext * context = &TestTLV::ctx;
    uint8_t backingStore[30];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TLVCircularBuffer buffer(backingStore, 30);
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = &TestTLV::ctx;

    writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    // the write below will evict an element that straddles the buffer boundary.
    WriteEncoding3(writer);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    WriteEncoding3(writer);

    EXPECT_EQ(writer.GetLengthWritten(),
              (8u * 11 + 7)); // 8 writes of Encoding3 (11 bytes each) and 7 bytes for the initial boolean.
    EXPECT_EQ(buffer.DataLength(), 22u);
    EXPECT_EQ((buffer.DataLength() + context->mEvictedBytes), writer.GetLengthWritten());
    EXPECT_EQ(context->mEvictionCount, 7);

    // At this point the buffer should contain 2 instances of Encoding3.
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);

    ReadEncoding3(reader);

    TestNext<TLVReader>(reader);

    ReadEncoding3(reader);

    // Check that the reader is out of data
    TestEnd<TLVReader>(reader);
}

TEST_F(TestTLV, CheckCircularTLVBufferEdge)
{
    TestTLVContext * context = &TestTLV::ctx;
    CHIP_ERROR err;
    uint8_t backingStore[7];
    uint8_t backingStore1[14];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TLVWriter writer1;

    TLVCircularBuffer buffer(backingStore, sizeof(backingStore));
    TLVCircularBuffer buffer1(backingStore1, sizeof(backingStore1));
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = &TestTLV::ctx;

    // Test eviction for an element that fits in the underlying buffer exactly
    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), false);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // At this point the buffer should contain only the boolean we just wrote
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);
    TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    // Check that the reader is out of data
    TestEnd<TLVReader>(reader);

    // verify that an element larger than the underlying buffer fails out.
    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
    EXPECT_EQ(err, CHIP_END_OF_TLV);

    // Verify reader correctness

    // Write an element that takes half of the buffer, and evict it.
    // Do it 2 times, so we test what happens when the head is at the
    // middle and at the end of the buffer but the buffer is empty.
    int i = 0;
    for (i = 0; i < 2; i++)
    {
        writer.Init(buffer1);
        writer.ImplicitProfileId = TestProfile_2;

        err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        reader.Init(buffer1);
        reader.ImplicitProfileId = TestProfile_2;

        TestNext<TLVReader>(reader);
        TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);
        TestEnd<TLVReader>(reader);

        buffer1.EvictHead();

        reader.Init(buffer1);
        reader.ImplicitProfileId = TestProfile_2;
        TestEnd<TLVReader>(reader);
    }

    writer.Init(buffer1);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer1.mProcessEvictedElement = CountEvictedMembers;
    buffer1.mAppData               = &TestTLV::ctx;

    // Two elements fit in the buffer exactly
    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), false);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();

    // Verify that we can read out two elements from the buffer
    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);
    TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

    TestNext<TLVReader>(reader);
    TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    TestEnd<TLVReader>(reader);

    // Check that the eviction works as expected

    buffer1.EvictHead();

    // At this point the buffer should contain only the second boolean
    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);
    TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    // Check that the reader is out of data
    TestEnd<TLVReader>(reader);

    // Write another boolean, verify that the buffer is full and contains two booleans

    writer.Init(buffer1);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify that we can read out two elements from the buffer
    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);
    TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    TestNext<TLVReader>(reader);
    TEST_GET_NOERROR(reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

    TestEnd<TLVReader>(reader);

    // Evict the elements from the buffer, verfiy that we have an
    // empty reader on our hands

    buffer1.EvictHead();
    buffer1.EvictHead();

    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestEnd<TLVReader>(reader);
}

TEST_F(TestTLV, CheckTLVPutStringOverrun)
{
    const size_t bufSize          = 8;
    uint8_t backingStore[bufSize] = {};

    const char * badPointer = "Foo <segfault here>";

    TLVWriter writer;

    writer.Init(backingStore, bufSize);

    CHIP_ERROR err = writer.PutString(ProfileTag(TestProfile_1, 1), badPointer);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST_F(TestTLV, CheckTLVPutStringF)
{
    const size_t bufsize = 24;
    char strBuffer[bufsize];
    char valStr[bufsize];
    uint8_t backingStore[bufsize];
    TLVWriter writer;
    TLVReader reader;
    size_t num     = 1;
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(backingStore, bufsize);
    snprintf(strBuffer, sizeof(strBuffer), "Sample string %u", static_cast<unsigned int>(num));

    err = writer.PutStringF(ProfileTag(TestProfile_1, 1), "Sample string %u", static_cast<unsigned int>(num));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reader.Init(backingStore, writer.GetLengthWritten());
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.GetString(valStr, bufsize);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strncmp(valStr, strBuffer, bufsize), 0);
}

TEST_F(TestTLV, CheckTLVPutStringSpan)
{
    const size_t bufsize    = 24;
    char strBuffer[bufsize] = "Sample string";
    char valStr[bufsize];
    uint8_t backingStore[bufsize];
    TLVWriter writer;
    TLVReader reader;
    CHIP_ERROR err = CHIP_NO_ERROR;
    Span<char> strSpan;

    //
    // Write a string that has a size that exceeds 32-bits. This is only possible
    // on platforms where size_t is bigger than uint32_t.
    //
    if (sizeof(size_t) > sizeof(uint32_t))
    {
        writer.Init(backingStore, bufsize);
        snprintf(strBuffer, sizeof(strBuffer), "Sample string");

        strSpan = { strBuffer, static_cast<size_t>(0xffffffffff) };

        err = writer.PutString(ProfileTag(TestProfile_1, 1), strSpan);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    {
        writer.Init(backingStore, bufsize);
        snprintf(strBuffer, sizeof(strBuffer), "Sample string");

        strSpan = { strBuffer, strlen("Sample string") };

        err = writer.PutString(ProfileTag(TestProfile_1, 1), strSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        reader.Init(backingStore, writer.GetLengthWritten());
        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = reader.GetString(valStr, bufsize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(strncmp(valStr, strBuffer, bufsize), 0);
    }
}

TEST_F(TestTLV, CheckTLVPutStringFCircular)
{
    const size_t bufsize = 40;
    char strBuffer[bufsize];
    char valStr[bufsize];
    uint8_t backingStore[bufsize];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TLVCircularBuffer buffer(backingStore, bufsize);
    size_t num     = 1;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initial test: Verify that a straight printf works as expected into continuous buffer.

    writer.Init(buffer);
    snprintf(strBuffer, sizeof(strBuffer), "Sample string %u", static_cast<unsigned int>(num));

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.PutStringF(ProfileTag(TestProfile_1, 1), "Sample string %u", static_cast<unsigned int>(num));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reader.Init(buffer);

    // Skip over the initial element
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.GetString(valStr, bufsize);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strncmp(valStr, strBuffer, bufsize), 0);

    // Verify that the PutStringF will handle correctly the case with the discontinuous buffer
    // This print will both stradle the boundary of the buffer and displace the previous two elements.
    num = 2;

    snprintf(strBuffer, sizeof(strBuffer), "Sample string %u", static_cast<unsigned int>(num));

    err = writer.PutStringF(ProfileTag(TestProfile_1, 1), "Sample string %u", static_cast<unsigned int>(num));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reader.Init(buffer);
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.GetString(valStr, bufsize);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(strncmp(valStr, strBuffer, bufsize), 0);
}

TEST_F(TestTLV, CheckTLVByteSpan)
{
    const size_t bufSize  = 14;
    uint8_t bytesBuffer[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
    uint8_t backingStore[bufSize];
    TLVWriter writer;
    TLVReader reader;
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(backingStore);

    ByteSpan writerSpan(bytesBuffer);
    err = writer.Put(ProfileTag(TestProfile_1, 1), writerSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reader.Init(backingStore, writer.GetLengthWritten());
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::ByteSpan readerSpan;
    err = reader.Get(readerSpan);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(memcmp(readerSpan.data(), bytesBuffer, sizeof(bytesBuffer)), 0);
}

#define IS1_CHAR "\x1F"

TEST_F(TestTLV, CheckTLVCharSpan)
{
    struct CharSpanTestCase
    {
        const char * testString;
        const char * expectedString;
    };

    // clang-format off
    static CharSpanTestCase sCharSpanTestCases[] = {
        // Test String                                                           Expected String from Get()
        // ==================================================================================================
        {  "This is a test case #0",                                             "This is a test case #0"  },
        {  "This is a test case #1" IS1_CHAR "Test Localized String Identifier", "This is a test case #1"  },
        {  "This is a test case #2 " IS1_CHAR "abc" IS1_CHAR "def",              "This is a test case #2 " },
        {  "This is a test case #3" IS1_CHAR,                                    "This is a test case #3"  },
        {  "Thé" IS1_CHAR,                                                       "Thé"                     },
        {  IS1_CHAR " abc " IS1_CHAR " def",                                     ""                        },
    };
    // clang-format on

    for (auto & testCase : sCharSpanTestCases)
    {
        uint8_t backingStore[100];
        TLVWriter writer;
        TLVReader reader;
        CHIP_ERROR err = CHIP_NO_ERROR;

        writer.Init(backingStore);

        err = writer.PutString(ProfileTag(TestProfile_1, 1), testCase.testString);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        reader.Init(backingStore, writer.GetLengthWritten());
        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        chip::CharSpan readerSpan;
        err = reader.Get(readerSpan);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        EXPECT_EQ(strlen(testCase.expectedString), readerSpan.size());
        EXPECT_EQ(memcmp(readerSpan.data(), testCase.expectedString, strlen(testCase.expectedString)), 0);
    }
}

TEST_F(TestTLV, CheckTLVGetLocalizedStringIdentifier)
{
    struct CharSpanTestCase
    {
        const char * testString;
        Optional<LocalizedStringIdentifier> expectedLSID;
        CHIP_ERROR expectedResult;
    };

    // clang-format off
    static CharSpanTestCase sCharSpanTestCases[] = {
        // Test String                                               Expected LocalizedStringIdentifier from Get()      Expected Return
        // =============================================================================================================================================
        {  "This is a test case #0",                                 chip::Optional<LocalizedStringIdentifier>(),       CHIP_NO_ERROR                  },
        {  "This is a test case #1" IS1_CHAR "0123",                 chip::Optional<LocalizedStringIdentifier>(),       CHIP_ERROR_INVALID_TLV_ELEMENT },
        {  "This is a test case #2" IS1_CHAR "123" IS1_CHAR "3210",  chip::Optional<LocalizedStringIdentifier>(0x123),  CHIP_NO_ERROR                  },
        {  "This is a test case #3" IS1_CHAR "012",                  chip::Optional<LocalizedStringIdentifier>(),       CHIP_ERROR_INVALID_TLV_ELEMENT },
        {  "This is a test case #3" IS1_CHAR "12",                   chip::Optional<LocalizedStringIdentifier>(0x12),   CHIP_NO_ERROR                  },
        {  "Thé" IS1_CHAR "",                                        chip::Optional<LocalizedStringIdentifier>(),       CHIP_NO_ERROR                  },
        {  "Thé" IS1_CHAR "7",                                       chip::Optional<LocalizedStringIdentifier>(0x7),    CHIP_NO_ERROR                  },
        {  "Thé" IS1_CHAR "1FA",                                     chip::Optional<LocalizedStringIdentifier>(0x1FA),  CHIP_NO_ERROR                  },
        {  "" IS1_CHAR "1FA",                                        chip::Optional<LocalizedStringIdentifier>(0x1FA),  CHIP_NO_ERROR                  },
        {  "Thé" IS1_CHAR "1FAB",                                    chip::Optional<LocalizedStringIdentifier>(0x1FAB), CHIP_NO_ERROR                  },
        {  "Thé" IS1_CHAR "1FAb",                                    chip::Optional<LocalizedStringIdentifier>(),       CHIP_ERROR_INVALID_TLV_ELEMENT },
        {  "Thé" IS1_CHAR "1FABC",                                   chip::Optional<LocalizedStringIdentifier>(),       CHIP_ERROR_INVALID_TLV_ELEMENT },
        {  "Thé" IS1_CHAR "1FA" IS1_CHAR "",                         chip::Optional<LocalizedStringIdentifier>(0x1FA),  CHIP_NO_ERROR                  },
        {  "Thé" IS1_CHAR "1FA" IS1_CHAR "F8sa===",                  chip::Optional<LocalizedStringIdentifier>(0x1FA),  CHIP_NO_ERROR                  },
    };
    // clang-format on

    for (auto & testCase : sCharSpanTestCases)
    {
        uint8_t backingStore[100];
        TLVWriter writer;
        TLVReader reader;
        CHIP_ERROR err = CHIP_NO_ERROR;

        writer.Init(backingStore);

        err = writer.PutString(ProfileTag(TestProfile_1, 1), testCase.testString);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        reader.Init(backingStore, writer.GetLengthWritten());
        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        Optional<LocalizedStringIdentifier> readerLSID;
        err = reader.Get(readerLSID);
        EXPECT_EQ(testCase.expectedResult, err);
        EXPECT_EQ(testCase.expectedLSID, readerLSID);
    }

    // Error case: A case of TLVReader buffer underrun.
    // Expected error after Next() call is: CHIP_ERROR_TLV_UNDERRUN
    {
        uint8_t backingStore[100];
        TLVWriter writer;
        TLVReader reader;
        CHIP_ERROR err = CHIP_NO_ERROR;

        writer.Init(backingStore);

        err = writer.PutString(ProfileTag(TestProfile_1, 1), sCharSpanTestCases[2].testString);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        reader.Init(backingStore, writer.GetLengthWritten() - 1);
        err = reader.Next();
        EXPECT_EQ(err, CHIP_ERROR_TLV_UNDERRUN);
    }

    // Error case: the reader is on a bytestring, not utf-8 string.
    // Expected error after Get(Optional<LocalizedStringIdentifier> &) call is: CHIP_ERROR_WRONG_TLV_TYPE
    {
        uint8_t backingStore[100];
        TLVWriter writer;
        TLVReader reader;
        CHIP_ERROR err = CHIP_NO_ERROR;

        writer.Init(backingStore);

        err = writer.PutBytes(ProfileTag(TestProfile_1, 1), reinterpret_cast<const uint8_t *>(sCharSpanTestCases[2].testString),
                              static_cast<uint32_t>(strlen(sCharSpanTestCases[2].testString)));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        reader.Init(backingStore, writer.GetLengthWritten());
        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        Optional<LocalizedStringIdentifier> readerLSID;
        err = reader.Get(readerLSID);
        EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);
        EXPECT_EQ(readerLSID, Optional<LocalizedStringIdentifier>());
    }
}

TEST_F(TestTLV, CheckTLVSkipCircular)
{
    const size_t bufsize = 40; // large enough s.t. 2 elements fit, 3rd causes eviction
    uint8_t backingStore[bufsize];
    char testString[] = "Sample string"; // 13 characters, without the trailing NULL, add 3 bytes for anon tag
    // Any pair of reader and writer would work here, either PacketBuffer based or CircularTLV based.
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TLVCircularBuffer buffer(backingStore, bufsize);
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(buffer);

    err = writer.PutString(AnonymousTag(), testString);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.PutString(AnonymousTag(), testString);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.PutString(AnonymousTag(), testString); // This event straddles the boundary
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.PutString(AnonymousTag(), testString); // This one does not.
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    reader.Init(buffer);

    err = reader.Next(); // position the reader at the straddling element
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.Skip(); // // Test that the buf ptr is handled correctly within the ReadData() function.
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

/**
 *  Test Buffer Overflow
 */
TEST_F(TestTLV, CheckBufferOverflow)
{
    System::PacketBufferTLVReader reader;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(sizeof(Encoding1), 0);
    uint32_t maxDataLen            = static_cast<uint32_t>(buf->MaxDataLength());
    uint32_t reserve = static_cast<uint32_t>((sizeof(Encoding1) < maxDataLen) ? (maxDataLen - sizeof(Encoding1)) + 2 : 0);

    // Repeatedly write and read a TLV encoding to a chain of PacketBuffers. Use progressively larger
    // and larger amounts of space in the first buffer to force the encoding to overlap the
    // end of the buffer and the beginning of the next.
    for (; reserve < maxDataLen; reserve++)
    {
        buf->SetStart(buf->Start() + reserve);

        {
            System::PacketBufferTLVWriter writer;
            // Scope for writer because we want it to go out of scope before we
            // mess with the chain after writing is done.
            writer.Init(buf.Retain(), /* useChainedBuffers = */ true);
            writer.ImplicitProfileId = TestProfile_2;

            WriteEncoding1(writer);
        }

        TestBufferContents(buf, Encoding1, sizeof(Encoding1));

        // Compact the buffer, since we don't allow reading from chained
        // buffers.
        buf->CompactHead();

        reader.Init(buf.Retain());
        reader.ImplicitProfileId = TestProfile_2;

        ReadEncoding1(reader);

        buf = System::PacketBufferHandle::New(sizeof(Encoding1), 0);
    }
}

/**
 * Test case to verify the correctness of TLVReader::GetTag()
 *
 * TLVReader::GetTag() does not return the correct tag value when the
 * the compiler optimization level contains strict aliasing. In the below
 * example, the returned tag value would be 0xe, instead of 0xe00000001.
 *
 * The issue has been spotted on debug builds.
 *
 */
// clang-format off
static const uint8_t sIdentifyResponseBuf[] =
{
    0xD5, 0x00, 0x00, 0x0E, 0x00, 0x01, 0x00, 0x25,
    0x00, 0x5A, 0x23, 0x24, 0x01, 0x07, 0x24, 0x02,
    0x05, 0x25, 0x03, 0x22, 0x1E, 0x2C, 0x04, 0x10,
    0x30, 0x34, 0x41, 0x41, 0x30, 0x31, 0x41, 0x43,
    0x32, 0x33, 0x31, 0x34, 0x30, 0x30, 0x4C, 0x50,
    0x2C, 0x09, 0x06, 0x31, 0x2E, 0x34, 0x72, 0x63,
    0x35, 0x24, 0x0C, 0x01, 0x18,
};
// clang-format on

static const uint32_t kIdentifyResponseLen = 53;

TEST_F(TestTLV, CheckStrictAliasing)
{
    const uint32_t kProfile_Id = 0x0000000e;
    CHIP_ERROR err             = CHIP_NO_ERROR;
    TLVReader reader;

    reader.Init(sIdentifyResponseBuf, kIdentifyResponseLen);
    reader.ImplicitProfileId = kProfile_Id;

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(reader.GetTag(), ProfileTag(kProfile_Id, 1));
}

/**
 *  Test CHIP TLV Writer Copy Container
 */
void TestTLVWriterCopyContainer()
{
    uint8_t buf[2048];

    {
        TLVWriter writer;
        TLVReader reader;

        reader.Init(Encoding1);
        reader.ImplicitProfileId = TestProfile_2;

        TestNext<TLVReader>(reader);

        writer.Init(buf);
        writer.ImplicitProfileId = TestProfile_2;

        CHIP_ERROR err = writer.CopyContainer(reader);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        uint32_t encodedLen = writer.GetLengthWritten();
        EXPECT_EQ(encodedLen, sizeof(Encoding1));

        int memcmpRes = memcmp(buf, Encoding1, encodedLen);
        EXPECT_EQ(memcmpRes, 0);
    }

    {
        TLVWriter writer;

        writer.Init(buf);
        writer.ImplicitProfileId = TestProfile_2;

        CHIP_ERROR err = writer.CopyContainer(ProfileTag(TestProfile_1, 1), Encoding1, sizeof(Encoding1));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        uint32_t encodedLen = writer.GetLengthWritten();
        EXPECT_EQ(encodedLen, sizeof(Encoding1));

        int memcmpRes = memcmp(buf, Encoding1, encodedLen);
        EXPECT_EQ(memcmpRes, 0);
    }
}

/**
 *  Test CHIP TLV Writer Copy Element
 */
void TestTLVWriterCopyElement()
{
    CHIP_ERROR err;
    uint8_t expectedBuf[2048], testBuf[2048];
    uint32_t expectedLen, testLen;
    TLVWriter writer;
    TLVType outerContainerType;
    enum
    {
        kRepeatCount = 3
    };

    writer.Init(expectedBuf);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    for (int i = 0; i < kRepeatCount; i++)
    {
        WriteEncoding1(writer);
    }

    err = writer.EndContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    expectedLen = writer.GetLengthWritten();

    writer.Init(testBuf);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    for (int i = 0; i < kRepeatCount; i++)
    {
        TLVReader reader;

        reader.Init(Encoding1);
        reader.ImplicitProfileId = TestProfile_2;

        TestNext<TLVReader>(reader);

        err = writer.CopyElement(reader);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.EndContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    testLen = writer.GetLengthWritten();

    EXPECT_EQ(testLen, expectedLen);

    int memcmpRes = memcmp(testBuf, expectedBuf, testLen);
    EXPECT_EQ(memcmpRes, 0);
}

void PreserveSizeWrite(TLVWriter & writer, bool preserveSize)
{
    CHIP_ERROR err;
    TLVWriter writer2;

    // TLVTagControl::FullyQualified_8Bytes
    err = writer.Put(ProfileTag(TestProfile_1, 4000000000ULL), static_cast<int64_t>(40000000000ULL), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(ProfileTag(TestProfile_1, 4000000000ULL), static_cast<int16_t>(12345), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Put(ProfileTag(TestProfile_1, 4000000000ULL), static_cast<float>(1.0));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        TLVWriter writer3;

        err = writer2.OpenContainer(ContextTag(0), kTLVType_Array, writer3);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<uint8_t>(42), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<uint16_t>(42), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<uint32_t>(42), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<uint64_t>(40000000000ULL), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int8_t>(-17), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int16_t>(-17), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int32_t>(-170000), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int64_t>(-170000), preserveSize);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // the below cases are for full coverage of PUTs
        err = writer3.Put(AnonymousTag(), static_cast<uint64_t>(65535), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int64_t>(32767), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag(), static_cast<int64_t>(40000000000ULL), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.CloseContainer(writer3);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

/**
 *  Test CHIP TLV Writer with Preserve Size
 */
void TestTLVWriterPreserveSize()
{
    uint8_t buf[2048];
    TLVWriter writer;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    PreserveSizeWrite(writer, true);

    uint32_t encodedLen = writer.GetLengthWritten();
    EXPECT_EQ(encodedLen, 105u);
}

/**
 *  Test error handling of CHIP TLV Writer
 */
void TestTLVWriterErrorHandling()
{
    CHIP_ERROR err;
    uint8_t buf[2048];
    TLVWriter writer, writer2, writer3;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    // OpenContainer() for non-container
    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Boolean, writer2);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // Since OpenContainer failed, writer2 remains uninitialized.
    writer2.Init(nullptr, 0);

    // CloseContainer() for non-container
    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // OpenContainer() failure
    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer3);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // CloseContainer() failure
    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_ERROR_TLV_CONTAINER_OPEN);

    // StartContainer()
    TLVType outerContainerType;
    err = writer.StartContainer(ProfileTag(TestProfile_2, 4000000000ULL), kTLVType_Boolean, outerContainerType);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // EndContainer()
    outerContainerType = kTLVType_Boolean;
    err                = writer.EndContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // PutPreEncodedContainer()
    TLVReader reader;
    reader.Init(buf, 2048);
    err = writer.PutPreEncodedContainer(ProfileTag(TestProfile_2, 4000000000ULL), kTLVType_Boolean, reader.GetReadPoint(),
                                        reader.GetRemainingLength());
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

void TestTLVEmptyString()
{
    uint8_t buf[2];
    TLVWriter writer;
    CHIP_ERROR err;
    ByteSpan s;

    writer.Init(buf);

    err = writer.PutString(AnonymousTag(), nullptr, 0);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLVReader reader;

    reader.Init(buf);

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.Get(s);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(s.data(), nullptr);
    EXPECT_EQ(s.size(), 0u);
}

/**
 *  Test CHIP TLV Writer
 */
TEST_F(TestTLV, CheckTLVWriter)
{
    TestTLVWriterCopyContainer();

    TestTLVWriterCopyElement();

    TestTLVWriterPreserveSize();

    TestTLVWriterErrorHandling();

    TestTLVEmptyString();
}

void SkipNonContainer()
{
    TLVReader reader;
    const uint8_t * readpoint1 = nullptr;
    const uint8_t * readpoint2 = nullptr;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestSkip(reader);

    readpoint1 = reader.GetReadPoint();

    // Skip again, to check the operation is idempotent
    TestSkip(reader);

    readpoint2 = reader.GetReadPoint();

    EXPECT_EQ(readpoint1, readpoint2);
}

void SkipContainer()
{
    TLVReader reader;
    const uint8_t * readpoint1 = nullptr;
    const uint8_t * readpoint2 = nullptr;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);

    TestSkip(reader);

    readpoint1 = reader.GetReadPoint();

    // Skip again, to check the operation is idempotent
    TestSkip(reader);

    readpoint2 = reader.GetReadPoint();

    EXPECT_EQ(readpoint1, readpoint2);
}

void NextContainer()
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);

    CHIP_ERROR err = reader.Next();
    EXPECT_EQ(err, CHIP_END_OF_TLV);
}

/**
 *  Test CHIP TLV Reader Skip functions
 */
void TestTLVReaderSkip()
{
    SkipNonContainer();

    SkipContainer();

    NextContainer();
}

/**
 *  Test CHIP TLV Reader Dup functions
 */
void TestTLVReaderDup()
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(reader);

    {
        TLVReader reader2;

        TestAndOpenContainer(reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(reader2);

        {
            TLVReader reader3;

            TestAndOpenContainer(reader2, kTLVType_Array, ContextTag(0), reader3);

            TestNext<TLVReader>(reader3);

            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(42));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(42));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(42));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(42));
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint32_t>(42), CHIP_ERROR_WRONG_TLV_TYPE);
            TEST_GET(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<uint64_t>(42), CHIP_ERROR_WRONG_TLV_TYPE);

            TestNext<TLVReader>(reader3);

            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int8_t>(-17));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int16_t>(-17));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(-17));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(-17));

            TestNext<TLVReader>(reader3);

            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int32_t>(-170000));
            TEST_GET_NOERROR(reader3, kTLVType_SignedInteger, AnonymousTag(), static_cast<int64_t>(-170000));

            TestNext<TLVReader>(reader3);

            TEST_GET(reader3, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<int64_t>(40000000000ULL),
                     CHIP_ERROR_WRONG_TLV_TYPE);
            TEST_GET_NOERROR(reader3, kTLVType_UnsignedInteger, AnonymousTag(), static_cast<uint64_t>(40000000000ULL));

            TestNext<TLVReader>(reader3);

            {
                TLVReader reader4;

                TestAndOpenContainer(reader3, kTLVType_Structure, AnonymousTag(), reader4);

                TestEndAndCloseContainer(reader3, reader4);
            }

            TestNext<TLVReader>(reader3);

            {
                TLVReader reader5;

                TestAndOpenContainer(reader3, kTLVType_List, AnonymousTag(), reader5);

                TestNext<TLVReader>(reader5);

                TestNull(reader5, ProfileTag(TestProfile_1, 17));

                TestNext<TLVReader>(reader5);

                TestNull(reader5, ProfileTag(TestProfile_2, 900000));

                TestNext<TLVReader>(reader5);

                TestNull(reader5, AnonymousTag());

                TestNext<TLVReader>(reader5);

                {
                    TLVType outerContainerType;

                    TestAndEnterContainer<TLVReader>(reader5, kTLVType_Structure, ProfileTag(TestProfile_2, 4000000000ULL),
                                                     outerContainerType);

                    TestNext<TLVReader>(reader5);

                    TestDupString(reader5, CommonTag(70000), sLargeString);

                    TestEndAndExitContainer<TLVReader>(reader5, outerContainerType);
                }

                TestEndAndCloseContainer(reader3, reader5);
            }

            TestEndAndCloseContainer(reader2, reader3);
        }

        TestNext<TLVReader>(reader2);

        TestDupBytes(reader2, ProfileTag(TestProfile_1, 5), reinterpret_cast<const uint8_t *>("This is a test"), 14);

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), 17.9f);
        TEST_GET_NOERROR(reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), static_cast<double>(17.9f));

        TestNext<TLVReader>(reader2);

        TEST_GET_NOERROR(reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65536), 17.9);

        TestEndAndCloseContainer(reader, reader2);
    }

    TestEnd<TLVReader>(reader);
}
/**
 *  Test error handling of CHIP TLV Reader
 */
void TestTLVReaderErrorHandling()
{
    CHIP_ERROR err;
    uint8_t buf[2048] = { 0 };
    TLVReader reader;

    reader.Init(buf);
    reader.ImplicitProfileId = TestProfile_2;

    // Get(bool&)
    bool val;
    err = reader.Get(val);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // Get(float&)
    float numF;
    err = reader.Get(numF);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // Get(double&)
    double numD;
    err = reader.Get(numD);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // Get(uint64_t&)
    uint64_t num;
    err = reader.Get(num);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // GetBytes()
    uint8_t bBuf[16];
    err = reader.GetBytes(bBuf, sizeof(bBuf));
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // GetString()
    char sBuf[16];
    err = reader.GetString(sBuf, sizeof(sBuf));
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);

    // OpenContainer()
    TLVReader reader2;
    err = reader.OpenContainer(reader2);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // CloseContainer()
    err = reader.CloseContainer(reader2);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // EnterContainer()
    TLVType outerContainerType = kTLVType_Boolean;
    err                        = reader.EnterContainer(outerContainerType);
    EXPECT_EQ(err, CHIP_ERROR_INCORRECT_STATE);

    // DupString()
    char * str = static_cast<char *>(chip::Platform::MemoryAlloc(16));
    err        = reader.DupString(str);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);
    chip::Platform::MemoryFree(str);

    // GetDataPtr()
    const uint8_t * data = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(16));
    err                  = reader.GetDataPtr(data);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);
    chip::Platform::MemoryFree(const_cast<uint8_t *>(data));
}

void TestTLVReaderExpect()
{
    // Prepare some test data
    uint8_t buffer[20];
    TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    TLVType outerContainer;
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainer));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBoolean(ContextTag(23), false));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(outerContainer));

    TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());

    // Positioned before the first element
    EXPECT_EQ(reader.GetType(), kTLVType_NotSpecified);

    EXPECT_EQ(reader.Expect(AnonymousTag()), CHIP_ERROR_WRONG_TLV_TYPE);
    EXPECT_EQ(reader.Expect(ContextTag(23)), CHIP_ERROR_WRONG_TLV_TYPE);
    EXPECT_EQ(reader.Expect(kTLVType_Boolean, AnonymousTag()), CHIP_ERROR_WRONG_TLV_TYPE);

    // Positioned on kTLVType_Structure / AnonymousTag(),
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(reader.GetType(), kTLVType_Structure);
    EXPECT_EQ(reader.GetTag(), AnonymousTag());

    EXPECT_EQ(reader.Expect(ContextTag(23)), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    EXPECT_EQ(CHIP_NO_ERROR, reader.Expect(AnonymousTag()));

    EXPECT_EQ(reader.Expect(kTLVType_SignedInteger, AnonymousTag()), CHIP_ERROR_WRONG_TLV_TYPE);
    EXPECT_EQ(CHIP_NO_ERROR, reader.Expect(kTLVType_Structure, AnonymousTag()));

    // Positioned before first struct element
    EXPECT_EQ(CHIP_NO_ERROR, reader.EnterContainer(outerContainer));
    EXPECT_EQ(reader.GetType(), kTLVType_NotSpecified);

    EXPECT_EQ(reader.Expect(AnonymousTag()), CHIP_ERROR_WRONG_TLV_TYPE);
    EXPECT_EQ(reader.Expect(ContextTag(23)), CHIP_ERROR_WRONG_TLV_TYPE);
    EXPECT_EQ(reader.Expect(kTLVType_Boolean, AnonymousTag()), CHIP_ERROR_WRONG_TLV_TYPE);

    // Positioned on kTLVType_Boolean / ContextTag(23)
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(reader.GetType(), kTLVType_Boolean);
    EXPECT_EQ(reader.GetTag(), ContextTag(23));

    EXPECT_EQ(reader.Expect(AnonymousTag()), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    EXPECT_EQ(reader.Expect(ContextTag(42)), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
    EXPECT_EQ(CHIP_NO_ERROR, reader.Expect(ContextTag(23)));

    EXPECT_EQ(reader.Expect(kTLVType_SignedInteger, ContextTag(23)), CHIP_ERROR_WRONG_TLV_TYPE);
    EXPECT_EQ(CHIP_NO_ERROR, reader.Expect(kTLVType_Boolean, ContextTag(23)));
}

/**
 *  Test that CHIP TLV reader returns an error when a read is requested that
 *  would truncate the output.
 */
void TestTLVReaderTruncatedReads()
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;

    CHIP_ERROR err;
    float outF;

    // Setup: Write some values into the buffer
    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.Put(AnonymousTag(), double{ 12.5 });
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Test reading values from the buffer
    reader.Init(buf);

    TestNext<TLVReader>(reader);

    TEST_GET_NOERROR(reader, kTLVType_FloatingPointNumber, AnonymousTag(), 12.5);

    err = reader.Get(outF);
    EXPECT_EQ(err, CHIP_ERROR_WRONG_TLV_TYPE);
}

/**
 *  Test CHIP TLV Reader in a use case
 */
void TestTLVReaderInPractice()
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    PreserveSizeWrite(writer, true);

    reader.Init(buf);

    TestNext<TLVReader>(reader);

    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, ProfileTag(TestProfile_1, 4000000000ULL),
                     static_cast<int64_t>(40000000000ULL));

    TestNext<TLVReader>(reader);

    TEST_GET_NOERROR(reader, kTLVType_SignedInteger, ProfileTag(TestProfile_1, 4000000000ULL), static_cast<int64_t>(12345));

    TestNext<TLVReader>(reader);

    TEST_GET_NOERROR(reader, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_1, 4000000000ULL), static_cast<float>(1.0));
}

void TestTLVReader_NextOverContainer_ProcessElement(TLVReader & reader, void * context)
{
    CHIP_ERROR err, nextRes1, nextRes2;
    TLVType outerContainerType;

    // If the current element is a container...
    if (TLVTypeIsContainer(reader.GetType()))
    {
        // Make two copies of the reader
        TLVReader readerClone1 = reader;
        TLVReader readerClone2 = reader;

        // Manually advance one of the readers to the element immediately after the container (if any).
        err = readerClone1.EnterContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        ForEachElement(readerClone1, nullptr, nullptr);
        err = readerClone1.ExitContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        nextRes1 = readerClone1.Next();
        EXPECT_TRUE(nextRes1 == CHIP_NO_ERROR || nextRes1 == CHIP_END_OF_TLV);

        // For the other reader, skip over the entire container using the Next() method.
        nextRes2 = readerClone2.Next();
        EXPECT_TRUE(nextRes2 == CHIP_NO_ERROR || nextRes2 == CHIP_END_OF_TLV);

        // Verify the two readers end up in the same state/position.
        EXPECT_EQ(nextRes1, nextRes2);
        EXPECT_EQ(readerClone1.GetType(), readerClone2.GetType());
        EXPECT_EQ(readerClone1.GetReadPoint(), readerClone2.GetReadPoint());
    }
}

/**
 * Test using CHIP TLV Reader Next() method to skip over containers.
 */
void TestTLVReader_NextOverContainer()
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    ForEachElement(reader, nullptr, TestTLVReader_NextOverContainer_ProcessElement);
}

void TestTLVReader_SkipOverContainer_ProcessElement(TLVReader & reader, void * context)
{
    CHIP_ERROR err;
    TLVType outerContainerType;

    // If the current element is a container...
    if (TLVTypeIsContainer(reader.GetType()))
    {
        // Make two copies of the reader
        TLVReader readerClone1 = reader;
        TLVReader readerClone2 = reader;

        // Manually advance one of the readers to immediately after the container.
        err = readerClone1.EnterContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        ForEachElement(readerClone1, nullptr, nullptr);
        err = readerClone1.ExitContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // For the other reader, skip over the entire container using the Skip() method.
        err = readerClone2.Skip();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // Verify the two readers end up in the same state/position.
        EXPECT_EQ(readerClone1.GetType(), readerClone2.GetType());
        EXPECT_EQ(readerClone1.GetReadPoint(), readerClone2.GetReadPoint());
    }
}

/**
 * Test using CHIP TLV Reader Skip() method to skip over containers.
 */
void TestTLVReader_SkipOverContainer()
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    ForEachElement(reader, nullptr, TestTLVReader_SkipOverContainer_ProcessElement);
}

/**
 * Tests using an uninitialized TLVReader.
 */
void TestTLVReaderUninitialized()
{
    TLVReader reader;

    EXPECT_EQ(reader.GetType(), kTLVType_NotSpecified);
    EXPECT_EQ(reader.GetLength(), 0u);
    EXPECT_EQ(reader.GetControlByte(), kTLVControlByte_NotSpecified);
    EXPECT_EQ(reader.GetContainerType(), kTLVType_NotSpecified);
    EXPECT_EQ(reader.GetLengthRead(), 0u);
    EXPECT_EQ(reader.GetRemainingLength(), 0u);
    EXPECT_EQ(reader.GetTotalLength(), 0u);
    EXPECT_EQ(reader.GetBackingStore(), nullptr);
    EXPECT_EQ(reader.IsElementDouble(), false);
    EXPECT_EQ(reader.GetReadPoint(), nullptr);
    EXPECT_EQ(reader.ImplicitProfileId, kProfileIdNotSpecified);
    EXPECT_EQ(reader.AppData, nullptr);
}

/**
 *  Test CHIP TLV Reader
 */
TEST_F(TestTLV, CheckTLVReader)
{
    TestTLVReaderSkip();

    TestTLVReaderDup();

    TestTLVReaderErrorHandling();

    TestTLVReaderExpect();

    TestTLVReaderTruncatedReads();

    TestTLVReaderInPractice();

    TestTLVReader_NextOverContainer();

    TestTLVReader_SkipOverContainer();

    TestTLVReaderUninitialized();
}

/**
 *  Test CHIP TLV Items
 */
static void TestItems()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t sBuffer[256];

    TLVWriter writer;
    writer.Init(sBuffer);

    TLVWriter writer2;
    err = writer.OpenContainer(AnonymousTag(), kTLVType_Array, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        err = writer2.PutBoolean(AnonymousTag(), true);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int8_t>(-1));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int16_t>(-2));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int32_t>(-3));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int64_t>(-4));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<float>(-5.5));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<double>(-3.14159265358979323846));
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.OpenContainer(AnonymousTag(), kTLVType_Array, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    {
        err = writer2.PutBoolean(AnonymousTag(), false);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int8_t>(1));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int16_t>(2));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int32_t>(3));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<int64_t>(4));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<uint8_t>(5));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<uint16_t>(6));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<uint32_t>(7));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<uint64_t>(8));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<float>(9.9));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag(), static_cast<double>(3.14159265358979323846));
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

/**
 *  Test CHIP TLV Containers
 */
static void TestContainers()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVWriter writer;

    uint8_t sBuffer[256];
    writer.Init(sBuffer);

    TLVWriter writer2;
    err = writer.OpenContainer(AnonymousTag(), kTLVType_Array, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLVType type = writer2.GetContainerType();
    EXPECT_EQ(type, kTLVType_Array);

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.OpenContainer(AnonymousTag(), kTLVType_Structure, writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    type = writer2.GetContainerType();
    EXPECT_EQ(type, kTLVType_Structure);

    err = writer.CloseContainer(writer2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

/**
 *  Test CHIP TLV Basics
 */
TEST_F(TestTLV, CheckTLVBasics)
{
    TestItems();
    TestContainers();
}

/**
 *  Test CHIP TLV Updater
 */
TEST_F(TestTLV, CheckCHIPUpdater)
{
    WriteAppendReadTest0();

    WriteAppendReadTest1();

    WriteFindAppendReadTest(false); // Find an element

    WriteFindAppendReadTest(true); // Find a container

    AppendReadTest();

    WriteDeleteReadTest();
}

/**
 * Test TLV CloseContainer symbol reservations
 */

class OptimisticTLVWriter : public TLVWriter
{
public:
    void Init(uint8_t * buf, size_t maxLen);
};

void OptimisticTLVWriter::Init(uint8_t * buf, size_t maxLen)
{
    TLVWriter::Init(buf, maxLen);
    SetCloseContainerReserved(false);
}

TEST_F(TestTLV, CheckCloseContainerReserve)
{
    // We are writing the structure looking like:
    //
    // [{TestProfile_1:2: true}]
    //
    // the above should consume 11 bytes in the TLV encoding. The
    // chosen buffer is too small for that, this test verifies that we
    // fail in the right places in the code.  With the standard
    // TLVWriter, we now make provisions to reserve the space for the
    // CloseContainer tag in the OpenContainer call.  As a result, we
    // expect to error out when we attempt to write out the value for
    // the TestProfile_1:2 tag.  In contrast, the
    // `OptimisticTLVWriter` implements the earlier TLVWriter behavior
    // and fails out in the last CloseContainer call.  The error
    // caught there is different because we run up against the mMaxLen
    // rather than mRemainingLen check.

    uint8_t buf[10];
    uint8_t buf1[7];
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVWriter writer1;
    OptimisticTLVWriter writer2;
    TLVWriter innerWriter1, innerWriter2;
    TLVType container1, container2;

    writer1.Init(buf);

    err = writer1.OpenContainer(AnonymousTag(), kTLVType_Array, innerWriter1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = innerWriter1.OpenContainer(AnonymousTag(), kTLVType_Structure, innerWriter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = innerWriter2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = innerWriter1.CloseContainer(innerWriter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer1.CloseContainer(innerWriter1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    writer2.Init(buf, sizeof(buf));

    err = writer2.OpenContainer(AnonymousTag(), kTLVType_Array, innerWriter1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = innerWriter1.OpenContainer(AnonymousTag(), kTLVType_Structure, innerWriter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = innerWriter2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = innerWriter1.CloseContainer(innerWriter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.CloseContainer(innerWriter1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // test the same scheme works on the Start/End container

    writer1.Init(buf);

    err = writer1.StartContainer(AnonymousTag(), kTLVType_Array, container1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer1.StartContainer(AnonymousTag(), kTLVType_Structure, container2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.EndContainer(container2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer1.EndContainer(container1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    writer2.Init(buf, sizeof(buf));

    err = writer2.StartContainer(AnonymousTag(), kTLVType_Array, container1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.StartContainer(AnonymousTag(), kTLVType_Structure, container2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.EndContainer(container2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.EndContainer(container1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test that the reservations work for the empty containers

    writer1.Init(buf1);
    err = writer1.OpenContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, innerWriter1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.CloseContainer(innerWriter1);
    EXPECT_NE(err, CHIP_NO_ERROR);

    writer2.Init(buf1, sizeof(buf1));
    err = writer2.OpenContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, innerWriter1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.CloseContainer(innerWriter1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    writer1.Init(buf1);

    err = writer1.StartContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, container1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.EndContainer(container1);
    EXPECT_NE(err, CHIP_NO_ERROR);

    writer2.Init(buf1, sizeof(buf1));

    err = writer2.StartContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, container1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer2.EndContainer(container1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test that the reservations work if the writer has a maxLen of 0.

    writer1.Init(buf1, 0);

    err = writer1.OpenContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, innerWriter1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.StartContainer(AnonymousTag(), kTLVType_Array, container1);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test again all cases from 0 to the length of buf1

    for (uint32_t maxLen = 0; maxLen <= sizeof(buf); maxLen++)
    {
        // Open/CloseContainer

        writer1.Init(buf, maxLen);

        err = writer1.OpenContainer(AnonymousTag(), kTLVType_Array, innerWriter1);

        if (err == CHIP_NO_ERROR)
            err = innerWriter1.OpenContainer(AnonymousTag(), kTLVType_Structure, innerWriter2);

        if (err == CHIP_NO_ERROR)
            err = innerWriter2.PutBoolean(ProfileTag(TestProfile_1, 2), true);

        if (err == CHIP_NO_ERROR)
            err = innerWriter1.CloseContainer(innerWriter2);

        if (err == CHIP_NO_ERROR)
            err = writer1.CloseContainer(innerWriter1);

        EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);

        // Start/EndContainer

        writer1.Init(buf, maxLen);

        if (err == CHIP_NO_ERROR)
            err = writer1.StartContainer(AnonymousTag(), kTLVType_Array, container1);

        if (err == CHIP_NO_ERROR)
            err = writer1.StartContainer(AnonymousTag(), kTLVType_Structure, container2);

        if (err == CHIP_NO_ERROR)
            err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);

        if (err == CHIP_NO_ERROR)
            err = writer1.EndContainer(container2);

        if (err == CHIP_NO_ERROR)
            err = writer1.EndContainer(container1);

        EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
    }
}

static CHIP_ERROR ReadFuzzedEncoding1(TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#define FUZZ_CHECK_VAL(TYPE, VAL)                                                                                                  \
    do                                                                                                                             \
    {                                                                                                                              \
        TYPE val;                                                                                                                  \
        ReturnErrorOnFailure(reader.Get(val));                                                                                     \
        VerifyOrReturnError(val == (VAL), CHIP_ERROR_INVALID_ARGUMENT);                                                            \
    } while (0)

#define FUZZ_CHECK_STRING(VAL)                                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        char buf[sizeof(VAL)];                                                                                                     \
        VerifyOrReturnError(reader.GetLength() == strlen(VAL), CHIP_ERROR_INVALID_ADDRESS);                                        \
        ReturnErrorOnFailure(reader.GetString(buf, sizeof(buf)));                                                                  \
        VerifyOrReturnError(strcmp(buf, (VAL)) == 0, CHIP_ERROR_INVALID_ADDRESS);                                                  \
    } while (0)

    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, ProfileTag(TestProfile_1, 1)));

    {
        TLVType outerContainer1Type;

        ReturnErrorOnFailure(reader.EnterContainer(outerContainer1Type));
        ReturnErrorOnFailure(reader.Next(kTLVType_Boolean, ProfileTag(TestProfile_1, 2)));

        FUZZ_CHECK_VAL(bool, true);

        ReturnErrorOnFailure(reader.Next(kTLVType_Boolean, ProfileTag(TestProfile_2, 2)));

        FUZZ_CHECK_VAL(bool, false);

        ReturnErrorOnFailure(reader.Next(kTLVType_Array, ContextTag(0)));

        {
            TLVType outerContainer2Type;

            ReturnErrorOnFailure(reader.EnterContainer(outerContainer2Type));
            ReturnErrorOnFailure(reader.Next(kTLVType_SignedInteger, AnonymousTag()));

            FUZZ_CHECK_VAL(int8_t, 42);
            FUZZ_CHECK_VAL(int16_t, 42);
            FUZZ_CHECK_VAL(int32_t, 42);
            FUZZ_CHECK_VAL(int64_t, 42);
            FUZZ_CHECK_VAL(uint8_t, 42);
            FUZZ_CHECK_VAL(uint16_t, 42);
            FUZZ_CHECK_VAL(uint32_t, 42);
            FUZZ_CHECK_VAL(uint64_t, 42);

            ReturnErrorOnFailure(reader.Next(kTLVType_SignedInteger, AnonymousTag()));

            FUZZ_CHECK_VAL(int8_t, -17);
            FUZZ_CHECK_VAL(int16_t, -17);
            FUZZ_CHECK_VAL(int32_t, -17);
            FUZZ_CHECK_VAL(int64_t, -17);

            ReturnErrorOnFailure(reader.Next(kTLVType_SignedInteger, AnonymousTag()));

            FUZZ_CHECK_VAL(int32_t, -170000);
            FUZZ_CHECK_VAL(int64_t, -170000);

            ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, AnonymousTag()));

            FUZZ_CHECK_VAL(int64_t, 40000000000ULL);
            FUZZ_CHECK_VAL(uint64_t, 40000000000ULL);

            ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag()));

            {
                TLVType outerContainer3Type;

                ReturnErrorOnFailure(reader.EnterContainer(outerContainer3Type));
                ReturnErrorOnFailure(reader.ExitContainer(outerContainer3Type));
            }

            ReturnErrorOnFailure(reader.Next(kTLVType_List, AnonymousTag()));

            {
                TLVType outerContainer3Type;

                ReturnErrorOnFailure(reader.EnterContainer(outerContainer3Type));
                ReturnErrorOnFailure(reader.Next(kTLVType_Null, ProfileTag(TestProfile_1, 17)));
                ReturnErrorOnFailure(reader.Next(kTLVType_Null, ProfileTag(TestProfile_2, 900000)));
                ReturnErrorOnFailure(reader.Next(kTLVType_Null, AnonymousTag()));
                ReturnErrorOnFailure(reader.Next(kTLVType_Structure, ProfileTag(TestProfile_2, 4000000000ULL)));

                {
                    TLVType outerContainer4Type;

                    ReturnErrorOnFailure(reader.EnterContainer(outerContainer4Type));
                    ReturnErrorOnFailure(reader.Next(kTLVType_UTF8String, CommonTag(70000)));

                    FUZZ_CHECK_STRING(sLargeString);

                    ReturnErrorOnFailure(reader.ExitContainer(outerContainer4Type));
                }

                ReturnErrorOnFailure(reader.ExitContainer(outerContainer3Type));
            }

            ReturnErrorOnFailure(reader.ExitContainer(outerContainer2Type));
        }

        ReturnErrorOnFailure(reader.Next(kTLVType_UTF8String, ProfileTag(TestProfile_1, 5)));

        FUZZ_CHECK_STRING("This is a test");

        ReturnErrorOnFailure(reader.Next(kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535)));

        FUZZ_CHECK_VAL(double, (float) 17.9);

        ReturnErrorOnFailure(reader.Next(kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65536)));

        FUZZ_CHECK_VAL(double, (double) 17.9);

        ReturnErrorOnFailure(reader.ExitContainer(outerContainer1Type));
    }

    err = reader.Next();
    if (err == CHIP_END_OF_TLV)
        err = CHIP_NO_ERROR;

    return CHIP_NO_ERROR;
}

static uint64_t sFuzzTestDurationMillis = 5000;
static uint8_t sFixedFuzzMask           = 0;

TEST_F(TestTLV, TLVReaderFuzzTest)
{
    uint64_t now, endTime;
    uint8_t fuzzedData[sizeof(Encoding1)];

    // clang-format off
    static uint8_t sFixedFuzzVals[] =
    {
        0x00,
        0x01,
        0xFF,
        0x20, // 1-byte signed integer with context tag
        0x21, // 2-byte signed integer with context tag
        0x22, // 4-byte signed integer with context tag
        0x23, // 8-byte signed integer with context tag
        0x24, // 1-byte unsigned integer with context tag
        0x25, // 1-byte unsigned integer with context tag
        0x26, // 1-byte unsigned integer with context tag
        0x27, // 1-byte unsigned integer with context tag
        0x28, // Boolean false with context tag
        0x29, // Boolean true with context tag
        0x27, // UTF-8 string with 1-byte length and context tag
        0x30, // Byte string with 1-byte length and context tag
        0x35, // Structure with context tag
        0x36, // Array with context tag
        0x18, // End of container
    };
    // clang-format on

    memcpy(fuzzedData, Encoding1, sizeof(fuzzedData));

    now     = chip::test_utils::TimeMonotonicMillis();
    endTime = now + sFuzzTestDurationMillis + 1;

    srand(static_cast<unsigned int>(now));

    size_t m = 0;
    while (true)
    {
        for (size_t i = 0; i < sizeof(fuzzedData); i++)
        {
            uint8_t origVal = fuzzedData[i];

            if (m < sizeof(sFixedFuzzVals))
            {
                if (origVal == sFixedFuzzVals[m])
                    continue;

                fuzzedData[i] = sFixedFuzzVals[m];
            }

            else
            {
                uint8_t fuzzMask = sFixedFuzzMask;
                while (fuzzMask == 0)
                    fuzzMask = static_cast<uint8_t>(rand() & 0xFF);

                fuzzedData[i] ^= fuzzMask;
            }

            TLVReader reader;
            reader.Init(fuzzedData);
            reader.ImplicitProfileId = TestProfile_2;

            CHIP_ERROR readRes = ReadFuzzedEncoding1(reader);
            EXPECT_NE(readRes, CHIP_NO_ERROR);

            if (readRes == CHIP_NO_ERROR)
            {
                printf("Unexpected success of fuzz test: offset %u, original value 0x%02X, mutated value 0x%02X\n",
                       static_cast<unsigned>(i), static_cast<unsigned>(origVal), static_cast<unsigned>(fuzzedData[i]));
                return;
            }

            now = chip::test_utils::TimeMonotonicMillis();
            if (now >= endTime)
                return;

            fuzzedData[i] = origVal;
        }

        if (m < sizeof(sFixedFuzzVals))
            m++;
    }
}

static void AssertCanReadString(ContiguousBufferTLVReader & reader, const char * expectedString)
{
    Span<const char> str;
    CHIP_ERROR err = reader.GetStringView(str);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(str.size(), strlen(expectedString));
    EXPECT_EQ(strncmp(str.data(), expectedString, str.size()), 0);
}

static void AssertCannotReadString(ContiguousBufferTLVReader & reader)
{
    Span<const char> str;
    CHIP_ERROR err = reader.GetStringView(str);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST_F(TestTLV, CheckGetStringView)
{
    uint8_t buf[256];
    static const char testString[] = "This is a test";
    {
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.PutString(CommonTag(0), testString);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // First check that basic read from entire buffer works.
        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCanReadString(reader, testString);

        // Now check that read from a buffer bounded by the number of bytes
        // written works.
        reader.Init(buf, writer.GetLengthWritten());
        reader.Next();
        AssertCanReadString(reader, testString);

        // Now check that read from a buffer bounded by fewer than the number of
        // bytes written fails.
        reader.Init(buf, writer.GetLengthWritten() - 1);
        reader.Next();
        AssertCannotReadString(reader);
    }

    {
        // Check that an integer cannot be read as a string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.Put(CommonTag(0), static_cast<uint8_t>(5));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadString(reader);
    }

    {
        // Check that an octet string cannot be read as a string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err =
            writer.PutBytes(CommonTag(0), reinterpret_cast<const uint8_t *>(testString), static_cast<uint32_t>(strlen(testString)));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadString(reader);
    }

    {
        // Check that a manually constructed string can be read as a string.
        const uint8_t shortString[] = { CHIP_TLV_UTF8_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 2, 'a', 'b') };
        ContiguousBufferTLVReader reader;
        reader.Init(shortString);
        reader.Next();
        AssertCanReadString(reader, "ab");
    }

    {
        // Check that a manually constructed string with bogus length cannot be read as a string.
        const uint8_t shortString[] = { CHIP_TLV_UTF8_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 3, 'a', 'b') };
        ContiguousBufferTLVReader reader;
        reader.Init(shortString);
        reader.Next();
        AssertCannotReadString(reader);
    }
}

static void AssertCanReadBytes(ContiguousBufferTLVReader & reader, const ByteSpan & expectedBytes)
{
    ByteSpan bytes;
    CHIP_ERROR err = reader.GetByteView(bytes);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_TRUE(bytes.data_equal(expectedBytes));
}

static void AssertCannotReadBytes(ContiguousBufferTLVReader & reader)
{
    ByteSpan bytes;
    CHIP_ERROR err = reader.GetByteView(bytes);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST_F(TestTLV, CheckGetByteView)
{
    uint8_t buf[256];
    const uint8_t testBytes[] = { 'T', 'h', 'i', 's', 'i', 's', 'a', 't', 'e', 's', 't', '\0' };
    {
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.PutBytes(CommonTag(0), testBytes, sizeof(testBytes));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        // First check that basic read from entire buffer works.
        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCanReadBytes(reader, ByteSpan(testBytes));

        // Now check that read from a buffer bounded by the number of bytes
        // written works.
        reader.Init(buf, writer.GetLengthWritten());
        reader.Next();
        AssertCanReadBytes(reader, ByteSpan(testBytes));

        // Now check that read from a buffer bounded by fewer than the number of
        // bytes written fails.
        reader.Init(buf, writer.GetLengthWritten() - 1);
        reader.Next();
        AssertCannotReadBytes(reader);
    }

    {
        // Check that an integer cannot be read as an octet string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.Put(CommonTag(0), static_cast<uint8_t>(5));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadBytes(reader);
    }

    {
        // Check that an string cannot be read as an octet string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.PutString(CommonTag(0), reinterpret_cast<const char *>(testBytes));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadBytes(reader);
    }

    {
        // Check that a manually constructed octet string can be read as octet string.
        const uint8_t shortBytes[] = { CHIP_TLV_BYTE_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 2, 1, 2) };
        ContiguousBufferTLVReader reader;
        reader.Init(shortBytes);
        reader.Next();
        const uint8_t expectedBytes[] = { 1, 2 };
        AssertCanReadBytes(reader, ByteSpan(expectedBytes));
    }

    {
        // Check that a manually constructed octet string with bogus length
        // cannot be read as an octet string.
        const uint8_t shortBytes[] = { CHIP_TLV_BYTE_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 3, 1, 2) };
        ContiguousBufferTLVReader reader;
        reader.Init(shortBytes);
        reader.Next();
        AssertCannotReadBytes(reader);
    }
}

TEST_F(TestTLV, CheckTLVScopedBuffer)
{
    Platform::ScopedMemoryBuffer<uint8_t> buf;
    CHIP_ERROR err;

    buf.Calloc(64);
    ASSERT_NE(buf.Get(), nullptr);

    {
        ScopedBufferTLVWriter writer(std::move(buf), 64);

        EXPECT_EQ(buf.Get(), nullptr); // // NOLINT(bugprone-use-after-move)

        err = writer.Put(TLV::AnonymousTag(), (uint8_t) 33);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize(buf);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        ASSERT_NE(buf.Get(), nullptr);

        err = writer.Put(TLV::AnonymousTag(), (uint8_t) 33);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }

    {
        ScopedBufferTLVReader reader;
        uint8_t val;

        reader.Init(std::move(buf), 64);

        err = reader.Next();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = reader.Get(val);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(val, 33);

        reader.TakeBuffer(buf);
        ASSERT_NE(buf.Get(), nullptr);

        err = reader.Get(val);
        EXPECT_NE(err, CHIP_NO_ERROR);
    }
}

TEST_F(TestTLV, TestUninitializedWriter)
{
    {
        TLVWriter writer;
        EXPECT_FALSE(writer.IsInitialized());
    }

    {
        TLVWriter writer;
        EXPECT_EQ(writer.Finalize(), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        EXPECT_EQ(writer.ReserveBuffer(123), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        EXPECT_EQ(writer.UnreserveBuffer(123), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint8_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int8_t v          = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int16_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int16_t v         = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int32_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int32_t v         = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int64_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        int64_t v         = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint8_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint8_t v         = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint16_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint16_t v        = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint32_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint32_t v        = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint64_t v = 3;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        uint64_t v        = 3;
        bool preserveSize = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v, preserveSize), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        double v = 1.23;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        float v = 1.23f;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        bool v = true;
        EXPECT_EQ(writer.PutBoolean(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        bool v = true;
        EXPECT_EQ(writer.Put(ContextTag(1), v), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        const uint8_t buf[] = { 1, 2, 3 };
        EXPECT_EQ(writer.PutBytes(ContextTag(1), buf, static_cast<uint32_t>(sizeof(buf))), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        const char * buf = "abc";
        EXPECT_EQ(writer.PutString(ContextTag(1), buf), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        const char * buf = "abc";
        EXPECT_EQ(writer.PutString(ContextTag(1), buf, static_cast<uint32_t>(strlen(buf))), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        CharSpan str = "abc"_span;
        EXPECT_EQ(writer.PutString(ContextTag(1), str), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        EXPECT_EQ(writer.PutStringF(ContextTag(1), "%d", 1), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        EXPECT_EQ(writer.PutNull(ContextTag(1)), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        const uint8_t buf[]{ 0, 0, 0 };
        TLVReader reader;
        reader.Init(buf);

        TLVWriter writer;
        EXPECT_EQ(writer.CopyElement(reader), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        const uint8_t buf[]{ 0, 0, 0 };
        TLVReader reader;
        reader.Init(buf);

        TLVWriter writer;
        EXPECT_EQ(writer.CopyElement(ContextTag(1), reader), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        TLVType outerContainerType;

        EXPECT_EQ(writer.StartContainer(ContextTag(1), TLVType::kTLVType_Structure, outerContainerType),
                  CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter writer;
        TLVType outerContainerType = TLVType::kTLVType_Structure;
        EXPECT_EQ(writer.EndContainer(outerContainerType), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter innerWriter;
        uint8_t buf[]{ 0, 0, 0 };
        innerWriter.Init(buf);
        EXPECT_TRUE(innerWriter.IsInitialized());

        TLVWriter writer;
        EXPECT_EQ(writer.OpenContainer(ContextTag(1), TLVType::kTLVType_Structure, innerWriter), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        TLVWriter innerWriter;
        uint8_t buf[]{ 0, 0, 0 };
        innerWriter.Init(buf);
        EXPECT_TRUE(innerWriter.IsInitialized());

        TLVWriter writer;
        EXPECT_EQ(writer.CloseContainer(innerWriter), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        uint8_t buf[]{ 0, 0, 0 };
        TLVWriter writer;
        EXPECT_EQ(
            writer.PutPreEncodedContainer(ContextTag(1), TLVType::kTLVType_Structure, buf, static_cast<uint32_t>(sizeof(buf))),
            CHIP_ERROR_INCORRECT_STATE);
    }

    {
        const uint8_t buf[]{ 0, 0, 0 };
        TLVReader reader;
        reader.Init(buf);

        TLVWriter writer;
        EXPECT_EQ(writer.CopyContainer(reader), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        const uint8_t buf[]{ 0, 0, 0 };
        TLVReader reader;
        reader.Init(buf);

        TLVWriter writer;
        EXPECT_EQ(writer.CopyContainer(ContextTag(1), reader), CHIP_ERROR_INCORRECT_STATE);
    }

    {
        uint8_t buf[]{ 0, 0, 0 };

        TLVWriter writer;
        EXPECT_EQ(writer.CopyContainer(ContextTag(1), buf, static_cast<uint16_t>(sizeof(buf))), CHIP_ERROR_INCORRECT_STATE);
    }
}
