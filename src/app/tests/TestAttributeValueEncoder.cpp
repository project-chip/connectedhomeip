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
 *      This file implements unit tests for CommandPathParams
 *
 */

#include <app/AttributeAccessInterface.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;

namespace {

struct TestSetup
{
    TestSetup(nlTestSuite * aSuite) : encoder(&writer, 0)
    {
        writer.Init(buf);
        TLVType ignored;
        CHIP_ERROR err = writer.StartContainer(AnonymousTag, kTLVType_Structure, ignored);
        NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    }

    AttributeValueEncoder encoder;
    uint8_t buf[1024];
    TLVWriter writer;
};

// Macro so we get better error reporting in terms of which test failed, because
// the reporting uses __LINE__.
#define VERIFY_BUFFER_STATE(aSuite, aSetup, aExpected)                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        NL_TEST_ASSERT(aSuite, aSetup.writer.GetLengthWritten() == sizeof(aExpected));                                             \
        NL_TEST_ASSERT(aSuite, memcmp(aSetup.buf, aExpected, sizeof(aExpected)) == 0);                                             \
    } while (0)

void TestEncodeNothing(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    // Just have an anonymous struct marker.
    const uint8_t expected[] = { 0x15 };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeBool(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    CHIP_ERROR err = test.encoder.Encode(true);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    // Anonymous tagged struct.
    // Control tag for boolean true with context tag.
    // Context tag with value AttributeDataIB::Tag::kData.
    const uint8_t expected[] = { 0x15, 0x29, 0x02 };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeListOfBools1(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    bool list[]    = { true, false };
    CHIP_ERROR err = test.encoder.Encode(DataModel::List<bool>(list));
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    // Anonymous tagged struct.
    // Control tag for array with context tag.
    // Context tag with value AttributeDataIB::Tag::kData.
    // Control tag for boolean true with anonymous tag.
    // Control tag for boolean false with anonymous tag.
    // End of list marker.
    const uint8_t expected[] = { 0x15, 0x36, 0x02, 0x09, 0x08, 0x18 };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeListOfBools2(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    bool list[]    = { true, false };
    CHIP_ERROR err = test.encoder.EncodeList([&list](const TagBoundEncoder & encoder) -> CHIP_ERROR {
        for (auto & item : list)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    // Anonymous tagged struct.
    // Control tag for array with context tag.
    // Context tag with value AttributeDataIB::Tag::kData.
    // Control tag for boolean true with anonymous tag.
    // Control tag for boolean false with anonymous tag.
    // End of list marker.
    const uint8_t expected[] = { 0x15, 0x36, 0x02, 0x09, 0x08, 0x18 };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

#undef VERIFY_STATE

} // anonymous namespace

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestEncodeNothing", TestEncodeNothing), NL_TEST_DEF("TestEncodeBool", TestEncodeBool),
                          NL_TEST_DEF("TestEncodeListOfBools1", TestEncodeListOfBools1),
                          NL_TEST_DEF("TestEncodeListOfBools2", TestEncodeListOfBools2), NL_TEST_SENTINEL() };
}

int TestAttributeValueEncoder()
{
    nlTestSuite theSuite = { "AttributeValueEncoder", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributeValueEncoder)
