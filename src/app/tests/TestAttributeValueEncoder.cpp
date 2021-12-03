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

// These values are easier to be recognized in the encoded buffer
constexpr EndpointId kRandomEndpointId   = 0x55;
constexpr ClusterId kRandomClusterId     = 0xaa;
constexpr AttributeId kRandomAttributeId = 0xcc;
constexpr DataVersion kRandomDataVersion = 0x99;

template <size_t N>
struct LimitedTestSetup
{
    LimitedTestSetup(nlTestSuite * aSuite,
                     const AttributeValueEncoder::AttributeEncodeState & aState = AttributeValueEncoder::AttributeEncodeState()) :
        encoder(builder, 0, ConcreteAttributePath(kRandomEndpointId, kRandomClusterId, kRandomAttributeId), kRandomDataVersion,
                aState)
    {
        writer.Init(buf);
        {
            TLVType ignored;
            CHIP_ERROR err = writer.StartContainer(AnonymousTag, kTLVType_Structure, ignored);
            NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
        }
        {
            CHIP_ERROR err = builder.Init(&writer, 1);
            NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
        }
    }

    AttributeReportIBs::Builder builder;
    AttributeValueEncoder encoder;
    uint8_t buf[N];
    TLVWriter writer;
};

using TestSetup = LimitedTestSetup<1024>;

// Macro so we get better error reporting in terms of which test failed, because
// the reporting uses __LINE__.
#define VERIFY_BUFFER_STATE(aSuite, aSetup, aExpected)                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        NL_TEST_ASSERT(aSuite, aSetup.writer.GetLengthWritten() == sizeof(aExpected));                                             \
        NL_TEST_ASSERT(aSuite, memcmp(aSetup.buf, aExpected, sizeof(aExpected)) == 0);                                             \
        if (aSetup.writer.GetLengthWritten() != sizeof(aExpected) || memcmp(aSetup.buf, aExpected, sizeof(aExpected)) != 0)        \
        {                                                                                                                          \
            printf("Encoded: \n");                                                                                                 \
            for (size_t i = 0; i < aSetup.writer.GetLengthWritten(); i++)                                                          \
            {                                                                                                                      \
                printf("0x%02" PRIx8 ",", aSetup.buf[i]);                                                                          \
            }                                                                                                                      \
            printf("\n");                                                                                                          \
            printf("Expected: \n");                                                                                                \
            for (size_t i = 0; i < sizeof(aExpected); i++)                                                                         \
            {                                                                                                                      \
                printf("0x%02" PRIx8 ",", aExpected[i]);                                                                           \
            }                                                                                                                      \
            printf("\n");                                                                                                          \
        }                                                                                                                          \
    } while (0)

