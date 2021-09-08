/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for the TLV pretty printer to ensure conformance
 *      against the Text Representation Format specified in Appendix C of the Matter
 *      specification.
 *
 */

#include <assert.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/CHIPTLVData.hpp>
#include <lib/core/CHIPTLVText.hpp>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace chip {

class TestTLVText
{
public:
    static void TestTLVText_BasicNumber(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicBool(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicUTF8String(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicByteString(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicFloat(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicNull(nlTestSuite * apSuite, void * apContext);

    static void TestTLVText_BasicStruct(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicArray(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_BasicList(nlTestSuite * apSuite, void * apContext);

    static void TestTLVText_NestedStruct(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_ArrayOfStructs(nlTestSuite * apSuite, void * apContext);
    static void TestTLVText_ArrayOfArrays(nlTestSuite * apSuite, void * apContext);

    static void TestTLVText_ProfileSpecificTags(nlTestSuite * apSuite, void * apContext);

    bool IsMatch(chip::Span<uint8_t> tlvData, const char * compareStr);

    char mPrintBuf[1024];
    char * mPrintPtr;
    uint32_t mWrittenLen;

private:
    void SetupBuf();

    chip::System::TLVPacketBufferBackingStore mStore;
    chip::TLV::TLVWriter mWriter;
    nlTestSuite * mpSuite;
};

using namespace chip::TLV;

TestTLVText gTestTLVText;

void PrintToBuffer(const char * fmt, ...)
{
    int len;
    va_list args;

    va_start(args, fmt);

    len = vsprintf(gTestTLVText.mPrintBuf + gTestTLVText.mWrittenLen, fmt, args);
    assert(len >= 0);

    gTestTLVText.mWrittenLen += (uint32_t) len;
    va_end(args);
}

bool TestTLVText::IsMatch(chip::Span<uint8_t> tlvData, const char * compareStr)
{
    CHIP_ERROR err;

    mPrintPtr   = mPrintBuf;
    mWrittenLen = 0;

    TLV::TLVReader reader;
    reader.Init(tlvData);

    err = Debug::Print(reader, PrintToBuffer);
    assert(err == CHIP_NO_ERROR);

    printf("%s", mPrintBuf);

    int r = strcmp(mPrintBuf, compareStr);
    return ((r == 0) ? true : false);
}

void TestTLVText::TestTLVText_BasicNumber(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    {
        uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS), CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                             CHIP_TLV_END_OF_CONTAINER };

        bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                    "{\n"
                                    "    1 = 300\n"
                                    "}\n");

        NL_TEST_ASSERT(apSuite, match);
    }

    {
        uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS), CHIP_TLV_INT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), -300),
                             CHIP_TLV_END_OF_CONTAINER };

        bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                    "{\n"
                                    "    1 = -300\n"
                                    "}\n");

        NL_TEST_ASSERT(apSuite, match);
    }
}

void TestTLVText::TestTLVText_BasicBool(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    {
        uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS), CHIP_TLV_BOOL(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), true),
                             CHIP_TLV_END_OF_CONTAINER };

        bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                    "{\n"
                                    "    1 = true\n"
                                    "}\n");

        NL_TEST_ASSERT(apSuite, match);
    }

    {
        uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS), CHIP_TLV_BOOL(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), false),
                             CHIP_TLV_END_OF_CONTAINER };

        bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                    "{\n"
                                    "    1 = false\n"
                                    "}\n");

        NL_TEST_ASSERT(apSuite, match);
    }
}

