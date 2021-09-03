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

/**
 *    @file
 *      This file implements a unit test suite for the additional payload generation
 *      code functionality.
 *
 */

#include <nlunit-test.h>
#include <stdio.h>

#include <setup_payload/AdditionalDataPayloadGenerator.cpp>
#include <setup_payload/AdditionalDataPayloadParser.cpp>
#include <setup_payload/SetupPayload.cpp>
#include <setup_payload/SetupPayload.h>
#include <system/SystemPacketBuffer.h>

#include <lib/support/UnitTestRegistration.h>
#include <lib/support/verhoeff/Verhoeff.h>

using namespace chip;

namespace {

void TestGeneratingAdditionalDataPayloadWithRotatingId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    char serialNumber[]      = "89051AAZZ236";
    uint16_t lifetimeCounter = 10;
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(lifetimeCounter, serialNumber, ArraySize(serialNumber),
                                                                         bufferHandle, additionalDataFields);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !bufferHandle.IsNull());

    char output[51];
    err = BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output, ArraySize(output));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, "5500003000120A00D1414E39C9A08469DA2E29F0ACAA086618") == 0);
}

void TestGeneratingAdditionalDataPayloadWithoutRotatingId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    char serialNumber[]      = "89051AAZZ236";
    uint16_t lifetimeCounter = 10;
    BitFlags<AdditionalDataFields> additionalDataFields;

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(lifetimeCounter, serialNumber, ArraySize(serialNumber),
                                                                         bufferHandle, additionalDataFields);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !bufferHandle.IsNull());

    char output[51];
    err = BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output, ArraySize(output));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, "55000018") == 0);
}

void TestGeneratingAdditionalDataPayloadWithRotatingIdAndParsingIt(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    char serialNumber[]      = "89051AAZZ236";
    uint16_t lifetimeCounter = 10;
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(lifetimeCounter, serialNumber, ArraySize(serialNumber),
                                                                         bufferHandle, additionalDataFields);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !bufferHandle.IsNull());

    char output[51];
    err = BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output, ArraySize(output));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, "5500003000120A00D1414E39C9A08469DA2E29F0ACAA086618") == 0);

    AdditionalDataPayload resultPayload;
    err = AdditionalDataPayloadParser(bufferHandle->Start(), bufferHandle->DataLength()).populatePayload(resultPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), "0A00D1414E39C9A08469DA2E29F0ACAA0866") == 0);
}

void TestGeneratingAdditionalDataPayloadWithoutRotatingIdAndParsingIt(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    char serialNumber[]      = "89051AAZZ236";
    uint16_t lifetimeCounter = 10;
    BitFlags<AdditionalDataFields> additionalDataFields;

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(lifetimeCounter, serialNumber, ArraySize(serialNumber),
                                                                         bufferHandle, additionalDataFields);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !bufferHandle.IsNull());

    char output[51];
    err = BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output, ArraySize(output));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, "55000018") == 0);

    AdditionalDataPayload resultPayload;
    err = AdditionalDataPayloadParser(bufferHandle->Start(), bufferHandle->DataLength()).populatePayload(resultPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), "") == 0);
}

/**
 *  Test Suite that lists all the Test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Generating Additional Data Payload with RotatingId", TestGeneratingAdditionalDataPayloadWithRotatingId),
    NL_TEST_DEF("Test Generating Additional Data Payload without RotatingId", TestGeneratingAdditionalDataPayloadWithoutRotatingId),
    NL_TEST_DEF("Test Generating Additional Data Payload with RotatingId + Parsing it", TestGeneratingAdditionalDataPayloadWithRotatingIdAndParsingIt),
    NL_TEST_DEF("Test Generating Additional Data Payload without RotatingId + Parsing it", TestGeneratingAdditionalDataPayloadWithoutRotatingIdAndParsingIt),
    NL_TEST_SENTINEL()
};
// clang-format on

struct TestContext
{
    nlTestSuite * mSuite;
};

} // namespace

/**
 *  Main
 */
int TestAdditionalDataPayload()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-manual-code-general-Tests",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on
    TestContext context;

    context.mSuite = &theSuite;

    // Generate machine-readable, comma-separated value (CSV) output.
    nl_test_set_output_style(OUTPUT_CSV);

    // Run Test suit against one context
    nlTestRunner(&theSuite, &context);

    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAdditionalDataPayload);
