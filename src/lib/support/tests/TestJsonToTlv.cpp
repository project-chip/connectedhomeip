/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <string>

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVReader.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TextFormat.h>
#include <lib/support/jsontlv/TlvToJson.h>

namespace {

using namespace chip::Encoding;
using namespace chip;
using namespace chip::app;

constexpr uint32_t kImplicitProfileId = 0x1234;

uint8_t gBuf1[1024];
uint8_t gBuf2[1024];
TLV::TLVWriter gWriter1;
TLV::TLVWriter gWriter2;

class TestJsonToTlv : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

void SetupWriters()
{
    gWriter1.Init(gBuf1);
    gWriter1.ImplicitProfileId = kImplicitProfileId;

    gWriter2.Init(gBuf2);
    gWriter2.ImplicitProfileId = kImplicitProfileId;
}

void PrintBytes(const uint8_t * buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

bool MatchWriter1and2()
{
    auto matches =
        (gWriter1.GetLengthWritten() == gWriter2.GetLengthWritten()) && (memcmp(gBuf1, gBuf2, gWriter1.GetLengthWritten()) == 0);

    if (!matches)
    {
        printf("Didn't match!\n");
        printf("Reference:\n");
        PrintBytes(gBuf1, gWriter1.GetLengthWritten());

        printf("Generated:\n");
        PrintBytes(gBuf2, gWriter2.GetLengthWritten());
    }

    return matches;
}

template <typename T>
void ConvertJsonToTlvAndValidate(T val, const std::string & jsonString)
{
    CHIP_ERROR err;
    TLV::TLVType container;

    SetupWriters();

    err = gWriter1.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(gWriter1, TLV::ContextTag(1), val);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = gWriter1.EndContainer(container);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = gWriter1.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = JsonToTlv(jsonString, gWriter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_TRUE(MatchWriter1and2());
}

TEST_F(TestJsonToTlv, TestConverter)
{
    std::string jsonString;

    jsonString = "{\n"
                 "   \"1:UINT\" : 30\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(static_cast<uint32_t>(30), jsonString);

    jsonString = "{\n"
                 "   \"1:INT\" : -30\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(static_cast<int32_t>(-30), jsonString);

    jsonString = "{\n"
                 "   \"1:BOOL\" : false\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(false, jsonString);

    jsonString = "{\n"
                 "   \"1:BOOL\" : true\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(true, jsonString);

    jsonString = "{\n"
                 "   \"1:FLOAT\" : 1.0\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(static_cast<float>(1.0), jsonString);

    jsonString = "{\n"
                 "   \"1:DOUBLE\" : 1.0\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(static_cast<double>(1.0), jsonString);

    jsonString = "{\n"
                 "   \"1:STRING\" : \"hello\"\n"
                 "}\n";
    ConvertJsonToTlvAndValidate("hello"_span, jsonString);

    // Validated using https://base64.guru/converter/encode/hex
    const uint8_t byteBuf[] = { 0x01, 0x02, 0x03, 0x04, 0xff, 0xfe, 0x99, 0x88, 0xdd, 0xcd };
    ByteSpan byteSpan(byteBuf);
    jsonString = "{\n"
                 "   \"1:BYTES\" : \"AQIDBP/+mYjdzQ==\"\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(byteSpan, jsonString);

    // Empty bytes.
    jsonString = "{\n"
                 "   \"1:BYTES\" : \"\"\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(ByteSpan{}, jsonString);

    DataModel::Nullable<uint8_t> nullValue;
    jsonString = "{\n"
                 "   \"1:NULL\" : null\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(nullValue, jsonString);

    Clusters::UnitTesting::Structs::SimpleStruct::Type structVal;
    structVal.a = 20;
    structVal.b = true;
    structVal.d = byteBuf;
    structVal.e = "hello"_span;
    structVal.g = static_cast<float>(1.0);
    structVal.h = static_cast<double>(1.0);

    jsonString = "{\n"
                 "   \"1:STRUCT\" : {\n"
                 "      \"0:UINT\" : 20,\n"
                 "      \"1:BOOL\" : true,\n"
                 "      \"2:UINT\" : 0,\n"
                 "      \"3:BYTES\" : \"AQIDBP/+mYjdzQ==\",\n"
                 "      \"4:STRING\" : \"hello\",\n"
                 "      \"5:UINT\" : 0,\n"
                 "      \"6:FLOAT\" : 1.0,\n"
                 "      \"7:DOUBLE\" : 1.0\n"
                 "   }\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(structVal, jsonString);

    uint8_t int8uListData[] = { 1, 2, 3, 4 };
    DataModel::List<uint8_t> int8uList;

    int8uList  = int8uListData;
    jsonString = "{\n"
                 "   \"1:ARRAY-UINT\" : [ 1, 2, 3, 4 ]\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(int8uList, jsonString);

    int8uList  = {};
    jsonString = "{\n"
                 "   \"1:ARRAY-?\" : [ ]\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(int8uList, jsonString);

    DataModel::Nullable<DataModel::List<uint8_t>> nullValueList;
    jsonString = "{\n"
                 "   \"1:NULL\" : null\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(nullValueList, jsonString);

    Clusters::UnitTesting::Structs::SimpleStruct::Type structListData[2] = { structVal, structVal };
    DataModel::List<Clusters::UnitTesting::Structs::SimpleStruct::Type> structList;

    structList = structListData;
    jsonString = "{\n"
                 "   \"1:ARRAY-STRUCT\" : [\n"
                 "      {\n"
                 "         \"0:UINT\" : 20,\n"
                 "         \"1:BOOL\" : true,\n"
                 "         \"2:UINT\" : 0,\n"
                 "         \"3:BYTES\" : \"AQIDBP/+mYjdzQ==\",\n"
                 "         \"4:STRING\" : \"hello\",\n"
                 "         \"5:UINT\" : 0,\n"
                 "         \"6:FLOAT\" : 1.0,\n"
                 "         \"7:DOUBLE\" : 1.0\n"
                 "      },\n"
                 "      {\n"
                 "         \"0:UINT\" : 20,\n"
                 "         \"1:BOOL\" : true,\n"
                 "         \"2:UINT\" : 0,\n"
                 "         \"3:BYTES\" : \"AQIDBP/+mYjdzQ==\",\n"
                 "         \"4:STRING\" : \"hello\",\n"
                 "         \"5:UINT\" : 0,\n"
                 "         \"6:FLOAT\" : 1.0,\n"
                 "         \"7:DOUBLE\" : 1.0\n"
                 "      }\n"
                 "   ]\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(structList, jsonString);
}

TEST_F(TestJsonToTlv, Test32BitConvert)
{
    // JSON TLV format explicitly wants to support 32-bit integer preservation.
    //
    // This is to support encode/decode of a format like:
    // { "123456:BOOL" : true } to be a compact way of encoding
    // "attribute id 123456 has value true"
    //
    // Such an encoding is NOT part of the matter spec, so best-effort is done here:
    // - low ids are encoded as context tags (this is in the spec for any structure encoding)
    // - large ids are encoded as implicit tags (NOT used in spec as spec never has such high ids)
    TLV::TLVReader reader;
    TLV::TLVType tlvType;
    int32_t value = 0;

    // convert a simple single value
    {
        SetupWriters();
        JsonToTlv("{\"1:INT\": 321}", gWriter1);
        EXPECT_EQ(gWriter1.Finalize(), CHIP_NO_ERROR);

        reader.Init(gBuf1, gWriter1.GetLengthWritten());
        reader.ImplicitProfileId = kImplicitProfileId;

        EXPECT_EQ(reader.Next(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_Structure);
        EXPECT_EQ(reader.EnterContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(TLV::ContextTag(1)), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_SignedInteger);
        EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 321);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
    }

    // convert a single value that is larger than 8 bit
    {
        SetupWriters();
        JsonToTlv("{\"1234:INT\": 321}", gWriter1);
        EXPECT_EQ(gWriter1.Finalize(), CHIP_NO_ERROR);

        reader.Init(gBuf1, gWriter1.GetLengthWritten());
        reader.ImplicitProfileId = kImplicitProfileId;

        EXPECT_EQ(reader.Next(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_Structure);
        EXPECT_EQ(reader.EnterContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(TLV::ProfileTag(kImplicitProfileId, 1234)), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_SignedInteger);
        EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 321);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
    }

    // Convert to a full 32-bit value, unsigned
    {
        SetupWriters();
        JsonToTlv("{\"4275878552:INT\": 321}", gWriter1);
        EXPECT_EQ(gWriter1.Finalize(), CHIP_NO_ERROR);

        reader.Init(gBuf1, gWriter1.GetLengthWritten());
        reader.ImplicitProfileId = kImplicitProfileId;

        EXPECT_EQ(reader.Next(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_Structure);
        EXPECT_EQ(reader.EnterContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(TLV::ProfileTag((4275878552 >> 16) & 0xFFFF, 0, 4275878552 & 0xFFFF)), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_SignedInteger);
        EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 321);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
    }

    // FIXME: implement
}

TEST_F(TestJsonToTlv, TestMEIConvert)
{
    TLV::TLVReader reader;
    TLV::TLVType tlvType;
    int32_t value = 0;

    // Vendor ID = 1, Tag ID = 0
    {
        SetupWriters();
        JsonToTlv("{\"65536:INT\": 321}", gWriter1);
        EXPECT_EQ(gWriter1.Finalize(), CHIP_NO_ERROR);

        reader.Init(gBuf1, gWriter1.GetLengthWritten());
        reader.ImplicitProfileId = kImplicitProfileId;

        EXPECT_EQ(reader.Next(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_Structure);
        EXPECT_EQ(reader.EnterContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(TLV::ProfileTag(1, 0, 0)), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_SignedInteger);
        EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 321);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
    }

    // Vendor ID = 0xFFFF, Tag ID = 0
    {
        SetupWriters();
        JsonToTlv("{\"4294901760:INT\": 123}", gWriter1);
        EXPECT_EQ(gWriter1.Finalize(), CHIP_NO_ERROR);

        reader.Init(gBuf1, gWriter1.GetLengthWritten());
        reader.ImplicitProfileId = kImplicitProfileId;

        EXPECT_EQ(reader.Next(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_Structure);
        EXPECT_EQ(reader.EnterContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(TLV::ProfileTag(0xFFFF, 0, 0)), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_SignedInteger);
        EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 123);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
    }

    // Vendor ID = 0xFFFF, Tag ID = 0xFFFF
    {
        SetupWriters();
        JsonToTlv("{\"4294967295:INT\": 123}", gWriter1);
        EXPECT_EQ(gWriter1.Finalize(), CHIP_NO_ERROR);

        reader.Init(gBuf1, gWriter1.GetLengthWritten());
        reader.ImplicitProfileId = kImplicitProfileId;

        EXPECT_EQ(reader.Next(TLV::AnonymousTag()), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_Structure);
        EXPECT_EQ(reader.EnterContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(TLV::ProfileTag(0xFFFF, 0, 0xFFFF)), CHIP_NO_ERROR);
        EXPECT_EQ(reader.GetType(), TLV::kTLVType_SignedInteger);
        EXPECT_EQ(reader.Get(value), CHIP_NO_ERROR);
        EXPECT_EQ(value, 123);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
        EXPECT_EQ(reader.ExitContainer(tlvType), CHIP_NO_ERROR);
        EXPECT_EQ(reader.Next(), CHIP_END_OF_TLV);
    }
}
} // namespace
