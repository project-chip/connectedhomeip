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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeValueDecoder.h>
#include <app/MessageDef/AttributeDataIB.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::TLV;

// TODO: This unit tests contains hard code TLV data, they should be replaced with some decoding code to improve readability.

namespace {

// These values are easier to be recognized in the encoded buffer
constexpr FabricIndex kTestFabricIndex = 1;
constexpr size_t kTestBufferSize       = 1024;
constexpr size_t kTestListElements     = 3;

struct TestSetup
{
    TestSetup() = default;

    template <typename T>
    CHIP_ERROR Encode(const T & value)
    {
        TLVType ignored;
        writer.Init(buf);
        ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, ignored));
        ReturnErrorOnFailure(DataModel::EncodeForWrite(writer, TLV::ContextTag(0), value));
        ReturnErrorOnFailure(writer.EndContainer(ignored));
        ReturnErrorOnFailure(writer.Finalize());
        return CHIP_NO_ERROR;
    }

    uint8_t buf[kTestBufferSize];
    TLVWriter writer;
};

TEST(TestAttributeValueDecoder, TestOverwriteFabricIndexInStruct)
{
    TestSetup setup;
    CHIP_ERROR err;
    Clusters::AccessControl::Structs::AccessControlExtensionStruct::Type item;
    Clusters::AccessControl::Structs::AccessControlExtensionStruct::DecodableType decodeItem;
    Access::SubjectDescriptor subjectDescriptor = { .fabricIndex = kTestFabricIndex };

    item.fabricIndex = 0;

    err = setup.Encode(item);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLV::TLVReader reader;
    TLVType ignored;
    reader.Init(setup.buf, setup.writer.GetLengthWritten());

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.EnterContainer(ignored);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    AttributeValueDecoder decoder(reader, subjectDescriptor);
    err = decoder.Decode(decodeItem);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(decodeItem.fabricIndex, kTestFabricIndex);
}

TEST(TestAttributeValueDecoder, TestOverwriteFabricIndexInListOfStructs)
{
    TestSetup setup;
    CHIP_ERROR err;
    Clusters::AccessControl::Structs::AccessControlExtensionStruct::Type items[kTestListElements];
    Clusters::AccessControl::Attributes::Extension::TypeInfo::DecodableType decodeItems;
    Access::SubjectDescriptor subjectDescriptor = { .fabricIndex = kTestFabricIndex };

    for (uint8_t i = 0; i < kTestListElements; i++)
    {
        items[i].fabricIndex = i;
    }

    err = setup.Encode(DataModel::List<Clusters::AccessControl::Structs::AccessControlExtensionStruct::Type>(items));
    EXPECT_EQ(err, CHIP_NO_ERROR);

    TLV::TLVReader reader;
    TLVType ignored;
    size_t decodeCount = 0;

    reader.Init(setup.buf, setup.writer.GetLengthWritten());

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.EnterContainer(ignored);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    AttributeValueDecoder decoder(reader, subjectDescriptor);
    err = decoder.Decode(decodeItems);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = decodeItems.ComputeSize(&decodeCount);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(decodeCount, kTestListElements);
    for (auto iter = decodeItems.begin(); iter.Next();)
    {
        const auto & entry = iter.GetValue();
        EXPECT_EQ(entry.fabricIndex, kTestFabricIndex);
    }
}

} // anonymous namespace
