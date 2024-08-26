/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a unit test suite for the manual setup
 *      code functionality.
 *
 */

#include <pw_unit_test/framework.h>
#include <stdio.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/verhoeff/Verhoeff.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/ManualSetupPayloadParser.h>
#include <setup_payload/SetupPayload.h>

#include <algorithm>
#include <math.h>
#include <string>

using namespace chip;

namespace {

bool CheckGenerator(const PayloadContents & payload, std::string expectedResult, bool allowInvalidPayload = false)
{
    std::string result;
    ManualSetupPayloadGenerator generator(payload);
    generator.SetAllowInvalidPayload(allowInvalidPayload);

    CHIP_ERROR err = generator.payloadDecimalStringRepresentation(result);

    if (err != CHIP_NO_ERROR)
    {
        printf("Failed to generate decimal representation with error: %s\n", ErrorStr(err));
        return false;
    }

    if (!expectedResult.empty())
    {
        expectedResult += Verhoeff10::ComputeCheckChar(expectedResult.c_str());
    }

    bool same = result == expectedResult;
    if (!same)
    {
        printf("Actual result: %s\n", result.c_str());
        printf("Expected result: %s\n", expectedResult.c_str());
    }

    return same;
}

PayloadContents GetDefaultPayload()
{
    PayloadContents payload;
    payload.setUpPINCode = 12345679;
    payload.discriminator.SetLongValue(2560);

    return payload;
}

TEST(TestManualCode, TestDecimalRepresentation_PartialPayload)
{
    PayloadContents payload = GetDefaultPayload();

    std::string expectedResult = "2412950753";

    EXPECT_TRUE(CheckGenerator(payload, expectedResult));
}

TEST(TestManualCode, TestDecimalRepresentation_PartialPayload_RequiresCustomFlow)
{
    PayloadContents payload   = GetDefaultPayload();
    payload.commissioningFlow = CommissioningFlow::kCustom;

    std::string expectedResult = "64129507530000000000";

    EXPECT_TRUE(CheckGenerator(payload, expectedResult));
}

TEST(TestManualCode, TestDecimalRepresentation_FullPayloadWithZeros)
{
    PayloadContents payload   = GetDefaultPayload();
    payload.commissioningFlow = CommissioningFlow::kCustom;
    payload.vendorID          = 1;
    payload.productID         = 1;

    std::string expectedResult = "64129507530000100001";

    EXPECT_TRUE(CheckGenerator(payload, expectedResult));
}

TEST(TestManualCode, TestDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow)
{
    PayloadContents payload = GetDefaultPayload();
    payload.vendorID        = 45367;
    payload.productID       = 14526;

    std::string expectedResult = "2412950753";

    EXPECT_TRUE(CheckGenerator(payload, expectedResult));
}

TEST(TestManualCode, TestDecimalRepresentation_FullPayloadWithoutZeros)
{
    PayloadContents payload   = GetDefaultPayload();
    payload.commissioningFlow = CommissioningFlow::kCustom;
    payload.vendorID          = 45367;
    payload.productID         = 14526;

    std::string expectedResult = "64129507534536714526";

    EXPECT_TRUE(CheckGenerator(payload, expectedResult));
}

void assertPayloadValues(CHIP_ERROR actualError, CHIP_ERROR expectedError, const PayloadContents & payload, uint32_t pinCode,
                         const SetupDiscriminator & discriminator, uint16_t vendorID, uint16_t productID)
{
    EXPECT_EQ(actualError, expectedError);
    EXPECT_EQ(payload.setUpPINCode, pinCode);
    EXPECT_EQ(payload.discriminator, discriminator);
    EXPECT_EQ(payload.vendorID, vendorID);
    EXPECT_EQ(payload.productID, productID);
}

TEST(TestManualCode, TestGenerateAndParser_ManualSetupCodeWithLongDiscriminator)
{
    PayloadContents payload = GetDefaultPayload();
    payload.discriminator.SetLongValue(0xa1f);

    {
        // Test short 11 digit code
        ManualSetupPayloadGenerator generator(payload);
        std::string result;
        EXPECT_EQ(generator.payloadDecimalStringRepresentation(result), CHIP_NO_ERROR);

        SetupPayload outPayload;
        CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        SetupDiscriminator discriminator;
        discriminator.SetShortValue(0xa);
        assertPayloadValues(err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, discriminator, payload.vendorID,
                            payload.productID);
    }

    payload.vendorID          = 1;
    payload.productID         = 1;
    payload.commissioningFlow = CommissioningFlow::kCustom;
    payload.discriminator.SetLongValue(0xb1f);

    {
        // Test long 21 digit code
        ManualSetupPayloadGenerator generator(payload);
        std::string result;
        EXPECT_EQ(generator.payloadDecimalStringRepresentation(result), CHIP_NO_ERROR);

        SetupPayload outPayload;
        CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
        EXPECT_EQ(err, CHIP_NO_ERROR);

        SetupDiscriminator discriminator;
        discriminator.SetShortValue(0xb);
        assertPayloadValues(err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, discriminator, payload.vendorID,
                            payload.productID);
    }
}

TEST(TestManualCode, TestDecimalRepresentation_AllZeros)
{
    PayloadContents payload;
    payload.setUpPINCode = 0;
    payload.discriminator.SetLongValue(0);

    std::string expectedResult;

    EXPECT_EQ(CheckGenerator(payload, expectedResult), false);
}

TEST(TestManualCode, TestDecimalRepresentation_AllOnes)
{
    PayloadContents payload;
    payload.setUpPINCode = 0x7FFFFFF;
    payload.discriminator.SetLongValue(0xFFF);
    payload.commissioningFlow = CommissioningFlow::kCustom;
    payload.vendorID          = 65535;
    payload.productID         = 65535;

    std::string expectedResult = "76553581916553565535";

    EXPECT_TRUE(CheckGenerator(payload, expectedResult, /*allowInvalidPayload*/ true));
}

char ComputeCheckChar(const std::string & str)
{
    // Strip out dashes, if any, from the string before computing the checksum.
    std::string copy(str);
    copy.erase(std::remove(copy.begin(), copy.end(), '-'), copy.end());
    return Verhoeff10::ComputeCheckChar(copy.c_str());
}

TEST(TestManualCode, TestPayloadParser_FullPayload)
{
    SetupPayload payload;
    std::string decimalString;

    decimalString = "63610875354536714526";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    CHIP_ERROR err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SetupDiscriminator discriminator;
    discriminator.SetShortValue(0xa);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 123456780, discriminator, 45367, 14526);

