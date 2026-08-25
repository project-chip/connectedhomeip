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

#include <lib/core/TLV.h>
#include <oob-accessors/OOBDataSerializer.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;

class TestOOBDataSerializer : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TestOOBDataSerializer, BuildAndParse_BooleanStateValue)
{
    ConcreteDataAttributePath path(1, 2, 3); // ep=1, cluster=2, attr=3

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), true), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    EXPECT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);
    EXPECT_GT(requestBuffer.size(), 0U);

    auto parseResult = OOBDataSerializer::ParseAttributeRequest(ByteSpan(requestBuffer.data(), requestBuffer.size()));
    EXPECT_FALSE(std::holds_alternative<CHIP_ERROR>(parseResult));

    auto & request = std::get<OOBDataSerializer::AttributeRequest>(parseResult);
    EXPECT_EQ(request.path.mEndpointId, 1U);
    EXPECT_EQ(request.path.mClusterId, 2U);
    EXPECT_EQ(request.path.mAttributeId, 3U);

    bool parsedValue = false;
    EXPECT_EQ(request.value.Get(parsedValue), CHIP_NO_ERROR);
    EXPECT_TRUE(parsedValue);
}

TEST_F(TestOOBDataSerializer, BuildAndParse_Uint32Value)
{
    ConcreteDataAttributePath path(10, 20, 30);

    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(123456)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    EXPECT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);

    auto parseResult = OOBDataSerializer::ParseAttributeRequest(ByteSpan(requestBuffer.data(), requestBuffer.size()));
    EXPECT_FALSE(std::holds_alternative<CHIP_ERROR>(parseResult));

    auto & request = std::get<OOBDataSerializer::AttributeRequest>(parseResult);
    EXPECT_EQ(request.path.mEndpointId, 10U);
    EXPECT_EQ(request.path.mClusterId, 20U);
    EXPECT_EQ(request.path.mAttributeId, 30U);

    uint32_t parsedValue = 0;
    EXPECT_EQ(request.value.Get(parsedValue), CHIP_NO_ERROR);
    EXPECT_EQ(parsedValue, 123456U);
}

TEST_F(TestOOBDataSerializer, Parse_InvalidStructure)
{
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(123)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    auto parseResult = OOBDataSerializer::ParseAttributeRequest(ByteSpan(buffer, writer.GetLengthWritten()));
    EXPECT_TRUE(std::holds_alternative<CHIP_ERROR>(parseResult));
    EXPECT_EQ(std::get<CHIP_ERROR>(parseResult), CHIP_ERROR_WRONG_TLV_TYPE);
}

TEST_F(TestOOBDataSerializer, Parse_MismatchedTags)
{
    uint8_t buffer[128];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType outerType;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Put(TLV::ContextTag(OOBDataSerializer::kTagClusterId), static_cast<uint32_t>(20)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(OOBDataSerializer::kTagEndpointId), static_cast<uint16_t>(10)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(OOBDataSerializer::kTagAttributeId), static_cast<uint32_t>(30)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::ContextTag(OOBDataSerializer::kTagValue), true), CHIP_NO_ERROR);

    EXPECT_EQ(writer.EndContainer(outerType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    auto parseResult = OOBDataSerializer::ParseAttributeRequest(ByteSpan(buffer, writer.GetLengthWritten()));
    EXPECT_TRUE(std::holds_alternative<CHIP_ERROR>(parseResult));
    EXPECT_EQ(std::get<CHIP_ERROR>(parseResult), CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);
}

TEST_F(TestOOBDataSerializer, BuildAndParse_ArrayValue)
{
    ConcreteDataAttributePath path(1, 2, 3);

    uint8_t buffer[128];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType arrayType;
    EXPECT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(10)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(20)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(30)), CHIP_NO_ERROR);
    EXPECT_EQ(writer.EndContainer(arrayType), CHIP_NO_ERROR);
    EXPECT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    EXPECT_EQ(reader.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(reader.GetType(), TLV::kTLVType_Array);

    auto buildResult = OOBDataSerializer::BuildSetAttributeRequest(path, reader);
    EXPECT_FALSE(std::holds_alternative<CHIP_ERROR>(buildResult));

    auto & requestBuffer = std::get<ReadOnlyBuffer<uint8_t>>(buildResult);

    auto parseResult = OOBDataSerializer::ParseAttributeRequest(ByteSpan(requestBuffer.data(), requestBuffer.size()));
    EXPECT_FALSE(std::holds_alternative<CHIP_ERROR>(parseResult));

    auto & request = std::get<OOBDataSerializer::AttributeRequest>(parseResult);
    EXPECT_EQ(request.path.mEndpointId, 1U);
    EXPECT_EQ(request.path.mClusterId, 2U);
    EXPECT_EQ(request.path.mAttributeId, 3U);

    EXPECT_EQ(request.value.GetType(), TLV::kTLVType_Array);

    TLV::TLVType outerType;
    EXPECT_EQ(request.value.EnterContainer(outerType), CHIP_NO_ERROR);

    uint32_t val1 = 0;
    EXPECT_EQ(request.value.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(request.value.Get(val1), CHIP_NO_ERROR);
    EXPECT_EQ(val1, 10U);

    uint32_t val2 = 0;
    EXPECT_EQ(request.value.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(request.value.Get(val2), CHIP_NO_ERROR);
    EXPECT_EQ(val2, 20U);

    uint32_t val3 = 0;
    EXPECT_EQ(request.value.Next(), CHIP_NO_ERROR);
    EXPECT_EQ(request.value.Get(val3), CHIP_NO_ERROR);
    EXPECT_EQ(val3, 30U);

    EXPECT_EQ(request.value.Next(), CHIP_END_OF_TLV);
    EXPECT_EQ(request.value.ExitContainer(outerType), CHIP_NO_ERROR);
}
