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

#include <stdio.h>
#include <string>

#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TextFormat.h>
#include <lib/support/jsontlv/TlvToJson.h>

namespace {

using namespace chip::Encoding;
using namespace chip;
using namespace chip::app;

constexpr uint32_t kImplicitProfileId = 0x1122;

class TestJsonToTlvToJson : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

void PrintSpan(const char * prefix, const ByteSpan & span)
{
    printf("%s", prefix);
    for (size_t i = 0; i < span.size(); i++)
    {
        printf("%02X ", span.data()[i]);
    }
    printf("\n");
}

void CheckValidConversion(const std::string & jsonOriginal, const ByteSpan & tlvEncoding, const std::string & jsonExpected)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool match     = false;

    uint8_t buf[256];
    MutableByteSpan tlvEncodingLocal(buf);
    err = JsonToTlv(jsonOriginal, tlvEncodingLocal);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    match = tlvEncodingLocal.data_equal(tlvEncoding);
    EXPECT_TRUE(match);
    if (!match)
    {
        printf("ERROR: TLV Encoding Doesn't Match!\n");
        PrintSpan("TLV Encoding Provided as Input for Reference:  ", tlvEncoding);
        PrintSpan("TLV Encoding Generated from Json Input String: ", tlvEncodingLocal);
    }

    std::string generatedJsonString;
    err = TlvToJson(tlvEncoding, generatedJsonString);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    auto compactExpectedString  = PrettyPrintJsonString(jsonExpected);
    auto compactGeneratedString = PrettyPrintJsonString(generatedJsonString);
    match                       = (compactGeneratedString == compactExpectedString);
    EXPECT_TRUE(match);
    if (!match)
    {
        printf("ERROR: Json String Doesn't Match!\n");
        printf("Expected  Json String:\n%s\n", compactExpectedString.c_str());
        printf("Generated Json String:\n%s\n", compactGeneratedString.c_str());
    }

    // Verify that Expected Json String Converts to the Same TLV Encoding
    tlvEncodingLocal = MutableByteSpan(buf);
    err              = JsonToTlv(jsonOriginal, tlvEncodingLocal);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    match = tlvEncodingLocal.data_equal(tlvEncoding);
    EXPECT_TRUE(match);
    if (!match)
    {
        printf("ERROR: TLV Encoding Doesn't Match!\n");
        PrintSpan("TLV Encoding Provided as Input for Reference:     ", tlvEncoding);
        PrintSpan("TLV Encoding Generated from Json Expected String: ", tlvEncodingLocal);
    }
}

// Boolean true
TEST_F(TestJsonToTlvToJson, TestConverter_Boolean_True)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:BOOL\" : true\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Integer 42, 1-octet
TEST_F(TestJsonToTlvToJson, TestConverter_SignedInt_1BytePositive)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<int8_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"2:INT\" : 42\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Integer -17, 1-octet
TEST_F(TestJsonToTlvToJson, TestConverter_SignedInt_1ByteNegative)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(3), static_cast<int8_t>(-17)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"3:INT\" : -17\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Unsigned Integer 42, 1-octet
TEST_F(TestJsonToTlvToJson, TestConverter_UnsignedInt_1Byte)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(4), static_cast<uint8_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"value:4:UINT\" : 42\n"
                             "}\n";

    std::string jsonExpected = "{\n"
                               "   \"4:UINT\" : 42\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

