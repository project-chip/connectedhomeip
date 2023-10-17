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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TextFormat.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <nlunit-test.h>

namespace {

using namespace chip::Encoding;
using namespace chip;
using namespace chip::app;

constexpr uint32_t kImplicitProfileId = 0x1122;

nlTestSuite * gSuite;

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
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    match = tlvEncodingLocal.data_equal(tlvEncoding);
    NL_TEST_ASSERT(gSuite, match);
    if (!match)
    {
        printf("ERROR: TLV Encoding Doesn't Match!\n");
        PrintSpan("TLV Encoding Provided as Input for Reference:  ", tlvEncoding);
        PrintSpan("TLV Encoding Generated from Json Input String: ", tlvEncodingLocal);
    }

    std::string generatedJsonString;
    err = TlvToJson(tlvEncoding, generatedJsonString);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    auto compactExpectedString  = PrettyPrintJsonString(jsonExpected);
    auto compactGeneratedString = PrettyPrintJsonString(generatedJsonString);
    match                       = (compactGeneratedString == compactExpectedString);
    NL_TEST_ASSERT(gSuite, match);
    if (!match)
    {
        printf("ERROR: Json String Doesn't Match!\n");
        printf("Expected  Json String:\n%s\n", compactExpectedString.c_str());
        printf("Generated Json String:\n%s\n", compactGeneratedString.c_str());
    }

    // Verify that Expected Json String Converts to the Same TLV Encoding
    tlvEncodingLocal = MutableByteSpan(buf);
    err              = JsonToTlv(jsonOriginal, tlvEncodingLocal);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    match = tlvEncodingLocal.data_equal(tlvEncoding);
    NL_TEST_ASSERT(gSuite, match);
    if (!match)
    {
        printf("ERROR: TLV Encoding Doesn't Match!\n");
        PrintSpan("TLV Encoding Provided as Input for Reference:     ", tlvEncoding);
        PrintSpan("TLV Encoding Generated from Json Expected String: ", tlvEncodingLocal);
    }
}

