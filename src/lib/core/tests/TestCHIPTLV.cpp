/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <nlunit-test.h>

#include <lib/core/CHIPCircularTLVBuffer.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVData.hpp>
#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>

#include <system/TLVPacketBufferBackingStore.h>

#include <string.h>

using namespace chip;
using namespace chip::TLV;

enum
{
    TestProfile_1 = 0xAABBCCDD,
    TestProfile_2 = 0x11223344
};

// clang-format off
static const char sLargeString [] =
    "START..."
    "!123456789ABCDEF@123456789ABCDEF#123456789ABCDEF$123456789ABCDEF%123456789ABCDEF^123456789ABCDEF&123456789ABCDEF*123456789ABCDEF"
    "01234567(9ABCDEF01234567)9ABCDEF01234567-9ABCDEF01234567=9ABCDEF01234567[9ABCDEF01234567]9ABCDEF01234567;9ABCDEF01234567'9ABCDEF"
    "...END";
// clang-format on

void TestAndOpenContainer(nlTestSuite * inSuite, TLVReader & reader, TLVType type, uint64_t tag, TLVReader & containerReader)
{
    NL_TEST_ASSERT(inSuite, reader.GetType() == type);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == tag);
    NL_TEST_ASSERT(inSuite, reader.GetLength() == 0);

    CHIP_ERROR err = reader.OpenContainer(containerReader);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, containerReader.GetContainerType() == type);
}

template <class T>
void TestAndEnterContainer(nlTestSuite * inSuite, T & t, TLVType type, uint64_t tag, TLVType & outerContainerType)
{
    NL_TEST_ASSERT(inSuite, t.GetType() == type);
    NL_TEST_ASSERT(inSuite, t.GetTag() == tag);
    NL_TEST_ASSERT(inSuite, t.GetLength() == 0);

    TLVType expectedContainerType = t.GetContainerType();

    CHIP_ERROR err = t.EnterContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, outerContainerType == expectedContainerType);
    NL_TEST_ASSERT(inSuite, t.GetContainerType() == type);
}

template <class T>
void TestNext(nlTestSuite * inSuite, T & t)
{
    CHIP_ERROR err = t.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestSkip(nlTestSuite * inSuite, TLVReader & reader)
{
    CHIP_ERROR err = reader.Skip();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestMove(nlTestSuite * inSuite, TLVUpdater & updater)
{
    CHIP_ERROR err = updater.Move();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

template <class T>
void TestEnd(nlTestSuite * inSuite, T & t)
{
    CHIP_ERROR err;

    err = t.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_END_OF_TLV);
}

void TestEndAndCloseContainer(nlTestSuite * inSuite, TLVReader & reader, TLVReader & containerReader)
{
    CHIP_ERROR err;

    TestEnd<TLVReader>(inSuite, containerReader);

    err = reader.CloseContainer(containerReader);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

template <class T>
void TestEndAndExitContainer(nlTestSuite * inSuite, T & t, TLVType outerContainerType)
{
    CHIP_ERROR err;

    TestEnd<T>(inSuite, t);

    err = t.ExitContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, t.GetContainerType() == outerContainerType);
}

template <class S, class T>
void TestGet(nlTestSuite * inSuite, S & s, TLVType type, uint64_t tag, T expectedVal)
{
    NL_TEST_ASSERT(inSuite, s.GetType() == type);
    NL_TEST_ASSERT(inSuite, s.GetTag() == tag);
    NL_TEST_ASSERT(inSuite, s.GetLength() == 0);

    T val;
    CHIP_ERROR err = s.Get(val);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, val == expectedVal);
}

void ForEachElement(nlTestSuite * inSuite, TLVReader & reader, void * context,
                    void (*cb)(nlTestSuite * inSuite, TLVReader & reader, void * context))
{
    CHIP_ERROR err;

    while (true)
    {
        err = reader.Next();
        if (err == CHIP_END_OF_TLV)
        {
            return;
        }
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        if (cb != nullptr)
        {
            cb(inSuite, reader, context);
        }

        if (TLVTypeIsContainer(reader.GetType()))
        {
            TLVType outerContainerType;

            err = reader.EnterContainer(outerContainerType);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ForEachElement(inSuite, reader, context, cb);

            err = reader.ExitContainer(outerContainerType);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }
    }
}

/**
 * context
 */

struct TestTLVContext
{
    nlTestSuite * mSuite;
    int mEvictionCount;
    uint32_t mEvictedBytes;
};

void TestNull(nlTestSuite * inSuite, TLVReader & reader, uint64_t tag)
{
    NL_TEST_ASSERT(inSuite, reader.GetType() == kTLVType_Null);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == tag);
    NL_TEST_ASSERT(inSuite, reader.GetLength() == 0);
}

void TestString(nlTestSuite * inSuite, TLVReader & reader, uint64_t tag, const char * expectedVal)
{
    NL_TEST_ASSERT(inSuite, reader.GetType() == kTLVType_UTF8String);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == tag);

    size_t expectedLen = strlen(expectedVal);
    NL_TEST_ASSERT(inSuite, reader.GetLength() == expectedLen);

    chip::Platform::ScopedMemoryBuffer<char> valBuffer;
    char * val = static_cast<char *>(valBuffer.Alloc(expectedLen + 1).Get());

    CHIP_ERROR err = reader.GetString(val, static_cast<uint32_t>(expectedLen) + 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(val, expectedVal, expectedLen + 1) == 0);
}

void TestDupString(nlTestSuite * inSuite, TLVReader & reader, uint64_t tag, const char * expectedVal)
{
    NL_TEST_ASSERT(inSuite, reader.GetType() == kTLVType_UTF8String);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == tag);

    size_t expectedLen = strlen(expectedVal);
    NL_TEST_ASSERT(inSuite, reader.GetLength() == expectedLen);

    chip::Platform::ScopedMemoryBuffer<char> valBuffer;
    char * val = valBuffer.Alloc(expectedLen + 1).Get();

    CHIP_ERROR err = reader.DupString(val);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(val, expectedVal, expectedLen + 1) == 0);
}

void TestDupBytes(nlTestSuite * inSuite, TLVReader & reader, uint64_t tag, const uint8_t * expectedVal, uint32_t expectedLen)
{
    NL_TEST_ASSERT(inSuite, reader.GetType() == kTLVType_UTF8String);
    NL_TEST_ASSERT(inSuite, reader.GetTag() == tag);

    NL_TEST_ASSERT(inSuite, reader.GetLength() == expectedLen);

    chip::Platform::ScopedMemoryBuffer<uint8_t> valBuffer;
    uint8_t * val  = valBuffer.Alloc(expectedLen).Get();
    CHIP_ERROR err = reader.DupBytes(val, expectedLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(val, expectedVal, expectedLen) == 0);
}

void TestBufferContents(nlTestSuite * inSuite, const System::PacketBufferHandle & buffer, const uint8_t * expectedVal,
                        uint32_t expectedLen)
{
    System::PacketBufferHandle buf = buffer.Retain();
    while (!buf.IsNull())
    {
        uint16_t len = buf->DataLength();
        NL_TEST_ASSERT(inSuite, len <= expectedLen);

        NL_TEST_ASSERT(inSuite, memcmp(buf->Start(), expectedVal, len) == 0);

        expectedVal += len;
        expectedLen -= len;

        buf.Advance();
    }

    NL_TEST_ASSERT(inSuite, expectedLen == 0);
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

void WriteEncoding1(nlTestSuite * inSuite, TLVWriter & writer)
{
    CHIP_ERROR err;
    TLVWriter writer2;

    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.PutBoolean(ProfileTag(TestProfile_2, 2), false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    {
        TLVWriter writer3;

        err = writer2.OpenContainer(ContextTag(0), kTLVType_Array, writer3);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // TODO(#1306): expand coverage of inttype encoding tests.
        err = writer3.Put(AnonymousTag, static_cast<int32_t>(42));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int32_t>(-17));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int32_t>(-170000));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<uint64_t>(40000000000ULL));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        {
            TLVWriter writer4;

            err = writer3.OpenContainer(AnonymousTag, kTLVType_Structure, writer4);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer3.CloseContainer(writer4);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        {
            TLVWriter writer5;

            err = writer3.OpenContainer(AnonymousTag, kTLVType_List, writer5);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer5.PutNull(ProfileTag(TestProfile_1, 17));
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer5.PutNull(ProfileTag(TestProfile_2, 900000));
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer5.PutNull(AnonymousTag);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            {
                TLVType outerContainerType;

                err = writer5.StartContainer(ProfileTag(TestProfile_2, 4000000000ULL), kTLVType_Structure, outerContainerType);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

                err = writer5.PutString(CommonTag(70000), sLargeString);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

                err = writer5.EndContainer(outerContainerType);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            }

            err = writer3.CloseContainer(writer5);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        err = writer2.CloseContainer(writer3);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer2.PutString(ProfileTag(TestProfile_1, 5), "This is a test");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.Put(ProfileTag(TestProfile_2, 65535), static_cast<float>(17.9));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.Put(ProfileTag(TestProfile_2, 65536), 17.9);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void WriteEmptyEncoding(nlTestSuite * inSuite, TLVWriter & writer)
{
    CHIP_ERROR err;
    TLVWriter writer2;

    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    {
        TLVWriter writer3;

        err = writer2.OpenContainer(ProfileTag(TestProfile_1, 256), kTLVType_Array, writer3);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.CloseContainer(writer3);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void ReadEncoding1(nlTestSuite * inSuite, TLVReader & reader)
{
    TestNext<TLVReader>(inSuite, reader);

    {
        TLVReader reader2;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(inSuite, reader2);

        {
            TLVReader reader3;

            TestAndOpenContainer(inSuite, reader2, kTLVType_Array, ContextTag(0), reader3);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int8_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, int16_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, int32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, uint8_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, uint16_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, uint32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, uint64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int8_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);
            TestGet<TLVReader, int16_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);
            TestGet<TLVReader, int32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);
            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -170000);
            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -170000);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_UnsignedInteger, AnonymousTag, 40000000000ULL);
            TestGet<TLVReader, uint64_t>(inSuite, reader3, kTLVType_UnsignedInteger, AnonymousTag, 40000000000ULL);

            TestNext<TLVReader>(inSuite, reader3);

            {
                TLVReader reader4;

                TestAndOpenContainer(inSuite, reader3, kTLVType_Structure, AnonymousTag, reader4);

                TestEndAndCloseContainer(inSuite, reader3, reader4);
            }

            TestNext<TLVReader>(inSuite, reader3);

            {
                TLVReader reader5;

                TestAndOpenContainer(inSuite, reader3, kTLVType_List, AnonymousTag, reader5);

                TestNext<TLVReader>(inSuite, reader5);

                TestNull(inSuite, reader5, ProfileTag(TestProfile_1, 17));

                TestNext<TLVReader>(inSuite, reader5);

                TestNull(inSuite, reader5, ProfileTag(TestProfile_2, 900000));

                TestNext<TLVReader>(inSuite, reader5);

                TestNull(inSuite, reader5, AnonymousTag);

                TestNext<TLVReader>(inSuite, reader5);

                {
                    TLVType outerContainerType;

                    TestAndEnterContainer<TLVReader>(inSuite, reader5, kTLVType_Structure, ProfileTag(TestProfile_2, 4000000000ULL),
                                                     outerContainerType);

                    TestNext<TLVReader>(inSuite, reader5);

                    TestString(inSuite, reader5, CommonTag(70000), sLargeString);

                    TestEndAndExitContainer<TLVReader>(inSuite, reader5, outerContainerType);
                }

                TestEndAndCloseContainer(inSuite, reader3, reader5);
            }

            TestEndAndCloseContainer(inSuite, reader2, reader3);
        }

        TestNext<TLVReader>(inSuite, reader2);

        TestString(inSuite, reader2, ProfileTag(TestProfile_1, 5), "This is a test");

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, float>(inSuite, reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), 17.9f);
        TestGet<TLVReader, double>(inSuite, reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), 17.9f);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, double>(inSuite, reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65536), 17.9);

        TestEndAndCloseContainer(inSuite, reader, reader2);
    }

    TestEnd<TLVReader>(inSuite, reader);
}

