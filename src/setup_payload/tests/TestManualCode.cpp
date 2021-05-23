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

#include <nlunit-test.h>
#include <stdio.h>

#include "ManualSetupPayloadGenerator.h"
#include "ManualSetupPayloadParser.cpp"
#include "SetupPayload.h"

#include <support/UnitTestRegistration.h>
#include <support/verhoeff/Verhoeff.h>

using namespace chip;

namespace {

bool CheckGenerator(const SetupPayload & payload, std::string expectedResult)
{
    std::string result;
    ManualSetupPayloadGenerator generator(payload);
    generator.payloadDecimalStringRepresentation(result);

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

SetupPayload GetDefaultPayload()
{
    SetupPayload payload;
    payload.setUpPINCode  = 123456780;
    payload.discriminator = 2560;

    return payload;
}

void TestDecimalRepresentation_PartialPayload(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    std::string expectedResult = "2361087535";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_PartialPayload_RequiresCustomFlow(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload       = GetDefaultPayload();
    payload.requiresCustomFlow = true;

    std::string expectedResult = "63610875350000000000";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_FullPayloadWithZeros(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload       = GetDefaultPayload();
    payload.requiresCustomFlow = true;
    payload.vendorID           = 1;
    payload.productID          = 1;

    std::string expectedResult = "63610875350000100001";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_FullPayloadWithoutZeros(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload       = GetDefaultPayload();
    payload.requiresCustomFlow = true;
    payload.vendorID           = 45367;
    payload.productID          = 14526;

    std::string expectedResult = "63610875354536714526";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();
    payload.vendorID     = 45367;
    payload.productID    = 14526;

    std::string expectedResult = "2361087535";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_AllZeros(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload;
    payload.setUpPINCode  = 0;
    payload.discriminator = 0;

    std::string expectedResult = "";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_AllOnes(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload;
    payload.setUpPINCode       = 0x7FFFFFF;
    payload.discriminator      = 0xFFF;
    payload.requiresCustomFlow = true;
    payload.vendorID           = 65535;
    payload.productID          = 65535;

    std::string expectedResult = "76553581916553565535";

    NL_TEST_ASSERT(inSuite, CheckGenerator(payload, expectedResult));
}

void TestDecimalRepresentation_InvalidPayload(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload  = GetDefaultPayload();
    payload.discriminator = 0x1f00;

    ManualSetupPayloadGenerator generator(payload);
    std::string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_ERROR_INVALID_ARGUMENT);
}

void assertPayloadValues(nlTestSuite * inSuite, CHIP_ERROR actualError, CHIP_ERROR expectedError, const SetupPayload & payload,
                         uint32_t pinCode, uint16_t discriminator, uint16_t vendorID, uint16_t productID)
{
    NL_TEST_ASSERT(inSuite, actualError == expectedError);
    NL_TEST_ASSERT(inSuite, payload.setUpPINCode == pinCode);
    NL_TEST_ASSERT(inSuite, payload.discriminator == discriminator);
    NL_TEST_ASSERT(inSuite, payload.vendorID == vendorID);
    NL_TEST_ASSERT(inSuite, payload.productID == productID);
}

void TestGenerateAndParser_ManualSetupCodeWithLongDiscriminator(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload       = GetDefaultPayload();
    payload.requiresCustomFlow = false;
    payload.discriminator      = 0xa1f;

    {
        // Test short 11 digit code
        ManualSetupPayloadGenerator generator(payload);
        std::string result;
        NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);

        SetupPayload outPayload;
        CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
        assertPayloadValues(inSuite, err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, 0xa00, payload.vendorID,
                            payload.productID);
    }

    payload.vendorID           = 1;
    payload.productID          = 1;
    payload.requiresCustomFlow = true;
    payload.discriminator      = 0xb1f;

    {
        // Test long 21 digit code
        ManualSetupPayloadGenerator generator(payload);
        std::string result;
        NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);

        SetupPayload outPayload;
        CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
        assertPayloadValues(inSuite, err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, 0xb00, payload.vendorID,
                            payload.productID);
    }
}

void assertEmptyPayloadWithError(nlTestSuite * inSuite, CHIP_ERROR actualError, CHIP_ERROR expectedError,
                                 const SetupPayload & payload)
{
    NL_TEST_ASSERT(inSuite, actualError == expectedError);
    NL_TEST_ASSERT(inSuite,
                   payload.setUpPINCode == 0 && payload.discriminator == 0 && payload.productID == 0 && payload.vendorID == 0);
}

void TestPayloadParser_FullPayload(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload;
    std::string decimalString;

    decimalString = "63610875354536714526";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    CHIP_ERROR err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 123456780, 2560, 45367, 14526);

    decimalString = "52927623630456200032";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 38728284, 1280, 4562, 32);

    decimalString = "40000100000000100001";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 1, 0, 1, 1);
}

void TestGenerateAndParser_FullPayload(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload       = GetDefaultPayload();
    payload.vendorID           = 1;
    payload.productID          = 1;
    payload.requiresCustomFlow = true;

    ManualSetupPayloadGenerator generator(payload);
    std::string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);

