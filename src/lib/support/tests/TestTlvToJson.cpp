/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
#include <lib/support/jsontlv/TextFormat.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace {

using namespace chip::Encoding;
using namespace chip;
using namespace chip::app;

System::TLVPacketBufferBackingStore gStore;
TLV::TLVWriter gWriter;
TLV::TLVReader gReader;

class TestTlvToJson : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite()
    {
        (void) gStore.Release();
        chip::Platform::MemoryShutdown();
    }
};

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

bool Matches(const std::string & referenceString, const std::string & generatedString)
{
    auto compactReferenceString = PrettyPrintJsonString(referenceString);
    auto compactGeneratedString = PrettyPrintJsonString(generatedString);

    auto matches = (compactGeneratedString == compactReferenceString);

    if (!matches)
    {
        printf("Didn't match!\n");
        printf("Reference:\n");
        printf("%s\n", compactReferenceString.c_str());

        printf("Generated:\n");
        printf("%s\n", compactGeneratedString.c_str());
    }

    return matches;
}

template <typename T>
void EncodeAndValidate(T val, const std::string & expectedJsonString)
{
    CHIP_ERROR err;
    TLV::TLVType container;

    SetupBuf();

    err = gWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = DataModel::Encode(gWriter, TLV::ContextTag(1), val);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = gWriter.EndContainer(container);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = gWriter.Finalize();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = SetupReader();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    std::string jsonString;
    err = TlvToJson(gReader, jsonString);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    bool matches = Matches(expectedJsonString, jsonString);
    EXPECT_TRUE(matches);
}

TEST_F(TestTlvToJson, TestConverter)
{
    std::string jsonString;

    jsonString = "{\n"
                 "   \"1:UINT\" : 30\n"
                 "}\n";
    EncodeAndValidate(static_cast<uint32_t>(30), jsonString);

    jsonString = "{\n"
                 "   \"1:INT\" : -30\n"
                 "}\n";
    EncodeAndValidate(static_cast<int32_t>(-30), jsonString);

    jsonString = "{\n"
                 "   \"1:BOOL\" : false\n"
                 "}\n";
    EncodeAndValidate(false, jsonString);

    jsonString = "{\n"
                 "   \"1:BOOL\" : true\n"
                 "}\n";
    EncodeAndValidate(true, jsonString);

    jsonString = "{\n"
                 "   \"1:FLOAT\" : 1.0\n"
                 "}\n";
    EncodeAndValidate(static_cast<float>(1.0), jsonString);

    jsonString = "{\n"
                 "   \"1:DOUBLE\" : 1.0\n"
                 "}\n";
    EncodeAndValidate(static_cast<double>(1.0), jsonString);

    CharSpan charSpan = "hello"_span;
    jsonString        = "{\n"
                        "   \"1:STRING\" : \"hello\"\n"
                        "}\n";
    EncodeAndValidate(charSpan, jsonString);

    // Validated using https://base64.guru/converter/encode/hex
    const uint8_t byteBuf[] = { 0x01, 0x02, 0x03, 0x04, 0xff, 0xfe, 0x99, 0x88, 0xdd, 0xcd };
    ByteSpan byteSpan(byteBuf);
    jsonString = "{\n"
                 "   \"1:BYTES\" : \"AQIDBP/+mYjdzQ==\"\n"
                 "}\n";
    EncodeAndValidate(byteSpan, jsonString);

    DataModel::Nullable<uint8_t> nullValue;
    jsonString = "{\n"
                 "   \"1:NULL\" : null\n"
                 "}\n";
    EncodeAndValidate(nullValue, jsonString);

    Clusters::UnitTesting::Structs::SimpleStruct::Type structVal;
    structVal.a = 20;
    structVal.b = true;
    structVal.d = byteBuf;
    structVal.e = charSpan;
    structVal.g = 1.0;
    structVal.h = 1.0;

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
    EncodeAndValidate(structVal, jsonString);

    uint8_t int8uListData[] = { 1, 2, 3, 4 };
    DataModel::List<uint8_t> int8uList;

    int8uList  = int8uListData;
    jsonString = "{\n"
                 "   \"1:ARRAY-UINT\" : [ 1, 2, 3, 4 ]\n"
                 "}\n";
    EncodeAndValidate(int8uList, jsonString);

    int8uList  = {};
    jsonString = "{\n"
                 "   \"1:ARRAY-?\" : [ ]\n"
                 "}\n";
    EncodeAndValidate(int8uList, jsonString);

    DataModel::Nullable<DataModel::List<uint8_t>> nullValueList;
    jsonString = "{\n"
                 "   \"1:NULL\" : null\n"
                 "}\n";
    EncodeAndValidate(nullValueList, jsonString);

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
    EncodeAndValidate(structList, jsonString);
}

} // namespace
