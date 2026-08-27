/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <lib/support/Span.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;

const uint8_t sTestShortStringDefault[]     = "\x05Hello";
const uint8_t sTestLongStringDefault[]      = "\x0b\x00Hello World";
const uint8_t sTestOctetStringDefault[]     = "\x04\x01\x02\x03\x04";
const uint8_t sTestNullShortStringDefault[] = "\xFF";
const uint8_t sTestNullLongStringDefault[]  = "\xFF\xFF";

EmberAfAttributeMinMaxValue sTestMinMaxInt16 = {
    .defaultValue = EmberAfDefaultAttributeValue(static_cast<uint16_t>(50)),
    .minValue     = EmberAfDefaultAttributeValue(static_cast<uint16_t>(10)),
    .maxValue     = EmberAfDefaultAttributeValue(static_cast<uint16_t>(100)),
};

// clang-format off
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(sTestDynamicAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(0x0001, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_SCALAR_DEFAULT(0x0002, INT8U, 1, 0, 42),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_SCALAR_DEFAULT(0x0003, INT16U, 2, 0, 0x1234),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_SCALAR_DEFAULT(0x0004, INT32U, 4, 0, 0x12345678),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_SCALAR_DEFAULT(0x0005, BOOLEAN, 1, 0, true),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_SCALAR_DEFAULT(0x0006, INT16S, 2, 0, -100),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_STRING_DEFAULT(0x0007, CHAR_STRING, 16, 0, sTestShortStringDefault),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_STRING_DEFAULT(0x0008, LONG_CHAR_STRING, 64, 0, sTestLongStringDefault),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_STRING_DEFAULT(0x0009, OCTET_STRING, 16, 0, sTestOctetStringDefault),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_MIN_MAX_DEFAULT(0x000A, INT16U, 2, 0, &sTestMinMaxInt16),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_STRING_DEFAULT(0x000B, CHAR_STRING, 16, MATTER_ATTRIBUTE_FLAG_NULLABLE, sTestNullShortStringDefault),
    DECLARE_DYNAMIC_ATTRIBUTE_WITH_STRING_DEFAULT(0x000C, LONG_CHAR_STRING, 64, MATTER_ATTRIBUTE_FLAG_NULLABLE, sTestNullLongStringDefault),
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END_WITH_REVISION(3);
// clang-format on

TEST(TestDynamicAttributeMetadata, EmptyDefault)
{
    AttributeDefaultValue val;
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[0], val), Protocols::InteractionModel::Status::Success);
    uint8_t u8Val = 0xFF;
    val.CopyScalar(&u8Val, sizeof(u8Val));
    EXPECT_EQ(u8Val, 0);
}

TEST(TestDynamicAttributeMetadata, ScalarDefaults)
{
    AttributeDefaultValue val;

    // INT8U: 42
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[1], val), Protocols::InteractionModel::Status::Success);
    uint8_t u8Val = 0;
    val.CopyScalar(&u8Val, sizeof(u8Val));
    EXPECT_EQ(u8Val, 42);

    // INT16U: 0x1234
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[2], val), Protocols::InteractionModel::Status::Success);
    uint16_t u16Val = 0;
    val.CopyScalar(&u16Val, sizeof(u16Val));
    EXPECT_EQ(u16Val, 0x1234);

    // INT32U: 0x12345678
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[3], val), Protocols::InteractionModel::Status::Success);
    uint32_t u32Val = 0;
    val.CopyScalar(&u32Val, sizeof(u32Val));
    EXPECT_EQ(u32Val, 0x12345678u);

    // BOOLEAN: true
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[4], val), Protocols::InteractionModel::Status::Success);
    bool bVal = false;
    val.CopyScalar(&bVal, sizeof(bVal));
    EXPECT_TRUE(bVal);

    // INT16S: -100
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[5], val), Protocols::InteractionModel::Status::Success);
    int16_t s16Val = 0;
    val.CopyScalar(&s16Val, sizeof(s16Val));
    EXPECT_EQ(s16Val, -100);
}