    // The same thing, but with dashes separating digit groups.
    decimalString = "6361-0875-3545-3671-4526";
    decimalString += ComputeCheckChar(decimalString);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    discriminator.SetShortValue(0xa);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 123456780, discriminator, 45367, 14526);

    decimalString = "52927623630456200032";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    discriminator.SetShortValue(0x5);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 38728284, discriminator, 4562, 32);

    decimalString = "40000100000000100001";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    discriminator.SetShortValue(0);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 1, discriminator, 1, 1);
}

TEST(TestManualCode, TestGenerateAndParser_FullPayload)
{
    PayloadContents payload   = GetDefaultPayload();
    payload.vendorID          = 1;
    payload.productID         = 1;
    payload.commissioningFlow = CommissioningFlow::kCustom;

    ManualSetupPayloadGenerator generator(payload);
    std::string result;
    EXPECT_EQ(generator.payloadDecimalStringRepresentation(result), CHIP_NO_ERROR);

    SetupPayload outPayload;
    CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SetupDiscriminator discriminator;
    discriminator.SetShortValue(payload.discriminator.GetShortValue());
    assertPayloadValues(err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, discriminator, payload.vendorID, payload.productID);
}

TEST(TestManualCode, TestGenerateAndParser_PartialPayload)
{
    PayloadContents payload = GetDefaultPayload();

    ManualSetupPayloadGenerator generator(payload);
    std::string result;
    EXPECT_EQ(generator.payloadDecimalStringRepresentation(result), CHIP_NO_ERROR);

    SetupPayload outPayload;
    CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SetupDiscriminator discriminator;
    discriminator.SetShortValue(payload.discriminator.GetShortValue());
    assertPayloadValues(err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, discriminator, payload.vendorID, payload.productID);
}