    SetupPayload outPayload;
    CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, payload.discriminator, payload.vendorID,
                        payload.productID);
}

void TestGenerateAndParser_PartialPayload(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload = GetDefaultPayload();

    ManualSetupPayloadGenerator generator(payload);
    std::string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);

    SetupPayload outPayload;
    CHIP_ERROR err = ManualSetupPayloadParser(result).populatePayload(outPayload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, outPayload, payload.setUpPINCode, payload.discriminator, payload.vendorID,
                        payload.productID);
}

void TestPayloadParser_PartialPayload(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    SetupPayload payload;
    std::string decimalString;

    decimalString = "2361087535";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    NL_TEST_ASSERT(inSuite, decimalString.length() == 11);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 123456780, 2560, 0, 0);

    decimalString = "0000010000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    NL_TEST_ASSERT(inSuite, decimalString.length() == 11);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 1, 0, 0, 0);

    decimalString = "63610875350000000000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    NL_TEST_ASSERT(inSuite, decimalString.length() == 21);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 123456780, 2560, 0, 0);

    // no discriminator (= 0)
    decimalString = "0033407535";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    NL_TEST_ASSERT(inSuite, decimalString.length() == 11);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // no vid (= 0)
    decimalString = "63610875350000014526";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    NL_TEST_ASSERT(inSuite, decimalString.length() == 21);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // no pid (= 0)
    decimalString = "63610875354536700000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    NL_TEST_ASSERT(inSuite, decimalString.length() == 21);
    err = ManualSetupPayloadParser(decimalString).populatePayload(payload);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestShortCodeReadWrite(nlTestSuite * inSuite, void * context)
{
    SetupPayload inPayload = GetDefaultPayload();
    SetupPayload outPayload;

    std::string result;
    ManualSetupPayloadGenerator generator(inPayload);
    generator.payloadDecimalStringRepresentation(result);
    ManualSetupPayloadParser(result).populatePayload(outPayload);

    NL_TEST_ASSERT(inSuite, inPayload == outPayload);
}

void TestLongCodeReadWrite(nlTestSuite * inSuite, void * context)
{
    SetupPayload inPayload       = GetDefaultPayload();
    inPayload.requiresCustomFlow = true;
    inPayload.vendorID           = 1;
    inPayload.productID          = 1;
    SetupPayload outPayload;

    std::string result;
    ManualSetupPayloadGenerator generator(inPayload);
    generator.payloadDecimalStringRepresentation(result);
    ManualSetupPayloadParser(result).populatePayload(outPayload);

    NL_TEST_ASSERT(inSuite, inPayload == outPayload);
}

