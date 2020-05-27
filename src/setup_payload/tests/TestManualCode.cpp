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

#include "TestManualCode.h"

#include <iostream>
#include <nlbyteorder.h>
#include <nlunit-test.h>

#include "ManualSetupPayloadGenerator.cpp"
#include "ManualSetupPayloadParser.cpp"
#include "SetupPayload.cpp"
#include "SetupPayload.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace chip;

void TestDecimalRepresentation_PartialPayload(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 2345;
    payload.discriminator = 13;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "3489665618";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    NL_TEST_ASSERT(inSuite, succeeded);
}

void TestDecimalRepresentation_PartialPayload_RequiresCustomFlow(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode       = 2345;
    payload.discriminator      = 13;
    payload.requiresCustomFlow = true;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "34896656190000000000";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result:   %s\n", result.c_str());
    }
    NL_TEST_ASSERT(inSuite, succeeded);
}

void TestDecimalRepresentation_FullPayloadWithoutZeros(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode       = 2345;
    payload.discriminator      = 13;
    payload.vendorID           = 45367;
    payload.productID          = 14526;
    payload.requiresCustomFlow = true;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "34896656194536714526";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    NL_TEST_ASSERT(inSuite, succeeded);
}

void TestDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 2345;
    payload.discriminator = 13;
    payload.vendorID      = 45367;
    payload.productID     = 14526;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "3489665618";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    NL_TEST_ASSERT(inSuite, succeeded);
}

void TestDecimalRepresentation_FullPayloadWithZeros(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode       = 2345;
    payload.discriminator      = 13;
    payload.vendorID           = 1;
    payload.productID          = 1;
    payload.requiresCustomFlow = true;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "34896656190000100001";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    NL_TEST_ASSERT(inSuite, succeeded);
}

void TestDecimalRepresentation_AllZeros(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 0;
    payload.discriminator = 0;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_ERROR_INVALID_ARGUMENT);
    string expectedResult = "";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    NL_TEST_ASSERT(inSuite, succeeded);
}

void TestDecimalRepresentation_InvalidPayload(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 2345;
    payload.discriminator = 18;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    NL_TEST_ASSERT(inSuite, generator.payloadDecimalStringRepresentation(result) == CHIP_ERROR_INVALID_ARGUMENT);
}

void assertPayloadValues(nlTestSuite * inSuite, CHIP_ERROR actualError, CHIP_ERROR expectedError, SetupPayload payload,
                         uint32_t pinCode, uint8_t discriminator, uint16_t vendorID, uint16_t productID)
{
    NL_TEST_ASSERT(inSuite, actualError == expectedError);
    NL_TEST_ASSERT(inSuite, payload.setUpPINCode == pinCode);
    NL_TEST_ASSERT(inSuite, payload.discriminator == discriminator);
    NL_TEST_ASSERT(inSuite, payload.vendorID == vendorID);
    NL_TEST_ASSERT(inSuite, payload.productID == productID);
}

void assertEmptyPayloadWithError(nlTestSuite * inSuite, CHIP_ERROR actualError, CHIP_ERROR expectedError, SetupPayload payload)
{
    NL_TEST_ASSERT(inSuite, actualError == expectedError);
    NL_TEST_ASSERT(inSuite,
                   payload.setUpPINCode == 0 && payload.discriminator == 0 && payload.productID == 0 && payload.vendorID == 0);
}

void TestPayloadParser_FullPayload(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    CHIP_ERROR err = ManualSetupPayloadParser("34896656194536714526").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 2345, 13, 45367, 14526);
    err = ManualSetupPayloadParser("41039884090456200032").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 38728284, 15, 4562, 32);
    err = ManualSetupPayloadParser("02684354590000100001").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 1, 1, 1, 1);
}

