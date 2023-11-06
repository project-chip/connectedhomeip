/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <math.h>
#include <memory>
#include <nlunit-test.h>
#include <stdio.h>

#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/UnitTestContext.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <setup_payload/AdditionalDataPayloadParser.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemPacketBuffer.h>

#include <lib/support/UnitTestRegistration.h>
#include <lib/support/verhoeff/Verhoeff.h>

using namespace chip;

namespace {

constexpr char kAdditionalDataPayloadWithoutRotatingDeviceId[]           = "1518";
constexpr char kAdditionalDataPayloadWithRotatingDeviceId[]              = "153000120A00D00561E77A68A9FD975057375B9283A818";
constexpr char kAdditionalDataPayloadWithInvalidRotatingDeviceIdLength[] = "153000FF0A001998AB7130E38B7E9A401CFE9F7B79AF18";
constexpr char kAdditionalDataPayloadWithLongRotatingDeviceId[]          = "153000130A00191998AB7130E38B7E9A401CFE9F7B79AF18";
constexpr char kAdditionalDataPayloadWithShortRotatingDeviceId[]         = "153000110A001998AB7130E38B7E9A401CFE9F7B7918";
constexpr char kRotatingDeviceId[]                                       = "0A00D00561E77A68A9FD975057375B9283A8";
constexpr char kShortRotatingDeviceId[]                                  = "0A001998AB7130E38B7E9A401CFE9F7B79";
constexpr uint16_t kAdditionalDataPayloadLength                          = 51;
#if CHIP_ENABLE_ROTATING_DEVICE_ID
constexpr uint8_t kUniqueId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
constexpr char kAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter[] = "15300012FFFF8BEA0C775F001981365D6362E1C0665A18";
constexpr uint16_t kLifetimeCounter                                              = 10;
constexpr uint16_t kShortRotatingIdLength                                        = 5;
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

CHIP_ERROR GenerateAdditionalDataPayload(nlTestSuite * inSuite, AdditionalDataPayloadGeneratorParams & additionalDataPayloadParams,
                                         BitFlags<AdditionalDataFields> additionalDataFields, char * additionalDataPayloadOutput)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, bufferHandle,
                                                                         additionalDataFields);
    if (err == CHIP_NO_ERROR)
    {
        NL_TEST_ASSERT(inSuite, !bufferHandle.IsNull());
    }
    else
    {
        return err;
    }
    char output[kAdditionalDataPayloadLength];
    err = chip::Encoding::BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output, ArraySize(output));

    if (err == CHIP_NO_ERROR)
    {
        memmove(additionalDataPayloadOutput, output, kAdditionalDataPayloadLength);
    }
    return err;
}

CHIP_ERROR ParseAdditionalDataPayload(const char * additionalDataPayload, size_t additionalDataPayloadLength,
                                      chip::SetupPayloadData::AdditionalDataPayload & outPayload)
{
    if (additionalDataPayloadLength % 2 != 0)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    size_t additionalDataPayloadBytesLength = additionalDataPayloadLength / 2;
    std::unique_ptr<uint8_t[]> additionalDataPayloadBytes(new uint8_t[additionalDataPayloadBytesLength]);
    size_t bufferSize = chip::Encoding::HexToBytes(additionalDataPayload, additionalDataPayloadLength,
                                                   additionalDataPayloadBytes.get(), additionalDataPayloadBytesLength);
    return AdditionalDataPayloadParser(additionalDataPayloadBytes.get(), bufferSize).populatePayload(outPayload);
}

void TestGeneratingAdditionalDataPayloadWithoutRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    char output[kAdditionalDataPayloadLength];
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

    NL_TEST_ASSERT(inSuite,
                   GenerateAdditionalDataPayload(inSuite, additionalDataPayloadParams, additionalDataFields, output) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, kAdditionalDataPayloadWithoutRotatingDeviceId) == 0);
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
void TestGeneratingAdditionalDataPayloadWithRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = kLifetimeCounter;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);

    char output[kAdditionalDataPayloadLength];
    NL_TEST_ASSERT(inSuite,
                   GenerateAdditionalDataPayload(inSuite, additionalDataPayloadParams, additionalDataFields, output) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, kAdditionalDataPayloadWithRotatingDeviceId) == 0);
}

void TestGeneratingAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = std::numeric_limits<uint16_t>::max();
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);

    char output[kAdditionalDataPayloadLength];
    NL_TEST_ASSERT(inSuite,
                   GenerateAdditionalDataPayload(inSuite, additionalDataPayloadParams, additionalDataFields, output) ==
                       CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(output, kAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter) == 0);
}

void TestGeneratingAdditionalDataPayloadWithRotatingDeviceIdWithNullInputs(nlTestSuite * inSuite, void * inContext)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

    char output[kAdditionalDataPayloadLength];
    NL_TEST_ASSERT(inSuite,
                   GenerateAdditionalDataPayload(inSuite, additionalDataPayloadParams, additionalDataFields, output) ==
                       CHIP_ERROR_INVALID_ARGUMENT);
}

void TestGeneratingRotatingDeviceIdAsString(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    size_t rotatingDeviceIdValueOutputSize = 0;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = kLifetimeCounter;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);
    err = AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer),
        rotatingDeviceIdValueOutputSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(rotatingDeviceIdHexBuffer, kRotatingDeviceId) == 0);
    // Parsing out the lifetime counter value
    long lifetimeCounter;
    char lifetimeCounterStr[3];
    Platform::CopyString(lifetimeCounterStr, rotatingDeviceIdHexBuffer);

    char * parseEnd;
    lifetimeCounter = strtol(lifetimeCounterStr, &parseEnd, 16);
    NL_TEST_ASSERT(inSuite, lifetimeCounter == kLifetimeCounter);
}

