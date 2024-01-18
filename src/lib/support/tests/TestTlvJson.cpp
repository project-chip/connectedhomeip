/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/jsontlv/TlvJson.h>
#include <nlunit-test.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace {

using namespace chip::Encoding;
using namespace chip;
using namespace chip::app;

System::TLVPacketBufferBackingStore gStore;
TLV::TLVWriter gWriter;
TLV::TLVReader gReader;
nlTestSuite * gSuite;

void SetupBuf()
{
    System::PacketBufferHandle buf;

    buf = System::PacketBufferHandle::New(1024);
    gStore.Init(std::move(buf));

    gWriter.Init(gStore);
    gReader.Init(gStore);
}

CHIP_ERROR SetupReader()
{
    gReader.Init(gStore);
    return gReader.Next();
}

bool Matches(const char * referenceString, Json::Value & generatedValue)
{
    auto generatedStr = JsonToString(generatedValue);

    // Normalize the reference string to the expected compact value.
    Json::Reader reader;
    Json::Value referenceValue;
    reader.parse(referenceString, referenceValue);

    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    auto compactReferenceString = writer.write(referenceValue);

    auto matches = (generatedStr == compactReferenceString);

    if (!matches)
    {
        printf("Didn't match!\n");
        printf("Reference:\n");
        printf("%s\n", compactReferenceString.c_str());

        printf("Generated:\n");
        printf("%s\n", generatedStr.c_str());
    }

    return matches;
}

template <typename T>
void EncodeAndValidate(T val, const char * expectedJsonString)
{
    CHIP_ERROR err;

    SetupBuf();

    err = DataModel::Encode(gWriter, TLV::AnonymousTag(), val);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = gWriter.Finalize();
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    err = SetupReader();
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    Json::Value d;
    err = TlvToJson(gReader, d);
    NL_TEST_ASSERT(gSuite, err == CHIP_NO_ERROR);

    bool matches = Matches(expectedJsonString, d);
    NL_TEST_ASSERT(gSuite, matches);
}

void TestConverter(nlTestSuite * inSuite, void * inContext)
{
    gSuite = inSuite;

    EncodeAndValidate(static_cast<uint32_t>(30),
                      "{\n"
                      "   \"value\" : 30\n"
                      "}\n");

    EncodeAndValidate(static_cast<int32_t>(-30),
                      "{\n"
                      "   \"value\" : -30\n"
                      "}\n");

    EncodeAndValidate(false,
                      "{\n"
                      "   \"value\" : false\n"
                      "}\n");

    EncodeAndValidate(true,
                      "{\n"
                      "   \"value\" : true\n"
                      "}\n");

    EncodeAndValidate(1.0,
                      "{\n"
                      "   \"value\" : 1.0\n"
                      "}\n");

    CharSpan charSpan = "hello"_span;
    EncodeAndValidate(charSpan,
                      "{\n"
                      "   \"value\" : \"hello\"\n"
                      "}\n");

    //
    // Validated using https://base64.guru/converter/encode/hex
    //
    const uint8_t byteBuf[] = { 0x01, 0x02, 0x03, 0x04, 0xff, 0xfe, 0x99, 0x88, 0xdd, 0xcd };
    ByteSpan byteSpan(byteBuf);
    EncodeAndValidate(byteSpan,
                      "{\n"
                      "   \"value\" : \"base64:AQIDBP/+mYjdzQ==\"\n"
                      "}\n");

    DataModel::Nullable<uint8_t> nullValue;
    EncodeAndValidate(nullValue,
                      "{\n"
                      "   \"value\" : null\n"
                      "}\n");

    Clusters::UnitTesting::Structs::SimpleStruct::Type structVal;
    structVal.a = 20;
    structVal.b = true;
    structVal.d = byteBuf;
    structVal.e = charSpan;
    structVal.g = 1.0;
    structVal.h = 1.0;

    EncodeAndValidate(structVal,
                      "{\n"
                      "   \"value\" : {\n"
                      "      \"0\" : 20,\n"
                      "      \"1\" : true,\n"
                      "      \"2\" : 0,\n"
                      "      \"3\" : \"base64:AQIDBP/+mYjdzQ==\",\n"
                      "      \"4\" : \"hello\",\n"
                      "      \"5\" : 0,\n"
                      "      \"6\" : 1.0,\n"
                      "      \"7\" : 1.0\n"
                      "   }\n"
                      "}\n");

    uint8_t int8uListData[] = { 1, 2, 3, 4 };
    DataModel::List<uint8_t> int8uList;

    int8uList = int8uListData;

    EncodeAndValidate(int8uList,
                      "{\n"
                      "   \"value\" : [ 1, 2, 3, 4 ]\n"
                      "}\n");

    int8uList = {};
    EncodeAndValidate(int8uList,
                      "{\n"
                      "   \"value\" : []\n"
                      "}\n");

    DataModel::Nullable<DataModel::List<uint8_t>> nullValueList;
    EncodeAndValidate(nullValueList,
                      "{\n"
                      "   \"value\" : null\n"
                      "}\n");

    Clusters::UnitTesting::Structs::SimpleStruct::Type structListData[2] = { structVal, structVal };
    DataModel::List<Clusters::UnitTesting::Structs::SimpleStruct::Type> structList;

    structList = structListData;

    EncodeAndValidate(structList,
                      "{\n"
                      "   \"value\" : [\n"
                      "      {\n"
                      "         \"0\" : 20,\n"
                      "         \"1\" : true,\n"
                      "         \"2\" : 0,\n"
                      "         \"3\" : \"base64:AQIDBP/+mYjdzQ==\",\n"
                      "         \"4\" : \"hello\",\n"
                      "         \"5\" : 0,\n"
                      "         \"6\" : 1.0,\n"
                      "         \"7\" : 1.0\n"
                      "      },\n"
                      "      {\n"
                      "         \"0\" : 20,\n"
                      "         \"1\" : true,\n"
                      "         \"2\" : 0,\n"
                      "         \"3\" : \"base64:AQIDBP/+mYjdzQ==\",\n"
                      "         \"4\" : \"hello\",\n"
                      "         \"5\" : 0,\n"
                      "         \"6\" : 1.0,\n"
                      "         \"7\" : 1.0\n"
                      "      }\n"
                      "   ]\n"
                      "}\n");
}

int Initialize(void * apSuite)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int Finalize(void * aContext)
{
    (void) gStore.Release();
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

const nlTest sTests[] = { NL_TEST_DEF("TestConverter", TestConverter), NL_TEST_SENTINEL() };

} // namespace

int TestTlvJson()
{
    nlTestSuite theSuite = { "TlvJson", sTests, Initialize, Finalize };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTlvJson)