TEST(TestManualCode, TestPayloadParser_PartialPayload)
{
    CHIP_ERROR err;
    SetupPayload payload;
    std::string decimalString;

    decimalString = "2361087535";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    EXPECT_EQ(decimalString.length(), 11u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    SetupDiscriminator discriminator;
    discriminator.SetShortValue(0xa);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 123456780, discriminator, 0, 0);

    // The same thing, but with dashes separating digit groups.
    decimalString = "236-108753-5";
    decimalString += ComputeCheckChar(decimalString);
    EXPECT_EQ(decimalString.length(), 13u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    discriminator.SetShortValue(0xa);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 123456780, discriminator, 0, 0);

    decimalString = "0000010000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    EXPECT_EQ(decimalString.length(), 11u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    discriminator.SetShortValue(0);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 1, discriminator, 0, 0);

    decimalString = "63610875350000000000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    EXPECT_EQ(decimalString.length(), 21u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    discriminator.SetShortValue(0xa);
    assertPayloadValues(err, CHIP_NO_ERROR, payload, 123456780, discriminator, 0, 0);

    // no discriminator (= 0)
    decimalString = "0033407535";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    EXPECT_EQ(decimalString.length(), 11u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // no vid (= 0)
    decimalString = "63610875350000014526";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    EXPECT_EQ(decimalString.length(), 21u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // no pid (= 0)
    decimalString = "63610875354536700000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    EXPECT_EQ(decimalString.length(), 21u);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(TestManualCode, TestShortCodeReadWrite)
{
    PayloadContents inPayload = GetDefaultPayload();

    SetupPayload outPayload;

    std::string result;
    ManualSetupPayloadGenerator generator(inPayload);
    generator.payloadDecimalStringRepresentation(result);
    ManualSetupPayloadParser(result).populatePayload(outPayload);

    // Override the discriminator in the input payload with the short version,
    // since that's what we will produce.
    inPayload.discriminator.SetShortValue(inPayload.discriminator.GetShortValue());
    EXPECT_TRUE(inPayload == outPayload);
}

TEST(TestManualCode, TestLongCodeReadWrite)
{
    PayloadContents inPayload   = GetDefaultPayload();
    inPayload.commissioningFlow = CommissioningFlow::kCustom;
    inPayload.vendorID          = 1;
    inPayload.productID         = 1;

    SetupPayload outPayload;

    std::string result;
    ManualSetupPayloadGenerator generator(inPayload);
    generator.payloadDecimalStringRepresentation(result);
    ManualSetupPayloadParser(result).populatePayload(outPayload);

    // Override the discriminator in the input payload with the short version,
    // since that's what we will produce.
    inPayload.discriminator.SetShortValue(inPayload.discriminator.GetShortValue());
    EXPECT_TRUE(inPayload == outPayload);
}

void assertEmptyPayloadWithError(CHIP_ERROR actualError, CHIP_ERROR expectedError, const SetupPayload & payload, int line)
{
    ChipLogProgress(Test, "Current check line: %d", line);
    EXPECT_EQ(actualError, expectedError);
    EXPECT_EQ(payload.setUpPINCode, 0u);
    EXPECT_EQ(payload.discriminator.GetLongValue(), 0u);
    EXPECT_EQ(payload.productID, 0u);
    EXPECT_EQ(payload.vendorID, 0u);
}

TEST(TestManualCode, TestPayloadParser_InvalidEntry)
{
    SetupPayload payload;
    std::string decimalString;

    // Empty input
    decimalString = "";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload, __LINE__);

    // Invalid character
    decimalString = "24184.2196";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_INTEGER_VALUE,
                                payload, __LINE__);

    // too short
    decimalString = "2456";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload, __LINE__);

    // too long for long code
    decimalString = "123456789123456785671";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload, __LINE__);

    // too long for short code
    decimalString = "12749875380";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload, __LINE__);

    // bit to indicate short code but long code length
    decimalString = "23456789123456785610";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload, __LINE__);
    // no pin code (= 0)
    decimalString = "2327680000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INVALID_ARGUMENT,
                                payload, __LINE__);
    // wrong check digit
    decimalString = "02684354589";
    assertEmptyPayloadWithError(ManualSetupPayloadParser(decimalString).populatePayload(payload), CHIP_ERROR_INTEGRITY_CHECK_FAILED,
                                payload, __LINE__);
}

TEST(TestManualCode, TestCheckDecimalStringValidity)
{
    std::string outReprensation;
    char checkDigit;
    std::string representationWithoutCheckDigit;
    std::string decimalString;

    representationWithoutCheckDigit = "";
    EXPECT_EQ(ManualSetupPayloadParser::CheckDecimalStringValidity(representationWithoutCheckDigit, outReprensation),
              CHIP_ERROR_INVALID_STRING_LENGTH);

    representationWithoutCheckDigit = "1";
    EXPECT_EQ(ManualSetupPayloadParser::CheckDecimalStringValidity(representationWithoutCheckDigit, outReprensation),
              CHIP_ERROR_INVALID_STRING_LENGTH);

    representationWithoutCheckDigit = "10109";
    checkDigit                      = Verhoeff10::ComputeCheckChar(representationWithoutCheckDigit.c_str());
    decimalString                   = representationWithoutCheckDigit + checkDigit;
    EXPECT_EQ(ManualSetupPayloadParser::CheckDecimalStringValidity(decimalString, outReprensation), CHIP_NO_ERROR);
    EXPECT_EQ(outReprensation, representationWithoutCheckDigit);

    representationWithoutCheckDigit = "0000";
    checkDigit                      = Verhoeff10::ComputeCheckChar(representationWithoutCheckDigit.c_str());
    decimalString                   = representationWithoutCheckDigit + checkDigit;
    EXPECT_EQ(ManualSetupPayloadParser::CheckDecimalStringValidity(decimalString, outReprensation), CHIP_NO_ERROR);
    EXPECT_EQ(outReprensation, representationWithoutCheckDigit);
}