// Signed Integer 4242, 2-octet
TEST_F(TestJsonToTlvToJson, TestConverter_SignedInt_2Bytes)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(7), static_cast<int16_t>(4242)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"7:INT\" : 4242\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Integer -170000, 4-octet
TEST_F(TestJsonToTlvToJson, TestConverter_SignedInt_4Bytes)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(80), static_cast<int32_t>(-170000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"80:INT\" : -170000\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Long Integer (int64_t) 40000000000, 8-octet
TEST_F(TestJsonToTlvToJson, TestConverter_SignedInt_8Bytes)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(202), static_cast<int64_t>(40000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"202:INT\" : \"40000000000\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Unsigned Long Integer (uint64_t) 40000000000, 8-octet
TEST_F(TestJsonToTlvToJson, TestConverter_UnsignedInt_8Bytes)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(222), static_cast<uint64_t>(40000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"222:UINT\" : \"40000000000\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// UTF-8 String, 1-octet length, "Hello!"
TEST_F(TestJsonToTlvToJson, TestConverter_UTF8String_Hello)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(0), "Hello!"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:STRING\" : \"Hello!\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Octet String, 1-octet length, octets { 00 01 02 03 04 }
TEST_F(TestJsonToTlvToJson, TestConverter_OctetString)
{

    uint8_t v[] = { 0, 1, 2, 3, 4 };
    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBytes(TLV::ContextTag(1), v, sizeof(v)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:BYTES\" : \"AAECAwQ=\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Octet String, empty
TEST_F(TestJsonToTlvToJson, TestConverter_OctetString_Empty)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBytes(TLV::ContextTag(1), nullptr, 0));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:BYTES\" : \"\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Null
TEST_F(TestJsonToTlvToJson, TestConverter_Null)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutNull(TLV::ContextTag(1)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:NULL\" : null\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Single precision floating point 0.0
TEST_F(TestJsonToTlvToJson, TestConverter_Float_0)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), static_cast<float>(0.0)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:FLOAT\" : 0.0\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Single precision floating point (1.0 / 3.0)
TEST_F(TestJsonToTlvToJson, TestConverter_Float_1third)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(100), static_cast<float>(1.0 / 3.0)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString   = "{\n"
                               "   \"100:FLOAT\" : 0.33333334\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"100:FLOAT\" : 0.3333333432674408\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

// Single precision floating point 17.9
TEST_F(TestJsonToTlvToJson, TestConverter_Float_17_9)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(101), static_cast<float>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString   = "{\n"
                               "   \"101:FLOAT\" : 17.9\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"101:FLOAT\" : 17.899999618530273\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

// Single precision floating point positive infinity
TEST_F(TestJsonToTlvToJson, TestConverter_Float_PositiveInfinity)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(101), std::numeric_limits<float>::infinity()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:FLOAT\" : \"Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Single precision floating point negative infinity
TEST_F(TestJsonToTlvToJson, TestConverter_Float_NegativeInfinity)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(101), -std::numeric_limits<float>::infinity()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:FLOAT\" : \"-Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point 0.0
TEST_F(TestJsonToTlvToJson, TestConverter_Double_0)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), static_cast<double>(0.0)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:DOUBLE\" : 0.0\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point (1.0 / 3.0)
TEST_F(TestJsonToTlvToJson, TestConverter_Double_1third)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(100), static_cast<double>(1.0 / 3.0)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"100:DOUBLE\" : 0.33333333333333331\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point 17.9
TEST_F(TestJsonToTlvToJson, TestConverter_Double_17_9)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(101), static_cast<double>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:DOUBLE\" : 17.899999999999999\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point positive infinity
TEST_F(TestJsonToTlvToJson, TestConverter_Double_PositiveInfinity)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(101), std::numeric_limits<double>::infinity()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:DOUBLE\" : \"Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point negative infinity
TEST_F(TestJsonToTlvToJson, TestConverter_Double_NegativeInfinity)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(101), -std::numeric_limits<double>::infinity()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:DOUBLE\" : \"-Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Empty Top-Level Structure, {}
TEST_F(TestJsonToTlvToJson, TestConverter_Structure_TopLevelEmpty)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{ }";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Empty Nested Structure, { {} }
TEST_F(TestJsonToTlvToJson, TestConverter_Structure_NestedEmpty)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:STRUCT\" : { }\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Empty Array, { [] }
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Empty)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:ARRAY-?\" : []\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

TEST_F(TestJsonToTlvToJson, TestConverter_Array_Empty_ImplicitProfileTag2)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;

    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR,
              writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 10000), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString   = "{\n"
                               "   \"emptyarray:10000:ARRAY-?\" : []\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"10000:ARRAY-?\" : []\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

TEST_F(TestJsonToTlvToJson, TestConverter_Array_Empty_ImplicitProfileTag4)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;

    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(
        CHIP_NO_ERROR,
        writer.StartContainer(TLV::ProfileTag((1000000 >> 16) & 0xFFFF, 0, 1000000 & 0xFFFF), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1000000:ARRAY-?\" : []\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Two Signed Integers with context specific tags: {0 = 42, 1 = -17}
TEST_F(TestJsonToTlvToJson, TestConverter_IntsWithContextTags)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int8_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), static_cast<int8_t>(-17)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:INT\" : 42,\n"
                             "   \"1:INT\" : -17\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Structure with Two Signed Integers with context specific tags: { {0 = 42, 1 = -17} }
TEST_F(TestJsonToTlvToJson, TestConverter_Struct_IntsWithContextTags)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int8_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), static_cast<int8_t>(-17)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:STRUCT\" : {\n"
                             "      \"0:INT\" : 42,\n"
                             "      \"1:INT\" : -17\n"
                             "   }\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Array of Signed Integers: { [0, 1, 2, 3, 4] }
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Ints)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(0)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(1)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(2)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(3)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(4)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:ARRAY-INT\" : [\n"
                             "      0,\n"
                             "      1,\n"
                             "      2,\n"
                             "      3,\n"
                             "      4\n"
                             "   ]\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Array of Long Signed Integers: { [42, -17, -170000, 40000000000] }
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Ints2)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(-17)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(-170000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(40000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:ARRAY-INT\" : [\n"
                             "      42,\n"
                             "      -17,\n"
                             "      -170000,\n"
                             "      \"40000000000\"\n"
                             "   ]\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Array of Signed Integers with MIN/MAX values for each type int8_t/int16_t/int32_t/int64_t
TEST_F(TestJsonToTlvToJson, TestConverter_Array_IntsMinMax)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT8_MIN)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT8_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT16_MIN)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT16_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT32_MIN)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT32_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT64_MIN)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT64_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString     = "{\n"
                                 "   \"0:ARRAY-INT\" : [\n"
                                 "      -128,\n"
                                 "      127,\n"
                                 "      -32768,\n"
                                 "      32767,\n"
                                 "      -2147483648,\n"
                                 "      2147483647,\n"
                                 "      -9223372036854775808,\n"
                                 "      9223372036854775807\n"
                                 "   ]\n"
                                 "}\n";
    std::string expectedString = "{\n"
                                 "   \"0:ARRAY-INT\" : [\n"
                                 "      -128,\n"
                                 "      127,\n"
                                 "      -32768,\n"
                                 "      32767,\n"
                                 "      -2147483648,\n"
                                 "      2147483647,\n"
                                 "      \"-9223372036854775808\",\n"
                                 "      \"9223372036854775807\"\n"
                                 "   ]\n"
                                 "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, expectedString);
}