void WriteEncoding2(nlTestSuite * inSuite, TLVWriter & writer)
{
    CHIP_ERROR err;

    { // Container 1
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    { // Container 2
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void WriteEncoding3(nlTestSuite * inSuite, TLVWriter & writer)
{
    CHIP_ERROR err;

    { // Container 1
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
        if (err != CHIP_NO_ERROR)
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void ReadEncoding3(nlTestSuite * inSuite, TLVReader & reader)
{
    TLVReader reader2;

    TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

    TestNext<TLVReader>(inSuite, reader2);

    TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

    TestEndAndCloseContainer(inSuite, reader, reader2);
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

void WriteEncoding5(nlTestSuite * inSuite, TLVWriter & writer)
{
    CHIP_ERROR err;

    { // Container 1
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        { // Inner Container 1
            TLVWriter writer2;

            err = writer1.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer1.CloseContainer(writer2);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        { // Inner Container 2
            TLVWriter writer2;

            err = writer1.OpenContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, writer2);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            err = writer1.CloseContainer(writer2);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        err = writer.CloseContainer(writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    { // Container 2
        TLVWriter writer1;

        err = writer.OpenContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer.CloseContainer(writer1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
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
void AppendEncoding2(nlTestSuite * inSuite, uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    TestNext<TLVUpdater>(inSuite, updater);

    {
        TLVType outerContainerType;

        TestAndEnterContainer<TLVUpdater>(inSuite, updater, kTLVType_Structure, ProfileTag(TestProfile_1, 1), outerContainerType);

        TestNext<TLVUpdater>(inSuite, updater);

        // Move the element without modification
        TestMove(inSuite, updater);

        TestNext<TLVUpdater>(inSuite, updater);

        // Read and copy the element with/without modification
        TestGet<TLVUpdater, bool>(inSuite, updater, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // TestEnd and add data at the end of the container
        TestEnd<TLVUpdater>(inSuite, updater);

        // Put new values in the encoding using the updater
        // Add <TestProfile_1, 2, false>
        err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add <TestProfile_2, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add a new container
        {
            TLVType outerContainerType1;

            err = updater.StartContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, outerContainerType1);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            // Add <TestProfile_1, 2, true>
            err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            // Add <TestProfile_1, 2, true>
            err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            // Close the container
            err = updater.EndContainer(outerContainerType1);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        // Add another new container
        {
            TLVType outerContainerType1;

            err = updater.StartContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, outerContainerType1);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            // Add <TestProfile_2, 2, false>
            err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            // Add <TestProfile_1, 2, true>
            err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            // Close the container
            err = updater.EndContainer(outerContainerType1);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        TestEndAndExitContainer<TLVUpdater>(inSuite, updater, outerContainerType);
    }

    TestNext<TLVUpdater>(inSuite, updater);

    // Move the container unmodified
    TestMove(inSuite, updater);

    TestEnd<TLVUpdater>(inSuite, updater);

    err = updater.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

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
void FindAppendEncoding2(nlTestSuite * inSuite, uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen,
                         bool findContainer)
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = tagReader.EnterContainer(outerContainerType);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        do
        {
            err = tagReader.Next();
        } while (err != CHIP_END_OF_TLV);

        TestEnd<TLVReader>(inSuite, tagReader);

        // Init a TLVUpdater using the TLVReader
        err = updater.Init(tagReader, maxLen - dataLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }
    else
    {
        // Find
        TLVReader tagReader;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 2), tagReader);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Test Find(recurse = true)
        TLVReader tagReader2;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 2), tagReader2, true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        //
        // Test Find(recurse = false)
        TLVReader tagReader3;
        err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 2), tagReader3, false);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_TLV_TAG_NOT_FOUND);

        // Init a TLVUpdater using the TLVReader
        err = updater.Init(tagReader, maxLen - dataLen);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        TestNext<TLVUpdater>(inSuite, updater);

        // Move the element without modification
        TestMove(inSuite, updater);
    }

    // Put new values in the encoding using the updater
    // Add <TestProfile_1, 2, false>
    err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Add <TestProfile_2, 2, true>
    err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Add a new container
    {
        TLVType outerContainerType1;

        err = updater.StartContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, outerContainerType1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Close the container
        err = updater.EndContainer(outerContainerType1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    // Add another new container
    {
        TLVType outerContainerType1;

        err = updater.StartContainer(ProfileTag(TestProfile_2, 1), kTLVType_Structure, outerContainerType1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add <TestProfile_2, 2, false>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_1, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Close the container
        err = updater.EndContainer(outerContainerType1);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    // Move everything else unmodified
    updater.MoveUntilEnd();

    TestEnd<TLVUpdater>(inSuite, updater);

    err = updater.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void AppendEncoding3(nlTestSuite * inSuite, uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    TestNext<TLVUpdater>(inSuite, updater);

    {
        TLVType outerContainerType;

        TestAndEnterContainer<TLVUpdater>(inSuite, updater, kTLVType_Structure, ProfileTag(TestProfile_1, 1), outerContainerType);

        TestNext<TLVUpdater>(inSuite, updater);

        // Move the element without modification
        TestMove(inSuite, updater);

        // Put new value in the encoding using the updater
        // Add <TestProfile_2, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        TestEndAndExitContainer<TLVUpdater>(inSuite, updater, outerContainerType);
    }

    TestEnd<TLVUpdater>(inSuite, updater);

    err = updater.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void AppendEncoding4(nlTestSuite * inSuite, uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    // Add a new container
    {
        TLVType outerContainerType;

        err = updater.StartContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, outerContainerType);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Add <TestProfile_1, 2, true>
        err = updater.PutBoolean(ProfileTag(TestProfile_2, 2), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Close the container
        err = updater.EndContainer(outerContainerType);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = updater.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void DeleteEncoding5(nlTestSuite * inSuite, uint8_t * buf, uint32_t dataLen, uint32_t maxLen, uint32_t & updatedLen)
{
    CHIP_ERROR err;

    TLVUpdater updater;

    err = updater.Init(buf, dataLen, maxLen);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updater.SetImplicitProfileId(TestProfile_2);

    TestNext<TLVUpdater>(inSuite, updater);

    {
        TLVType outerContainerType;

        TestAndEnterContainer<TLVUpdater>(inSuite, updater, kTLVType_Structure, ProfileTag(TestProfile_1, 1), outerContainerType);

        TestNext<TLVUpdater>(inSuite, updater);

        TestMove(inSuite, updater);

        TestNext<TLVUpdater>(inSuite, updater);

        TestMove(inSuite, updater);

        TestNext<TLVUpdater>(inSuite, updater);

        // Get the value to inspect and skip writing it
        TestGet<TLVUpdater, bool>(inSuite, updater, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

        TestNext<TLVUpdater>(inSuite, updater);

        // Skip the next boolean type and don't copy by doing nothing

        TestNext<TLVUpdater>(inSuite, updater);

        // Read ahead into the next container and decide whether to skip or
        // not based on elements in the container
        {
            TLVReader reader;
            TLVType containerType;

            updater.GetReader(reader);

            TestAndEnterContainer<TLVReader>(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), containerType);

            TestNext<TLVReader>(inSuite, reader);

            // If the container's first element has the tag <TestProfile_1, 2>
            // skip the whole container, and if NOT copy the container
            if (reader.GetTag() != ProfileTag(TestProfile_1, 2))
                TestMove(inSuite, updater);
        }

        TestNext<TLVUpdater>(inSuite, updater);

        // Skip the next container and don't copy by doing nothing

        TestEndAndExitContainer<TLVUpdater>(inSuite, updater, outerContainerType);
    }

    // Move everything else unmodified
    updater.MoveUntilEnd();

    TestEnd<TLVUpdater>(inSuite, updater);

    err = updater.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    updatedLen = updater.GetLengthWritten();
}

void ReadAppendedEncoding2(nlTestSuite * inSuite, TLVReader & reader)
{
    TestNext<TLVReader>(inSuite, reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), true);

        TestNext<TLVReader>(inSuite, reader1);

        {
            TLVReader reader2;

            TestAndOpenContainer(inSuite, reader1, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

            TestNext<TLVReader>(inSuite, reader2);

            TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

            TestNext<TLVReader>(inSuite, reader2);

            TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

            TestEndAndCloseContainer(inSuite, reader1, reader2);
        }

        TestNext<TLVReader>(inSuite, reader1);

        {
            TLVReader reader2;

            TestAndOpenContainer(inSuite, reader1, kTLVType_Structure, ProfileTag(TestProfile_2, 1), reader2);

            TestNext<TLVReader>(inSuite, reader2);

            TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

            TestNext<TLVReader>(inSuite, reader2);

            TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

            TestEndAndCloseContainer(inSuite, reader1, reader2);
        }

        TestEndAndCloseContainer(inSuite, reader, reader1);
    }

    TestNext<TLVReader>(inSuite, reader);

    { // Container 2
        TLVReader reader1;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_2, 1), reader1);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestEndAndCloseContainer(inSuite, reader, reader1);
    }

    TestEnd<TLVReader>(inSuite, reader);
}

void ReadAppendedEncoding3(nlTestSuite * inSuite, TLVReader & reader)
{
    TestNext<TLVReader>(inSuite, reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), true);

        TestEndAndCloseContainer(inSuite, reader, reader1);
    }

    TestEnd<TLVReader>(inSuite, reader);
}

void ReadAppendedEncoding4(nlTestSuite * inSuite, TLVReader & reader)
{
    TestNext<TLVReader>(inSuite, reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestEndAndCloseContainer(inSuite, reader, reader1);
    }

    TestEnd<TLVReader>(inSuite, reader);
}

void ReadDeletedEncoding5(nlTestSuite * inSuite, TLVReader & reader)
{
    TestNext<TLVReader>(inSuite, reader);

    { // Container 1
        TLVReader reader1;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader1);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestEndAndCloseContainer(inSuite, reader, reader1);
    }

    TestNext<TLVReader>(inSuite, reader);

    { // Container 2
        TLVReader reader1;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_2, 1), reader1);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(inSuite, reader1);

        TestGet<TLVReader, bool>(inSuite, reader1, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestEndAndCloseContainer(inSuite, reader, reader1);
    }

    TestEnd<TLVReader>(inSuite, reader);
}

/**
 *  Test Simple Write and Reader
 */
void CheckSimpleWriteRead(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;
    uint32_t remainingFreedLen;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    remainingFreedLen = writer.GetRemainingFreeLength();
    NL_TEST_ASSERT(inSuite, sizeof(buf) == remainingFreedLen);

    WriteEncoding1(inSuite, writer);

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

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding1));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding1, encodedLen) == 0);

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadEncoding1(inSuite, reader);
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
void SimpleDumpWriter(const char * aFormat, ...)
{
    va_list args;

    va_start(args, aFormat);

    vprintf(aFormat, args);

    va_end(args);
}

/**
 *  Test Pretty Printer
 */
void CheckPrettyPrinter(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding1(inSuite, writer);

    uint32_t encodedLen = writer.GetLengthWritten();

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding1));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding1, encodedLen) == 0);

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;
    chip::TLV::Debug::Dump(reader, SimpleDumpWriter);
}

/**
 *  Test Data Macros
 */
void CheckDataMacro(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, sizeof(Encoding1_DataMacro) == sizeof(Encoding1));
    NL_TEST_ASSERT(inSuite, memcmp(Encoding1, Encoding1_DataMacro, sizeof(Encoding1)) == 0);

    uint8_t buf[2048];
    TLVWriter writer;
    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;
    WriteEncoding5(inSuite, writer);
    uint32_t encodedLen = writer.GetLengthWritten();

    NL_TEST_ASSERT(inSuite, sizeof(Encoding5_DataMacro) == encodedLen);
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding5_DataMacro, encodedLen) == 0);
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
    uint64_t * tag = static_cast<uint64_t *>(aContext);
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
void CheckCHIPTLVUtilities(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader, reader1;
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding1(inSuite, writer);

    uint32_t encodedLen = writer.GetLengthWritten();

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding1));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding1, encodedLen) == 0);

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;

    reader1.Init(reader);
    err = reader1.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Find a tag
    TLVReader tagReader;
    err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 65536), tagReader);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Find with reader positioned "on" the element of interest
    err = chip::TLV::Utilities::Find(reader1, ProfileTag(TestProfile_1, 1), tagReader);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Find a tag that's not present
    err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_2, 1024), tagReader);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_TLV_TAG_NOT_FOUND);

    // Find with a predicate
    {
        uint8_t buf1[74];

        writer.Init(buf1);
        writer.ImplicitProfileId = TestProfile_2;

        WriteEncoding2(inSuite, writer);

        // Initialize a reader
        reader1.Init(buf1, writer.GetLengthWritten());
        reader1.ImplicitProfileId = TestProfile_2;

        // position the reader on the first element
        reader1.Next();
        uint64_t tag = ProfileTag(TestProfile_1, 1);
        err          = chip::TLV::Utilities::Find(reader1, FindContainerWithElement, &tag, tagReader, false);
        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_TLV_TAG_NOT_FOUND);

        tag = ProfileTag(TestProfile_2, 2);
        err = chip::TLV::Utilities::Find(reader1, FindContainerWithElement, &tag, tagReader, false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, tagReader.GetType() == kTLVType_Structure);
        NL_TEST_ASSERT(inSuite, tagReader.GetTag() == ProfileTag(TestProfile_1, 1));

        // Position the reader on the second element
        reader1.Next();
        err = chip::TLV::Utilities::Find(reader1, FindContainerWithElement, &tag, tagReader, false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        NL_TEST_ASSERT(inSuite, tagReader.GetType() == kTLVType_Structure);
        NL_TEST_ASSERT(inSuite, tagReader.GetTag() == ProfileTag(TestProfile_2, 1));
    }

    // Count
    size_t count;
    const size_t expectedCount = 18;
    reader1.Init(reader);
    reader1.Next();

    err = chip::TLV::Utilities::Count(reader, count);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == expectedCount);

    // Count with reader already positioned "on" the first element in the encoding
    err = chip::TLV::Utilities::Count(reader1, count);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, count == expectedCount);

    // Iterate
    err = chip::TLV::Utilities::Iterate(reader, NullIterateHandler, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_END_OF_TLV);
}

