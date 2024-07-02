/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/AppConfig.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <system/TLVPacketBufferBackingStore.h>

namespace {

using namespace chip::app;
constexpr chip::Protocols::InteractionModel::Status statusValue        = chip::Protocols::InteractionModel::Status::Success;
constexpr chip::Protocols::InteractionModel::Status invalidStatusValue = chip::Protocols::InteractionModel::Status::Failure;

class TestStatusResponseMessage : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

void BuildStatusResponseMessage(chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusResponseMessage::Builder statusResponse;

    err = statusResponse.Init(&aWriter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    statusResponse.Status(statusValue);
    EXPECT_EQ(statusResponse.GetError(), CHIP_NO_ERROR);
}

void ParseStatusResponseMessage(chip::TLV::TLVReader & aReader, bool aTestPositiveCase)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    StatusResponseMessage::Parser statusResponse;
    chip::Protocols::InteractionModel::Status status;

    err = statusResponse.Init(aReader);
    EXPECT_EQ(err, CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    statusResponse.PrettyPrint();
#endif

    err = statusResponse.GetStatus(status);
    if (aTestPositiveCase)
    {
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(status, statusValue);
    }
    else
    {
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_NE(status, invalidStatusValue);
    }
}

TEST_F(TestStatusResponseMessage, TestStatusResponseMessagePositive)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildStatusResponseMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    reader.Init(std::move(buf));
    ParseStatusResponseMessage(reader, true /*aTestPositiveCase*/);
}

TEST_F(TestStatusResponseMessage, TestStatusResponseMessageNegative)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildStatusResponseMessage(writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    reader.Init(std::move(buf));
    ParseStatusResponseMessage(reader, false /*aTestPositiveCase*/);
}

} // namespace