TEST(TestDynamicAttributeMetadata, StringDefaults)
{
    AttributeDefaultValue val;

    // Short string
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[6], val), Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(val.ToCharSpan().data_equal("Hello"_span));

    // Long string
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[7], val), Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(val.ToCharSpan().data_equal("Hello World"_span));

    // Octet string
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[8], val), Protocols::InteractionModel::Status::Success);
    const uint8_t expectedBytes[] = { 1, 2, 3, 4 };
    EXPECT_TRUE(val.ToByteSpan().data_equal(ByteSpan(expectedBytes, sizeof(expectedBytes))));
}

TEST(TestDynamicAttributeMetadata, MinMaxDefault)
{
    AttributeDefaultValue val;
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[9], val), Protocols::InteractionModel::Status::Success);
    uint16_t u16Val = 0;
    val.CopyScalar(&u16Val, sizeof(u16Val));
    EXPECT_EQ(u16Val, 50);
}

TEST(TestDynamicAttributeMetadata, NullableStringDefaults)
{
    AttributeDefaultValue val;

    // Null short string
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[10], val), Protocols::InteractionModel::Status::Success);
    auto nullableShort = val.ToNullableCharSpan();
    EXPECT_TRUE(nullableShort.IsNull());

    // Null long string
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[11], val), Protocols::InteractionModel::Status::Success);
    auto nullableLong = val.ToNullableCharSpan();
    EXPECT_TRUE(nullableLong.IsNull());
}

TEST(TestDynamicAttributeMetadata, ClusterRevision)
{
    AttributeDefaultValue val;
    EXPECT_EQ(emberAfGetAttributeDefaultValue(&sTestDynamicAttrs[12], val), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(sTestDynamicAttrs[12].attributeId, 0xFFFDu);
    uint16_t revision = 0;
    val.CopyScalar(&revision, sizeof(revision));
    EXPECT_EQ(revision, 3);
}

TEST(TestDynamicAttributeMetadata, EndpointLevelLookup)
{
    constexpr EndpointId kTestEndpointId = 1;
    constexpr ClusterId kTestClusterId   = 0xFFF1'0001;

    Testing::MockNodeConfig nodeConfig({
        Testing::MockEndpointConfig(kTestEndpointId, {
            Testing::MockClusterConfig(
                kTestClusterId,
                {
                    Testing::MockAttributeConfig(0x0001, sTestDynamicAttrs[0]),
                    Testing::MockAttributeConfig(0x0002, sTestDynamicAttrs[1]),
                    Testing::MockAttributeConfig(0x0007, sTestDynamicAttrs[6]),
                }),
        }),
    });

    Testing::SetMockNodeConfig(nodeConfig);

    AttributeDefaultValue val;
    EXPECT_EQ(emberAfGetAttributeDefaultValue(kTestEndpointId, kTestClusterId, 0x0002, val),
              Protocols::InteractionModel::Status::Success);
    uint8_t u8Val = 0;
    val.CopyScalar(&u8Val, sizeof(u8Val));
    EXPECT_EQ(u8Val, 42);

    EXPECT_EQ(emberAfGetAttributeDefaultValue(kTestEndpointId, kTestClusterId, 0x0007, val),
              Protocols::InteractionModel::Status::Success);
    EXPECT_TRUE(val.ToCharSpan().data_equal("Hello"_span));

    EXPECT_EQ(emberAfGetAttributeDefaultValue(kTestEndpointId, kTestClusterId, 0x9999, val),
              Protocols::InteractionModel::Status::UnsupportedAttribute);

    EXPECT_EQ(emberAfGetAttributeDefaultValue(99, kTestClusterId, 0x0002, val),
              Protocols::InteractionModel::Status::UnsupportedAttribute);

    Testing::ResetMockNodeConfig();
}

} // namespace