// Array of Long Unsigned Integers: { [42, 170000, 40000000000] }
TEST_F(TestJsonToTlvToJson, TestConverter_Array_UInts)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(170000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(40000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:ARRAY-UINT\" : [\n"
                             "      42,\n"
                             "      170000,\n"
                             "      \"40000000000\"\n"
                             "   ]\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Array of Unsigned Integers, where each element represents MAX possible value for unsigned
// integere types uint8_t, uint16_t, uint32_t, uint64_t: [0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFF_FFFFFFFF]
TEST_F(TestJsonToTlvToJson, TestConverter_Array_UIntsMax)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(UINT8_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint16_t>(UINT16_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(UINT32_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(UINT64_MAX)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString     = "{\n"
                                 "   \"0:ARRAY-UINT\" : [\n"
                                 "      255,\n"
                                 "      65535,\n"
                                 "      4294967295,\n"
                                 "      18446744073709551615\n"
                                 "   ]\n"
                                 "}\n";
    std::string expectedString = "{\n"
                                 "   \"0:ARRAY-UINT\" : [\n"
                                 "      255,\n"
                                 "      65535,\n"
                                 "      4294967295,\n"
                                 "      \"18446744073709551615\"\n"
                                 "   ]\n"
                                 "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, expectedString);
}

// Array of Doubles: { [1.1, 134.2763, -12345.87] }
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Doubles)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(1.1)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(134.2763)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(-12345.87)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString     = "{\n"
                                 "   \"0:ARRAY-DOUBLE\" : [\n"
                                 "      1.1,\n"
                                 "      134.2763,\n"
                                 "      -12345.87\n"
                                 "   ]\n"
                                 "}\n";
    std::string expectedString = "{\n"
                                 "   \"0:ARRAY-DOUBLE\" : [\n"
                                 "      1.1000000000000001,\n"
                                 "      134.27629999999999,\n"
                                 "      -12345.870000000001\n"
                                 "   ]\n"
                                 "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, expectedString);
}

// Array of Floats: { [1.1, 134.2763, -12345.87] }
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Floats)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 1000), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<float>(1.1)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<float>(134.2763)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<float>(-12345.87)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString     = "{\n"
                                 "   \"1000:ARRAY-FLOAT\" : [\n"
                                 "      1.1,\n"
                                 "      134.2763,\n"
                                 "      -12345.87\n"
                                 "   ]\n"
                                 "}\n";
    std::string expectedString = "{\n"
                                 "   \"1000:ARRAY-FLOAT\" : [\n"
                                 "      1.1000000238418579,\n"
                                 "      134.27630615234375,\n"
                                 "      -12345.8701171875\n"
                                 "   ]\n"
                                 "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, expectedString);
}

