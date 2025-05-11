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

} // anonymous namespace