void TestEncodeNothing(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    // Just have an anonymous struct marker, and the AttributeReportIBs opened.
    const uint8_t expected[] = { 0x15, 0x36, 0x01 };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeBool(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    CHIP_ERROR err = test.encoder.Encode(true);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    const uint8_t expected[] = {
        // clang-format off
        0x15, 0x36, 0x01, // Test overhead, Start Anonymous struct + Start 1 byte Tag Array + Tag (01)
        0x15, // Start anonymous struct
          0x35, 0x01, // Start 1 byte tag struct + Tag (01)
            0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
            0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
              0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
              0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
              0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
            0x18, // End of container
            0x29, 0x02, // Tag (02) Value True (Attribute Value)
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeListOfBools1(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    bool list[]    = { true, false };
    CHIP_ERROR err = test.encoder.Encode(DataModel::List<bool>(list));
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    const uint8_t expected[] = {
        // clang-format off
        0x15, 0x36, 0x01, // Test overhead, Start Anonymous struct + Start 1 byte Tag Array + Tag (01)
        0x15, // Start anonymous struct
          0x35, 0x01, // Start 1 byte tag struct + Tag (01)
            0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
            0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
              0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
              0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
              0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
            0x18, // End of container
            0x36, 0x02, // Start 1 byte tag array + Tag (02) (Attribute Value)
              0x09, // True
              0x08, // False
            0x18,
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeListOfBools2(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    bool list[]    = { true, false };
    CHIP_ERROR err = test.encoder.EncodeList([&list](const auto & encoder) -> CHIP_ERROR {
        for (auto & item : list)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    const uint8_t expected[] = {
        // clang-format off
        0x15, 0x36, 0x01, // Test overhead, Start Anonymous struct + Start 1 byte Tag Array + Tag (01)
        0x15, // Start anonymous struct
          0x35, 0x01, // Start 1 byte tag struct + Tag (01)
            0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
            0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
              0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
              0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
              0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
            0x18, // End of container
            // Intended empty array
            0x36, 0x02, // Start 1 byte tag array + Tag (02) (Attribute Value)
            0x18, // End of container
          0x18, // End of container
        0x18, // End of container

        0x15, // Start anonymous struct
          0x35, 0x01, // Start 1 byte tag struct + Tag (01)
            0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
            0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
              0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
              0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
              0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
              0x34, 0x05, // Tag (05) Null
            0x18, // End of container
            0x29, 0x02, // Tag (02) Value True (Attribute Value)
          0x18, // End of container
        0x18, // End of container

        0x15, // Start anonymous struct
          0x35, 0x01, // Start 1 byte tag struct + Tag (01)
            0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
            0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
              0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
              0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
              0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
              0x34, 0x05, // Tag (05) Null
            0x18, // End of container
            0x28, 0x02, // Tag (02) Value False (Attribute Value)
          0x18, // End of container
        0x18, // End of container

        // clang-format on
    };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeEmptyList(nlTestSuite * aSuite, void * aContext)
{
    TestSetup test(aSuite);
    CHIP_ERROR err = test.encoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
    const uint8_t expected[] = {
        // clang-format off
        0x15, 0x36, 0x01, // Test overhead, Start Anonymous struct + Start 1 byte Tag Array + Tag (01)
        0x15, // Start anonymous struct
          0x35, 0x01, // Start 1 byte tag struct + Tag (01)
            0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
            0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
              0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
              0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
              0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
            0x18, // End of container
            // Intended empty array
            0x36, 0x02, // Start 1 byte tag array + Tag (02) (Attribute Value)
            0x18, // End of container
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(aSuite, test, expected);
}

void TestEncodeListChunking(nlTestSuite * aSuite, void * aContext)
{
    AttributeValueEncoder::AttributeEncodeState state;

    bool list[]      = { true, false };
    auto listEncoder = [&list](const auto & encoder) -> CHIP_ERROR {
        for (auto & item : list)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    };

    {
        LimitedTestSetup<60> test1(aSuite);
        CHIP_ERROR err = test1.encoder.EncodeList(listEncoder);
        NL_TEST_ASSERT(aSuite, err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL);
        state = test1.encoder.GetState();

        const uint8_t expected[] = {
            // clang-format off
            0x15, 0x36, 0x01, // Test overhead, Start Anonymous struct + Start 1 byte Tag Array + Tag (01)
            0x15, // Start anonymous struct
              0x35, 0x01, // Start 1 byte tag struct + Tag (01)
                0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
                0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
                  0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
                  0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
                  0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
                0x18, // End of container
                // Intended empty array
                0x36, 0x02, // Start 1 byte tag array + Tag (02) (Attribute Value)
                0x18, // End of container
              0x18, // End of container
            0x18, // End of container

            0x15, // Start anonymous struct
              0x35, 0x01, // Start 1 byte tag struct + Tag (01)
                0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
                0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
                  0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
                  0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
                  0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
                  0x34, 0x05, // Tag (05) Null
                0x18, // End of container
                0x29, 0x02, // Tag (02) Value True (Attribute Value)
              0x18, // End of container
            0x18, // End of container
            // clang-format on
        };
        VERIFY_BUFFER_STATE(aSuite, test1, expected);
    }
    {
        LimitedTestSetup<60> test2(aSuite, state);
        CHIP_ERROR err = test2.encoder.EncodeList(listEncoder);
        NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

        const uint8_t expected[] = {
            // clang-format off
            0x15, 0x36, 0x01, // Test overhead, Start Anonymous struct + Start 1 byte Tag Array + Tag (01)
            0x15, // Start anonymous struct
              0x35, 0x01, // Start 1 byte tag struct + Tag (01)
                0x24, 0x00, 0x99, // Tag (00) Value (1 byte uint) 0x99 (Attribute Version)
                0x37, 0x01, // Start 1 byte tag list + Tag (01) (Attribute Path)
                  0x24, 0x02, 0x55, // Tag (02) Value (1 byte uint) 0x55
                  0x24, 0x03, 0xaa, // Tag (03) Value (1 byte uint) 0xaa
                  0x24, 0x04, 0xcc, // Tag (04) Value (1 byte uint) 0xcc
                  0x34, 0x05, // Tag (05) Null
                0x18, // End of container
                0x28, 0x02, // Tag (02) Value False (Attribute Value)
              0x18, // End of container
            0x18, // End of container
            // clang-format on
        };
        VERIFY_BUFFER_STATE(aSuite, test2, expected);
    }
}

#undef VERIFY_STATE

} // anonymous namespace

namespace {
const nlTest sTests[] = { NL_TEST_DEF("TestEncodeNothing", TestEncodeNothing),
                          NL_TEST_DEF("TestEncodeBool", TestEncodeBool),
                          NL_TEST_DEF("TestEncodeEmptyList", TestEncodeEmptyList),
                          NL_TEST_DEF("TestEncodeListOfBools1", TestEncodeListOfBools1),
                          NL_TEST_DEF("TestEncodeListOfBools2", TestEncodeListOfBools2),
                          NL_TEST_DEF("TestEncodeListChunking", TestEncodeListChunking),
                          NL_TEST_SENTINEL() };
}

int TestAttributeValueEncoder()
{
    nlTestSuite theSuite = { "AttributeValueEncoder", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestAttributeValueEncoder)
