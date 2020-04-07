/*
 *
 *    <COPYRIGHT>
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

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "ManualSetupPayloadGenerator.cpp"
#include "ManualSetupPayloadParser.cpp"
#include "SetupPayload.cpp"
#include "SetupPayload.h"

using namespace chip;

void testDecimalRepresentation_PartialPayload()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 2345;
    payload.discriminator = 13;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "3489665618";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    assert(succeeded);
}

void testDecimalRepresentation_PartialPayload_RequiresCustomFlow()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode       = 2345;
    payload.discriminator      = 13;
    payload.requiresCustomFlow = true;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "34896656190000000000";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result:   %s\n", result.c_str());
    }
    assert(succeeded);
}

void testDecimalRepresentation_FullPayloadWithoutZeros()
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
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "34896656194536714526";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    assert(succeeded);
}

void testDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 2345;
    payload.discriminator = 13;
    payload.vendorID      = 45367;
    payload.productID     = 14526;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "3489665618";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    assert(succeeded);
}

void testDecimalRepresentation_FullPayloadWithZeros()
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
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_NO_ERROR);
    string expectedResult = "34896656190000100001";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    assert(succeeded);
}

void testDecimalRepresentation_AllZeros()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 0;
    payload.discriminator = 0;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_ERROR_INVALID_ARGUMENT);
    string expectedResult = "";

    bool succeeded = result.compare(expectedResult) == 0;
    if (!succeeded)
    {
        printf("Expected result: %s\n", expectedResult.c_str());
        printf("Actual result: %s\n", result.c_str());
    }
    assert(succeeded);
}

void testDecimalRepresentation_InvalidPayload()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    payload.setUpPINCode  = 2345;
    payload.discriminator = 18;

    ManualSetupPayloadGenerator generator(payload);
    string result;
    assert(generator.payloadDecimalStringRepresentation(result) == CHIP_ERROR_INVALID_ARGUMENT);
}

void assertPayloadValues(CHIP_ERROR actualError, CHIP_ERROR expectedError, SetupPayload payload, uint32_t pinCode,
                         uint8_t discriminator, uint16_t vendorID, uint16_t productID)
{
    assert(actualError == expectedError);
    assert(payload.setUpPINCode == pinCode);
    assert(payload.discriminator == discriminator);
    assert(payload.vendorID == vendorID);
    assert(payload.productID == productID);
}

void assertEmptyPayloadWithError(CHIP_ERROR actualError, CHIP_ERROR expectedError, SetupPayload payload)
{
    assert(actualError == expectedError);
    assert(payload.setUpPINCode == 0 && payload.discriminator == 0 && payload.productID == 0 && payload.vendorID == 0);
}

void testPayloadParser_FullPayload()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    assertPayloadValues(ManualSetupPayloadParser("34896656194536714526").populatePayload(payload), CHIP_NO_ERROR, payload, 2345, 13,
                        45367, 14526);
    assertPayloadValues(ManualSetupPayloadParser("41039884090456200032").populatePayload(payload), CHIP_NO_ERROR, payload, 38728284,
                        15, 4562, 32);
    assertPayloadValues(ManualSetupPayloadParser("02684354590000100001").populatePayload(payload), CHIP_NO_ERROR, payload, 1, 1, 1,
                        1);
}

void testPayloadParser_PartialPayload()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;
    assertPayloadValues(ManualSetupPayloadParser("2418402196").populatePayload(payload), CHIP_NO_ERROR, payload, 1241546, 9, 0, 0);
    assertPayloadValues(ManualSetupPayloadParser("3669676692").populatePayload(payload), CHIP_NO_ERROR, payload, 90007882, 13, 0,
                        0);
    assertPayloadValues(ManualSetupPayloadParser("0268435458").populatePayload(payload), CHIP_NO_ERROR, payload, 1, 1, 0, 0);
    assertPayloadValues(ManualSetupPayloadParser("00000000030000000000").populatePayload(payload), CHIP_NO_ERROR, payload, 1, 0, 0,
                        0);

    // no discriminator (= 0)
    assert(ManualSetupPayloadParser("0000070548").populatePayload(payload) == CHIP_NO_ERROR);

    // no vid (= 0)
    assert(ManualSetupPayloadParser("40321242450000014536").populatePayload(payload) == CHIP_NO_ERROR);

    // no pid (= 0)
    assert(ManualSetupPayloadParser("40321242452645300000").populatePayload(payload) == CHIP_NO_ERROR);
}

void testExtractBits()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t dest;
    CHIP_ERROR result = extractBits(5, dest, 2, 1, 3);
    assert(result == CHIP_NO_ERROR);
    assert(dest == 1);

    result = extractBits(4567, dest, 4, 8, 13);
    assert(result == CHIP_NO_ERROR);
    assert(dest == 29);

    assert(extractBits(4567, dest, 4, 18, 13) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(extractBits(4567, dest, 14, 2, 13) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(extractBits(5, dest, 3, 1, 3) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(extractBits(5, dest, 2, 2, 3) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void testPayloadParser_InvalidEntry()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    SetupPayload payload;

    // Empty input
    assertEmptyPayloadWithError(ManualSetupPayloadParser("").populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // Invalid character
    assertEmptyPayloadWithError(ManualSetupPayloadParser("24184.2196").populatePayload(payload), CHIP_ERROR_INVALID_INTEGER_VALUE,
                                payload);

    // too short
    assertEmptyPayloadWithError(ManualSetupPayloadParser("2456").populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload);

    // too long for long code
    assertEmptyPayloadWithError(ManualSetupPayloadParser("123456789123456785671").populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);

    // too long for short code
    assertEmptyPayloadWithError(ManualSetupPayloadParser("12749875380").populatePayload(payload), CHIP_ERROR_INVALID_STRING_LENGTH,
                                payload);

    // bit to indicate short code but long code length
    assertEmptyPayloadWithError(ManualSetupPayloadParser("23456789123456785610").populatePayload(payload),
                                CHIP_ERROR_INVALID_STRING_LENGTH, payload);
    // no pin code (= 0)
    assertEmptyPayloadWithError(ManualSetupPayloadParser("3221225472").populatePayload(payload), CHIP_ERROR_INVALID_ARGUMENT,
                                payload);
}

void testCheckDecimalStringValidity()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    assert(checkDecimalStringValidity("") == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(checkDecimalStringValidity("1010.9") == CHIP_NO_ERROR);
    assert(checkDecimalStringValidity("0000") == CHIP_NO_ERROR);
}

void testCheckCodeLengthValidity()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    assert(checkCodeLengthValidity("01234567890123456789", true) == CHIP_NO_ERROR);
    assert(checkCodeLengthValidity("0123456789", false) == CHIP_NO_ERROR);

    assert(checkCodeLengthValidity("01234567891", false) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(checkCodeLengthValidity("012345678", false) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(checkCodeLengthValidity("012345678901234567891", true) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(checkCodeLengthValidity("0123456789012345678", true) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

void testDecimalStringToNumber()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t number;
    assert(toNumber("12345", number) == CHIP_NO_ERROR);
    assert(number == 12345);

    assert(toNumber("01234567890123456789", number) == CHIP_NO_ERROR);
    assert(number == 1234567890123456789);

    assert(toNumber("00000001", number) == CHIP_NO_ERROR);
    assert(number == 1);

    assert(toNumber("0", number) == CHIP_NO_ERROR);
    assert(number == 0);

    assert(toNumber("012345.123456789", number) == CHIP_ERROR_INVALID_INTEGER_VALUE);
    assert(toNumber("/", number) == CHIP_ERROR_INVALID_INTEGER_VALUE);
}

void testReadCharsFromDecimalString()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t number;
    int index = 3;
    assert(readDigitsFromDecimalString("12345", index, number, 2) == CHIP_NO_ERROR);
    assert(number == 45);

    index = 2;
    assert(readDigitsFromDecimalString("6256276377282", index, number, 7) == CHIP_NO_ERROR);
    assert(number == 5627637);

    index = 0;
    assert(readDigitsFromDecimalString("10", index, number, 2) == CHIP_NO_ERROR);
    assert(number == 10);

    index = 0;
    assert(readDigitsFromDecimalString("01", index, number, 2) == CHIP_NO_ERROR);
    assert(number == 1);

    index = 1;
    assert(readDigitsFromDecimalString("11", index, number, 1) == CHIP_NO_ERROR);
    assert(number == 1);

    index = 2;
    assert(readDigitsFromDecimalString("100001", index, number, 3) == CHIP_NO_ERROR);

    index = 1;
    assert(readDigitsFromDecimalString("12345", index, number, 5) == CHIP_ERROR_INVALID_STRING_LENGTH);
    assert(readDigitsFromDecimalString("12", index, number, 5) == CHIP_ERROR_INVALID_STRING_LENGTH);

    index = -2;
    assert(readDigitsFromDecimalString("6256276377282", index, number, 7) == CHIP_ERROR_INVALID_ARGUMENT);
}

void testReadBitsFromNumber()
{
    printf("\n---Running Test--- %s\n", __FUNCTION__);
    uint64_t number;
    int index = 3;
    assert(readBitsFromNumber(12345, index, number, 6, 14) == CHIP_NO_ERROR);
    assert(number == 7);

    index = 0;
    assert(readBitsFromNumber(12345, index, number, 14, 14) == CHIP_NO_ERROR);
    assert(number == 12345);

    index = 0;
    assert(readBitsFromNumber(1, index, number, 1, 14) == CHIP_NO_ERROR);
    assert(number == 1);

    index = 0;
    assert(readBitsFromNumber(12345, index, number, 20, 22) == CHIP_NO_ERROR);
    assert(number == 12345);

    index = 1;
    assert(readBitsFromNumber(1, index, number, 1, 14) == CHIP_NO_ERROR);
    assert(number == 0);

    index = 0;
    assert(readBitsFromNumber(12345, index, number, 15, 14) == CHIP_ERROR_INVALID_STRING_LENGTH);

    index = 14;
    assert(readBitsFromNumber(12345, index, number, 1, 14) == CHIP_ERROR_INVALID_STRING_LENGTH);
}

int main()
{
    printf("---Running Test--- tests from %s\n", __FILE__);
    testDecimalRepresentation_PartialPayload();
    testDecimalRepresentation_PartialPayload_RequiresCustomFlow();
    testDecimalRepresentation_FullPayloadWithZeros();
    testDecimalRepresentation_FullPayloadWithoutZeros_DoesNotRequireCustomFlow();
    testDecimalRepresentation_FullPayloadWithoutZeros();

    testDecimalRepresentation_InvalidPayload();
    testDecimalRepresentation_AllZeros();

    testPayloadParser_PartialPayload();
    testPayloadParser_FullPayload();
    testPayloadParser_InvalidEntry();

    testExtractBits();
    testCheckDecimalStringValidity();
    testCheckCodeLengthValidity();
    testDecimalStringToNumber();
    testReadCharsFromDecimalString();
    testReadBitsFromNumber();
}