/**
 *  Test CHIP TLV Empty Find
 */
void CheckCHIPTLVEmptyFind(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf[30];
    TLVWriter writer;
    TLVReader reader;
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEmptyEncoding(inSuite, writer);

    uint32_t encodedLen = writer.GetLengthWritten();

    reader.Init(buf, encodedLen);
    reader.ImplicitProfileId = TestProfile_2;

    // Find the empty container
    TLVReader tagReader;
    err = chip::TLV::Utilities::Find(reader, ProfileTag(TestProfile_1, 256), tagReader);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
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

void WriteAppendReadTest0(nlTestSuite * inSuite)
{
    uint8_t buf[74];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding2(inSuite, writer);

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

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding2));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding2, encodedLen) == 0);

    // Append new data into encoding
    AppendEncoding2(inSuite, buf, encodedLen, sizeof(buf), updatedLen);

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

    NL_TEST_ASSERT(inSuite, updatedLen == sizeof(AppendedEncoding2));
    NL_TEST_ASSERT(inSuite, memcmp(buf, AppendedEncoding2, updatedLen) == 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding2(inSuite, reader);
}

void WriteFindAppendReadTest(nlTestSuite * inSuite, bool findContainer)
{
    uint8_t buf[74];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding2(inSuite, writer);

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

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding2));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding2, encodedLen) == 0);

    // Append new data into encoding
    FindAppendEncoding2(inSuite, buf, encodedLen, sizeof(buf), updatedLen, findContainer);

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

    NL_TEST_ASSERT(inSuite, updatedLen == sizeof(AppendedEncoding2));
    NL_TEST_ASSERT(inSuite, memcmp(buf, AppendedEncoding2, updatedLen) == 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding2(inSuite, reader);
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

void WriteAppendReadTest1(nlTestSuite * inSuite)
{
    uint8_t buf[14];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding3(inSuite, writer);

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

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding3));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding3, encodedLen) == 0);

    // Append new data into encoding
    AppendEncoding3(inSuite, buf, encodedLen, sizeof(buf), updatedLen);

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

    NL_TEST_ASSERT(inSuite, updatedLen == sizeof(AppendedEncoding3));
    NL_TEST_ASSERT(inSuite, memcmp(buf, AppendedEncoding3, updatedLen) == 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding3(inSuite, reader);
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

void AppendReadTest(nlTestSuite * inSuite)
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
    AppendEncoding4(inSuite, buf, 0, sizeof(buf), updatedLen);

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

    NL_TEST_ASSERT(inSuite, updatedLen == sizeof(AppendedEncoding4));
    NL_TEST_ASSERT(inSuite, memcmp(buf, AppendedEncoding4, updatedLen) == 0);

    TLVReader reader;
    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadAppendedEncoding4(inSuite, reader);
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

void WriteDeleteReadTest(nlTestSuite * inSuite)
{
    uint8_t buf[74];
    uint32_t updatedLen;

    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding5(inSuite, writer);

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

    NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding5));
    NL_TEST_ASSERT(inSuite, memcmp(buf, Encoding5, encodedLen) == 0);

    // Delete some elements from the encoding
    DeleteEncoding5(inSuite, buf, encodedLen, sizeof(buf), updatedLen);

    NL_TEST_ASSERT(inSuite, updatedLen == sizeof(DeletedEncoding5));
    NL_TEST_ASSERT(inSuite, memcmp(buf, DeletedEncoding5, updatedLen) == 0);

    reader.Init(buf, updatedLen);
    reader.ImplicitProfileId = TestProfile_2;

    ReadDeletedEncoding5(inSuite, reader);
}

