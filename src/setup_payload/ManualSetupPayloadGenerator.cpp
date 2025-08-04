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
 *      This file describes a Manual Entry Code Generator.
 *
 */

#include "ManualSetupPayloadGenerator.h"

#include <inttypes.h>

#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/verhoeff/Verhoeff.h>

namespace chip {

static uint32_t chunk1PayloadRepresentation(const PayloadContents & payload)
{
    /* <1 digit> Represents:
     *     - <bits 1..0> Discriminator <bits 11.10>
     *     - <bit 2> VID/PID present flag
     */

    constexpr int kDiscriminatorShift = (kManualSetupDiscriminatorFieldLengthInBits - kManualSetupChunk1DiscriminatorMsbitsLength);
    constexpr uint32_t kDiscriminatorMask = (1 << kManualSetupChunk1DiscriminatorMsbitsLength) - 1;

    static_assert(kManualSetupChunk1VidPidPresentBitPos >=
                      kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength,
                  "Discriminator won't fit");

    uint32_t discriminatorChunk = (payload.discriminator.GetShortValue() >> kDiscriminatorShift) & kDiscriminatorMask;
    uint32_t vidPidPresentFlag  = payload.commissioningFlow != CommissioningFlow::kStandard ? 1 : 0;

    uint32_t result = (discriminatorChunk << kManualSetupChunk1DiscriminatorMsbitsPos) |
        (vidPidPresentFlag << kManualSetupChunk1VidPidPresentBitPos);

    return result;
}

static uint32_t chunk2PayloadRepresentation(const PayloadContents & payload)
{
    /* <5 digits> Represents:
     *     - <bits 13..0> PIN Code <bits 13..0>
     *     - <bits 15..14> Discriminator <bits 9..8>
     */

    constexpr uint32_t kDiscriminatorMask = (1 << kManualSetupChunk2DiscriminatorLsbitsLength) - 1;
    constexpr uint32_t kPincodeMask       = (1 << kManualSetupChunk2PINCodeLsbitsLength) - 1;

    uint32_t discriminatorChunk = payload.discriminator.GetShortValue() & kDiscriminatorMask;

    uint32_t result = ((payload.setUpPINCode & kPincodeMask) << kManualSetupChunk2PINCodeLsbitsPos) |
        (discriminatorChunk << kManualSetupChunk2DiscriminatorLsbitsPos);

    return result;
}

static uint32_t chunk3PayloadRepresentation(const PayloadContents & payload)
{
    /* <4 digits> Represents:
     *     - <bits 12..0> PIN Code <bits 26..14>
     */

    constexpr int kPincodeShift     = (kSetupPINCodeFieldLengthInBits - kManualSetupChunk3PINCodeMsbitsLength);
    constexpr uint32_t kPincodeMask = (1 << kManualSetupChunk3PINCodeMsbitsLength) - 1;

    uint32_t result = ((payload.setUpPINCode >> kPincodeShift) & kPincodeMask) << kManualSetupChunk3PINCodeMsbitsPos;

    return result;
}

static CHIP_ERROR decimalStringWithPadding(MutableCharSpan buffer, uint32_t number)
{
    int len    = static_cast<int>(buffer.size() - 1);
    int retval = snprintf(buffer.data(), buffer.size(), "%0*" PRIu32, len, number);

    return (retval >= static_cast<int>(buffer.size())) ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
}

CHIP_ERROR ManualSetupPayloadGenerator::payloadDecimalStringRepresentation(MutableCharSpan & outBuffer)
{
    static_assert(kManualSetupCodeChunk1CharLength + kManualSetupCodeChunk2CharLength + kManualSetupCodeChunk3CharLength ==
                      kManualSetupShortCodeCharLength,
                  "Manual code length mismatch (short)");
    static_assert(kManualSetupShortCodeCharLength + kManualSetupVendorIdCharLength + kManualSetupProductIdCharLength ==
                      kManualSetupLongCodeCharLength,
                  "Manual code length mismatch (long)");
    static_assert(kManualSetupChunk1DiscriminatorMsbitsLength + kManualSetupChunk2DiscriminatorLsbitsLength ==
                      kManualSetupDiscriminatorFieldLengthInBits,
                  "Discriminator won't fit");
    static_assert(kManualSetupChunk2PINCodeLsbitsLength + kManualSetupChunk3PINCodeMsbitsLength == kSetupPINCodeFieldLengthInBits,
                  "PIN code won't fit");

    if (!mAllowInvalidPayload && !mPayloadContents.isValidManualCode())
    {
        ChipLogError(SetupPayload, "Failed encoding invalid payload");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool useLongCode = (mPayloadContents.commissioningFlow != CommissioningFlow::kStandard) && !mForceShortCode;

    // Add two for the check digit and null terminator.
    if ((useLongCode && outBuffer.size() < kManualSetupLongCodeCharLength + 2) ||
        (!useLongCode && outBuffer.size() < kManualSetupShortCodeCharLength + 2))
    {
        ChipLogError(SetupPayload, "Failed encoding payload to buffer");
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    uint32_t chunk1 = chunk1PayloadRepresentation(mPayloadContents);
    uint32_t chunk2 = chunk2PayloadRepresentation(mPayloadContents);
    uint32_t chunk3 = chunk3PayloadRepresentation(mPayloadContents);

    size_t offset = 0;

    // Add one to the length of each chunk, since snprintf writes a null terminator.
    ReturnErrorOnFailure(decimalStringWithPadding(outBuffer.SubSpan(offset, kManualSetupCodeChunk1CharLength + 1), chunk1));
    offset += kManualSetupCodeChunk1CharLength;
    ReturnErrorOnFailure(decimalStringWithPadding(outBuffer.SubSpan(offset, kManualSetupCodeChunk2CharLength + 1), chunk2));
    offset += kManualSetupCodeChunk2CharLength;
    ReturnErrorOnFailure(decimalStringWithPadding(outBuffer.SubSpan(offset, kManualSetupCodeChunk3CharLength + 1), chunk3));
    offset += kManualSetupCodeChunk3CharLength;

    if (useLongCode)
    {
        ReturnErrorOnFailure(
            decimalStringWithPadding(outBuffer.SubSpan(offset, kManualSetupVendorIdCharLength + 1), mPayloadContents.vendorID));
        offset += kManualSetupVendorIdCharLength;
        ReturnErrorOnFailure(
            decimalStringWithPadding(outBuffer.SubSpan(offset, kManualSetupProductIdCharLength + 1), mPayloadContents.productID));
        offset += kManualSetupProductIdCharLength;
    }

    int checkDigit = Verhoeff10::CharToVal(Verhoeff10::ComputeCheckChar(outBuffer.data()));
    ReturnErrorOnFailure(decimalStringWithPadding(outBuffer.SubSpan(offset, 2), static_cast<uint32_t>(checkDigit)));
    offset += 1;

    // Reduce outBuffer span size to be the size of written data and to not include null-terminator.
    outBuffer.reduce_size(offset);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualSetupPayloadGenerator::payloadDecimalStringRepresentation(std::string & outDecimalString)
{
    // One extra char for the check digit, another for the null terminator.
    char decimalString[kManualSetupLongCodeCharLength + 1 + 1] = "";
    MutableCharSpan outBuffer(decimalString);

    ReturnErrorOnFailure(payloadDecimalStringRepresentation(outBuffer));
    outDecimalString.assign(decimalString);

    return CHIP_NO_ERROR;
}

} // namespace chip