void TestGeneratingRotatingDeviceIdAsStringWithNullInputs(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    size_t rotatingDeviceIdValueOutputSize = 0;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = 0;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan();
    err = AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer),
        rotatingDeviceIdValueOutputSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_INVALID_ARGUMENT);
}

void TestGeneratingRotatingDeviceIdWithSmallBuffer(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char rotatingDeviceIdHexBuffer[kShortRotatingIdLength];
    size_t rotatingDeviceIdValueOutputSize = 0;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = kLifetimeCounter;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);
    err = AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, ArraySize(rotatingDeviceIdHexBuffer),
        rotatingDeviceIdValueOutputSize);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_BUFFER_TOO_SMALL);
}
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

void TestParsingAdditionalDataPayloadWithRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    NL_TEST_ASSERT(inSuite,
                   ParseAdditionalDataPayload(kAdditionalDataPayloadWithRotatingDeviceId,
                                              strlen(kAdditionalDataPayloadWithRotatingDeviceId), resultPayload) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), kRotatingDeviceId) == 0);
}

void TestParsingAdditionalDataPayloadWithoutRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    NL_TEST_ASSERT(inSuite,
                   ParseAdditionalDataPayload(kAdditionalDataPayloadWithoutRotatingDeviceId,
                                              strlen(kAdditionalDataPayloadWithoutRotatingDeviceId),
                                              resultPayload) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), "") == 0);
}

void TestParsingAdditionalDataPayloadWithInvalidRotatingDeviceIdLength(nlTestSuite * inSuite, void * inContext)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    NL_TEST_ASSERT(inSuite,
                   ParseAdditionalDataPayload(kAdditionalDataPayloadWithInvalidRotatingDeviceIdLength,
                                              strlen(kAdditionalDataPayloadWithInvalidRotatingDeviceIdLength),
                                              resultPayload) == CHIP_ERROR_TLV_UNDERRUN);
}

void TestParsingAdditionalDataPayloadWithLongRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    NL_TEST_ASSERT(inSuite,
                   ParseAdditionalDataPayload(kAdditionalDataPayloadWithLongRotatingDeviceId,
                                              strlen(kAdditionalDataPayloadWithLongRotatingDeviceId),
                                              resultPayload) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestParsingAdditionalDataPayloadWithShortRotatingDeviceId(nlTestSuite * inSuite, void * inContext)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    NL_TEST_ASSERT(inSuite,
                   ParseAdditionalDataPayload(kAdditionalDataPayloadWithShortRotatingDeviceId,
                                              strlen(kAdditionalDataPayloadWithShortRotatingDeviceId),
                                              resultPayload) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, strcmp(resultPayload.rotatingDeviceId.c_str(), kShortRotatingDeviceId) == 0);
}

/**
 *  Test Suite that lists all the Test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test Generating Additional Data Payload without Rotatin gDevice Id", TestGeneratingAdditionalDataPayloadWithoutRotatingDeviceId),
    #if CHIP_ENABLE_ROTATING_DEVICE_ID
    NL_TEST_DEF("Test Generating Additional Data Payload with Rotating Device Id", TestGeneratingAdditionalDataPayloadWithRotatingDeviceId),
    NL_TEST_DEF("Test Generating Additional Data Payload with Rotating Device Id + Max Lifetime Counter", TestGeneratingAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter),
    NL_TEST_DEF("Test Generating Additional Data Payload with Rotating Device Id + Null/Empty Inputs", TestGeneratingAdditionalDataPayloadWithRotatingDeviceIdWithNullInputs),
    NL_TEST_DEF("Test Generating Rotating Device Id as string", TestGeneratingRotatingDeviceIdAsString),
    NL_TEST_DEF("Test Generating Rotating Device Id as string with null/invalid inputs", TestGeneratingRotatingDeviceIdAsStringWithNullInputs),
    NL_TEST_DEF("Test Generating Rotating Device Id as string with small buffer", TestGeneratingRotatingDeviceIdWithSmallBuffer),
    #endif
    NL_TEST_DEF("Test Parsing Additional Data Payload with Rotating Device Id", TestParsingAdditionalDataPayloadWithRotatingDeviceId),
    NL_TEST_DEF("Test Parsing Additional Data Payload without Rotating Device Id", TestParsingAdditionalDataPayloadWithoutRotatingDeviceId),
    NL_TEST_DEF("Test Parsing Additional Data Payload with Invalid Rotating Device Id Length", TestParsingAdditionalDataPayloadWithInvalidRotatingDeviceIdLength),
    NL_TEST_DEF("Test Parsing Additional Data Payload with Long Rotating Device Id", TestParsingAdditionalDataPayloadWithLongRotatingDeviceId),
    NL_TEST_DEF("Test Parsing Additional Data Payload with Short Rotating Device Id", TestParsingAdditionalDataPayloadWithShortRotatingDeviceId),
    NL_TEST_SENTINEL()
};
// clang-format on

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

    // Generate machine-readable, comma-separated value (CSV) output.
    nl_test_set_output_style(OUTPUT_CSV);

    return chip::ExecuteTestsWithoutContext(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAdditionalDataPayload);
