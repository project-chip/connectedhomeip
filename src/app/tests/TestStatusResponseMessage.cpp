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

/**
 *    @file
 *      This file implements a test for  CHIP Interaction Model Message Def
 *
 */

#include <app/AppConfig.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

using namespace chip::app;
constexpr chip::Protocols::InteractionModel::Status statusValue        = chip::Protocols::InteractionModel::Status::Success;
constexpr chip::Protocols::InteractionModel::Status invalidStatusValue = chip::Protocols::InteractionModel::Status::Failure;

void BuildStatusResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusResponseMessage::Builder statusResponse;

    err = statusResponse.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    statusResponse.Status(statusValue);
    NL_TEST_ASSERT(apSuite, statusResponse.GetError() == CHIP_NO_ERROR);
}

void ParseStatusResponseMessage(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader, bool aTestPositiveCase)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    StatusResponseMessage::Parser statusResponse;
    chip::Protocols::InteractionModel::Status status;

    err = statusResponse.Init(aReader);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = statusResponse.CheckSchemaValidity();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
#endif

    err = statusResponse.GetStatus(status);
    if (aTestPositiveCase)
    {
        NL_TEST_ASSERT(apSuite, status == statusValue && err == CHIP_NO_ERROR);
    }
    else
    {
        NL_TEST_ASSERT(apSuite, status != invalidStatusValue && err == CHIP_NO_ERROR);
    }
}

void StatusResponseMessagePositiveTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildStatusResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    reader.Init(std::move(buf));
    ParseStatusResponseMessage(apSuite, reader, true /*aTestPositiveCase*/);
}

void StatusResponseMessageNegativeTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildStatusResponseMessage(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    reader.Init(std::move(buf));
    ParseStatusResponseMessage(apSuite, reader, false /*aTestPositiveCase*/);
}

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("StatusResponseMessagePositiveTest", StatusResponseMessagePositiveTest),
                NL_TEST_DEF("StatusResponseMessageNegativeTest", StatusResponseMessageNegativeTest),
                NL_TEST_SENTINEL()
        };
// clang-format on
} // namespace

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

int TestStatusResponseMessage()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "StatusResponseMessage",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestStatusResponseMessage)
