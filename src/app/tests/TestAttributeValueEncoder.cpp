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

#include <optional>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueEncoder.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <app/data-model/FabricScopedPreEncodedValue.h>
#include <app/data-model/PreEncodedValue.h>
#include <lib/core/TLVTags.h>
#include <lib/core/TLVWriter.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;

// TODO: This unit tests contains hard code TLV data, they should be replaced with some decoding code to improve readability.

namespace {

// These values are easier to be recognized in the encoded buffer
constexpr EndpointId kRandomEndpointId   = 0x55;
constexpr ClusterId kRandomClusterId     = 0xaa;
constexpr AttributeId kRandomAttributeId = 0xcc;
constexpr DataVersion kRandomDataVersion = 0x99;
constexpr FabricIndex kTestFabricIndex   = 1;
constexpr NodeId kFakeNodeId             = 1;
constexpr TLV::Tag kFabricIndexTag       = TLV::ContextTag(254);

Access::SubjectDescriptor DescriptorWithFabric(FabricIndex fabricIndex)
{
    Access::SubjectDescriptor result;

    result.fabricIndex = fabricIndex;
    result.subject     = kFakeNodeId;

    if (fabricIndex == kUndefinedFabricIndex)
    {
        // Make it seem somewhat valid: a fabric index is not available in PASE
        // before AddNOC
        result.authMode = Access::AuthMode::kPase;
    }
    else
    {
        result.authMode = Access::AuthMode::kCase;
    }
    return result;
}

template <size_t N>
struct LimitedTestSetup
{
    LimitedTestSetup(const FabricIndex aFabricIndex      = kUndefinedFabricIndex,
                     const AttributeEncodeState & aState = AttributeEncodeState()) :
        encoder(builder, DescriptorWithFabric(aFabricIndex),
                ConcreteAttributePath(kRandomEndpointId, kRandomClusterId, kRandomAttributeId), kRandomDataVersion,
                aFabricIndex != kUndefinedFabricIndex, aState)
    {
        writer.Init(buf);
        {
            TLVType ignored;
            CHIP_ERROR err = writer.StartContainer(AnonymousTag(), kTLVType_Structure, ignored);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }
        {
            CHIP_ERROR err = builder.Init(&writer, 1);
            EXPECT_EQ(err, CHIP_NO_ERROR);
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
#define VERIFY_BUFFER_STATE(aSetup, aExpected)                                                                                     \
    do                                                                                                                             \
    {                                                                                                                              \
        EXPECT_EQ(aSetup.writer.GetLengthWritten(), sizeof(aExpected));                                                            \
        EXPECT_EQ(memcmp(aSetup.buf, aExpected, sizeof(aExpected)), 0);                                                            \
        if (aSetup.writer.GetLengthWritten() != sizeof(aExpected) || memcmp(aSetup.buf, aExpected, sizeof(aExpected)) != 0)        \
        {                                                                                                                          \
            printf("Encoded: \n");                                                                                                 \
            for (size_t i = 0; i < aSetup.writer.GetLengthWritten(); i++)                                                          \
            {                                                                                                                      \
                printf("0x%02x,", aSetup.buf[i]);                                                                                  \
            }                                                                                                                      \
            printf("\n");                                                                                                          \
            printf("Expected: \n");                                                                                                \
            for (size_t i = 0; i < sizeof(aExpected); i++)                                                                         \
            {                                                                                                                      \
                printf("0x%02x,", aExpected[i]);                                                                                   \
            }                                                                                                                      \
            printf("\n");                                                                                                          \
        }                                                                                                                          \
    } while (0)

TEST(TestAttributeValueEncoder, TestEncodeNothing)
{
    TestSetup test{};
    // Just have an anonymous struct marker, and the AttributeReportIBs opened.
    const uint8_t expected[] = { 0x15, 0x36, 0x01 };
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeBool)
{
    TestSetup test{};
    CHIP_ERROR err = test.encoder.Encode(true);
    EXPECT_EQ(err, CHIP_NO_ERROR);
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
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeListOfBools1)
{
    TestSetup test{};
    bool list[]    = { true, false };
    CHIP_ERROR err = test.encoder.Encode(DataModel::List<bool>(list));
    EXPECT_EQ(err, CHIP_NO_ERROR);
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
            0x18, // End of array
          0x18, // End of attribute data structure
        0x18, // End of attribute structure
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeListOfBools2)
{
    TestSetup test{};
    bool list[]    = { true, false };
    CHIP_ERROR err = test.encoder.EncodeList([&list](const auto & encoder) -> CHIP_ERROR {
        for (auto & item : list)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(err, CHIP_NO_ERROR);
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
            0x18, // End of array
          0x18, // End of attribute data structure
        0x18, // End of attribute structure
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

constexpr uint8_t emptyListExpected[] = {
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

TEST(TestAttributeValueEncoder, TestEncodeEmptyList1)
{
    TestSetup test{};
    CHIP_ERROR err = test.encoder.EncodeList([](const auto & encoder) -> CHIP_ERROR { return CHIP_NO_ERROR; });
    EXPECT_EQ(err, CHIP_NO_ERROR);
    VERIFY_BUFFER_STATE(test, emptyListExpected);
}

TEST(TestAttributeValueEncoder, TestEncodeEmptyList2)
{
    TestSetup test{};
    CHIP_ERROR err = test.encoder.EncodeEmptyList();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    VERIFY_BUFFER_STATE(test, emptyListExpected);
}

TEST(TestAttributeValueEncoder, TestEncodeFabricScoped)
{
    TestSetup test(kTestFabricIndex);
    Clusters::AccessControl::Structs::AccessControlExtensionStruct::Type items[3];
    items[0].fabricIndex = 1;
    items[1].fabricIndex = 2;
    items[2].fabricIndex = 3;

    // We tried to encode three items, however, the encoder should only put the item with matching fabric index into the final list.
    CHIP_ERROR err = test.encoder.EncodeList([items](const auto & encoder) -> CHIP_ERROR {
        for (const auto & item : items)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(err, CHIP_NO_ERROR);
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
              0x15, // Start anonymous structure
                0x30, 0x01, 0x00, // Tag 1, OCTET_STRING length 0 (data)
                0x24, 0xFE, 0x01, // Tag 0xFE, UINT8 Value 1 (fabric index)
              0x18, // End of array element (structure)
            0x18, // End of array
          0x18, // End of attribute data structure
        0x18, // End of attribute structure
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeListChunking)
{
    AttributeEncodeState state;

    bool list[]      = { true, false, false, true, true, false };
    auto listEncoder = [&list](const auto & encoder) -> CHIP_ERROR {
        for (auto & item : list)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    };

    {
        // Use 30 bytes buffer to force chunking after the first "false". The kTestFabricIndex is
        // not effective in this test.
        //
        // We only encode 28 bytes, because we don't encode our last two "close container" bits
        // corresponding to the "test overhead" container starts.  But TLVWriter automatically
        // reserves space when containers are opened, so we have to have enough space to have
        // encoded those last two close containers.
        LimitedTestSetup<30> test1(kTestFabricIndex);
        CHIP_ERROR err = test1.encoder.EncodeList(listEncoder);
        EXPECT_TRUE(err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL);
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
                0x36, 0x02, // Start 1 byte tag array + Tag (02) (Attribute Value)
                  0x09, // True
                  0x08, // False
                0x18, // End of array
              0x18, // End of attribute data structure
            0x18, // End of attribute structure
            // clang-format on
        };
        VERIFY_BUFFER_STATE(test1, expected);
    }
    {
        // Use 30 bytes buffer to force chunking after the second "false". The kTestFabricIndex is
        // not effective in this test.
        LimitedTestSetup<30> test2(0, state);
        CHIP_ERROR err = test2.encoder.EncodeList(listEncoder);
        EXPECT_TRUE(err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL);
        state = test2.encoder.GetState();

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
        VERIFY_BUFFER_STATE(test2, expected);
    }
    {
        // Allow encoding everything else. The kTestFabricIndex is not effective in this test.
        TestSetup test3(0, state);
        CHIP_ERROR err = test3.encoder.EncodeList(listEncoder);
        EXPECT_EQ(err, CHIP_NO_ERROR);

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
        VERIFY_BUFFER_STATE(test3, expected);
    }
}

TEST(TestAttributeValueEncoder, TestEncodeListChunking2)
{
    AttributeEncodeState state;

    bool list[]      = { true, false, false, true, true, false };
    auto listEncoder = [&list](const auto & encoder) -> CHIP_ERROR {
        for (auto & item : list)
        {
            ReturnErrorOnFailure(encoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    };

    {
        // Use 28 bytes buffer to force chunking right after we start the list. kTestFabricIndex is
        // not effective in this test.
        //
        // We only encode 26 bytes, because we don't encode our last two "close container" bits
        // corresponding to the "test overhead" container starts.  But TLVWriter automatically
        // reserves space when containers are opened, so we have to have enough space to have
        // encoded those last two close containers.
        LimitedTestSetup<28> test1(kTestFabricIndex);
        CHIP_ERROR err = test1.encoder.EncodeList(listEncoder);
        EXPECT_TRUE(err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL);
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
                0x36, 0x02, // Start 1 byte tag array + Tag (02) (Attribute Value)
                0x18, // End of array
              0x18, // End of attribute data structure
            0x18, // End of attribute structure
            // clang-format on
        };
        VERIFY_BUFFER_STATE(test1, expected);
    }
    {
        // Use 30 bytes buffer to force chunking after the first "true". The kTestFabricIndex is not
        // effective in this test.
        LimitedTestSetup<30> test2(0, state);
        CHIP_ERROR err = test2.encoder.EncodeList(listEncoder);
        EXPECT_TRUE(err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL);
        state = test2.encoder.GetState();

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
                0x29, 0x02, // Tag (02) Value True (Attribute Value)
              0x18, // End of container
            0x18, // End of container
            // clang-format on
        };
        VERIFY_BUFFER_STATE(test2, expected);
    }
    {
        // Use 60 bytes buffer to force chunking after the second "false". The kTestFabricIndex is not
        // effective in this test.
        LimitedTestSetup<60> test3(0, state);
        CHIP_ERROR err = test3.encoder.EncodeList(listEncoder);
        EXPECT_TRUE(err == CHIP_ERROR_NO_MEMORY || err == CHIP_ERROR_BUFFER_TOO_SMALL);
        state = test3.encoder.GetState();

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
        VERIFY_BUFFER_STATE(test3, expected);
    }
    {
        // Allow encoding everything else. The kTestFabricIndex is not effective in this test.
        TestSetup test4(0, state);
        CHIP_ERROR err = test4.encoder.EncodeList(listEncoder);
        EXPECT_EQ(err, CHIP_NO_ERROR);

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
        VERIFY_BUFFER_STATE(test4, expected);
    }
}

TEST(TestAttributeValueEncoder, TestEncodePreEncoded)
{
    TestSetup test{};

    uint8_t buffer[128];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    // Use a random tag that is not the right tag.
    CHIP_ERROR err = writer.PutString(TLV::ProfileTag(0x1234abcd, 0x5678fedc), "hello");
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = writer.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ByteSpan value(buffer, writer.GetLengthWritten());
    err = test.encoder.Encode(DataModel::PreEncodedValue(value));
    EXPECT_EQ(err, CHIP_NO_ERROR);

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
            0x2C, 0x02, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, // Tag (02) Value "hello", with 1-byte length of 0x05 (Attribute Value)
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeListOfPreEncoded)
{
    TestSetup test{};

    uint8_t buffers[2][128];
    std::optional<DataModel::PreEncodedValue> values[2];

    {
        TLV::TLVWriter writer;
        writer.Init(buffers[0]);
        // Use a random tag that is not the right tag.
        CHIP_ERROR err = writer.PutString(TLV::ProfileTag(0x1234abcd, 0x5678fedc), "hello");
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        values[0].emplace(ByteSpan(buffers[0], writer.GetLengthWritten()));
    }

    {
        TLV::TLVWriter writer;
        writer.Init(buffers[1]);
        // Use a random tag that is not the right tag.
        CHIP_ERROR err = writer.PutString(TLV::ProfileTag(0x1234abcd, 0x00010002), "bye");
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        values[1].emplace(ByteSpan(buffers[1], writer.GetLengthWritten()));
    }

    CHIP_ERROR err = test.encoder.EncodeList([&values](const auto & encoder) {
        for (auto & value : values)
        {
            ReturnErrorOnFailure(encoder.Encode(value.value()));
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(err, CHIP_NO_ERROR);

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
              0x0C, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, // "hello", with 1-byte length of 0x05
              0x0C, 0x03, 0x62, 0x79, 0x65, // "bye", with 1-byte length of 0x03
            0x18, // End of array
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeListOfFabricScopedPreEncoded)
{
    TestSetup test{};

    uint8_t buffers[2][128];
    std::optional<DataModel::FabricScopedPreEncodedValue> values[2];

    {
        TLV::TLVWriter writer;
        writer.Init(buffers[0]);
        // Use a random tag that is not the right tag.
        TLV::TLVType outerContainerType;
        CHIP_ERROR err =
            writer.StartContainer(TLV::ProfileTag(0x1234abcd, 0x5678fedc), TLV::kTLVType_Structure, outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutString(TLV::ContextTag(7), "hello");
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Put(kFabricIndexTag, kTestFabricIndex);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.EndContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        values[0].emplace(ByteSpan(buffers[0], writer.GetLengthWritten()));
    }

    {
        TLV::TLVWriter writer;
        writer.Init(buffers[1]);
        // Use a random tag that is not the right tag.
        TLV::TLVType outerContainerType;
        CHIP_ERROR err =
            writer.StartContainer(TLV::ProfileTag(0x1234abcd, 0x00010002), TLV::kTLVType_Structure, outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutString(TLV::ContextTag(7), "bye");
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Put(kFabricIndexTag, static_cast<FabricIndex>(kTestFabricIndex + 1));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.EndContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        values[1].emplace(ByteSpan(buffers[1], writer.GetLengthWritten()));
    }

    CHIP_ERROR err = test.encoder.EncodeList([&values](const auto & encoder) {
        for (auto & value : values)
        {
            ReturnErrorOnFailure(encoder.Encode(value.value()));
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(err, CHIP_NO_ERROR);

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
              0x15, // Start of struct
                0x2C, 0x07, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, // "hello", with tag 07 and 1-byte length of 0x05
                0x24, 0xFE, 0x01, // Fabric index, tag 254, value 1
              0x18, // End of struct
              0x15, // Start of struct
                0x2C, 0x07, 0x03, 0x62, 0x79, 0x65, // "bye", with tag 07 and 1-byte length of 0x03
                0x24, 0xFE, 0x02, // Fabric index, tag 254, value 2
              0x18, // End of struct
            0x18, // End of array
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

TEST(TestAttributeValueEncoder, TestEncodeFabricFilteredListOfPreEncoded)
{
    TestSetup test(kTestFabricIndex);

    uint8_t buffers[2][128];
    std::optional<DataModel::FabricScopedPreEncodedValue> values[2];

    {
        TLV::TLVWriter writer;
        writer.Init(buffers[0]);
        // Use a random tag that is not the right tag.
        TLV::TLVType outerContainerType;
        CHIP_ERROR err =
            writer.StartContainer(TLV::ProfileTag(0x1234abcd, 0x5678fedc), TLV::kTLVType_Structure, outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutString(TLV::ContextTag(7), "hello");
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Put(kFabricIndexTag, kTestFabricIndex);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.EndContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        values[0].emplace(ByteSpan(buffers[0], writer.GetLengthWritten()));
    }

    {
        TLV::TLVWriter writer;
        writer.Init(buffers[1]);
        // Use a random tag that is not the right tag.
        TLV::TLVType outerContainerType;
        CHIP_ERROR err =
            writer.StartContainer(TLV::ProfileTag(0x1234abcd, 0x00010002), TLV::kTLVType_Structure, outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.PutString(TLV::ContextTag(7), "bye");
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Put(kFabricIndexTag, static_cast<FabricIndex>(kTestFabricIndex + 1));
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.EndContainer(outerContainerType);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        err = writer.Finalize();
        EXPECT_EQ(err, CHIP_NO_ERROR);

        values[1].emplace(ByteSpan(buffers[1], writer.GetLengthWritten()));
    }

    CHIP_ERROR err = test.encoder.EncodeList([&values](const auto & encoder) {
        for (auto & value : values)
        {
            ReturnErrorOnFailure(encoder.Encode(value.value()));
        }
        return CHIP_NO_ERROR;
    });
    EXPECT_EQ(err, CHIP_NO_ERROR);

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
              0x15, // Start of struct
                0x2C, 0x07, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, // "hello", with tag 07 and 1-byte length of 0x05
                0x24, 0xFE, 0x01, // Fabric index, tag 254, value 1
              0x18, // End of struct
              // No second struct, because we are doing fabric filtering
            0x18, // End of array
          0x18, // End of container
        0x18, // End of container
        // clang-format on
    };
    VERIFY_BUFFER_STATE(test, expected);
}

#undef VERIFY_BUFFER_STATE

} // anonymous namespace