// Array of Strings: ["ABC", "Options", "more"]
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Strings)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(
        CHIP_NO_ERROR,
        writer.StartContainer(TLV::ProfileTag((100000 >> 16) & 0xFFFF, 0, 100000 & 0xFFFF), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::AnonymousTag(), "ABC"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::AnonymousTag(), "Options"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::AnonymousTag(), "more"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"100000:ARRAY-STRING\" : [\n"
                             "      \"ABC\",\n"
                             "      \"Options\",\n"
                             "      \"more\"\n"
                             "   ]\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Array of Booleans: [true, false, false]
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Booleans)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(255), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), false));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), false));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"255:ARRAY-BOOL\" : [\n"
                             "      true,\n"
                             "      false,\n"
                             "      false\n"
                             "   ]\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Array of Nulls: [null, null]
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Nulls)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutNull(TLV::AnonymousTag()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutNull(TLV::AnonymousTag()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:ARRAY-NULL\" : [\n"
                             "      null,\n"
                             "      null\n"
                             "   ]\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Context tag 255 (max), Unsigned Integer, 1-octet value: {255 = 42U}
TEST_F(TestJsonToTlvToJson, TestConverter_Struct_UInt)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(255), static_cast<uint8_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString   = "{\n"
                               "   \"value:0:STRUCT\" : {\n"
                               "      \"name:255:UINT\" : 42\n"
                               "   }\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"0:STRUCT\" : {\n"
                               "      \"255:UINT\" : 42\n"
                               "   }\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

// Context and Common Profile tags, Unsigned Integer structure: {255 = 42, 256 = 17000, 65535 =
// 1, 65536 = 345678, 4294967295 = 500000000000}
TEST_F(TestJsonToTlvToJson, TestConverter_Struct_MixedTags)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(255), static_cast<uint64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(0x0001u, 0, 0), static_cast<uint64_t>(345678)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(kImplicitProfileId, 256), static_cast<uint64_t>(17000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(0xFFFFu, 0, 0xFFFFu), static_cast<uint64_t>(500000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(kImplicitProfileId, 65535), static_cast<uint64_t>(1)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:STRUCT\" : {\n"
                             "      \"255:UINT\" : 42,\n"
                             "      \"256:UINT\" : 17000,\n"
                             "      \"65535:UINT\" : 1,\n"
                             "      \"65536:UINT\" : 345678,\n"
                             "      \"4294967295:UINT\" : \"500000000000\"\n"
                             "   }\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Structure with mixed elements
TEST_F(TestJsonToTlvToJson, TestConverter_Struct_MixedElements)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    char bytes[] = "Test ByteString Value";

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int64_t>(20)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(0)));
    EXPECT_EQ(CHIP_NO_ERROR,
              writer.PutBytes(TLV::ContextTag(3), reinterpret_cast<uint8_t *>(bytes), static_cast<uint32_t>(strlen(bytes))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(4), "hello"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(5), static_cast<int64_t>(-500000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(6), static_cast<double>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(7), static_cast<float>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString     = "{\n"
                                 "   \"c:0:STRUCT\" : {\n"
                                 "      \"z:0:INT\" : 20,\n"
                                 "      \"b:1:BOOL\" : true,\n"
                                 "      \"2:UINT\" : 0,\n"
                                 "      \"abc:3:BYTES\" : \"VGVzdCBCeXRlU3RyaW5nIFZhbHVl\",\n"
                                 "      \"cc:4:STRING\" : \"hello\",\n"
                                 "      \"tt:5:INT\" : -500000,\n"
                                 "      \"AA:6:DOUBLE\" : 17.9,\n"
                                 "      \"B:7:FLOAT\" : 17.9\n"
                                 "   }\n"
                                 "}\n";
    std::string expectedString = "{\n"
                                 "   \"0:STRUCT\" : {\n"
                                 "      \"0:INT\" : 20,\n"
                                 "      \"1:BOOL\" : true,\n"
                                 "      \"2:UINT\" : 0,\n"
                                 "      \"3:BYTES\" : \"VGVzdCBCeXRlU3RyaW5nIFZhbHVl\",\n"
                                 "      \"4:STRING\" : \"hello\",\n"
                                 "      \"5:INT\" : -500000,\n"
                                 "      \"6:DOUBLE\" : 17.899999999999999,\n"
                                 "      \"7:FLOAT\" : 17.899999618530273\n"
                                 "   }\n"
                                 "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, expectedString);
}

// Array of structures with mixed elements
TEST_F(TestJsonToTlvToJson, TestConverter_Array_Structures)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    TLV::TLVType containerType3;
    char bytes1[] = "Test ByteString Value 1";
    char bytes2[] = "Test ByteString Value 2";

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 1000), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int64_t>(20)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(0)));
    EXPECT_EQ(CHIP_NO_ERROR,
              writer.PutBytes(TLV::ContextTag(3), reinterpret_cast<uint8_t *>(bytes1), static_cast<uint32_t>(strlen(bytes1))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(4), "hello1"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(5), static_cast<int64_t>(-500000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(6), static_cast<double>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(7), static_cast<float>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int64_t>(-10)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), false));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(128)));
    EXPECT_EQ(CHIP_NO_ERROR,
              writer.PutBytes(TLV::ContextTag(3), reinterpret_cast<uint8_t *>(bytes2), static_cast<uint32_t>(strlen(bytes2))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(4), "hello2"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(5), static_cast<int64_t>(40000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(6), static_cast<double>(-1754.923)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(7), static_cast<float>(97.945)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString     = "{\n"
                                 "   \"1000:ARRAY-STRUCT\": [\n"
                                 "      {\n"
                                 "         \"0:INT\" : 20,\n"
                                 "         \"1:BOOL\" : true,\n"
                                 "         \"2:UINT\" : 0,\n"
                                 "         \"3:BYTES\" : \"VGVzdCBCeXRlU3RyaW5nIFZhbHVlIDE=\",\n"
                                 "         \"4:STRING\" : \"hello1\",\n"
                                 "         \"5:INT\" : -500000,\n"
                                 "         \"6:DOUBLE\" : 17.9,\n"
                                 "         \"7:FLOAT\" : 17.9\n"
                                 "      },\n"
                                 "      {\n"
                                 "         \"0:INT\" : -10,\n"
                                 "         \"1:BOOL\" : false,\n"
                                 "         \"2:UINT\" : 128,\n"
                                 "         \"3:BYTES\" : \"VGVzdCBCeXRlU3RyaW5nIFZhbHVlIDI=\",\n"
                                 "         \"4:STRING\" : \"hello2\",\n"
                                 "         \"5:INT\" : \"40000000000\",\n"
                                 "         \"6:DOUBLE\" : -1754.923,\n"
                                 "         \"7:FLOAT\" : 97.945\n"
                                 "      }\n"
                                 "   ]\n"
                                 "}\n";
    std::string expectedString = "{\n"
                                 "   \"1000:ARRAY-STRUCT\": [\n"
                                 "      {\n"
                                 "         \"0:INT\" : 20,\n"
                                 "         \"1:BOOL\" : true,\n"
                                 "         \"2:UINT\" : 0,\n"
                                 "         \"3:BYTES\" : \"VGVzdCBCeXRlU3RyaW5nIFZhbHVlIDE=\",\n"
                                 "         \"4:STRING\" : \"hello1\",\n"
                                 "         \"5:INT\" : -500000,\n"
                                 "         \"6:DOUBLE\" : 17.899999999999999,\n"
                                 "         \"7:FLOAT\" : 17.899999618530273\n"
                                 "      },\n"
                                 "      {\n"
                                 "         \"0:INT\" : -10,\n"
                                 "         \"1:BOOL\" : false,\n"
                                 "         \"2:UINT\" : 128,\n"
                                 "         \"3:BYTES\" : \"VGVzdCBCeXRlU3RyaW5nIFZhbHVlIDI=\",\n"
                                 "         \"4:STRING\" : \"hello2\",\n"
                                 "         \"5:INT\" : \"40000000000\",\n"
                                 "         \"6:DOUBLE\" : -1754.923,\n"
                                 "         \"7:FLOAT\" : 97.944999694824219\n"
                                 "      }\n"
                                 "   ]\n"
                                 "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, expectedString);
}

// Top level with mixed elements
TEST_F(TestJsonToTlvToJson, TestConverter_TopLevel_MixedElements)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    TLV::TLVType containerType3;
    char bytes[] = "Test array member 0";

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR,
              writer.PutBytes(TLV::ContextTag(1), reinterpret_cast<uint8_t *>(bytes), static_cast<uint32_t>(strlen(bytes))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<double>(156.398)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(3), static_cast<uint64_t>(73709551615)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(4), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutNull(TLV::ContextTag(5)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(6), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(1), "John"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(34)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(3), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Array, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(5)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(10)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(5), TLV::kTLVType_Array, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::AnonymousTag(), "Ammy"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::AnonymousTag(), "David"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::AnonymousTag(), "Larry"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(6), TLV::kTLVType_Array, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), false));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(7), static_cast<float>(0.0)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString   = "{\n"
                               "   \"value:0:INT\": 42,\n"
                               "   \"value:1:BYTES\": \"VGVzdCBhcnJheSBtZW1iZXIgMA==\",\n"
                               "   \"value:2:DOUBLE\": 156.398,\n"
                               "   \"value:3:UINT\": \"73709551615\",\n"
                               "   \"value:4:BOOL\": true,\n"
                               "   \"value:5:NULL\": null,\n"
                               "   \"value:6:STRUCT\": {\n"
                               "      \"name:1:STRING\": \"John\",\n"
                               "      \"age:2:UINT\": 34,\n"
                               "      \"approved:3:BOOL\": true,\n"
                               "      \"kids:4:ARRAY-INT\": [\n"
                               "         5,\n"
                               "         9,\n"
                               "         10\n"
                               "      ],\n"
                               "      \"names:5:ARRAY-STRING\": [\n"
                               "         \"Ammy\",\n"
                               "         \"David\",\n"
                               "         \"Larry\"\n"
                               "      ],\n"
                               "      \"6:ARRAY-BOOL\": [\n"
                               "         true,\n"
                               "         false,\n"
                               "         true\n"
                               "      ]\n"
                               "   },\n"
                               "   \"value:7:FLOAT\": 0.0\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"0:INT\": 42,\n"
                               "   \"1:BYTES\": \"VGVzdCBhcnJheSBtZW1iZXIgMA==\",\n"
                               "   \"2:DOUBLE\": 156.398,\n"
                               "   \"3:UINT\": \"73709551615\",\n"
                               "   \"4:BOOL\": true,\n"
                               "   \"5:NULL\": null,\n"
                               "   \"6:STRUCT\": {\n"
                               "      \"1:STRING\": \"John\",\n"
                               "      \"2:UINT\": 34,\n"
                               "      \"3:BOOL\": true,\n"
                               "      \"4:ARRAY-INT\": [\n"
                               "         5,\n"
                               "         9,\n"
                               "         10\n"
                               "      ],\n"
                               "      \"5:ARRAY-STRING\": [\n"
                               "         \"Ammy\",\n"
                               "         \"David\",\n"
                               "         \"Larry\"\n"
                               "      ],\n"
                               "      \"6:ARRAY-BOOL\": [\n"
                               "         true,\n"
                               "         false,\n"
                               "         true\n"
                               "      ]\n"
                               "   },\n"
                               "   \"7:FLOAT\": 0.0\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

// Complex Structure from README
TEST_F(TestJsonToTlvToJson, TestConverter_Structure_FromReadme)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    TLV::TLVType containerType3;
    uint8_t bytes1[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };
    uint8_t bytes2[] = { 0xFF };
    uint8_t bytes3[] = { 0x4A, 0xEF, 0x88 };
    char bytes4[]    = "Test Bytes";

    writer.Init(buf);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int8_t>(8)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(0), static_cast<int8_t>(12)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), false));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(2), "example"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<int64_t>(40000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(3), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(5), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(1.1)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(134.2763)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(-12345.87)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), std::numeric_limits<double>::infinity()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(62534)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<double>(-62534)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(6), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBytes(TLV::AnonymousTag(), bytes1, static_cast<uint32_t>(sizeof(bytes1))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBytes(TLV::AnonymousTag(), bytes2, static_cast<uint32_t>(sizeof(bytes2))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutBytes(TLV::AnonymousTag(), bytes3, static_cast<uint32_t>(sizeof(bytes3))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR,
              writer.PutBytes(TLV::ContextTag(7), reinterpret_cast<uint8_t *>(bytes4), static_cast<uint32_t>(strlen(bytes4))));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(8), static_cast<double>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(9), static_cast<float>(17.9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(10), -std::numeric_limits<float>::infinity()));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(11), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.PutString(TLV::ContextTag(1), "John"));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(2), static_cast<uint32_t>(34)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(3), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Array, containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(5)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(9)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(10)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType3));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));

    std::string jsonString   = "{\n"
                               "   \"0:ARRAY-STRUCT\" : [\n"
                               "      {\n"
                               "         \"0:INT\" : 8,\n"
                               "         \"1:BOOL\" : true\n"
                               "      }\n"
                               "   ],\n"
                               "   \"1:STRUCT\" : {\n"
                               "       \"0:INT\" : 12,\n"
                               "       \"1:BOOL\" : false,\n"
                               "       \"2:STRING\" : \"example\"\n"
                               "   },\n"
                               "   \"2:INT\" : \"40000000000\",\n"
                               "   \"isQualified:3:BOOL\" : true,\n"
                               "   \"4:ARRAY-?\" : [],\n"
                               "   \"5:ARRAY-DOUBLE\" : [\n"
                               "      1.1000000000000001,\n"
                               "      134.27629999999999,\n"
                               "      -12345.870000000001,\n"
                               "      \"Infinity\",\n"
                               "      62534.0,\n"
                               "      -62534.0\n"
                               "   ],\n"
                               "   \"6:ARRAY-BYTES\" : [\n"
                               "      \"AAECAwQ=\",\n"
                               "      \"/w==\",\n"
                               "      \"Su+I\"\n"
                               "   ],\n"
                               "   \"7:BYTES\" : \"VGVzdCBCeXRlcw==\",\n"
                               "   \"8:DOUBLE\" : 17.899999999999999,\n"
                               "   \"9:FLOAT\" : 17.899999618530273,\n"
                               "   \"10:FLOAT\" : \"-Infinity\",\n"
                               "   \"contact:11:STRUCT\" : {\n"
                               "      \"name:1:STRING\" : \"John\",\n"
                               "      \"age:2:UINT\" : 34,\n"
                               "      \"approved:3:BOOL\" : true,\n"
                               "      \"kids:4:ARRAY-INT\" : [\n"
                               "         5,\n"
                               "         9,\n"
                               "         10\n"
                               "      ]\n"
                               "   }\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"0:ARRAY-STRUCT\" : [\n"
                               "      {\n"
                               "         \"0:INT\" : 8,\n"
                               "         \"1:BOOL\" : true\n"
                               "      }\n"
                               "   ],\n"
                               "   \"1:STRUCT\" : {\n"
                               "       \"0:INT\" : 12,\n"
                               "       \"1:BOOL\" : false,\n"
                               "       \"2:STRING\" : \"example\"\n"
                               "   },\n"
                               "   \"2:INT\" : \"40000000000\",\n"
                               "   \"3:BOOL\" : true,\n"
                               "   \"4:ARRAY-?\" : [],\n"
                               "   \"5:ARRAY-DOUBLE\" : [\n"
                               "      1.1000000000000001,\n"
                               "      134.27629999999999,\n"
                               "      -12345.870000000001,\n"
                               "      \"Infinity\",\n"
                               "      62534.0,\n"
                               "      -62534.0\n"
                               "   ],\n"
                               "   \"6:ARRAY-BYTES\" : [\n"
                               "      \"AAECAwQ=\",\n"
                               "      \"/w==\",\n"
                               "      \"Su+I\"\n"
                               "   ],\n"
                               "   \"7:BYTES\" : \"VGVzdCBCeXRlcw==\",\n"
                               "   \"8:DOUBLE\" : 17.899999999999999,\n"
                               "   \"9:FLOAT\" : 17.899999618530273,\n"
                               "   \"10:FLOAT\" : \"-Infinity\",\n"
                               "   \"11:STRUCT\" : {\n"
                               "      \"1:STRING\" : \"John\",\n"
                               "      \"2:UINT\" : 34,\n"
                               "      \"3:BOOL\" : true,\n"
                               "      \"4:ARRAY-INT\" : [\n"
                               "         5,\n"
                               "         9,\n"
                               "         10\n"
                               "      ]\n"
                               "   }\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