/**
 *  Test Packet Buffer
 */
void CheckPacketBuffer(nlTestSuite * inSuite, void * inContext)
{
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(sizeof(Encoding1), 0);
    System::PacketBufferTLVWriter writer;
    System::PacketBufferTLVReader reader;

    writer.Init(buf.Retain());
    writer.ImplicitProfileId = TestProfile_2;

    WriteEncoding1(inSuite, writer);

    TestBufferContents(inSuite, buf, Encoding1, sizeof(Encoding1));

    reader.Init(buf.Retain());
    reader.ImplicitProfileId = TestProfile_2;

    ReadEncoding1(inSuite, reader);

    reader.Init(buf.Retain(), buf->MaxDataLength());
    reader.ImplicitProfileId = TestProfile_2;

    ReadEncoding1(inSuite, reader);
}

CHIP_ERROR CountEvictedMembers(CHIPCircularTLVBuffer & inBuffer, void * inAppData, TLVReader & inReader)
{
    TestTLVContext * context = static_cast<TestTLVContext *>(inAppData);
    CHIP_ERROR err;

    // "Process" the first element in the reader
    err = inReader.Next();
    NL_TEST_ASSERT(context->mSuite, err == CHIP_NO_ERROR);

    err = inReader.Skip();
    NL_TEST_ASSERT(context->mSuite, err == CHIP_NO_ERROR);

    context->mEvictionCount++;
    context->mEvictedBytes += inReader.GetLengthRead();

    return CHIP_NO_ERROR;
}

void CheckCircularTLVBufferSimple(nlTestSuite * inSuite, void * inContext)
{
    // Write 40 bytes as 4 separate events into a 30 byte buffer.  On
    // completion of the test, the buffer should contain 2 elements
    // and 2 elements should have been evicted in the last call to
    // WriteEncoding.

    uint8_t backingStore[30];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TestTLVContext * context = static_cast<TestTLVContext *>(inContext);
    CHIPCircularTLVBuffer buffer(backingStore, 30);
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = inContext;

    writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    NL_TEST_ASSERT(inSuite, context->mEvictionCount == 2);
    NL_TEST_ASSERT(inSuite, context->mEvictedBytes == 18);
    NL_TEST_ASSERT(inSuite, buffer.DataLength() == 22);
    NL_TEST_ASSERT(inSuite, (buffer.DataLength() + context->mEvictedBytes) == writer.GetLengthWritten());

    // At this point the buffer should contain 2 instances of Encoding3.
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);

    ReadEncoding3(inSuite, reader);

    TestNext<TLVReader>(inSuite, reader);

    ReadEncoding3(inSuite, reader);

    // Check that the reader is out of data
    TestEnd<TLVReader>(inSuite, reader);
}

void CheckCircularTLVBufferStartMidway(nlTestSuite * inSuite, void * inContext)
{
    // Write 40 bytes as 4 separate events into a 30 byte buffer.  On
    // completion of the test, the buffer should contain 2 elements
    // and 2 elements should have been evicted in the last call to
    // WriteEncoding.

    uint8_t backingStore[30];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TestTLVContext * context = static_cast<TestTLVContext *>(inContext);
    CHIPCircularTLVBuffer buffer(backingStore, 30, &(backingStore[15]));
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = inContext;

    writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    NL_TEST_ASSERT(inSuite, context->mEvictionCount == 2);
    NL_TEST_ASSERT(inSuite, context->mEvictedBytes == 18);
    NL_TEST_ASSERT(inSuite, buffer.DataLength() == 22);
    NL_TEST_ASSERT(inSuite, (buffer.DataLength() + context->mEvictedBytes) == writer.GetLengthWritten());

    // At this point the buffer should contain 2 instances of Encoding3.
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);

    ReadEncoding3(inSuite, reader);

    TestNext<TLVReader>(inSuite, reader);

    ReadEncoding3(inSuite, reader);

    // Check that the reader is out of data
    TestEnd<TLVReader>(inSuite, reader);
}

void CheckCircularTLVBufferEvictStraddlingEvent(nlTestSuite * inSuite, void * inContext)
{
    // Write 95 bytes to the buffer as 9 different TLV elements: 1
    // 7-byte element and 8 11-byte elements.
    // On completion of the test, the buffer should contain 2 elements
    // and 7 elements should have been evicted in the last call to
    // WriteEncoding.

    TestTLVContext * context = static_cast<TestTLVContext *>(inContext);
    uint8_t backingStore[30];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    CHIPCircularTLVBuffer buffer(backingStore, 30);
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = inContext;

    writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    // the write below will evict an element that straddles the buffer boundary.
    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    WriteEncoding3(inSuite, writer);

    NL_TEST_ASSERT(inSuite,
                   writer.GetLengthWritten() ==
                       (8 * 11 + 7)); // 8 writes of Encoding3 (11 bytes each) and 7 bytes for the initial boolean.
    NL_TEST_ASSERT(inSuite, buffer.DataLength() == 22);
    NL_TEST_ASSERT(inSuite, (buffer.DataLength() + context->mEvictedBytes) == writer.GetLengthWritten());
    NL_TEST_ASSERT(inSuite, context->mEvictionCount == 7);

    // At this point the buffer should contain 2 instances of Encoding3.
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);

    ReadEncoding3(inSuite, reader);

    TestNext<TLVReader>(inSuite, reader);

    ReadEncoding3(inSuite, reader);

    // Check that the reader is out of data
    TestEnd<TLVReader>(inSuite, reader);
}

void CheckCircularTLVBufferEdge(nlTestSuite * inSuite, void * inContext)
{
    TestTLVContext * context = static_cast<TestTLVContext *>(inContext);
    CHIP_ERROR err;
    uint8_t backingStore[7];
    uint8_t backingStore1[14];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    TLVWriter writer1;

    CHIPCircularTLVBuffer buffer(backingStore, sizeof(backingStore));
    CHIPCircularTLVBuffer buffer1(backingStore1, sizeof(backingStore1));
    writer.Init(buffer);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer.mProcessEvictedElement = CountEvictedMembers;
    buffer.mAppData               = inContext;

    // Test eviction for an element that fits in the underlying buffer exactly
    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // At this point the buffer should contain only the boolean we just wrote
    reader.Init(buffer);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);
    TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    // Check that the reader is out of data
    TestEnd<TLVReader>(inSuite, reader);

    // verify that an element larger than the underlying buffer fails out.
    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer1.PutBoolean(ProfileTag(TestProfile_2, 2), false);
    NL_TEST_ASSERT(inSuite, err == CHIP_END_OF_TLV);

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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer.Finalize();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        reader.Init(buffer1);
        reader.ImplicitProfileId = TestProfile_2;

        TestNext<TLVReader>(inSuite, reader);
        TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);
        TestEnd<TLVReader>(inSuite, reader);

        buffer1.EvictHead();

        reader.Init(buffer1);
        reader.ImplicitProfileId = TestProfile_2;
        TestEnd<TLVReader>(inSuite, reader);
    }

    writer.Init(buffer1);
    writer.ImplicitProfileId = TestProfile_2;

    context->mEvictionCount = 0;
    context->mEvictedBytes  = 0;

    buffer1.mProcessEvictedElement = CountEvictedMembers;
    buffer1.mAppData               = inContext;

    // Two elements fit in the buffer exactly
    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();

    // Verify that we can read out two elements from the buffer
    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);
    TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

    TestNext<TLVReader>(inSuite, reader);
    TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    TestEnd<TLVReader>(inSuite, reader);

    // Check that the eviction works as expected

    buffer1.EvictHead();

    // At this point the buffer should contain only the second boolean
    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);
    TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    // Check that the reader is out of data
    TestEnd<TLVReader>(inSuite, reader);

    // Write another boolean, verify that the buffer is full and contains two booleans

    writer.Init(buffer1);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Verify that we can read out two elements from the buffer
    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);
    TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), false);

    TestNext<TLVReader>(inSuite, reader);
    TestGet<TLVReader, bool>(inSuite, reader, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

    TestEnd<TLVReader>(inSuite, reader);

    // Evict the elements from the buffer, verfiy that we have an
    // empty reader on our hands

    buffer1.EvictHead();
    buffer1.EvictHead();

    reader.Init(buffer1);
    reader.ImplicitProfileId = TestProfile_2;

    TestEnd<TLVReader>(inSuite, reader);
}
void CheckCHIPTLVPutStringF(nlTestSuite * inSuite, void * inContext)
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
    snprintf(strBuffer, sizeof(strBuffer), "Sample string %zu", num);

    err = writer.PutStringF(ProfileTag(TestProfile_1, 1), "Sample string %zu", num);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    reader.Init(backingStore, writer.GetLengthWritten());
    err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = reader.GetString(valStr, 256);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strncmp(valStr, strBuffer, 256) == 0);
}

