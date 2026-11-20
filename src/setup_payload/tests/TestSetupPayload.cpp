/*
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file implements a unit test suite for SetupPayload
 *
 */

#include "TestHelpers.h"

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>

using namespace chip;

namespace {

TEST(TestSetupPayload, TestFromStringInvalidValues)
{
    std::vector<SetupPayload> payloads;
    EXPECT_NE(SetupPayload::FromStringRepresentation("", payloads), CHIP_NO_ERROR);

    // Has invalid setup passcode 111111111
    EXPECT_NE(SetupPayload::FromStringRepresentation("MT:M5L90MP500W-GT68D20", payloads), CHIP_NO_ERROR);
    EXPECT_NE(SetupPayload::FromStringRepresentation("01120767810", payloads), CHIP_NO_ERROR);

    // Has valid payload (kDefaultPayloadQRCode) followed by one with invalid setup passcode 111111111
    EXPECT_NE(SetupPayload::FromStringRepresentation("MT:M5L90MP500K64J00000*M5L90MP500W-GT68D20", payloads), CHIP_NO_ERROR);

    // Has payload with invalid setup passcode 111111111 followed by valid payload (kDefaultPayloadQRCode)
    EXPECT_NE(SetupPayload::FromStringRepresentation("MT:M5L90MP500W-GT68D20*M5L90MP500K64J00000", payloads), CHIP_NO_ERROR);

    // Payload not starting with MT:
    EXPECT_NE(SetupPayload::FromStringRepresentation("AT:M5L90MP500K64J00000", payloads), CHIP_NO_ERROR);
}

TEST(TestSetupPayload, TestFromStringNumericCode)
{
    // Numeric code equivalent of kDefaultPayloadQRCode.
    std::vector<SetupPayload> payloads;
    auto err = SetupPayload::FromStringRepresentation("00204800002", payloads);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ASSERT_EQ(payloads.size(), 1u);
    auto & payload      = payloads[0];
    auto defaultPayload = GetDefaultPayload();

    EXPECT_TRUE(payload.discriminator.IsShortDiscriminator());
    EXPECT_EQ(payload.discriminator.GetShortValue(), defaultPayload.discriminator.GetShortValue());
    EXPECT_EQ(payload.setUpPINCode, defaultPayload.setUpPINCode);
}

TEST(TestSetupPayload, TestFromStringSinglePayload)
{
    std::vector<SetupPayload> payloads;
    auto err = SetupPayload::FromStringRepresentation(kDefaultPayloadQRCode, payloads);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ASSERT_EQ(payloads.size(), 1u);

    auto & payload      = payloads[0];
    auto defaultPayload = GetDefaultPayload();

    EXPECT_EQ(payload, defaultPayload);
}

TEST(TestSetupPayload, TestFromStringMultiplePayloads)
{
    std::vector<SetupPayload> payloads;
    auto err = SetupPayload::FromStringRepresentation(kConcatenatedQRCode, payloads);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    ASSERT_EQ(payloads.size(), 4u);

    for (size_t idx = 0; idx < payloads.size(); ++idx)
    {
        auto comparisonPayload = GetDefaultPayload();
        comparisonPayload.discriminator.SetLongValue(comparisonPayload.discriminator.GetLongValue() + static_cast<uint16_t>(idx));
        comparisonPayload.setUpPINCode += static_cast<uint32_t>(idx);

        EXPECT_EQ(payloads[idx], comparisonPayload);
    }
}

TEST(TestSetupPayload, FullDiscriminatorQRCode)
{
    for (uint16_t discriminator : { uint16_t{ 3840 }, uint16_t{ 4021 } })
    {
        SetupPayload payload;
        payload.setUpPINCode = 20202021;
        payload.discriminator.SetLongValue(discriminator);
        payload.rendezvousInformation.SetValue(RendezvousInformationFlags(
            RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kBLE, RendezvousInformationFlag::kOnNetwork,
            RendezvousInformationFlag::kWiFiPAF, RendezvousInformationFlag::kNFC, RendezvousInformationFlag::kThread));
        ASSERT_TRUE(payload.isValidQRCodePayload());

        std::string code;
        ASSERT_EQ(QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(code), CHIP_NO_ERROR);
        std::vector<SetupPayload> parsed;
        ASSERT_EQ(SetupPayload::FromStringRepresentation(code, parsed), CHIP_NO_ERROR);
        ASSERT_EQ(parsed.size(), 1u);
        EXPECT_EQ(parsed[0].vendorID, 0);
        EXPECT_EQ(parsed[0].productID, 0);
        EXPECT_EQ(parsed[0].setUpPINCode, payload.setUpPINCode);
        EXPECT_FALSE(parsed[0].discriminator.IsShortDiscriminator());
        EXPECT_TRUE(parsed[0].discriminator.MatchesLongDiscriminator(discriminator));
        EXPECT_FALSE(parsed[0].discriminator.MatchesLongDiscriminator(discriminator == 3840 ? 4021 : 3840));
        ASSERT_TRUE(parsed[0].rendezvousInformation.HasValue());
        EXPECT_EQ(parsed[0].rendezvousInformation.Value().Raw(), 0x3F);
    }
}

TEST(TestSetupPayload, ManualCodeKeepsShortDiscriminatorBucket)
{
    std::string previousCode;
    for (uint16_t discriminator : { uint16_t{ 3840 }, uint16_t{ 4021 } })
    {
        SetupPayload payload;
        payload.setUpPINCode = 20202021;
        payload.discriminator.SetLongValue(discriminator);
        std::string code;
        ASSERT_EQ(ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(code), CHIP_NO_ERROR);
        if (!previousCode.empty())
        {
            EXPECT_EQ(code, previousCode);
        }
        previousCode = code;

        std::vector<SetupPayload> parsed;
        ASSERT_EQ(SetupPayload::FromStringRepresentation(code, parsed), CHIP_NO_ERROR);
        ASSERT_EQ(parsed.size(), 1u);
        EXPECT_TRUE(parsed[0].discriminator.IsShortDiscriminator());
        EXPECT_EQ(parsed[0].discriminator.GetShortValue(), 15);
        EXPECT_TRUE(parsed[0].discriminator.MatchesLongDiscriminator(3840));
        EXPECT_TRUE(parsed[0].discriminator.MatchesLongDiscriminator(4021));
        EXPECT_FALSE(parsed[0].discriminator.MatchesLongDiscriminator(3839));
        EXPECT_FALSE(parsed[0].rendezvousInformation.HasValue());
    }
}

TEST(TestSetupPayload, RecognizedAndReservedRendezvousFlags)
{
    SetupPayload payload;
    payload.setUpPINCode = 20202021;
    payload.discriminator.SetLongValue(3840);
    RendezvousInformationFlags recognized(RendezvousInformationFlag::kSoftAP, RendezvousInformationFlag::kBLE,
                                          RendezvousInformationFlag::kOnNetwork, RendezvousInformationFlag::kWiFiPAF,
                                          RendezvousInformationFlag::kNFC, RendezvousInformationFlag::kThread);
    payload.rendezvousInformation.SetValue(recognized);
    EXPECT_EQ(recognized.Raw(), 0x3F);
    EXPECT_TRUE(payload.isValidQRCodePayload());

    for (uint8_t reserved : { uint8_t{ 0x40 }, uint8_t{ 0x80 } })
    {
        payload.rendezvousInformation.Value().SetRaw(static_cast<uint8_t>(recognized.Raw() | reserved));
        EXPECT_FALSE(payload.isValidQRCodePayload());
        EXPECT_TRUE(payload.isValidQRCodePayload(PayloadContents::ValidationMode::kConsume));
    }
}

TEST(TestSetupPayload, UnspecifiedIdentityRetainsPasscodeValidation)
{
    SetupPayload payload;
    payload.discriminator.SetLongValue(3840);
    payload.rendezvousInformation.SetValue(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));
    for (uint32_t passcode : { 0u, 11111111u, 12345678u, 87654321u, 99999999u })
    {
        payload.setUpPINCode = passcode;
        std::string code;
        EXPECT_NE(QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(code), CHIP_NO_ERROR);
    }
}

} // anonymous namespace
