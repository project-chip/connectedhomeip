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
#include <stdio.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/verhoeff/Verhoeff.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <setup_payload/AdditionalDataPayloadParser.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemPacketBuffer.h>

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

CHIP_ERROR GenerateAdditionalDataPayload(AdditionalDataPayloadGeneratorParams & additionalDataPayloadParams,
                                         BitFlags<AdditionalDataFields> additionalDataFields, char * additionalDataPayloadOutput)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, bufferHandle,
                                                                         additionalDataFields);
    if (err == CHIP_NO_ERROR)
    {
        EXPECT_FALSE(bufferHandle.IsNull());
    }
    else
    {
        return err;
    }
    char output[kAdditionalDataPayloadLength];
    err = chip::Encoding::BytesToUppercaseHexString(bufferHandle->Start(), bufferHandle->DataLength(), output,
                                                    MATTER_ARRAY_SIZE(output));

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

    std::vector<uint8_t> additionalDataPayloadBytes;
    additionalDataPayloadBytes.resize(additionalDataPayloadBytesLength);
    size_t bufferSize = chip::Encoding::HexToBytes(additionalDataPayload, additionalDataPayloadLength,
                                                   additionalDataPayloadBytes.data(), additionalDataPayloadBytesLength);
    return AdditionalDataPayloadParser(additionalDataPayloadBytes.data(), bufferSize).populatePayload(outPayload);
}

class TestAdditionalDataPayload : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestAdditionalDataPayload, TestGeneratingAdditionalDataPayloadWithoutRotatingDeviceId)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    char output[kAdditionalDataPayloadLength];
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

    EXPECT_EQ(GenerateAdditionalDataPayload(additionalDataPayloadParams, additionalDataFields, output), CHIP_NO_ERROR);
    EXPECT_STREQ(output, kAdditionalDataPayloadWithoutRotatingDeviceId);
}

#if CHIP_ENABLE_ROTATING_DEVICE_ID
TEST_F(TestAdditionalDataPayload, TestGeneratingAdditionalDataPayloadWithRotatingDeviceId)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = kLifetimeCounter;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);

    char output[kAdditionalDataPayloadLength];
    EXPECT_EQ(GenerateAdditionalDataPayload(additionalDataPayloadParams, additionalDataFields, output), CHIP_NO_ERROR);
    EXPECT_STREQ(output, kAdditionalDataPayloadWithRotatingDeviceId);
}

TEST_F(TestAdditionalDataPayload, TestGeneratingAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = std::numeric_limits<uint16_t>::max();
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);

    char output[kAdditionalDataPayloadLength];
    EXPECT_EQ(GenerateAdditionalDataPayload(additionalDataPayloadParams, additionalDataFields, output), CHIP_NO_ERROR);
    EXPECT_STREQ(output, kAdditionalDataPayloadWithRotatingDeviceIdAndMaxLifetimeCounter);
}

TEST_F(TestAdditionalDataPayload, TestGeneratingAdditionalDataPayloadWithRotatingDeviceIdWithNullInputs)
{
    BitFlags<AdditionalDataFields> additionalDataFields;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

    char output[kAdditionalDataPayloadLength];
    EXPECT_EQ(GenerateAdditionalDataPayload(additionalDataPayloadParams, additionalDataFields, output),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestAdditionalDataPayload, TestGeneratingRotatingDeviceIdAsString)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    size_t rotatingDeviceIdValueOutputSize = 0;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = kLifetimeCounter;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);
    err = AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, MATTER_ARRAY_SIZE(rotatingDeviceIdHexBuffer),
        rotatingDeviceIdValueOutputSize);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_STREQ(rotatingDeviceIdHexBuffer, kRotatingDeviceId);
    // Parsing out the lifetime counter value
    long lifetimeCounter;
    char lifetimeCounterStr[3];
    Platform::CopyString(lifetimeCounterStr, rotatingDeviceIdHexBuffer);

    char * parseEnd;
    lifetimeCounter = strtol(lifetimeCounterStr, &parseEnd, 16);
    EXPECT_EQ(lifetimeCounter, kLifetimeCounter);
}