void TestPayloadParser_PartialPayload(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    CHIP_ERROR err = ManualSetupPayloadParser("2418402196").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 1241546, 9, 0, 0);

    err = ManualSetupPayloadParser("3669676692").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 90007882, 13, 0, 0);

    err = ManualSetupPayloadParser("0268435458").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 1, 1, 0, 0);

    err = ManualSetupPayloadParser("00000000030000000000").populatePayload(payload);
    assertPayloadValues(inSuite, err, CHIP_NO_ERROR, payload, 1, 0, 0, 0);

    // no discriminator (= 0)
    NL_TEST_ASSERT(inSuite, ManualSetupPayloadParser("0000070548").populatePayload(payload) == CHIP_NO_ERROR);

    // no vid (= 0)
    NL_TEST_ASSERT(inSuite, ManualSetupPayloadParser("40321242450000014536").populatePayload(payload) == CHIP_NO_ERROR);

    // no pid (= 0)
    NL_TEST_ASSERT(inSuite, ManualSetupPayloadParser("40321242452645300000").populatePayload(payload) == CHIP_NO_ERROR);
}

void TestExtractBits(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t dest;
    CHIP_ERROR result = extractBits(5, dest, 2, 1, 3);
    NL_TEST_ASSERT(inSuite, result == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dest == 1);

    result = extractBits(4567, dest, 4, 8, 13);
    NL_TEST_ASSERT(inSuite, result == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, dest == 29);

    NL_TEST_ASSERT(inSuite, extractBits(4567, dest, 4, 18, 13) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, extractBits(4567, dest, 14, 2, 13) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, extractBits(5, dest, 3, 1, 3) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, extractBits(5, dest, 2, 2, 3) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestPayloadParser_InvalidEntry(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;

    // Empty input
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("").populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload);

    // Invalid character
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("24184.2196").populatePayload(payload),
                                CHIP_ERROR_INVALID_INTEGER_VALUE, payload);

    // too short
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("2456").populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // too long for long code
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("123456789123456785671").populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // too long for short code
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("12749875380").populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // bit to indicate short code but long code length
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("23456789123456785610").populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);
    // no pin code (= 0)
    assertEmptyPayloadWithError(inSuite, ManualSetupPayloadParser("3221225472").populatePayload(payload),
                                CHIP_ERROR_INVALID_ARGUMENT, payload);
}

void TestCheckDecimalStringValidity(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    NL_TEST_ASSERT(inSuite, checkDecimalStringValidity("") == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkDecimalStringValidity("1010.9") == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, checkDecimalStringValidity("0000") == CHIP_NO_ERROR);
}