void CheckCHIPTLVPutStringFCircular(nlTestSuite * inSuite, void * inContext)
{
    const size_t bufsize = 40;
    char strBuffer[bufsize];
    char valStr[bufsize];
    uint8_t backingStore[bufsize];
    CircularTLVWriter writer;
    CircularTLVReader reader;
    CHIPCircularTLVBuffer buffer(backingStore, bufsize);
    size_t num     = 1;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initial test: Verify that a straight printf works as expected into continuous buffer.

    writer.Init(buffer);
    snprintf(strBuffer, sizeof(strBuffer), "Sample string %zu", num);

    err = writer.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.PutStringF(ProfileTag(TestProfile_1, 1), "Sample string %zu", num);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    reader.Init(buffer);

    // Skip over the initial element
    err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = reader.GetString(valStr, bufsize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strncmp(valStr, strBuffer, bufsize) == 0);

    // Verify that the PutStringF will handle correctly the case with the discontinuous buffer
    // This print will both stradle the boundary of the buffer and displace the previous two elements.
    num = 2;

    snprintf(strBuffer, sizeof(strBuffer), "Sample string %zu", num);

    err = writer.PutStringF(ProfileTag(TestProfile_1, 1), "Sample string %zu", num);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    reader.Init(buffer);
    err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = reader.GetString(valStr, bufsize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, strncmp(valStr, strBuffer, bufsize) == 0);
}

void CheckCHIPTLVByteSpan(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    reader.Init(backingStore, writer.GetLengthWritten());
    err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    chip::ByteSpan readerSpan;
    err = reader.Get(readerSpan);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, memcmp(readerSpan.data(), bytesBuffer, sizeof(bytesBuffer)) == 0);
}

void CheckCHIPTLVSkipCircular(nlTestSuite * inSuite, void * inContext)
{
    const size_t bufsize = 40; // large enough s.t. 2 elements fit, 3rd causes eviction
    uint8_t backingStore[bufsize];
    char testString[] = "Sample string"; // 13 characters, without the trailing NULL, add 3 bytes for anon tag
    // Any pair of reader and writer would work here, either PacketBuffer based or CircularTLV based.
    CircularTLVWriter writer;
    CircularTLVReader reader;
    CHIPCircularTLVBuffer buffer(backingStore, bufsize);
    CHIP_ERROR err = CHIP_NO_ERROR;

    writer.Init(buffer);

    err = writer.PutString(AnonymousTag, testString);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.PutString(AnonymousTag, testString);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.PutString(AnonymousTag, testString); // This event straddles the boundary
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.PutString(AnonymousTag, testString); // This one does not.
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    reader.Init(buffer);

    err = reader.Next(); // position the reader at the straddling element
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = reader.Skip(); // // Test that the buf ptr is handled correctly within the ReadData() function.
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *  Test Buffer Overflow
 */
void CheckBufferOverflow(nlTestSuite * inSuite, void * inContext)
{
    System::PacketBufferTLVWriter writer;
    System::PacketBufferTLVReader reader;

    System::PacketBufferHandle buf = System::PacketBufferHandle::New(sizeof(Encoding1), 0);
    uint16_t maxDataLen            = buf->MaxDataLength();
    uint16_t reserve = static_cast<uint16_t>((sizeof(Encoding1) < maxDataLen) ? (maxDataLen - sizeof(Encoding1)) + 2 : 0);

    // Repeatedly write and read a TLV encoding to a chain of PacketBuffers. Use progressively larger
    // and larger amounts of space in the first buffer to force the encoding/decoding to overlap the
    // end of the buffer and the beginning of the next.
    for (; reserve < maxDataLen; reserve++)
    {
        buf->SetStart(buf->Start() + reserve);

        writer.Init(buf.Retain(), /* useChainedBuffers = */ true);
        writer.ImplicitProfileId = TestProfile_2;

        WriteEncoding1(inSuite, writer);

        TestBufferContents(inSuite, buf, Encoding1, sizeof(Encoding1));

        reader.Init(buf.Retain(), /* useChainedBuffers = */ true);
        reader.ImplicitProfileId = TestProfile_2;

        ReadEncoding1(inSuite, reader);

        buf = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);
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

void CheckStrictAliasing(nlTestSuite * inSuite, void * inContext)
{
    const uint32_t kProfile_Id = 0x0000000e;
    CHIP_ERROR err             = CHIP_NO_ERROR;
    TLVReader reader;

    reader.Init(sIdentifyResponseBuf, kIdentifyResponseLen);
    reader.ImplicitProfileId = kProfile_Id;

    err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, reader.GetTag() == 0xe00000001);
}

/**
 *  Test CHIP TLV Writer Copy Container
 */
void TestCHIPTLVWriterCopyContainer(nlTestSuite * inSuite)
{
    uint8_t buf[2048];

    {
        TLVWriter writer;
        TLVReader reader;

        reader.Init(Encoding1);
        reader.ImplicitProfileId = TestProfile_2;

        TestNext<TLVReader>(inSuite, reader);

        writer.Init(buf);
        writer.ImplicitProfileId = TestProfile_2;

        CHIP_ERROR err = writer.CopyContainer(reader);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        uint32_t encodedLen = writer.GetLengthWritten();
        NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding1));

        int memcmpRes = memcmp(buf, Encoding1, encodedLen);
        NL_TEST_ASSERT(inSuite, memcmpRes == 0);
    }

    {
        TLVWriter writer;

        writer.Init(buf);
        writer.ImplicitProfileId = TestProfile_2;

        CHIP_ERROR err = writer.CopyContainer(ProfileTag(TestProfile_1, 1), Encoding1, sizeof(Encoding1));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        uint32_t encodedLen = writer.GetLengthWritten();
        NL_TEST_ASSERT(inSuite, encodedLen == sizeof(Encoding1));

        int memcmpRes = memcmp(buf, Encoding1, encodedLen);
        NL_TEST_ASSERT(inSuite, memcmpRes == 0);
    }
}

/**
 *  Test CHIP TLV Writer Copy Element
 */