TEST(TestManualCode, TestCheckCodeLengthValidity)
{
    EXPECT_EQ(ManualSetupPayloadParser::CheckCodeLengthValidity("01234567890123456789", true), CHIP_NO_ERROR);
    EXPECT_EQ(ManualSetupPayloadParser::CheckCodeLengthValidity("0123456789", false), CHIP_NO_ERROR);

    EXPECT_EQ(ManualSetupPayloadParser::CheckCodeLengthValidity("01234567891", false), CHIP_ERROR_INVALID_STRING_LENGTH);
    EXPECT_EQ(ManualSetupPayloadParser::CheckCodeLengthValidity("012345678", false), CHIP_ERROR_INVALID_STRING_LENGTH);
    EXPECT_EQ(ManualSetupPayloadParser::CheckCodeLengthValidity("012345678901234567891", true), CHIP_ERROR_INVALID_STRING_LENGTH);
    EXPECT_EQ(ManualSetupPayloadParser::CheckCodeLengthValidity("0123456789012345678", true), CHIP_ERROR_INVALID_STRING_LENGTH);
}

TEST(TestManualCode, TestDecimalStringToNumber)
{
    uint32_t number;
    EXPECT_EQ(ManualSetupPayloadParser::ToNumber("12345", number), CHIP_NO_ERROR);
    EXPECT_EQ(number, 12345u);

    EXPECT_EQ(ManualSetupPayloadParser::ToNumber("01234567890", number), CHIP_NO_ERROR);
    EXPECT_EQ(number, 1234567890u);

    EXPECT_EQ(ManualSetupPayloadParser::ToNumber("00000001", number), CHIP_NO_ERROR);
    EXPECT_TRUE(number);

    EXPECT_EQ(ManualSetupPayloadParser::ToNumber("0", number), CHIP_NO_ERROR);
    EXPECT_FALSE(number);

    EXPECT_EQ(ManualSetupPayloadParser::ToNumber("012345.123456789", number), CHIP_ERROR_INVALID_INTEGER_VALUE);
    EXPECT_EQ(ManualSetupPayloadParser::ToNumber("/", number), CHIP_ERROR_INVALID_INTEGER_VALUE);
}

TEST(TestManualCode, TestReadCharsFromDecimalString)
{
    uint32_t number;
    size_t index = 3;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("12345", index, number, 2), CHIP_NO_ERROR);
    EXPECT_EQ(number, 45u);

    index = 2;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("6256276377282", index, number, 7), CHIP_NO_ERROR);
    EXPECT_EQ(number, 5627637u);

    index = 0;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("10", index, number, 2), CHIP_NO_ERROR);
    EXPECT_EQ(number, 10u);

    index = 0;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("01", index, number, 2), CHIP_NO_ERROR);
    EXPECT_TRUE(number);

    index = 1;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("11", index, number, 1), CHIP_NO_ERROR);
    EXPECT_TRUE(number);

    index = 2;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("100001", index, number, 3), CHIP_NO_ERROR);

    index = 1;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("12345", index, number, 5), CHIP_ERROR_INVALID_STRING_LENGTH);
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("12", index, number, 5), CHIP_ERROR_INVALID_STRING_LENGTH);

    index = 200;
    EXPECT_EQ(ManualSetupPayloadParser::ReadDigitsFromDecimalString("6256276377282", index, number, 1),
              CHIP_ERROR_INVALID_STRING_LENGTH);
}

TEST(TestManualCode, TestShortCodeCharLengths)
{
    size_t numBits                        = 1 + kSetupPINCodeFieldLengthInBits + kManualSetupDiscriminatorFieldLengthInBits;
    size_t manualSetupShortCodeCharLength = static_cast<size_t>(ceil(log10(pow(2, numBits))));
    EXPECT_EQ(manualSetupShortCodeCharLength, size_t(kManualSetupShortCodeCharLength));

    size_t manualSetupVendorIdCharLength = static_cast<size_t>(ceil(log10(pow(2, kVendorIDFieldLengthInBits))));
    EXPECT_EQ(manualSetupVendorIdCharLength, size_t(kManualSetupVendorIdCharLength));

    size_t manualSetupProductIdCharLength = static_cast<size_t>(ceil(log10(pow(2, kProductIDFieldLengthInBits))));
    EXPECT_EQ(manualSetupProductIdCharLength, size_t(kManualSetupProductIdCharLength));

    size_t manualSetupLongCodeCharLength =
        kManualSetupShortCodeCharLength + kManualSetupVendorIdCharLength + kManualSetupProductIdCharLength;
    EXPECT_EQ(manualSetupLongCodeCharLength, size_t(kManualSetupLongCodeCharLength));
}

} // namespace