void TestCheckCodeLengthValidity(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("01234567890123456789", true) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("0123456789", false) == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("01234567891", false) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("012345678", false) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("012345678901234567891", true) == CHIP_ERROR_INVALID_STRING_LENGTH);
    NL_TEST_ASSERT(inSuite, checkCodeLengthValidity("0123456789012345678", true) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestDecimalStringToNumber(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t number;
    NL_TEST_ASSERT(inSuite, toNumber("12345", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 12345);

    NL_TEST_ASSERT(inSuite, toNumber("01234567890123456789", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1234567890123456789);

    NL_TEST_ASSERT(inSuite, toNumber("00000001", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1);

    NL_TEST_ASSERT(inSuite, toNumber("0", number) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 0);

    NL_TEST_ASSERT(inSuite, toNumber("012345.123456789", number) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    NL_TEST_ASSERT(inSuite, toNumber("/", number) == CHIP_ERROR_INVALID_INTEGER_VALUE);
}

void TestReadCharsFromDecimalString(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t number;
    int index = 3;
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

    index = -2;
    NL_TEST_ASSERT(inSuite, readDigitsFromDecimalString("6256276377282", index, number, 7) == CHIP_ERROR_INVALID_ARGUMENT);
}

void TestReadBitsFromNumber(nlTestSuite * inSuite, void * inContext)
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t number;
    int index = 3;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(12345, index, number, 6, 14) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 7);

    index = 0;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(12345, index, number, 14, 14) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 12345);

    index = 0;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(1, index, number, 1, 14) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 1);

    index = 0;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(12345, index, number, 20, 22) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 12345);

    index = 1;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(1, index, number, 1, 14) == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, number == 0);

    index = 0;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(12345, index, number, 15, 14) == CHIP_ERROR_INVALID_STRING_LENGTH);

    index = 14;
    NL_TEST_ASSERT(inSuite, readBitsFromNumber(12345, index, number, 1, 14) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void TestShortCodeCharLengths(nlTestSuite * inSuite, void * inContext)
{
    size_t numBits                        = 1 + kSetupPINCodeFieldLengthInBits + kManualSetupDiscriminatorFieldLengthInBits;
    size_t manualSetupShortCodeCharLength = ceil(log10(pow(2, numBits)));
    NL_TEST_ASSERT(inSuite, manualSetupShortCodeCharLength == kManualSetupShortCodeCharLength);

    size_t manualSetupVendorIdCharLength = ceil(log10(pow(2, kVendorIDFieldLengthInBits)));
    NL_TEST_ASSERT(inSuite, manualSetupVendorIdCharLength == kManualSetupVendorIdCharLength);

    size_t manualSetupProductIdCharLength = ceil(log10(pow(2, kProductIDFieldLengthInBits)));
    NL_TEST_ASSERT(inSuite, manualSetupProductIdCharLength == kManualSetupProductIdCharLength);

    size_t manualSetupLongCodeCharLength =
        kManualSetupShortCodeCharLength + kManualSetupVendorIdCharLength + kManualSetupProductIdCharLength;
    NL_TEST_ASSERT(inSuite, manualSetupLongCodeCharLength == kManualSetupLongCodeCharLength);
}

/**
 *  Test Suite that lists all the Test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("Generate Decimal Representation from Partial Payload",                 TestDecimalRepresentation_PartialPayload),
    NL_TEST_DEF("Generate Decimal Representation from Partial Payload (Custom Flow)",   TestDecimalRepresentation_PartialPayload_RequiresCustomFlow),
    NL_TEST_DEF("Generate Decimal Representation from Full Payload with Zeros",         TestDecimalRepresentation_FullPayloadWithZeros),
    NL_TEST_DEF("Decimal Representation from Full Payload without Zeros",               TestDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow),
    NL_TEST_DEF("Decimal Representation from Full Payload without Zeros (Custom Flow)", TestDecimalRepresentation_FullPayloadWithoutZeros),
    NL_TEST_DEF("Test Decimal Representation - Invalid Payload",                        TestDecimalRepresentation_InvalidPayload),
    NL_TEST_DEF("Test Decimal Representation - All Zeros",                              TestDecimalRepresentation_AllZeros),
    NL_TEST_DEF("Parse from Partial Payload",                                           TestPayloadParser_PartialPayload),
    NL_TEST_DEF("Parse from Full Payload",                                              TestPayloadParser_FullPayload),
    NL_TEST_DEF("Test Invalid Entry To QR Code Parser",                                 TestPayloadParser_InvalidEntry),
    NL_TEST_DEF("Test Extract Bits",                                                    TestExtractBits),
    NL_TEST_DEF("Check Decimal String Validity",                                        TestCheckDecimalStringValidity),
    NL_TEST_DEF("Check QR Code Length Validity",                                        TestCheckCodeLengthValidity),
    NL_TEST_DEF("Test Decimal String to Number",                                        TestDecimalStringToNumber),
    NL_TEST_DEF("Read Bits from Number",                                                TestReadBitsFromNumber),
    NL_TEST_DEF("Test Short Code Character Lengths",                                    TestShortCodeCharLengths),
    NL_TEST_DEF("Test Read Charatcers from Decimal String",                             TestReadCharsFromDecimalString),


    NL_TEST_SENTINEL()
};
// clang-format on

struct TestContext
{
    nlTestSuite * mSuite;
};

/**
 *  Main
 */
int TestManualSetupCode(void)
{
    // clang-format off
    nlTestSuite theSuite =
    {
        "chip-manual-code-general-Tests",
        &sTests[0],
        NULL,
        NULL
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
