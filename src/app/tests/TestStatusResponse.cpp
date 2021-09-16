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

#include <app/AppBuildConfig.h>
#include <app/MessageDef/StatusResponse.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

using namespace chip::app;
constexpr chip::Protocols::InteractionModel::ProtocolCode statusValue = chip::Protocols::InteractionModel::ProtocolCode::Success;
constexpr chip::Protocols::InteractionModel::ProtocolCode invalidStatusValue =
    chip::Protocols::InteractionModel::ProtocolCode::Failure;

void BuildStatusResponse(nlTestSuite * apSuite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    StatusResponse::Builder statusResponse;

    err = statusResponse.Init(&aWriter);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    statusResponse.Status(statusValue);
    NL_TEST_ASSERT(apSuite, statusResponse.GetError() == CHIP_NO_ERROR);
}

void ParseStatusResponse(nlTestSuite * apSuite, chip::TLV::TLVReader & aReader, bool aTestPositiveCase)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    StatusResponse::Parser statusResponse;
    chip::Protocols::InteractionModel::ProtocolCode status;

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

void StatusResponsePositiveTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildStatusResponse(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseStatusResponse(apSuite, reader, true /*aTestPositiveCase*/);
}

void StatusResponseNegativeTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    BuildStatusResponse(apSuite, writer);
    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    ParseStatusResponse(apSuite, reader, false /*aTestPositiveCase*/);
}

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("StatusResponsePositiveTest", StatusResponsePositiveTest),
                NL_TEST_DEF("StatusResponseNegativeTest", StatusResponseNegativeTest),
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

int TestStatusResponse()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "StatusResponse",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestStatusResponse)
