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
#include <lib/support/UnitTestRegistration.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <nlunit-test.h>

namespace {

using namespace chip::app;

void ListTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    ListBuilder listBuilder;
    NL_TEST_ASSERT(apSuite, listBuilder.Init(&writer) == CHIP_NO_ERROR);
    listBuilder.EndOfContainer();

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);

    ListParser listParser;
    reader.Init(std::move(buf));
    err = reader.Next();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(apSuite, listParser.Init(reader) == CHIP_NO_ERROR);
}

void StructTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    StructBuilder structBuilder;
    NL_TEST_ASSERT(apSuite, structBuilder.Init(&writer) == CHIP_NO_ERROR);
    structBuilder.EndOfContainer();

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);

    StructParser structParser;
    reader.Init(std::move(buf));
    err = reader.Next();

    NL_TEST_ASSERT(apSuite, structParser.Init(reader) == CHIP_NO_ERROR);
}

void ArrayTest(nlTestSuite * apSuite, void * apContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVWriter writer;
    chip::System::PacketBufferTLVReader reader;
    writer.Init(chip::System::PacketBufferHandle::New(chip::System::PacketBuffer::kMaxSize));
    ArrayBuilder arrayBuilder;
    NL_TEST_ASSERT(apSuite, arrayBuilder.Init(&writer) == CHIP_NO_ERROR);
    arrayBuilder.EndOfContainer();

    chip::System::PacketBufferHandle buf;
    err = writer.Finalize(&buf);

    ArrayParser arrayParser;
    reader.Init(std::move(buf));
    err = reader.Next();

    NL_TEST_ASSERT(apSuite, arrayParser.Init(reader) == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
        {
                NL_TEST_DEF("ListTest", ListTest),
                NL_TEST_DEF("StructTest", StructTest),
                NL_TEST_DEF("ArrayTest", ArrayTest),
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

int TestBuilderParser()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "TestBuilderParser",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };
    // clang-format on

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestBuilderParser)