void TestPayloadParser_InvalidEntry(nlTestSuite * inSuite, void * inContext)
{
    SetupPayload payload;
    std::string decimalString;

    // Empty input
    decimalString = "";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // Invalid character
    decimalString = "24184.2196";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_INTEGER_VALUE, payload);

    // too short
    decimalString = "2456";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // too long for long code
    decimalString = "123456789123456785671";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // too long for short code
    decimalString = "12749875380";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // bit to indicate short code but long code length
    decimalString = "23456789123456785610";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);
    // no pin code (= 0)
    decimalString = "2327680000";
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INVALID_ARGUMENT, payload);
    // wrong check digit
    decimalString = "02684354589";
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser(decimalString).populatePayload(payload),
                                CHIP_ERROR_INTEGRITY_CHECK_FAILED, payload);
}

void TestCheckDecimalStringValidity(nlTestSuite * inSuite, void * inContext)
{
    std::string outReprensation;
    char checkDigit;
    std::string representationWithoutCheckDigit;
    std::string decimalString;

    representationWithoutCheckDigit = "";
    NL_TEST_ASSERT(
        inSuite, checkDecimalStringValidity(representationWithoutCheckDigit, outReprensation) == CHIP_ERROR_INVALID_STRING_LENGTH);

    representationWithoutCheckDigit = "1";
    NL_TEST_ASSERT(
        inSuite, checkDecimalStringValidity(representationWithoutCheckDigit, outReprensation) == CHIP_ERROR_INVALID_STRING_LENGTH);

    representationWithoutCheckDigit = "10109";
    checkDigit                      = Verhoeff10::ComputeCheckChar(representationWithoutCheckDigit.c_str());
    decimalString                   = representationWithoutCheckDigit + checkDigit;
    NL_TEST_ASSERT(inSuite, checkDecimalStringValidity(decimalString, outReprensation) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, outReprensation == representationWithoutCheckDigit);

    representationWithoutCheckDigit = "0000";
    checkDigit                      = Verhoeff10::ComputeCheckChar(representationWithoutCheckDigit.c_str());
    decimalString                   = representationWithoutCheckDigit + checkDigit;
    NL_TEST_ASSERT(inSuite, checkDecimalStringValidity(decimalString, outReprensation) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, outReprensation == representationWithoutCheckDigit);
}

