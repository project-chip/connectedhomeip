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
#include <lib/support/jsontlv/TlvToJson.h>
#include <nlunit-test.h>

namespace {

using namespace chip::Encoding;
using namespace chip;
using namespace chip::app;

uint8_t gBuf1[1024];
uint8_t gBuf2[1024];
TLV::TLVWriter gWriter1;
TLV::TLVWriter gWriter2;
nlTestSuite * gSuite;

void SetupWriters()
{
    gWriter1.Init(gBuf1);
    gWriter2.Init(gBuf2);
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
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = DataModel::Encode(gWriter1, TLV::ContextTag(1), val);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = gWriter1.EndContainer(container);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = gWriter1.Finalize();
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = JsonToTlv(jsonString, gWriter2);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(gSuite, MatchWriter1and2());
}

void TestConverter(nlTestSuite * inSuite, void * inContext)
{
    std::string jsonString;

    gSuite = inSuite;

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
    ConvertJsonToTlvAndValidate(CharSpan::fromCharString("hello"), jsonString);

    // Validated using https://base64.guru/converter/encode/hex
    const uint8_t byteBuf[] = { 0x01, 0x02, 0x03, 0x04, 0xff, 0xfe, 0x99, 0x88, 0xdd, 0xcd };
    ByteSpan byteSpan(byteBuf);
    jsonString = "{\n"
                 "   \"1:BYTES\" : \"AQIDBP/+mYjdzQ==\"\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(byteSpan, jsonString);

    DataModel::Nullable<uint8_t> nullValue;
    jsonString = "{\n"
                 "   \"1:NULL\" : null\n"
                 "}\n";
    ConvertJsonToTlvAndValidate(nullValue, jsonString);

    Clusters::UnitTesting::Structs::SimpleStruct::Type structVal;
    structVal.a = 20;
    structVal.b = true;
    structVal.d = byteBuf;
    structVal.e = CharSpan::fromCharString("hello");
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

const nlTest sTests[] = { NL_TEST_DEF("TestConverter", TestConverter), NL_TEST_SENTINEL() };

} // namespace

int TestJsonToTlv()
{
    nlTestSuite theSuite = { "JsonToTlv", sTests, Initialize, Finalize };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestJsonToTlv)