// Boolean true
void TestConverter_Boolean_True(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:BOOL\" : true\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Integer 42, 1-octet
void TestConverter_SignedInt_1BytePositive(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<int8_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"2:INT\" : 42\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Integer -17, 1-octet
void TestConverter_SignedInt_1ByteNegative(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(3), static_cast<int8_t>(-17)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"3:INT\" : -17\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Unsigned Integer 42, 1-octet
void TestConverter_UnsignedInt_1Byte(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(4), static_cast<uint8_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_SignedInt_2Bytes(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(7), static_cast<int16_t>(4242)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"7:INT\" : 4242\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Integer -170000, 4-octet
void TestConverter_SignedInt_4Bytes(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(80), static_cast<int32_t>(-170000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"80:INT\" : -170000\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Signed Long Integer (int64_t) 40000000000, 8-octet
void TestConverter_SignedInt_8Bytes(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(202), static_cast<int64_t>(40000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"202:INT\" : \"40000000000\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Unsigned Long Integer (uint64_t) 40000000000, 8-octet
void TestConverter_UnsignedInt_8Bytes(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(222), static_cast<uint64_t>(40000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"222:UINT\" : \"40000000000\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// UTF-8 String, 1-octet length, "Hello!"
void TestConverter_UTF8String_Hello(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(0), "Hello!"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:STRING\" : \"Hello!\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Octet String, 1-octet length, octets { 00 01 02 03 04 }
void TestConverter_OctetString(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t v[] = { 0, 1, 2, 3, 4 };
    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutBytes(TLV::ContextTag(1), v, sizeof(v)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:BYTES\" : \"AAECAwQ=\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Null
void TestConverter_Null(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutNull(TLV::ContextTag(1)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:NULL\" : null\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Single precision floating point 0.0
void TestConverter_Float_0(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), static_cast<float>(0.0)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:FLOAT\" : 0.0\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Single precision floating point (1.0 / 3.0)
void TestConverter_Float_1third(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(100), static_cast<float>(1.0 / 3.0)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Float_17_9(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(101), static_cast<float>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Float_PositiveInfinity(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(101), std::numeric_limits<float>::infinity()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:FLOAT\" : \"Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Single precision floating point negative infinity
void TestConverter_Float_NegativeInfinity(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(101), -std::numeric_limits<float>::infinity()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:FLOAT\" : \"-Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point 0.0
void TestConverter_Double_0(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), static_cast<double>(0.0)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:DOUBLE\" : 0.0\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point (1.0 / 3.0)
void TestConverter_Double_1third(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(100), static_cast<double>(1.0 / 3.0)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"100:DOUBLE\" : 0.33333333333333331\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point 17.9
void TestConverter_Double_17_9(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(101), static_cast<double>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:DOUBLE\" : 17.899999999999999\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point positive infinity
void TestConverter_Double_PositiveInfinity(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(101), std::numeric_limits<double>::infinity()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:DOUBLE\" : \"Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Double precision floating point negative infinity
void TestConverter_Double_NegativeInfinity(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(101), -std::numeric_limits<double>::infinity()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"101:DOUBLE\" : \"-Infinity\"\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Empty Top-Level Structure, {}
void TestConverter_Structure_TopLevelEmpty(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{ }";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Empty Nested Structure, { {} }
void TestConverter_Structure_NestedEmpty(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:STRUCT\" : { }\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Empty Array, { [] }
void TestConverter_Array_Empty(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1:ARRAY-?\" : []\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

void TestConverter_Array_Empty_ImplicitProfileTag2(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;

    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite,
                   CHIP_NO_ERROR ==
                       writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 10000), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString   = "{\n"
                               "   \"emptyarray:10000:ARRAY-?\" : []\n"
                               "}\n";
    std::string jsonExpected = "{\n"
                               "   \"10000:ARRAY-?\" : []\n"
                               "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonExpected);
}

void TestConverter_Array_Empty_ImplicitProfileTag4(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;

    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite,
                   CHIP_NO_ERROR ==
                       writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 1000000), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"1000000:ARRAY-?\" : []\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Two Signed Integers with context specific tags: {0 = 42, 1 = -17}
void TestConverter_IntsWithContextTags(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int8_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), static_cast<int8_t>(-17)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

    std::string jsonString = "{\n"
                             "   \"0:INT\" : 42,\n"
                             "   \"1:INT\" : -17\n"
                             "}\n";

    ByteSpan tlvSpan(buf, writer.GetLengthWritten());
    CheckValidConversion(jsonString, tlvSpan, jsonString);
}

// Structure with Two Signed Integers with context specific tags: { {0 = 42, 1 = -17} }
void TestConverter_Struct_IntsWithContextTags(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int8_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), static_cast<int8_t>(-17)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Ints(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(0)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(1)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(2)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(3)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(4)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Ints2(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(-17)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(-170000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(40000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_IntsMinMax(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT8_MIN)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT8_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT16_MIN)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT16_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT32_MIN)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT32_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT64_MIN)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(INT64_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_UInts(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(170000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(40000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_UIntsMax(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint8_t>(UINT8_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint16_t>(UINT16_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(UINT32_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(UINT64_MAX)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Doubles(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(1.1)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(134.2763)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(-12345.87)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Floats(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite,
                   CHIP_NO_ERROR ==
                       writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 1000), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<float>(1.1)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<float>(134.2763)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<float>(-12345.87)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Strings(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite,
                   CHIP_NO_ERROR ==
                       writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 100000), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::AnonymousTag(), "ABC"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::AnonymousTag(), "Options"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::AnonymousTag(), "more"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Booleans(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(255), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), false));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), false));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Nulls(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutNull(TLV::AnonymousTag()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutNull(TLV::AnonymousTag()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Struct_UInt(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(255), static_cast<uint8_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Struct_MixedTags(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(255), static_cast<uint64_t>(42)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ProfileTag(kImplicitProfileId, 256), static_cast<uint64_t>(17000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ProfileTag(kImplicitProfileId, 65535), static_cast<uint64_t>(1)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ProfileTag(kImplicitProfileId, 65536), static_cast<uint64_t>(345678)));
    NL_TEST_ASSERT(
        gSuite, CHIP_NO_ERROR == writer.Put(TLV::ProfileTag(kImplicitProfileId, 4294967295), static_cast<uint64_t>(500000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Struct_MixedElements(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    char bytes[] = "Test ByteString Value";

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int64_t>(20)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(0)));
    NL_TEST_ASSERT(
        gSuite,
        CHIP_NO_ERROR ==
            writer.PutBytes(TLV::ContextTag(3), reinterpret_cast<uint8_t *>(bytes), static_cast<uint32_t>(strlen(bytes))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(4), "hello"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(5), static_cast<int64_t>(-500000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(6), static_cast<double>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(7), static_cast<float>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Array_Structures(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    TLV::TLVType containerType3;
    char bytes1[] = "Test ByteString Value 1";
    char bytes2[] = "Test ByteString Value 2";

    writer.Init(buf);
    writer.ImplicitProfileId = kImplicitProfileId;

    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite,
                   CHIP_NO_ERROR ==
                       writer.StartContainer(TLV::ProfileTag(kImplicitProfileId, 1000), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int64_t>(20)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(0)));
    NL_TEST_ASSERT(
        gSuite,
        CHIP_NO_ERROR ==
            writer.PutBytes(TLV::ContextTag(3), reinterpret_cast<uint8_t *>(bytes1), static_cast<uint32_t>(strlen(bytes1))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(4), "hello1"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(5), static_cast<int64_t>(-500000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(6), static_cast<double>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(7), static_cast<float>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int64_t>(-10)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), false));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(128)));
    NL_TEST_ASSERT(
        gSuite,
        CHIP_NO_ERROR ==
            writer.PutBytes(TLV::ContextTag(3), reinterpret_cast<uint8_t *>(bytes2), static_cast<uint32_t>(strlen(bytes2))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(4), "hello2"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(5), static_cast<int64_t>(40000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(6), static_cast<double>(-1754.923)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(7), static_cast<float>(97.945)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_TopLevel_MixedElements(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    uint8_t buf[256];
    TLV::TLVWriter writer;
    TLV::TLVType containerType;
    TLV::TLVType containerType2;
    TLV::TLVType containerType3;
    char bytes[] = "Test array member 0";

    writer.Init(buf);
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int64_t>(42)));
    NL_TEST_ASSERT(
        gSuite,
        CHIP_NO_ERROR ==
            writer.PutBytes(TLV::ContextTag(1), reinterpret_cast<uint8_t *>(bytes), static_cast<uint32_t>(strlen(bytes))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<double>(156.398)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(3), static_cast<uint64_t>(73709551615)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(4), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutNull(TLV::ContextTag(5)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(6), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(1), "John"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<uint64_t>(34)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(3), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Array, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(5)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(10)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(5), TLV::kTLVType_Array, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::AnonymousTag(), "Ammy"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::AnonymousTag(), "David"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::AnonymousTag(), "Larry"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(6), TLV::kTLVType_Array, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), false));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(7), static_cast<float>(0.0)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Finalize());

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
void TestConverter_Structure_FromReadme(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

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
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(0), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int8_t>(8)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(0), static_cast<int8_t>(12)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), false));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(2), "example"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<int64_t>(40000000000)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(3), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(5), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(1.1)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(134.2763)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(-12345.87)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), std::numeric_limits<double>::infinity()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(62534)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<double>(-62534)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(6), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutBytes(TLV::AnonymousTag(), bytes1, static_cast<uint32_t>(sizeof(bytes1))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutBytes(TLV::AnonymousTag(), bytes2, static_cast<uint32_t>(sizeof(bytes2))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutBytes(TLV::AnonymousTag(), bytes3, static_cast<uint32_t>(sizeof(bytes3))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(
        gSuite,
        CHIP_NO_ERROR ==
            writer.PutBytes(TLV::ContextTag(7), reinterpret_cast<uint8_t *>(bytes4), static_cast<uint32_t>(strlen(bytes4))));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(8), static_cast<double>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(9), static_cast<float>(17.9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(10), -std::numeric_limits<float>::infinity()));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(11), TLV::kTLVType_Structure, containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.PutString(TLV::ContextTag(1), "John"));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(2), static_cast<uint32_t>(34)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(3), true));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(4), TLV::kTLVType_Array, containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(5)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(9)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int8_t>(10)));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType3));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(gSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));

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

void TestConverter_TlvToJson_ErrorCases(nlTestSuite * inSuite, void * inContext)
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
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::CommonTag(1), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), true));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Finalize());
    ByteSpan topLevelStructWithTag(buf1, writer.GetLengthWritten());

    uint8_t buf2[32];
    writer.Init(buf2);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), true));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Finalize());
    ByteSpan topLevelIsArray(buf2, writer.GetLengthWritten());

    uint8_t buf3[32];
    writer.Init(buf3);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_List, containerType2));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::ContextTag(1), true));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Finalize());
    ByteSpan usingList(buf3, writer.GetLengthWritten());

    uint8_t buf8[32];
    writer.Init(buf8);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::ContextTag(1), TLV::kTLVType_Array, containerType2));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(42)));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<int64_t>(-170000)));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::AnonymousTag(), static_cast<uint64_t>(42456)));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType2));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Finalize());
    ByteSpan arrayWithMixedElements(buf8, writer.GetLengthWritten());

    uint8_t buf9[32];
    writer.Init(buf9);
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Put(TLV::ProfileTag(0xAA55FEED, 234), static_cast<uint64_t>(42)));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.EndContainer(containerType));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == writer.Finalize());
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
        NL_TEST_ASSERT(inSuite, err == testCase.mExpectedResult);
    }
}