void TestTLVText::TestTLVText_BasicUTF8String(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UTF8_STRING_1ByteLength(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 5, 'h', 'e', 'l', 'l', 'o'),
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "{\n"
                                "    1 = \"hello\"\n"
                                "}\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_BasicByteString(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_BYTE_STRING_1ByteLength(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 5, 0x00, 0x01, 0x02, 0x03, 0x04),
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "{\n"
                                "    1 = 0 1 2 3 4\n"
                                "}\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_BasicFloat(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_FLOAT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 0x33, 0x33, 0x8f, 0x41), // 17.9f
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "{\n"
                                "    1 = 17.9000\n"
                                "}\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_BasicNull(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS), CHIP_TLV_NULL(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1)),
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "{\n"
                                "    1 = null\n"
                                "}\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_BasicStruct(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS), CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UTF8_STRING_1ByteLength(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 5, 'h', 'e', 'l', 'l', 'o'),
                         CHIP_TLV_BOOL(CHIP_TLV_TAG_CONTEXT_SPECIFIC(3), true), CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "{\n"
                                "    1 = 300,\n"
                                "    2 = \"hello\",\n"
                                "    3 = true\n"
                                "}\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_BasicArray(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_ARRAY(CHIP_TLV_TAG_ANONYMOUS),       CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 400), CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 500),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 600), CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "[\n"
                                "    300,\n"
                                "    400,\n"
                                "    500,\n"
                                "    600\n"
                                "]\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_BasicList(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_LIST(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 400),
                         CHIP_TLV_BOOL(CHIP_TLV_TAG_CONTEXT_SPECIFIC(3), true),
                         CHIP_TLV_BOOL(CHIP_TLV_TAG_CONTEXT_SPECIFIC(3), false),
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "[[\n"
                                "    1 = 300,\n"
                                "    2 = 400,\n"
                                "    3 = true,\n"
                                "    3 = false\n"
                                "]]\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_NestedStruct(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 400),
                         CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_CONTEXT_SPECIFIC(3)),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "{\n"
                                "    1 = 300,\n"
                                "    2 = 400,\n"
                                "    3 = \n"
                                "    {\n"
                                "        1 = 300,\n"
                                "        2 = 400\n"
                                "    }\n"
                                "}\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_ArrayOfStructs(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_ARRAY(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "[\n"
                                "    {\n"
                                "        1 = 300,\n"
                                "        2 = 400\n"
                                "    },\n"
                                "    {\n"
                                "        1 = 300,\n"
                                "        2 = 400\n"
                                "    },\n"
                                "    {\n"
                                "        1 = 300,\n"
                                "        2 = 400\n"
                                "    }\n"
                                "]\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_ArrayOfArrays(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    uint8_t Encode[] = { CHIP_TLV_ARRAY(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_ARRAY(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_ARRAY(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_ARRAY(CHIP_TLV_TAG_ANONYMOUS),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 300),
                         CHIP_TLV_UINT32(CHIP_TLV_TAG_ANONYMOUS, 400),
                         CHIP_TLV_END_OF_CONTAINER,
                         CHIP_TLV_END_OF_CONTAINER };

    bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                "[\n"
                                "    [\n"
                                "        300,\n"
                                "        400\n"
                                "    ],\n"
                                "    [\n"
                                "        300,\n"
                                "        400\n"
                                "    ],\n"
                                "    [\n"
                                "        300,\n"
                                "        400\n"
                                "    ]\n"
                                "]\n");

    NL_TEST_ASSERT(apSuite, match);
}

void TestTLVText::TestTLVText_ProfileSpecificTags(nlTestSuite * apSuite, void * apContext)
{
    chip::TestTLVText * _this = static_cast<chip::TestTLVText *>(apContext);
    _this->mpSuite            = apSuite;

    {
        uint8_t Encode[] = { CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                             CHIP_TLV_INT32(CHIP_TLV_TAG_FULLY_QUALIFIED_8Bytes(0xAAAABBBB, 0xCF), -300),
                             CHIP_TLV_END_OF_CONTAINER };

        bool match = _this->IsMatch({ Encode, sizeof(Encode) },
                                    "{\n"
                                    "    0xaaaa::0xbbbb:0xcf = -300\n"
                                    "}\n");

        NL_TEST_ASSERT(apSuite, match);
    }
}

void InitializeChip(nlTestSuite * apSuite)
{
    chip::Platform::MemoryInit();
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestTLVText_BasicNumber", chip::TestTLVText::TestTLVText_BasicNumber),
    NL_TEST_DEF("TestTLVText_BasicBool", chip::TestTLVText::TestTLVText_BasicBool),
    NL_TEST_DEF("TestTLVText_BasicUTF8String", chip::TestTLVText::TestTLVText_BasicUTF8String),
    NL_TEST_DEF("TestTLVText_BasicByteString", chip::TestTLVText::TestTLVText_BasicByteString),
    NL_TEST_DEF("TestTLVText_BasicFloat", chip::TestTLVText::TestTLVText_BasicFloat),
    NL_TEST_DEF("TestTLVText_BasicNull", chip::TestTLVText::TestTLVText_BasicNull),

    NL_TEST_DEF("TestTLVText_BasicStruct", chip::TestTLVText::TestTLVText_BasicStruct),
    NL_TEST_DEF("TestTLVText_BasicArray", chip::TestTLVText::TestTLVText_BasicArray),
    NL_TEST_DEF("TestTLVText_BasicList", chip::TestTLVText::TestTLVText_BasicList),

    NL_TEST_DEF("TestTLVText_NestedStruct", chip::TestTLVText::TestTLVText_NestedStruct),
    NL_TEST_DEF("TestTLVText_ArrayOfStructs", chip::TestTLVText::TestTLVText_ArrayOfStructs),
    NL_TEST_DEF("TestTLVText_ArrayOfArrays", chip::TestTLVText::TestTLVText_ArrayOfArrays),

    NL_TEST_DEF("TestTLVText_ProfileSpecificTags", chip::TestTLVText::TestTLVText_ProfileSpecificTags),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace chip

int TestClusterObjectUtils()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestClusterObjectUtils",
        &chip::sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    chip::InitializeChip(&theSuite);

    nlTestRunner(&theSuite, &chip::gTestTLVText);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestClusterObjectUtils)
