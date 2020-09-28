/**
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
 *      This file describes a Manul Setup Payload parser based on the
 *      CHIP specification.
 */

#include "ManualSetupPayloadParser.h"

#include <support/logging/CHIPLogging.h>
#include <support/verhoeff/Verhoeff.h>

#include <math.h>
#include <string>
#include <vector>

using namespace chip;
using namespace std;

static CHIP_ERROR checkDecimalStringValidity(string decimalString, string & decimalStringWithoutCheckDigit)
{
    if (decimalString.length() < 2)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Input was empty. %zu", decimalString.length());
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    string repWithoutCheckChar = decimalString.substr(0, decimalString.length() - 1);
    char checkChar             = decimalString.back();

    if (!Verhoeff10::ValidateCheckChar(checkChar, repWithoutCheckChar.c_str()))
    {
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    }
    decimalStringWithoutCheckDigit = repWithoutCheckChar;
    return CHIP_NO_ERROR;
}

static CHIP_ERROR checkCodeLengthValidity(string decimalString, bool isLongCode)
{
    size_t expectedCharLength = isLongCode ? kManualSetupLongCodeCharLength : kManualSetupShortCodeCharLength;
    if (decimalString.length() != expectedCharLength)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Input length %zu was not expected length %zu", decimalString.length(),
                     expectedCharLength);
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    return CHIP_NO_ERROR;
}

// Extract n bits starting at index i and store it in dest
static CHIP_ERROR extractBits(uint32_t number, uint64_t & dest, int index, int numberBits, int maxBits)
{
    if ((index + numberBits) > maxBits)
    {
        ChipLogError(SetupPayload, "Number %u maxBits %d index %d n %d", number, maxBits, index, numberBits);
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    dest = (((1 << numberBits) - 1) & (number >> index));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR toNumber(string decimalString, uint64_t & dest)
{
    uint64_t number = 0;
    for (char c : decimalString)
    {
        if (!isdigit(c))
        {
            ChipLogError(SetupPayload, "Failed decoding base10. Character was invalid %c", c);
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }
        number *= 10;
        number += c - '0';
    }
    dest = number;
    return CHIP_NO_ERROR;
}

// Populate numberOfChars into dest from decimalString starting at startIndex (least significant digit = left-most digit)
static CHIP_ERROR readDigitsFromDecimalString(string decimalString, int & index, uint64_t & dest, size_t numberOfCharsToRead)
{
    if (decimalString.length() < numberOfCharsToRead || (numberOfCharsToRead + index > decimalString.length()))
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Input was too short. %zu", decimalString.length());
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    if (index < 0)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Index was negative. %d", index);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    string decimalSubstring = decimalString.substr(index, numberOfCharsToRead);
    index += numberOfCharsToRead;
    return toNumber(decimalSubstring, dest);
}

// Populate numberOfBits into dest from number starting at startIndex (LSB = right-most bit)
static CHIP_ERROR readBitsFromNumber(int32_t number, int & index, uint64_t & dest, size_t numberOfBitsToRead, size_t maxBits)
{
    uint64_t bits     = 0;
    CHIP_ERROR result = extractBits(number, bits, index, numberOfBitsToRead, maxBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    index += numberOfBitsToRead;
    dest = bits;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualSetupPayloadParser::populatePayload(SetupPayload & outPayload)
{
    CHIP_ERROR result = CHIP_NO_ERROR;
    SetupPayload payload;
    string representationWithoutCheckDigit;

    result = checkDecimalStringValidity(mDecimalStringRepresentation, representationWithoutCheckDigit);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    int stringOffset = 0;
    uint64_t shortCode;
    result = readDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, shortCode, kManualSetupShortCodeCharLength);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    bool isLongCode = (shortCode & 1) == 1;
    result          = checkCodeLengthValidity(representationWithoutCheckDigit, isLongCode);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    int numberOffset = 1;
    uint64_t setUpPINCode;
    size_t maxShortCodeBitsLength = 1 + kSetupPINCodeFieldLengthInBits + kManualSetupDiscriminatorFieldLengthInBits;

    uint64_t discriminator;
    result = readBitsFromNumber(shortCode, numberOffset, discriminator, kManualSetupDiscriminatorFieldLengthInBits,
                                maxShortCodeBitsLength);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    result = readBitsFromNumber(shortCode, numberOffset, setUpPINCode, kSetupPINCodeFieldLengthInBits, maxShortCodeBitsLength);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    if (setUpPINCode == 0)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. SetUpPINCode was 0.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (isLongCode)
    {
        uint64_t vendorID;
        result =
            readDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, vendorID, kManualSetupVendorIdCharLength);
        if (result != CHIP_NO_ERROR)
        {
            return result;
        }

        uint64_t productID;
        result =
            readDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, productID, kManualSetupProductIdCharLength);
        if (result != CHIP_NO_ERROR)
        {
            return result;
        }
        outPayload.vendorID  = vendorID;
        outPayload.productID = productID;
    }
    outPayload.requiresCustomFlow = isLongCode ? 1 : 0;
    outPayload.setUpPINCode       = setUpPINCode;
    outPayload.discriminator      = discriminator;

    return result;
}