void TestCHIPTLVWriterCopyElement(nlTestSuite * inSuite)
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

    err = writer.StartContainer(AnonymousTag, kTLVType_Structure, outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    for (int i = 0; i < kRepeatCount; i++)
    {
        WriteEncoding1(inSuite, writer);
    }

    err = writer.EndContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    expectedLen = writer.GetLengthWritten();

    writer.Init(testBuf);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.StartContainer(AnonymousTag, kTLVType_Structure, outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    for (int i = 0; i < kRepeatCount; i++)
    {
        TLVReader reader;

        reader.Init(Encoding1);
        reader.ImplicitProfileId = TestProfile_2;

        TestNext<TLVReader>(inSuite, reader);

        err = writer.CopyElement(reader);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.EndContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    testLen = writer.GetLengthWritten();

    NL_TEST_ASSERT(inSuite, testLen == expectedLen);

    int memcmpRes = memcmp(testBuf, expectedBuf, testLen);
    NL_TEST_ASSERT(inSuite, memcmpRes == 0);
}

void PreserveSizeWrite(nlTestSuite * inSuite, TLVWriter & writer, bool preserveSize)
{
    CHIP_ERROR err;
    TLVWriter writer2;

    // TLVTagControl::FullyQualified_8Bytes
    err = writer.Put(ProfileTag(TestProfile_1, 4000000000ULL), static_cast<int64_t>(40000000000ULL), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Put(ProfileTag(TestProfile_1, 4000000000ULL), static_cast<int16_t>(12345), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Put(ProfileTag(TestProfile_1, 4000000000ULL), static_cast<float>(1.0));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    {
        TLVWriter writer3;

        err = writer2.OpenContainer(ContextTag(0), kTLVType_Array, writer3);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<uint8_t>(42), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<uint16_t>(42), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<uint32_t>(42), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<uint64_t>(40000000000ULL), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int8_t>(-17), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int16_t>(-17), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int32_t>(-170000), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int64_t>(-170000), preserveSize);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // the below cases are for full coverage of PUTs
        err = writer3.Put(AnonymousTag, static_cast<uint64_t>(65535), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int64_t>(32767), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer3.Put(AnonymousTag, static_cast<int64_t>(40000000000ULL), false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.CloseContainer(writer3);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *  Test CHIP TLV Writer with Preserve Size
 */
void TestCHIPTLVWriterPreserveSize(nlTestSuite * inSuite)
{
    uint8_t buf[2048];
    TLVWriter writer;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    PreserveSizeWrite(inSuite, writer, true);

    uint32_t encodedLen = writer.GetLengthWritten();
    NL_TEST_ASSERT(inSuite, encodedLen == 105);
}

/**
 *  Test error handling of CHIP TLV Writer
 */
void TestCHIPTLVWriterErrorHandling(nlTestSuite * inSuite)
{
    CHIP_ERROR err;
    uint8_t buf[2048];
    TLVWriter writer, writer2, writer3;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    // OpenContainer() for non-container
    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Boolean, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // Since OpenContainer failed, writer2 remains uninitialized.
    writer2.Init(nullptr, 0);

    // CloseContainer() for non-container
    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // OpenContainer() failure
    err = writer.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.OpenContainer(ProfileTag(TestProfile_1, 1), kTLVType_Structure, writer3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // CloseContainer() failure
    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_TLV_CONTAINER_OPEN);

    // StartContainer()
    TLVType outerContainerType;
    err = writer.StartContainer(ProfileTag(TestProfile_2, 4000000000ULL), kTLVType_Boolean, outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // EndContainer()
    outerContainerType = kTLVType_Boolean;
    err                = writer.EndContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // PutPreEncodedContainer()
    TLVReader reader;
    reader.Init(buf, 2048);
    err = writer.PutPreEncodedContainer(ProfileTag(TestProfile_2, 4000000000ULL), kTLVType_Boolean, reader.GetReadPoint(),
                                        reader.GetRemainingLength());
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
}

/**
 *  Test CHIP TLV Writer
 */
void CheckCHIPTLVWriter(nlTestSuite * inSuite, void * inContext)
{
    TestCHIPTLVWriterCopyContainer(inSuite);

    TestCHIPTLVWriterCopyElement(inSuite);

    TestCHIPTLVWriterPreserveSize(inSuite);

    TestCHIPTLVWriterErrorHandling(inSuite);
}

void SkipNonContainer(nlTestSuite * inSuite)
{
    TLVReader reader;
    const uint8_t * readpoint1 = nullptr;
    const uint8_t * readpoint2 = nullptr;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestSkip(inSuite, reader);

    readpoint1 = reader.GetReadPoint();

    // Skip again, to check the operation is idempotent
    TestSkip(inSuite, reader);

    readpoint2 = reader.GetReadPoint();

    NL_TEST_ASSERT(inSuite, readpoint1 == readpoint2);
}

void SkipContainer(nlTestSuite * inSuite)
{
    TLVReader reader;
    const uint8_t * readpoint1 = nullptr;
    const uint8_t * readpoint2 = nullptr;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);

    TestSkip(inSuite, reader);

    readpoint1 = reader.GetReadPoint();

    // Skip again, to check the operation is idempotent
    TestSkip(inSuite, reader);

    readpoint2 = reader.GetReadPoint();

    NL_TEST_ASSERT(inSuite, readpoint1 == readpoint2);
}

void NextContainer(nlTestSuite * inSuite)
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);

    CHIP_ERROR err = reader.Next();
    NL_TEST_ASSERT(inSuite, err == CHIP_END_OF_TLV);
}

/**
 *  Test CHIP TLV Reader Skip functions
 */
void TestCHIPTLVReaderSkip(nlTestSuite * inSuite)
{
    SkipNonContainer(inSuite);

    SkipContainer(inSuite);

    NextContainer(inSuite);
}

/**
 *  Test CHIP TLV Reader Dup functions
 */
void TestCHIPTLVReaderDup(nlTestSuite * inSuite)
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    TestNext<TLVReader>(inSuite, reader);

    {
        TLVReader reader2;

        TestAndOpenContainer(inSuite, reader, kTLVType_Structure, ProfileTag(TestProfile_1, 1), reader2);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_1, 2), true);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, bool>(inSuite, reader2, kTLVType_Boolean, ProfileTag(TestProfile_2, 2), false);

        TestNext<TLVReader>(inSuite, reader2);

        {
            TLVReader reader3;

            TestAndOpenContainer(inSuite, reader2, kTLVType_Array, ContextTag(0), reader3);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int8_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, int16_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, int32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, uint32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);
            TestGet<TLVReader, uint64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, 42);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int8_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);
            TestGet<TLVReader, int16_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);
            TestGet<TLVReader, int32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);
            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -17);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int32_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -170000);
            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_SignedInteger, AnonymousTag, -170000);

            TestNext<TLVReader>(inSuite, reader3);

            TestGet<TLVReader, int64_t>(inSuite, reader3, kTLVType_UnsignedInteger, AnonymousTag, 40000000000ULL);
            TestGet<TLVReader, uint64_t>(inSuite, reader3, kTLVType_UnsignedInteger, AnonymousTag, 40000000000ULL);

            TestNext<TLVReader>(inSuite, reader3);

            {
                TLVReader reader4;

                TestAndOpenContainer(inSuite, reader3, kTLVType_Structure, AnonymousTag, reader4);

                TestEndAndCloseContainer(inSuite, reader3, reader4);
            }

            TestNext<TLVReader>(inSuite, reader3);

            {
                TLVReader reader5;

                TestAndOpenContainer(inSuite, reader3, kTLVType_List, AnonymousTag, reader5);

                TestNext<TLVReader>(inSuite, reader5);

                TestNull(inSuite, reader5, ProfileTag(TestProfile_1, 17));

                TestNext<TLVReader>(inSuite, reader5);

                TestNull(inSuite, reader5, ProfileTag(TestProfile_2, 900000));

                TestNext<TLVReader>(inSuite, reader5);

                TestNull(inSuite, reader5, AnonymousTag);

                TestNext<TLVReader>(inSuite, reader5);

                {
                    TLVType outerContainerType;

                    TestAndEnterContainer<TLVReader>(inSuite, reader5, kTLVType_Structure, ProfileTag(TestProfile_2, 4000000000ULL),
                                                     outerContainerType);

                    TestNext<TLVReader>(inSuite, reader5);

                    TestDupString(inSuite, reader5, CommonTag(70000), sLargeString);

                    TestEndAndExitContainer<TLVReader>(inSuite, reader5, outerContainerType);
                }

                TestEndAndCloseContainer(inSuite, reader3, reader5);
            }

            TestEndAndCloseContainer(inSuite, reader2, reader3);
        }

        TestNext<TLVReader>(inSuite, reader2);

        TestDupBytes(inSuite, reader2, ProfileTag(TestProfile_1, 5), reinterpret_cast<const uint8_t *>("This is a test"), 14);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, float>(inSuite, reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), 17.9f);
        TestGet<TLVReader, double>(inSuite, reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65535), 17.9f);

        TestNext<TLVReader>(inSuite, reader2);

        TestGet<TLVReader, double>(inSuite, reader2, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_2, 65536), 17.9);

        TestEndAndCloseContainer(inSuite, reader, reader2);
    }

    TestEnd<TLVReader>(inSuite, reader);
}
/**
 *  Test error handling of CHIP TLV Reader
 */
void TestCHIPTLVReaderErrorHandling(nlTestSuite * inSuite)
{
    CHIP_ERROR err;
    uint8_t buf[2048];
    TLVReader reader;

    reader.Init(buf);
    reader.ImplicitProfileId = TestProfile_2;

    // Get(bool&)
    bool val;
    err = reader.Get(val);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // Get(float&)
    float numF;
    err = reader.Get(numF);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // Get(double&)
    double numD;
    err = reader.Get(numD);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // Get(uint64_t&)
    uint64_t num;
    err = reader.Get(num);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // GetBytes()
    uint8_t bBuf[16];
    err = reader.GetBytes(bBuf, sizeof(bBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // GetString()
    char sBuf[16];
    err = reader.GetString(sBuf, sizeof(sBuf));
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);

    // OpenContainer()
    TLVReader reader2;
    err = reader.OpenContainer(reader2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // CloseContainer()
    err = reader.CloseContainer(reader2);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // EnterContainer()
    TLVType outerContainerType = kTLVType_Boolean;
    err                        = reader.EnterContainer(outerContainerType);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INCORRECT_STATE);

    // DupString()
    char * str = static_cast<char *>(chip::Platform::MemoryAlloc(16));
    err        = reader.DupString(str);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);
    chip::Platform::MemoryFree(str);

    // GetDataPtr()
    const uint8_t * data = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(16));
    err                  = reader.GetDataPtr(data);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);
    chip::Platform::MemoryFree(const_cast<uint8_t *>(data));
}

/**
 *  Test that CHIP TLV reader returns an error when a read is requested that
 *  would truncate the output.
 */
void TestCHIPTLVReaderTruncatedReads(nlTestSuite * inSuite)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;

    CHIP_ERROR err;
    float outF;

    // Setup: Write some values into the buffer
    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    err = writer.Put(AnonymousTag, double{ 12.5 });
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Test reading values from the buffer
    reader.Init(buf);

    TestNext<TLVReader>(inSuite, reader);

    TestGet<TLVReader, double>(inSuite, reader, kTLVType_FloatingPointNumber, AnonymousTag, 12.5);

    err = reader.Get(outF);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_WRONG_TLV_TYPE);
}

/**
 *  Test CHIP TLV Reader in a use case
 */
void TestCHIPTLVReaderInPractice(nlTestSuite * inSuite)
{
    uint8_t buf[2048];
    TLVWriter writer;
    TLVReader reader;

    writer.Init(buf);
    writer.ImplicitProfileId = TestProfile_2;

    PreserveSizeWrite(inSuite, writer, true);

    reader.Init(buf);

    TestNext<TLVReader>(inSuite, reader);

    TestGet<TLVReader, int64_t>(inSuite, reader, kTLVType_SignedInteger, ProfileTag(TestProfile_1, 4000000000ULL),
                                static_cast<int64_t>(40000000000ULL));

    TestNext<TLVReader>(inSuite, reader);

    TestGet<TLVReader, int64_t>(inSuite, reader, kTLVType_SignedInteger, ProfileTag(TestProfile_1, 4000000000ULL),
                                static_cast<int16_t>(12345));

    TestNext<TLVReader>(inSuite, reader);

    TestGet<TLVReader, float>(inSuite, reader, kTLVType_FloatingPointNumber, ProfileTag(TestProfile_1, 4000000000ULL),
                              static_cast<float>(1.0));
}

