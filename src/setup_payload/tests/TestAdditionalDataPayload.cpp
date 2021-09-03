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

constexpr char kSerialNumber[] = "89051AAZZ236";
constexpr char kAdditionalDataPayloadWithoutRotatingDeviceId[] = "55000018";
constexpr char kAdditionalDataPayloadWithRotatingDeviceId[] = "5500003000120A001998AB7130E38B7E9A401CFE9F7B79AF18";
constexpr char kAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter[] = "550000300012FFFFFC1670A9F9666D1C4587FCBC4811549018";
constexpr char kRotatingDeviceId[] = "0A001998AB7130E38B7E9A401CFE9F7B79AF";
constexpr uint16_t kLifetimeCounter = 10;
constexpr uint16_t kAdditionalDataPayloadLength = 51;

void GenerateAdditionalDataPayload(nlTestSuite * inSuite, uint16_t lifetimeCounter, const char * serialNumberBuffer,
                                     size_t serialNumberBufferSize, BitFlags<AdditionalDataFields> additionalDataFields,
                                     char * additionalDataPayloadOutput)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(lifetimeCounter, serialNumberBuffer, serialNumberBufferSize,
                                                                         bufferHandle, additionalDataFields);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !bufferHandle.IsNull());

    char output[kAdditionalDataPayloadLength];
    err = BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output, ArraySize(output));

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    memmove(additionalDataPayloadOutput, output, kAdditionalDataPayloadLength);
}

AdditionalDataPayload ParseAdditionalDataPayload(nlTestSuite * inSuite, const char * additionalDataPayload)
{
    std::vector<uint8_t> payloadData;
    AdditionalDataPayload resultPayload;
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::string payloadString(additionalDataPayload);

    // Decode input payload
    size_t len = payloadString.length();

    for (size_t i = 0; i < len; i += 2)
    {
        auto str  = payloadString.substr(i, 2);
        uint8_t x = (uint8_t) stoi(str, 0, 16);
        payloadData.push_back(x);
    }

    err = AdditionalDataPayloadParser(payloadData.data(), (uint32_t) payloadData.size()).populatePayload(resultPayload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    return resultPayload;
}

void TestGeneratingAdditionalDataPayloadWithRotatingId(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);

    char output[kAdditionalDataPayloadLength];
    GenerateAdditionalDataPayload(inSuite, kLifetimeCounter, kSerialNumber, strlen(kSerialNumber),
                                  additionalDataFields, output);

    NL_TEST_ASSERT(inSuite, strcmp(output, kAdditionalDataPayloadWithRotatingDeviceId) == 0);
}

void TestGeneratingAdditionalDataPayloadWithRotatingIdAndMaxLifetimeCounter(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);

    char output[kAdditionalDataPayloadLength];
    GenerateAdditionalDataPayload(inSuite, std::numeric_limits<uint16_t>::max(),
                                  kSerialNumber, strlen(kSerialNumber),
                                  additionalDataFields, output);

    NL_TEST_ASSERT(inSuite, strcmp(output, kAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter) == 0);
}

void TestGeneratingAdditionalDataPayloadWithoutRotatingId(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    char output[kAdditionalDataPayloadLength];
    GenerateAdditionalDataPayload(inSuite, kLifetimeCounter, kSerialNumber, strlen(kSerialNumber),
                                  additionalDataFields, output);
    NL_TEST_ASSERT(inSuite, strcmp(output, kAdditionalDataPayloadWithoutRotatingDeviceId) == 0);
}

void TestGeneratingAdditionalDataPayloadWithRotatingIdAndParsingIt(nlTestSuite * inSuite, void * inContext)
{
    AdditionalDataPayload resultPayload = ParseAdditionalDataPayload(inSuite, kAdditionalDataPayloadWithRotatingDeviceId);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), kRotatingDeviceId) == 0);
}

void TestGeneratingAdditionalDataPayloadWithoutRotatingIdAndParsingIt(nlTestSuite * inSuite, void * inContext)
{
    AdditionalDataPayload resultPayload = ParseAdditionalDataPayload(inSuite, kAdditionalDataPayloadWithoutRotatingDeviceId);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), "") == 0);
}

/**
 *  Test Suite that lists all the Test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Generating Additional Data Payload with RotatingId", TestGeneratingAdditionalDataPayloadWithRotatingId),
    NL_TEST_DEF("Test Generating Additional Data Payload with RotatingId + Max Lifetime Counter", TestGeneratingAdditionalDataPayloadWithRotatingIdAndMaxLifetimeCounter),
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
 *  Set up the test suite.
 */
int TestAdditionalDataPayload_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestAdditionalDataPayload_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

/**
 *  Main
 */
int TestAdditionalDataPayload()
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-additional-data-payload-general-Tests",
        &sTests[0],
        TestAdditionalDataPayload_Setup,
        TestAdditionalDataPayload_Teardown
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
