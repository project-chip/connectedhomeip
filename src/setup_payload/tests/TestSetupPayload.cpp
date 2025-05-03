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
    EXPECT_TRUE(std::holds_alternative<CHIP_ERROR>(SetupPayload::FromStringRepresentation("")));

    // Has invalid setup passcode 111111111
    EXPECT_TRUE(std::holds_alternative<CHIP_ERROR>(SetupPayload::FromStringRepresentation("MT:M5L90MP500W-GT68D20")));
    EXPECT_TRUE(std::holds_alternative<CHIP_ERROR>(SetupPayload::FromStringRepresentation("01120767810")));

    // Has valid payload (kDefaultPayloadQRCode) followed by one with invalid setup passcode 111111111
    EXPECT_TRUE(
        std::holds_alternative<CHIP_ERROR>(SetupPayload::FromStringRepresentation("MT:M5L90MP500K64J00000*M5L90MP500W-GT68D20")));

    // Has payload with invalid setup passcode 111111111 followed by valid payload (kDefaultPayloadQRCode)
    EXPECT_TRUE(
        std::holds_alternative<CHIP_ERROR>(SetupPayload::FromStringRepresentation("MT:M5L90MP500W-GT68D20*M5L90MP500K64J00000")));

    // Payload not starting with MT:
    EXPECT_TRUE(std::holds_alternative<CHIP_ERROR>(SetupPayload::FromStringRepresentation("AT:M5L90MP500K64J00000")));
}

TEST(TestSetupPayload, TestFromStringNumericCode)
{
    // Numeric code equivalent of kDefaultPayloadQRCode.
    auto parseResult = SetupPayload::FromStringRepresentation("00204800002");
    ASSERT_TRUE(std::holds_alternative<std::vector<SetupPayload>>(parseResult));

    auto & payloads = std::get<std::vector<SetupPayload>>(parseResult);
    ASSERT_TRUE(payloads.size() == 1);

    auto & payload      = payloads[0];
    auto defaultPayload = GetDefaultPayload();

    EXPECT_TRUE(payload.discriminator.IsShortDiscriminator());
    EXPECT_EQ(payload.discriminator.GetShortValue(), defaultPayload.discriminator.GetShortValue());
    EXPECT_EQ(payload.setUpPINCode, defaultPayload.setUpPINCode);
}

TEST(TestSetupPayload, TestFromStringSinglePayload)
{
    auto parseResult = SetupPayload::FromStringRepresentation(kDefaultPayloadQRCode);
    ASSERT_TRUE(std::holds_alternative<std::vector<SetupPayload>>(parseResult));

    auto & payloads = std::get<std::vector<SetupPayload>>(parseResult);
    ASSERT_EQ(payloads.size(), 1u);

    auto & payload      = payloads[0];
    auto defaultPayload = GetDefaultPayload();

    EXPECT_EQ(payload, defaultPayload);
}

TEST(TestSetupPayload, TestFromStringMultiplePayloads)
{
    // kDefaultPayloadQRCode followed by the same but with passcode and
    // discriminator incremented by 1, then 2, then 3.
    auto parseResult = SetupPayload::FromStringRepresentation(
        "MT:M5L90MP500K64J00000*M5L90U.D010K4J00000*M5L900CM02IX4J00000*M5L908OU03-85J00000");
    ASSERT_TRUE(std::holds_alternative<std::vector<SetupPayload>>(parseResult));

    auto & payloads = std::get<std::vector<SetupPayload>>(parseResult);
    ASSERT_EQ(payloads.size(), 4u);

    for (size_t idx = 0; idx < payloads.size(); ++idx)
    {
        auto comparisonPayload = GetDefaultPayload();
        comparisonPayload.discriminator.SetLongValue(comparisonPayload.discriminator.GetLongValue() + static_cast<uint16_t>(idx));
        comparisonPayload.setUpPINCode += static_cast<uint32_t>(idx);

        ASSERT_EQ(payloads[idx], comparisonPayload);
    }
}

} // anonymous namespace
