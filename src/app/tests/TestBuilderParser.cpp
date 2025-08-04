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

#include <app/AppConfig.h>
#include <app/MessageDef/ArrayBuilder.h>
#include <app/MessageDef/ArrayParser.h>
#include <app/MessageDef/ListBuilder.h>
#include <app/MessageDef/ListParser.h>
#include <app/MessageDef/StructBuilder.h>
#include <app/MessageDef/StructParser.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip::app;

class TestBuilderParser : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestBuilderParser, TestList)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    ListBuilder listBuilder;
    EXPECT_EQ(listBuilder.Init(&writer), CHIP_NO_ERROR);
    listBuilder.EndOfContainer();

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);

    ListParser listParser;
    reader.Init(std::move(buf));
    err = reader.Next();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(listParser.Init(reader), CHIP_NO_ERROR);
}

TEST_F(TestBuilderParser, TestStruct)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    StructBuilder structBuilder;
    EXPECT_EQ(structBuilder.Init(&writer), CHIP_NO_ERROR);
    structBuilder.EndOfContainer();

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);

    StructParser structParser;
    reader.Init(std::move(buf));
    err = reader.Next();

    EXPECT_EQ(structParser.Init(reader), CHIP_NO_ERROR);
}

TEST_F(TestBuilderParser, TestArray)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    ArrayBuilder arrayBuilder;
    EXPECT_EQ(arrayBuilder.Init(&writer), CHIP_NO_ERROR);
    arrayBuilder.EndOfContainer();

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);

    ArrayParser arrayParser;
    reader.Init(std::move(buf));
    err = reader.Next();

    EXPECT_EQ(arrayParser.Init(reader), CHIP_NO_ERROR);
}

} // namespace