TEST_F(TestJsonToTlvToJson, TestConverter_TlvToJson_ErrorCases)
{
    CHIP_ERROR err;
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    struct TestCase
    {
        const ByteSpan nEncodedTlv;
        CHIP_ERROR mExpectedResult;
        const char * mNameString;
    };

    uint8_t buf1[32];
    writer.Init(buf1);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::CommonTag(1), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());
    ByteSpan topLevelStructWithTag(buf1, writer.GetLengthWritten());

    uint8_t buf2[32];
    writer.Init(buf2);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());
    ByteSpan topLevelIsArray(buf2, writer.GetLengthWritten());

    uint8_t buf3[32];
    writer.Init(buf3);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_List, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ContextTag(1), true));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());
    ByteSpan usingList(buf3, writer.GetLengthWritten());

    uint8_t buf8[32];
    writer.Init(buf8);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(-170000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(42456)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());
    ByteSpan arrayWithMixedElements(buf8, writer.GetLengthWritten());

    uint8_t buf9[32];
    writer.Init(buf9);
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(0xFEED, 234), static_cast<uint64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());
    ByteSpan useFullyQualifiedTag(buf9, writer.GetLengthWritten());

    // clang-format off
    static const TestCase sTestCases[] = {
        // TLV Encoded Input       Expected Error                  Test Case String
        // ==================================================================================================
        {  topLevelStructWithTag,  CHIP_ERROR_INVALID_TLV_TAG,     "Top-Level Struct is Not Anonymous"     },
        {  topLevelIsArray,        CHIP_ERROR_WRONG_TLV_TYPE,      "Top-Level is an Array"                 },
        {  usingList,              CHIP_ERROR_INVALID_TLV_ELEMENT, "Using Unsupported List Type"           },
        {  arrayWithMixedElements, CHIP_ERROR_INVALID_TLV_ELEMENT, "Array with Mixed Elements"             },
        {  useFullyQualifiedTag,   CHIP_ERROR_INVALID_TLV_TAG,     "Using Unsupported Fully Qualified Tag" },
    };
    // clang-format on

    for (const auto & testCase : sTestCases)
    {
        std::string jsonString;
        err = TlvToJson(testCase.nEncodedTlv, jsonString);
        EXPECT_EQ(err, testCase.mExpectedResult);
    }
}