void TestCHIPTLVReader_NextOverContainer_ProcessElement(nlTestSuite * inSuite, TLVReader & reader, void * context)
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        ForEachElement(inSuite, readerClone1, nullptr, nullptr);
        err = readerClone1.ExitContainer(outerContainerType);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        nextRes1 = readerClone1.Next();
        NL_TEST_ASSERT(inSuite, nextRes1 == CHIP_NO_ERROR || nextRes1 == CHIP_END_OF_TLV);

        // For the other reader, skip over the entire container using the Next() method.
        nextRes2 = readerClone2.Next();
        NL_TEST_ASSERT(inSuite, nextRes2 == CHIP_NO_ERROR || nextRes2 == CHIP_END_OF_TLV);

        // Verify the two readers end up in the same state/position.
        NL_TEST_ASSERT(inSuite, nextRes1 == nextRes2);
        NL_TEST_ASSERT(inSuite, readerClone1.GetType() == readerClone2.GetType());
        NL_TEST_ASSERT(inSuite, readerClone1.GetReadPoint() == readerClone2.GetReadPoint());
    }
}

/**
 * Test using CHIP TLV Reader Next() method to skip over containers.
 */
void TestCHIPTLVReader_NextOverContainer(nlTestSuite * inSuite)
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    ForEachElement(inSuite, reader, nullptr, TestCHIPTLVReader_NextOverContainer_ProcessElement);
}

void TestCHIPTLVReader_SkipOverContainer_ProcessElement(nlTestSuite * inSuite, TLVReader & reader, void * context)
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
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        ForEachElement(inSuite, readerClone1, nullptr, nullptr);
        err = readerClone1.ExitContainer(outerContainerType);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // For the other reader, skip over the entire container using the Skip() method.
        err = readerClone2.Skip();
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // Verify the two readers end up in the same state/position.
        NL_TEST_ASSERT(inSuite, readerClone1.GetType() == readerClone2.GetType());
        NL_TEST_ASSERT(inSuite, readerClone1.GetReadPoint() == readerClone2.GetReadPoint());
    }
}

/**
 * Test using CHIP TLV Reader Skip() method to skip over containers.
 */
void TestCHIPTLVReader_SkipOverContainer(nlTestSuite * inSuite)
{
    TLVReader reader;

    reader.Init(Encoding1);
    reader.ImplicitProfileId = TestProfile_2;

    ForEachElement(inSuite, reader, nullptr, TestCHIPTLVReader_SkipOverContainer_ProcessElement);
}

/**
 *  Test CHIP TLV Reader
 */
void CheckCHIPTLVReader(nlTestSuite * inSuite, void * inContext)
{
    TestCHIPTLVReaderSkip(inSuite);

    TestCHIPTLVReaderDup(inSuite);

    TestCHIPTLVReaderErrorHandling(inSuite);

    TestCHIPTLVReaderTruncatedReads(inSuite);

    TestCHIPTLVReaderInPractice(inSuite);

    TestCHIPTLVReader_NextOverContainer(inSuite);

    TestCHIPTLVReader_SkipOverContainer(inSuite);
}

/**
 *  Test CHIP TLV Items
 */
static void TestItems(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t sBuffer[256];

    TLVWriter writer;
    writer.Init(sBuffer);

    TLVWriter writer2;
    err = writer.OpenContainer(AnonymousTag, kTLVType_Array, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    {
        err = writer2.PutBoolean(AnonymousTag, true);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int8_t>(-1));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int16_t>(-2));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int32_t>(-3));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int64_t>(-4));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<float>(-5.5));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<double>(-3.14159265358979323846));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.OpenContainer(AnonymousTag, kTLVType_Array, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    {
        err = writer2.PutBoolean(AnonymousTag, false);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int8_t>(1));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int16_t>(2));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int32_t>(3));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<int64_t>(4));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<uint8_t>(5));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<uint16_t>(6));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<uint32_t>(7));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<uint64_t>(8));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<float>(9.9));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        err = writer2.Put(AnonymousTag, static_cast<double>(3.14159265358979323846));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    }

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *  Test CHIP TLV Containers
 */
