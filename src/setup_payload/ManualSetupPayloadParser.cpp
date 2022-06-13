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

#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/verhoeff/Verhoeff.h>

#include <math.h>
#include <string>
#include <vector>

namespace chip {

CHIP_ERROR ManualSetupPayloadParser::CheckDecimalStringValidity(std::string decimalString,
                                                                std::string & decimalStringWithoutCheckDigit)
{
    if (decimalString.length() < 2)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Input was empty. %u",
                     static_cast<unsigned int>(decimalString.length()));
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    std::string repWithoutCheckChar = decimalString.substr(0, decimalString.length() - 1);
    char checkChar                  = decimalString.back();

    if (!Verhoeff10::ValidateCheckChar(checkChar, repWithoutCheckChar.c_str()))
    {
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    }
    decimalStringWithoutCheckDigit = repWithoutCheckChar;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualSetupPayloadParser::CheckCodeLengthValidity(const std::string & decimalString, bool isLongCode)
{
    size_t expectedCharLength = isLongCode ? kManualSetupLongCodeCharLength : kManualSetupShortCodeCharLength;
    if (decimalString.length() != expectedCharLength)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Input length %u was not expected length %u",
                     static_cast<unsigned int>(decimalString.length()), static_cast<unsigned int>(expectedCharLength));
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualSetupPayloadParser::ToNumber(const std::string & decimalString, uint32_t & dest)
{
    uint32_t number = 0;
    for (char c : decimalString)
    {
        if (!isdigit(c))
        {
            ChipLogError(SetupPayload, "Failed decoding base10. Character was invalid %c", c);
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }
        number *= 10;
        number += static_cast<uint32_t>(c - '0');
    }
    dest = number;
    return CHIP_NO_ERROR;
}

// Populate numberOfChars into dest from decimalString starting at startIndex (least significant digit = left-most digit)
CHIP_ERROR ManualSetupPayloadParser::ReadDigitsFromDecimalString(const std::string & decimalString, size_t & index, uint32_t & dest,
                                                                 size_t numberOfCharsToRead)
{
    if (decimalString.length() < numberOfCharsToRead || (numberOfCharsToRead + index > decimalString.length()))
    {
        ChipLogError(SetupPayload, "Failed decoding base10. Input was too short. %u",
                     static_cast<unsigned int>(decimalString.length()));
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    std::string decimalSubstring = decimalString.substr(index, numberOfCharsToRead);
    index += numberOfCharsToRead;
    return ToNumber(decimalSubstring, dest);
}

CHIP_ERROR ManualSetupPayloadParser::populatePayload(SetupPayload & outPayload)
{
    CHIP_ERROR result = CHIP_NO_ERROR;
    SetupPayload payload;
    std::string representationWithoutCheckDigit;

    result = CheckDecimalStringValidity(mDecimalStringRepresentation, representationWithoutCheckDigit);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    size_t stringOffset = 0;
    uint32_t chunk1, chunk2, chunk3;

    result = ReadDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, chunk1, kManualSetupCodeChunk1CharLength);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    result = ReadDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, chunk2, kManualSetupCodeChunk2CharLength);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    result = ReadDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, chunk3, kManualSetupCodeChunk3CharLength);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    // First digit of '8' or '9' would be invalid for v1 and would indicate new format (e.g. version 2)
    if (chunk1 == 8 || chunk1 == 9)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool isLongCode = ((chunk1 >> kManualSetupChunk1VidPidPresentBitPos) & 1) == 1;
    result          = CheckCodeLengthValidity(representationWithoutCheckDigit, isLongCode);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }

    constexpr uint32_t kDiscriminatorMsbitsMask = (1 << kManualSetupChunk1DiscriminatorMsbitsLength) - 1;
    constexpr uint32_t kDiscriminatorLsbitsMask = (1 << kManualSetupChunk2DiscriminatorLsbitsLength) - 1;

    uint32_t discriminator = ((chunk2 >> kManualSetupChunk2DiscriminatorLsbitsPos) & kDiscriminatorLsbitsMask);
    discriminator |= ((chunk1 >> kManualSetupChunk1DiscriminatorMsbitsPos) & kDiscriminatorMsbitsMask)
        << kManualSetupChunk2DiscriminatorLsbitsLength;

    // Since manual code only contains upper msbits of discriminator, re-align
    constexpr int kDiscriminatorShift = (kPayloadDiscriminatorFieldLengthInBits - kManualSetupDiscriminatorFieldLengthInBits);
    discriminator <<= kDiscriminatorShift;

    constexpr uint32_t kPincodeMsbitsMask = (1 << kManualSetupChunk3PINCodeMsbitsLength) - 1;
    constexpr uint32_t kPincodeLsbitsMask = (1 << kManualSetupChunk2PINCodeLsbitsLength) - 1;

    uint32_t setUpPINCode = ((chunk2 >> kManualSetupChunk2PINCodeLsbitsPos) & kPincodeLsbitsMask);
    setUpPINCode |= ((chunk3 >> kManualSetupChunk3PINCodeMsbitsPos) & kPincodeMsbitsMask) << kManualSetupChunk2PINCodeLsbitsLength;

    if (setUpPINCode == 0)
    {
        ChipLogError(SetupPayload, "Failed decoding base10. SetUpPINCode was 0.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (isLongCode)
    {
        uint32_t vendorID;
        result =
            ReadDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, vendorID, kManualSetupVendorIdCharLength);
        if (result != CHIP_NO_ERROR)
        {
            return result;
        }

        uint32_t productID;
        result =
            ReadDigitsFromDecimalString(representationWithoutCheckDigit, stringOffset, productID, kManualSetupProductIdCharLength);
        if (result != CHIP_NO_ERROR)
        {
            return result;
        }
        // Need to do dynamic checks, because we are reading 5 chars, so could
        // have 99,999 here or something.
        if (!CanCastTo<uint16_t>(vendorID))
        {
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }
        outPayload.vendorID = static_cast<uint16_t>(vendorID);
        if (!CanCastTo<uint16_t>(productID))
        {
            return CHIP_ERROR_INVALID_INTEGER_VALUE;
        }
        outPayload.productID = static_cast<uint16_t>(productID);
    }
    outPayload.commissioningFlow = isLongCode ? CommissioningFlow::kCustom : CommissioningFlow::kStandard;
    static_assert(kSetupPINCodeFieldLengthInBits <= 32, "Won't fit in uint32_t");
    outPayload.setUpPINCode = static_cast<uint32_t>(setUpPINCode);
    static_assert(kManualSetupDiscriminatorFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.discriminator        = static_cast<uint16_t>(discriminator);
    outPayload.isShortDiscriminator = true;

    return result;
}

} // namespace chip