void TestConverter_JsonToTlv_ErrorCases(nlTestSuite * inSuite, void * inContext)
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

    std::string invalidNameTagValueTooBig = "{\n"
                                            "   \"invalid:4294967296:UINT\" : 42\n"
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
        {  invalidNameTagValueTooBig,       CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name String Tag Value Larger than UINT32_MAX"    },
        {  invalidNameWithNegativeTag,      CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name With Negative Tag Value"                    },
        {  invalidNameWithInvalidTypeField, CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Name With Invalid Type Field"                    },
        {  invalidBytesBase64Value1,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid Character"              },
        {  invalidBytesBase64Value2,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid Character"              },
        {  invalidBytesBase64Value3,        CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Base64 Encoding: Invalid length"                 },
        {  invalidPositiveInfinityValue,    CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Double Positive Infinity Encoding"               },
        {  invalidFloatValueAsString,       CHIP_ERROR_INVALID_ARGUMENT,  "Invalid Float Value Encoding as a String"                },
    };
    // clang-format on

    for (const auto & testCase : sTestCases)
    {
        uint8_t buf[256];
        MutableByteSpan tlvSpan(buf);
        err = JsonToTlv(testCase.mJsonString, tlvSpan);
        NL_TEST_ASSERT(inSuite, err == testCase.mExpectedResult);
    }
}

int Initialize(void * apSuite)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int Finalize(void * aContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

const nlTest sTests[] = {
    NL_TEST_DEF("Test Json Tlv Converter - Boolean True", TestConverter_Boolean_True),
    NL_TEST_DEF("Test Json Tlv Converter - Signed Integer 1-Byte Positive", TestConverter_SignedInt_1BytePositive),
    NL_TEST_DEF("Test Json Tlv Converter - Signed Integer 1-Byte Negative", TestConverter_SignedInt_1ByteNegative),
    NL_TEST_DEF("Test Json Tlv Converter - Unsigned Integer 1-Byte", TestConverter_UnsignedInt_1Byte),
    NL_TEST_DEF("Test Json Tlv Converter - Signed Integer 2-Bytes", TestConverter_SignedInt_2Bytes),
    NL_TEST_DEF("Test Json Tlv Converter - Signed Integer 4-Bytes", TestConverter_SignedInt_4Bytes),
    NL_TEST_DEF("Test Json Tlv Converter - Signed Integer 8-Bytes", TestConverter_SignedInt_8Bytes),
    NL_TEST_DEF("Test Json Tlv Converter - Unsigned Integer 8-Bytes", TestConverter_UnsignedInt_8Bytes),
    NL_TEST_DEF("Test Json Tlv Converter - UTF-8 String Hello!", TestConverter_UTF8String_Hello),
    NL_TEST_DEF("Test Json Tlv Converter - Octet String", TestConverter_OctetString),
    NL_TEST_DEF("Test Json Tlv Converter - Null", TestConverter_Null),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Single Precision 0.0", TestConverter_Float_0),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Single Precision 1/3", TestConverter_Float_1third),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Single Precision 17.9", TestConverter_Float_17_9),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Single Precision Positive Infinity",
                TestConverter_Float_PositiveInfinity),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Single Precision Negative Infinity",
                TestConverter_Float_NegativeInfinity),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Double Precision 0.0", TestConverter_Double_0),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Double Precision 1/3", TestConverter_Double_1third),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Double Precision 17.9", TestConverter_Double_17_9),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Double Precision Positive Infinity",
                TestConverter_Double_PositiveInfinity),
    NL_TEST_DEF("Test Json Tlv Converter - Floating Point Double Precision Negative Infinity",
                TestConverter_Double_NegativeInfinity),
    NL_TEST_DEF("Test Json Tlv Converter - Structure Top-Level Empty", TestConverter_Structure_TopLevelEmpty),
    NL_TEST_DEF("Test Json Tlv Converter - Structure Nested Empty", TestConverter_Structure_NestedEmpty),
    NL_TEST_DEF("Test Json Tlv Converter - Array Empty", TestConverter_Array_Empty),
    NL_TEST_DEF("Test Json Tlv Converter - Array Empty with Implicit Profile Tag (length 2)",
                TestConverter_Array_Empty_ImplicitProfileTag2),
    NL_TEST_DEF("Test Json Tlv Converter - Array Empty with Implicit Profile Tag (length 4)",
                TestConverter_Array_Empty_ImplicitProfileTag4),
    NL_TEST_DEF("Test Json Tlv Converter - Two Signed Integers", TestConverter_IntsWithContextTags),
    NL_TEST_DEF("Test Json Tlv Converter - Structure With Two Signed Integers", TestConverter_Struct_IntsWithContextTags),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Signed Integers", TestConverter_Array_Ints),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Long Signed Integers", TestConverter_Array_Ints2),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Min/Max Signed Integers", TestConverter_Array_IntsMinMax),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Long Unsigned Integers", TestConverter_Array_UInts),
    NL_TEST_DEF("Test Json Tlv Converter - Array of Unsigned Integers with Max values", TestConverter_Array_UIntsMax),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Doubles", TestConverter_Array_Doubles),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Floats", TestConverter_Array_Floats),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Strings", TestConverter_Array_Strings),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Booleans", TestConverter_Array_Booleans),
    NL_TEST_DEF("Test Json Tlv Converter - Array Of Nulls", TestConverter_Array_Nulls),
    NL_TEST_DEF("Test Json Tlv Converter - Structure with Unsigned Integer", TestConverter_Struct_UInt),
    NL_TEST_DEF("Test Json Tlv Converter - Structure Elements with Mixed Tags", TestConverter_Struct_MixedTags),
    NL_TEST_DEF("Test Json Tlv Converter - Structure with Mixed Elements", TestConverter_Struct_MixedElements),
    NL_TEST_DEF("Test Json Tlv Converter - Array of Structures with Mixed Elements", TestConverter_Array_Structures),
    NL_TEST_DEF("Test Json Tlv Converter - Top-Level Structure with Mixed Elements", TestConverter_TopLevel_MixedElements),
    NL_TEST_DEF("Test Json Tlv Converter - Complex Structure from the README File", TestConverter_Structure_FromReadme),
    NL_TEST_DEF("Test Json Tlv Converter - Tlv to Json Error Cases", TestConverter_TlvToJson_ErrorCases),
    NL_TEST_DEF("Test Json Tlv Converter - Json To Tlv Error Cases", TestConverter_JsonToTlv_ErrorCases),
    NL_TEST_SENTINEL()
};

} // namespace

int TestJsonToTlvToJson()
{
    nlTestSuite theSuite = { "JsonToTlvToJson", sTests, Initialize, Finalize };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestJsonToTlvToJson)