static void TestContainers(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVWriter writer;

    uint8_t sBuffer[256];
    writer.Init(sBuffer);

    TLVWriter writer2;
    err = writer.OpenContainer(AnonymousTag, kTLVType_Array, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    TLVType type = writer2.GetContainerType();
    NL_TEST_ASSERT(inSuite, type == kTLVType_Array);

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.OpenContainer(AnonymousTag, kTLVType_Structure, writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    type = writer2.GetContainerType();
    NL_TEST_ASSERT(inSuite, type == kTLVType_Structure);

    err = writer.CloseContainer(writer2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer.Finalize();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

/**
 *  Test CHIP TLV Basics
 */
static void CheckCHIPTLVBasics(nlTestSuite * inSuite, void * inContext)
{
    TestItems(inSuite, inContext);
    TestContainers(inSuite, inContext);
}

/**
 *  Test CHIP TLV Updater
 */
static void CheckCHIPUpdater(nlTestSuite * inSuite, void * inContext)
{
    WriteAppendReadTest0(inSuite);

    WriteAppendReadTest1(inSuite);

    WriteFindAppendReadTest(inSuite, false); // Find an element

    WriteFindAppendReadTest(inSuite, true); // Find a container

    AppendReadTest(inSuite);

    WriteDeleteReadTest(inSuite);
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

static void CheckCloseContainerReserve(nlTestSuite * inSuite, void * inContext)
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

    err = writer1.OpenContainer(AnonymousTag, kTLVType_Array, innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = innerWriter1.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = innerWriter2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    err = innerWriter1.CloseContainer(innerWriter2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer1.CloseContainer(innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    writer2.Init(buf, sizeof(buf));

    err = writer2.OpenContainer(AnonymousTag, kTLVType_Array, innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = innerWriter1.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = innerWriter2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = innerWriter1.CloseContainer(innerWriter2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.CloseContainer(innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // test the same scheme works on the Start/End container

    writer1.Init(buf);

    err = writer1.StartContainer(AnonymousTag, kTLVType_Array, container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer1.StartContainer(AnonymousTag, kTLVType_Structure, container2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.EndContainer(container2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer1.EndContainer(container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    writer2.Init(buf, sizeof(buf));

    err = writer2.StartContainer(AnonymousTag, kTLVType_Array, container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.StartContainer(AnonymousTag, kTLVType_Structure, container2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.PutBoolean(ProfileTag(TestProfile_1, 2), true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.EndContainer(container2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.EndContainer(container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test that the reservations work for the empty containers

    writer1.Init(buf1);
    err = writer1.OpenContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.CloseContainer(innerWriter1);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    writer2.Init(buf1, sizeof(buf1));
    err = writer2.OpenContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.CloseContainer(innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    writer1.Init(buf1);

    err = writer1.StartContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.EndContainer(container1);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    writer2.Init(buf1, sizeof(buf1));

    err = writer2.StartContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = writer2.EndContainer(container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test that the reservations work if the writer has a maxLen of 0.

    writer1.Init(buf1, 0);

    err = writer1.OpenContainer(ProfileTag(TestProfile_1, 2), kTLVType_Structure, innerWriter1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    err = writer1.StartContainer(AnonymousTag, kTLVType_Array, container1);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

    // Test again all cases from 0 to the length of buf1

    for (uint32_t maxLen = 0; maxLen <= sizeof(buf); maxLen++)
    {
        // Open/CloseContainer

        writer1.Init(buf, maxLen);

        err = writer1.OpenContainer(AnonymousTag, kTLVType_Array, innerWriter1);

        if (err == CHIP_NO_ERROR)
            err = innerWriter1.OpenContainer(AnonymousTag, kTLVType_Structure, innerWriter2);

        if (err == CHIP_NO_ERROR)
            err = innerWriter2.PutBoolean(ProfileTag(TestProfile_1, 2), true);

        if (err == CHIP_NO_ERROR)
            err = innerWriter1.CloseContainer(innerWriter2);

        if (err == CHIP_NO_ERROR)
            err = writer1.CloseContainer(innerWriter1);

        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);

        // Start/EndContainer

        writer1.Init(buf, maxLen);

        if (err == CHIP_NO_ERROR)
            err = writer1.StartContainer(AnonymousTag, kTLVType_Array, container1);

        if (err == CHIP_NO_ERROR)
            err = writer1.StartContainer(AnonymousTag, kTLVType_Structure, container2);

        if (err == CHIP_NO_ERROR)
            err = writer1.PutBoolean(ProfileTag(TestProfile_1, 2), true);

        if (err == CHIP_NO_ERROR)
            err = writer1.EndContainer(container2);

        if (err == CHIP_NO_ERROR)
            err = writer1.EndContainer(container1);

        NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
    }
}

static CHIP_ERROR ReadFuzzedEncoding1(nlTestSuite * inSuite, TLVReader & reader)
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
            ReturnErrorOnFailure(reader.Next(kTLVType_SignedInteger, AnonymousTag));

            FUZZ_CHECK_VAL(int8_t, 42);
            FUZZ_CHECK_VAL(int16_t, 42);
            FUZZ_CHECK_VAL(int32_t, 42);
            FUZZ_CHECK_VAL(int64_t, 42);
            FUZZ_CHECK_VAL(uint8_t, 42);
            FUZZ_CHECK_VAL(uint16_t, 42);
            FUZZ_CHECK_VAL(uint32_t, 42);
            FUZZ_CHECK_VAL(uint64_t, 42);

            ReturnErrorOnFailure(reader.Next(kTLVType_SignedInteger, AnonymousTag));

            FUZZ_CHECK_VAL(int8_t, -17);
            FUZZ_CHECK_VAL(int16_t, -17);
            FUZZ_CHECK_VAL(int32_t, -17);
            FUZZ_CHECK_VAL(int64_t, -17);

            ReturnErrorOnFailure(reader.Next(kTLVType_SignedInteger, AnonymousTag));

            FUZZ_CHECK_VAL(int32_t, -170000);
            FUZZ_CHECK_VAL(int64_t, -170000);

            ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, AnonymousTag));

            FUZZ_CHECK_VAL(int64_t, 40000000000ULL);
            FUZZ_CHECK_VAL(uint64_t, 40000000000ULL);

            ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag));

            {
                TLVType outerContainer3Type;

                ReturnErrorOnFailure(reader.EnterContainer(outerContainer3Type));
                ReturnErrorOnFailure(reader.ExitContainer(outerContainer3Type));
            }

            ReturnErrorOnFailure(reader.Next(kTLVType_List, AnonymousTag));

            {
                TLVType outerContainer3Type;

                ReturnErrorOnFailure(reader.EnterContainer(outerContainer3Type));
                ReturnErrorOnFailure(reader.Next(kTLVType_Null, ProfileTag(TestProfile_1, 17)));
                ReturnErrorOnFailure(reader.Next(kTLVType_Null, ProfileTag(TestProfile_2, 900000)));
                ReturnErrorOnFailure(reader.Next(kTLVType_Null, AnonymousTag));
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

static void TLVReaderFuzzTest(nlTestSuite * inSuite, void * inContext)
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
                    fuzzMask = GetRandU8();

                fuzzedData[i] ^= fuzzMask;
            }

            TLVReader reader;
            reader.Init(fuzzedData);
            reader.ImplicitProfileId = TestProfile_2;

            CHIP_ERROR readRes = ReadFuzzedEncoding1(inSuite, reader);
            NL_TEST_ASSERT(inSuite, readRes != CHIP_NO_ERROR);

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

static void AssertCanReadString(nlTestSuite * inSuite, ContiguousBufferTLVReader & reader, const char * expectedString)
{
    Span<const char> str;
    CHIP_ERROR err = reader.GetStringView(str);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, str.size() == strlen(expectedString));
    NL_TEST_ASSERT(inSuite, strncmp(str.data(), expectedString, str.size()) == 0);
}

static void AssertCannotReadString(nlTestSuite * inSuite, ContiguousBufferTLVReader & reader)
{
    Span<const char> str;
    CHIP_ERROR err = reader.GetStringView(str);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

static void CheckGetStringView(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf[256];
    const char testString[] = "This is a test";
    {
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.PutString(CommonTag(0), testString);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // First check that basic read from entire buffer works.
        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCanReadString(inSuite, reader, testString);

        // Now check that read from a buffer bounded by the number of bytes
        // written works.
        reader.Init(buf, writer.GetLengthWritten());
        reader.Next();
        AssertCanReadString(inSuite, reader, testString);

        // Now check that read from a buffer bounded by fewer than the number of
        // bytes written fails.
        reader.Init(buf, writer.GetLengthWritten() - 1);
        reader.Next();
        AssertCannotReadString(inSuite, reader);
    }

    {
        // Check that an integer cannot be read as a string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.Put(CommonTag(0), static_cast<uint8_t>(5));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadString(inSuite, reader);
    }

    {
        // Check that an octet string cannot be read as a string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err =
            writer.PutBytes(CommonTag(0), reinterpret_cast<const uint8_t *>(testString), static_cast<uint32_t>(strlen(testString)));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadString(inSuite, reader);
    }

    {
        // Check that a manually constructed string can be read as a string.
        const uint8_t shortString[] = { CHIP_TLV_UTF8_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 2, 'a', 'b') };
        ContiguousBufferTLVReader reader;
        reader.Init(shortString);
        reader.Next();
        AssertCanReadString(inSuite, reader, "ab");
    }

    {
        // Check that a manually constructed string with bogus length cannot be read as a string.
        const uint8_t shortString[] = { CHIP_TLV_UTF8_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 3, 'a', 'b') };
        ContiguousBufferTLVReader reader;
        reader.Init(shortString);
        reader.Next();
        AssertCannotReadString(inSuite, reader);
    }
}

static void AssertCanReadBytes(nlTestSuite * inSuite, ContiguousBufferTLVReader & reader, const ByteSpan & expectedBytes)
{
    ByteSpan bytes;
    CHIP_ERROR err = reader.GetByteView(bytes);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, bytes.data_equal(expectedBytes));
}

static void AssertCannotReadBytes(nlTestSuite * inSuite, ContiguousBufferTLVReader & reader)
{
    ByteSpan bytes;
    CHIP_ERROR err = reader.GetByteView(bytes);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

static void CheckGetByteView(nlTestSuite * inSuite, void * inContext)
{
    uint8_t buf[256];
    const uint8_t testBytes[] = { 'T', 'h', 'i', 's', 'i', 's', 'a', 't', 'e', 's', 't', '\0' };
    {
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.PutBytes(CommonTag(0), testBytes, sizeof(testBytes));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        // First check that basic read from entire buffer works.
        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCanReadBytes(inSuite, reader, ByteSpan(testBytes));

        // Now check that read from a buffer bounded by the number of bytes
        // written works.
        reader.Init(buf, writer.GetLengthWritten());
        reader.Next();
        AssertCanReadBytes(inSuite, reader, ByteSpan(testBytes));

        // Now check that read from a buffer bounded by fewer than the number of
        // bytes written fails.
        reader.Init(buf, writer.GetLengthWritten() - 1);
        reader.Next();
        AssertCannotReadBytes(inSuite, reader);
    }

    {
        // Check that an integer cannot be read as an octet string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.Put(CommonTag(0), static_cast<uint8_t>(5));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadBytes(inSuite, reader);
    }

    {
        // Check that an string cannot be read as an octet string.
        TLVWriter writer;
        writer.Init(buf);
        CHIP_ERROR err = writer.PutString(CommonTag(0), reinterpret_cast<const char *>(testBytes));
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        ContiguousBufferTLVReader reader;
        reader.Init(buf);
        reader.Next();
        AssertCannotReadBytes(inSuite, reader);
    }

    {
        // Check that a manually constructed octet string can be read as octet string.
        const uint8_t shortBytes[] = { CHIP_TLV_BYTE_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 2, 1, 2) };
        ContiguousBufferTLVReader reader;
        reader.Init(shortBytes);
        reader.Next();
        const uint8_t expectedBytes[] = { 1, 2 };
        AssertCanReadBytes(inSuite, reader, ByteSpan(expectedBytes));
    }

    {
        // Check that a manually constructed octet string with bogus length
        // cannot be read as an octet string.
        const uint8_t shortBytes[] = { CHIP_TLV_BYTE_STRING_2ByteLength(CHIP_TLV_TAG_COMMON_PROFILE_2Bytes(0), 3, 1, 2) };
        ContiguousBufferTLVReader reader;
        reader.Init(shortBytes);
        reader.Next();
        AssertCannotReadBytes(inSuite, reader);
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Simple Write Read Test",              CheckSimpleWriteRead),
    NL_TEST_DEF("Inet Buffer Test",                    CheckPacketBuffer),
    NL_TEST_DEF("Buffer Overflow Test",                CheckBufferOverflow),
    NL_TEST_DEF("Pretty Print Test",                   CheckPrettyPrinter),
    NL_TEST_DEF("Data Macro Test",                     CheckDataMacro),
    NL_TEST_DEF("Strict Aliasing Test",                CheckStrictAliasing),
    NL_TEST_DEF("CHIP TLV Basics",                     CheckCHIPTLVBasics),
    NL_TEST_DEF("CHIP TLV Writer",                     CheckCHIPTLVWriter),
    NL_TEST_DEF("CHIP TLV Reader",                     CheckCHIPTLVReader),
    NL_TEST_DEF("CHIP TLV Utilities",                  CheckCHIPTLVUtilities),
    NL_TEST_DEF("CHIP TLV Updater",                    CheckCHIPUpdater),
    NL_TEST_DEF("CHIP TLV Empty Find",                 CheckCHIPTLVEmptyFind),
    NL_TEST_DEF("CHIP Circular TLV buffer, simple",    CheckCircularTLVBufferSimple),
    NL_TEST_DEF("CHIP Circular TLV buffer, mid-buffer start", CheckCircularTLVBufferStartMidway),
    NL_TEST_DEF("CHIP Circular TLV buffer, straddle",  CheckCircularTLVBufferEvictStraddlingEvent),
    NL_TEST_DEF("CHIP Circular TLV buffer, edge",      CheckCircularTLVBufferEdge),
    NL_TEST_DEF("CHIP TLV Printf",                     CheckCHIPTLVPutStringF),
    NL_TEST_DEF("CHIP TLV Printf, Circular TLV buf",   CheckCHIPTLVPutStringFCircular),
    NL_TEST_DEF("CHIP TLV Skip non-contiguous",        CheckCHIPTLVSkipCircular),
    NL_TEST_DEF("CHIP TLV ByteSpan",                   CheckCHIPTLVByteSpan),
    NL_TEST_DEF("CHIP TLV Check reserve",              CheckCloseContainerReserve),
    NL_TEST_DEF("CHIP TLV Reader Fuzz Test",           TLVReaderFuzzTest),
    NL_TEST_DEF("CHIP TLV GetStringView Test",         CheckGetStringView),
    NL_TEST_DEF("CHIP TLV GetByteView Test",           CheckGetByteView),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
int TestCHIPTLV_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestCHIPTLV_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestCHIPTLV(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-tlv",
        &sTests[0],
        TestCHIPTLV_Setup,
        TestCHIPTLV_Teardown
    };
    // clang-format on
    TestTLVContext context;

    context.mSuite = &theSuite;

    // Run test suit against one context
    nlTestRunner(&theSuite, &context);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestCHIPTLV)
