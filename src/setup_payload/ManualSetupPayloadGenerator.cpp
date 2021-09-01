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
#include <limits>

#include <support/logging/CHIPLogging.h>
#include <support/verhoeff/Verhoeff.h>

namespace chip {

static uint32_t chunk1PayloadRepresentation(const PayloadContents & payload)
{
    /* <1 digit> Represents:
     *     - <bits 1..0> Discriminator <bits 11.10>
     *     - <bit 2> VID/PID present flag
     */

    constexpr int kDiscriminatorShift     = (kPayloadDiscriminatorFieldLengthInBits - kManualSetupChunk1DiscriminatorMsbitsLength);
    constexpr uint32_t kDiscriminatorMask = (1 << kManualSetupChunk1DiscriminatorMsbitsLength) - 1;

    static_assert(kManualSetupChunk1VidPidPresentBitPos >=
                      kManualSetupChunk1DiscriminatorMsbitsPos + kManualSetupChunk1DiscriminatorMsbitsLength,
                  "Discriminator won't fit");

    uint32_t discriminatorChunk = (payload.discriminator >> kDiscriminatorShift) & kDiscriminatorMask;
    uint32_t vidPidPresentFlag  = payload.commissioningFlow == CommissioningFlow::kCustom ? 1 : 0;

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

    constexpr int kDiscriminatorShift     = (kPayloadDiscriminatorFieldLengthInBits - kManualSetupDiscriminatorFieldLengthInBits);
    constexpr uint32_t kDiscriminatorMask = (1 << kManualSetupChunk2DiscriminatorLsbitsLength) - 1;
    constexpr uint32_t kPincodeMask       = (1 << kManualSetupChunk2PINCodeLsbitsLength) - 1;

    uint32_t discriminatorChunk = (payload.discriminator >> kDiscriminatorShift) & kDiscriminatorMask;

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

static void decimalStringWithPadding(char * buf, size_t len, uint32_t number)
{
    snprintf(buf, len + 1, "%0*" PRIu32, static_cast<int>(len), number);
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

    if (!mPayloadContents.isValidManualCode())
    {
        ChipLogError(SetupPayload, "Failed encoding invalid payload");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    bool useLongCode = (mPayloadContents.commissioningFlow == CommissioningFlow::kCustom);

    if ((useLongCode && outBuffer.size() < kManualSetupLongCodeCharLength + 2) ||
        (!useLongCode && outBuffer.size() < kManualSetupShortCodeCharLength + 2))
    {
        ChipLogError(SetupPayload, "Failed encoding payload to buffer");
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    uint32_t chunk1 = chunk1PayloadRepresentation(mPayloadContents);
    uint32_t chunk2 = chunk2PayloadRepresentation(mPayloadContents);
    uint32_t chunk3 = chunk3PayloadRepresentation(mPayloadContents);

    char * stringPtr = outBuffer.data();

    decimalStringWithPadding(stringPtr, kManualSetupCodeChunk1CharLength, chunk1);
    stringPtr += kManualSetupCodeChunk1CharLength;
    decimalStringWithPadding(stringPtr, kManualSetupCodeChunk2CharLength, chunk2);
    stringPtr += kManualSetupCodeChunk2CharLength;
    decimalStringWithPadding(stringPtr, kManualSetupCodeChunk3CharLength, chunk3);
    stringPtr += kManualSetupCodeChunk3CharLength;

    if (useLongCode)
    {
        decimalStringWithPadding(stringPtr, kManualSetupVendorIdCharLength, mPayloadContents.vendorID);
        stringPtr += kManualSetupVendorIdCharLength;
        decimalStringWithPadding(stringPtr, kManualSetupProductIdCharLength, mPayloadContents.productID);
        stringPtr += kManualSetupProductIdCharLength;
    }

    snprintf(stringPtr, 2, "%c", Verhoeff10::ComputeCheckChar(outBuffer.data()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualSetupPayloadGenerator::payloadDecimalStringRepresentation(std::string & outDecimalString)
{
    CHIP_ERROR err;

    // One extra char for the check digit, another for the null terminator.
    char decimalString[kManualSetupLongCodeCharLength + 1 + 1] = "";
    MutableCharSpan outBuffer(decimalString);

    err = payloadDecimalStringRepresentation(outBuffer);
    outDecimalString.assign(decimalString);

    return err;
}

} // namespace chip