TEST_F(TestAdditionalDataPayload, TestGeneratingRotatingDeviceIdAsStringWithNullInputs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char rotatingDeviceIdHexBuffer[RotatingDeviceId::kHexMaxLength];
    size_t rotatingDeviceIdValueOutputSize = 0;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = 0;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan();
    err = AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, MATTER_ARRAY_SIZE(rotatingDeviceIdHexBuffer),
        rotatingDeviceIdValueOutputSize);
    EXPECT_EQ(err, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestAdditionalDataPayload, TestGeneratingRotatingDeviceIdWithSmallBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    char rotatingDeviceIdHexBuffer[kShortRotatingIdLength];
    size_t rotatingDeviceIdValueOutputSize = 0;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter = kLifetimeCounter;
    additionalDataPayloadParams.rotatingDeviceIdUniqueId        = ByteSpan(kUniqueId);
    err = AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsHexString(
        additionalDataPayloadParams, rotatingDeviceIdHexBuffer, MATTER_ARRAY_SIZE(rotatingDeviceIdHexBuffer),
        rotatingDeviceIdValueOutputSize);
    EXPECT_EQ(err, CHIP_ERROR_BUFFER_TOO_SMALL);
}
#endif // CHIP_ENABLE_ROTATING_DEVICE_ID

TEST_F(TestAdditionalDataPayload, TestParsingAdditionalDataPayloadWithRotatingDeviceId)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    EXPECT_EQ(ParseAdditionalDataPayload(kAdditionalDataPayloadWithRotatingDeviceId,
                                         strlen(kAdditionalDataPayloadWithRotatingDeviceId), resultPayload),
              CHIP_NO_ERROR);
    EXPECT_STREQ(resultPayload.rotatingDeviceId.c_str(), kRotatingDeviceId);
}

TEST_F(TestAdditionalDataPayload, TestParsingAdditionalDataPayloadWithoutRotatingDeviceId)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    EXPECT_EQ(ParseAdditionalDataPayload(kAdditionalDataPayloadWithoutRotatingDeviceId,
                                         strlen(kAdditionalDataPayloadWithoutRotatingDeviceId), resultPayload),
              CHIP_NO_ERROR);
    EXPECT_STREQ(resultPayload.rotatingDeviceId.c_str(), "");
}

TEST_F(TestAdditionalDataPayload, TestParsingAdditionalDataPayloadWithInvalidRotatingDeviceIdLength)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    EXPECT_EQ(ParseAdditionalDataPayload(kAdditionalDataPayloadWithInvalidRotatingDeviceIdLength,
                                         strlen(kAdditionalDataPayloadWithInvalidRotatingDeviceIdLength), resultPayload),
              CHIP_ERROR_TLV_UNDERRUN);
}

TEST_F(TestAdditionalDataPayload, TestParsingAdditionalDataPayloadWithLongRotatingDeviceId)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    EXPECT_EQ(ParseAdditionalDataPayload(kAdditionalDataPayloadWithLongRotatingDeviceId,
                                         strlen(kAdditionalDataPayloadWithLongRotatingDeviceId), resultPayload),
              CHIP_ERROR_INVALID_STRING_LENGTH);
}

TEST_F(TestAdditionalDataPayload, TestParsingAdditionalDataPayloadWithShortRotatingDeviceId)
{
    chip::SetupPayloadData::AdditionalDataPayload resultPayload;
    EXPECT_EQ(ParseAdditionalDataPayload(kAdditionalDataPayloadWithShortRotatingDeviceId,
                                         strlen(kAdditionalDataPayloadWithShortRotatingDeviceId), resultPayload),
              CHIP_NO_ERROR);
    EXPECT_STREQ(resultPayload.rotatingDeviceId.c_str(), kShortRotatingDeviceId);
}

} // namespace