void TestCheckCodeLengthValidity(nlTestSuite * inSuite, void * inContext)
{
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("01234567890123456789", true) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("0123456789", false) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("01234567891", false) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("012345678", false) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("012345678901234567891", true) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("0123456789012345678", true) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestDecimalStringToNumber(nlTestSuite * inSuite, void * inContext)
{
    uint32_t number;
    NL_TEST_ASSERT(inSuite, toNumber("12345", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 12345);

    NL_TEST_ASSERT(inSuite, toNumber("01234567890", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1234567890);

    NL_TEST_ASSERT(inSuite, toNumber("00000001", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1);

    NL_TEST_ASSERT(inSuite, toNumber("0", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 0);

    NL_TEST_ASSERT(inSuite, toNumber("012345.123456789", number) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, toNumber("/", number) == CHIP_ERROR_INVALID_INTEGER_VALUE);
}

void TestReadCharsFromDecimalString(nlTestSuite * inSuite, void * inContext)
{
    uint32_t number;
    size_t index = 3;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("12345", index, number, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 45);

    index = 2;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("6256276377282", index, number, 7) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 5627637);

    index = 0;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("10", index, number, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 10);

    index = 0;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("01", index, number, 2) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1);

    index = 1;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("11", index, number, 1) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1);

    index = 2;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("100001", index, number, 3) == CHIP_NO_ERROR);

    index = 1;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("12345", index, number, 5) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("12", index, number, 5) == CHIP_ERROR_INVALID_STRING_LENGTH);

    index = 200;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("6256276377282", index, number, 1) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestShortCodeCharLengths(nlTestSuite * inSuite, void * inContext)
{
    size_t numBits                        = 1 + kSetupPINCodeFieldLengthInBits + kManualSetupDiscriminatorFieldLengthInBits;
    size_t manualSetupShortCodeCharLength = static_cast<size_t>(ceil(log10(pow(2, numBits))));
    NL_TEST_ASSERT(inSuite, manualSetupShortCodeCharLength == kManualSetupShortCodeCharLength);

    size_t manualSetupVendorIdCharLength = static_cast<size_t>(ceil(log10(pow(2, kVendorIDFieldLengthInBits))));
    NL_TEST_ASSERT(inSuite, manualSetupVendorIdCharLength == kManualSetupVendorIdCharLength);

    size_t manualSetupProductIdCharLength = static_cast<size_t>(ceil(log10(pow(2, kProductIDFieldLengthInBits))));
    NL_TEST_ASSERT(inSuite, manualSetupProductIdCharLength == kManualSetupProductIdCharLength);

    size_t manualSetupLongCodeCharLength =
        kManualSetupShortCodeCharLength + kManualSetupVendorIdCharLength + kManualSetupProductIdCharLength;
    NL_TEST_ASSERT(inSuite, manualSetupLongCodeCharLength == kManualSetupLongCodeCharLength);
}

/**
 *  Test Suite that lists all the Test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Generate Decimal Representation from Partial Payload",                 TestDecimalRepresentation_PartialPayload),
    NL_TEST_DEF("Generate Decimal Representation from Partial Payload (Custom Flow)",   TestDecimalRepresentation_PartialPayload_RequiresCustomFlow),
    NL_TEST_DEF("Generate Decimal Representation from Full Payload with Zeros",         TestDecimalRepresentation_FullPayloadWithZeros),
    NL_TEST_DEF("Decimal Representation from Full Payload without Zeros",               TestDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow),
    NL_TEST_DEF("Decimal Representation from Full Payload without Zeros (Custom Flow)", TestDecimalRepresentation_FullPayloadWithoutZeros),
    NL_TEST_DEF("Test Decimal Representation - Invalid Payload",                        TestDecimalRepresentation_InvalidPayload),
    NL_TEST_DEF("Test 12 bit discriminator for manual setup code",                      TestGenerateAndParser_ManualSetupCodeWithLongDiscriminator),
    NL_TEST_DEF("Test Decimal Representation - All Zeros",                              TestDecimalRepresentation_AllZeros),
    NL_TEST_DEF("Test Decimal Representation - All Ones",                               TestDecimalRepresentation_AllOnes),
    NL_TEST_DEF("Parse from Partial Payload",                                           TestPayloadParser_PartialPayload),
    NL_TEST_DEF("Parse from Full Payload",                                              TestPayloadParser_FullPayload),
    NL_TEST_DEF("Test Invalid Entry To QR Code Parser",                                 TestPayloadParser_InvalidEntry),
    NL_TEST_DEF("Test Short Read Write",                                                TestShortCodeReadWrite),
    NL_TEST_DEF("Test Long Read Write",                                                 TestLongCodeReadWrite),
    NL_TEST_DEF("Check Decimal String Validity",                                        TestCheckDecimalStringValidity),
    NL_TEST_DEF("Check QR Code Length Validity",                                        TestCheckCodeLengthValidity),
    NL_TEST_DEF("Test Decimal String to Number",                                        TestDecimalStringToNumber),
    NL_TEST_DEF("Test Short Code Character Lengths",                                    TestShortCodeCharLengths),
    NL_TEST_DEF("Test Read Characters from Decimal String",                             TestReadCharsFromDecimalString),
    NL_TEST_DEF("Generate Full Payload and Parse it",                                   TestGenerateAndParser_FullPayload),
    NL_TEST_DEF("Generate Partial Payload and Parse it",                                TestGenerateAndParser_PartialPayload),

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
int TestManualSetupCode()
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

CHIP_REGISTER_TEST_SUITE(TestManualSetupCode);