TEST_F(TestJsonToTlvToJson, TestConverter_JsonToTlv_ErrorCases)
{
    CHIP_ERROR err;

    struct TestCase
    {
        const std::string mJsonString;
        CHIP_ERROR mExpectedResult;
        const char * mNameString;
    };

    std::string arrayTypeStringExpectedBool = "{\n"
                                              "   \"value:1:ARRAY-BOOL\" : [\n"
                                              "      \"yes\",\n"
                                              "      \"no\"\n"
                                              "   ]\n"
                                              "}\n";

    std::string arrayTypeIntExpectedUInt = "{\n"
                                           "   \"value:1:ARRAY-UINT\" : [\n"
                                           "      45,\n"
                                           "      -367\n"
                                           "   ]\n"
                                           "}\n";

    std::string arrayElementsWithName = "{\n"
                                        "   \"value:1:ARRAY-INT\" : [\n"
                                        "      \"1:INT\" : 45,\n"
                                        "      \"2:INT\" : -367\n"
                                        "   ]\n"
                                        "}\n";

    std::string invalidNameWithoutTagField = "{\n"
                                             "   \"value:ARRAY-BYTES\" : [\n"
                                             "      \"AAECAwQ=\"\n"
                                             "   ]\n"
                                             "}\n";

    std::string invalidNameWithoutTagField2 = "{\n"
                                              "   \"UINT\" : 42\n"
                                              "}\n";

    std::string invalidNameWithNegativeTag = "{\n"
                                             "   \"-1:UINT\" : 42\n"
                                             "}\n";

    std::string invalidNameWithInvalidTypeField = "{\n"
                                                  "   \"1:INTEGER\" : 42\n"
                                                  "}\n";

    std::string invalidBytesBase64Value1 = "{\n"
                                           "   \"1:BYTES\" : \"SGVsbG8!\"\n"
                                           "}\n";

    std::string invalidBytesBase64Value2 = "{\n"
                                           "   \"1:BYTES\" : \"AAECw=Q\"\n"
                                           "}\n";

    std::string invalidBytesBase64Value3 = "{\n"
                                           "   \"1:BYTES\" : \"AAECwQ=\"\n"
                                           "}\n";

    std::string invalidBytesBase64Value4 = "{\n"
                                           "   \"1:BYTES\" : \"AAECwQ\"\n"
                                           "}\n";

    std::string invalidBytesBase64Padding1 = "{\n"
                                             "   \"1:BYTES\" : \"=\"\n"
                                             "}\n";

    std::string invalidBytesBase64Padding2 = "{\n"
                                             "   \"1:BYTES\" : \"==\"\n"
                                             "}\n";

    std::string invalidBytesBase64Padding3 = "{\n"
                                             "   \"1:BYTES\" : \"===\"\n"
                                             "}\n";

    std::string invalidPositiveInfinityValue = "{\n"
                                               "   \"1:DOUBLE\" : \"+Infinity\"\n"
                                               "}\n";

    std::string invalidFloatValueAsString = "{\n"
                                            "   \"1:FLOAT\" : \"1.1\"\n"
                                            "}\n";

    // clang-format off
    static const TestCase sTestCases[] = {
        // Json String                      Expected Error                Test Case String
        // ==========================================================================================================================
        {  arrayTypeStringExpectedBool,     CHIP_ERROR_INVALID_ARGUMENT,  "Array Type Is String While Bool Is Expected"             },
        {  arrayTypeIntExpectedUInt,        CHIP_ERROR_INVALID_ARGUMENT,  "Array Type Is Signed Integer While Unsigned Is Expected" },
        {  arrayElementsWithName,           CHIP_ERROR_INTERNAL,          "Array Elements With Json Name"                           },
        {  invalidNameWithoutTagField,      CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name String Without Tag Field"                   },
        {  invalidNameWithoutTagField2,     CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name String Without Tag Field 2"                 },
        {  invalidNameWithNegativeTag,      CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name With Negative Tag Value"                    },
        {  invalidNameWithInvalidTypeField, CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name With Invalid Type Field"                    },
        {  invalidBytesBase64Value1,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid Character"              },
        {  invalidBytesBase64Value2,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid Character"              },
        {  invalidBytesBase64Value3,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid padding (missing 1)"    },
        {  invalidBytesBase64Value4,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid padding (missing 2)"    },
        {  invalidBytesBase64Padding1,      CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid padding (start 1)"      },
        {  invalidBytesBase64Padding2,      CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid padding (start 2)"      },
        {  invalidBytesBase64Padding3,      CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid padding (start 3)"      },
        {  invalidPositiveInfinityValue,    CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Double Positive Infinity Encoding"               },
        {  invalidFloatValueAsString,       CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Float Value Encoding as a String"                },
    };
    // clang-format on

    for (const auto & testCase : sTestCases)
    {
        uint8_t buf[256];
        MutableByteSpan tlvSpan(buf);
        err = JsonToTlv(testCase.mJsonString, tlvSpan);
        EXPECT_EQ(err, testCase.mExpectedResult);
#if CHIP_CONFIG_ERROR_FORMAT_AS_STRING
        if (err != testCase.mExpectedResult)
        {
            std::string errStr{ err.Format() };
            std::string expectedErrStr{ testCase.mExpectedResult.Format() };

            printf("Case: %s, Error: %" CHIP_ERROR_FORMAT ", Expected: %" CHIP_ERROR_FORMAT ", Data: %s\n", testCase.mNameString,
                   errStr.c_str(), expectedErrStr.c_str(), testCase.mJsonString.c_str());
        }
#endif // CHIP_CONFIG_ERROR_FORMAT_AS_STRING
    }
}

// Full Qualified Profile tags, Unsigned Integer structure: {65536 = 42, 4294901760 = 17000, 4294967295 = 500000000000}
TEST_F(TestJsonToTlvToJson, TestConverter_Struct_MEITags)
{

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(0xFFFFu, 0, 0), static_cast<uint64_t>(17000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(0x0001u, 0, 0), static_cast<uint64_t>(42)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Put(TLV::ProfileTag(0xFFFFu, 0, 0xFFFFu), static_cast<uint64_t>(500000000000)));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType2));
    EXPECT_EQ(CHIP_NO_ERROR, writer.EndContainer(containerType));
    EXPECT_EQ(CHIP_NO_ERROR, writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:STRUCT\" : {\n"
                             "      \"65536:UINT\" : 42,\n"
                             "      \"4294901760:UINT\" : 17000,\n"
                             "      \"4294967295:UINT\" : \"500000000000\"\n"
                             "   }\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}
} // namespace
