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

#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/verhoeff/Verhoeff.h>

namespace chip {

static uint32_t chunk1PayloadRepresentation(const SetupPayload & payload)
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

static uint32_t chunk2PayloadRepresentation(const SetupPayload & payload)
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

static uint32_t chunk3PayloadRepresentation(const SetupPayload & payload)
{
    /* <4 digits> Represents:
     *     - <bits 12..0> PIN Code <bits 26..14>
     */

    constexpr int kPincodeShift     = (kSetupPINCodeFieldLengthInBits - kManualSetupChunk3PINCodeMsbitsLength);
    constexpr uint32_t kPincodeMask = (1 << kManualSetupChunk3PINCodeMsbitsLength) - 1;

    uint32_t result = ((payload.setUpPINCode >> kPincodeShift) & kPincodeMask) << kManualSetupChunk3PINCodeMsbitsPos;

    return result;
}

// TODO: issue #3663 - Unbounded stack in src/setup_payload
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif
#pragma GCC diagnostic ignored "-Wvla"

static std::string decimalStringWithPadding(uint32_t number, int minLength)
{
    char buf[minLength + 1];
    snprintf(buf, sizeof(buf), "%0*" PRIu32, minLength, number);
    return std::string(buf);
}

#pragma GCC diagnostic pop

CHIP_ERROR ManualSetupPayloadGenerator::payloadDecimalStringRepresentation(std::string & outDecimalString)
{
    if (!mSetupPayload.isValidManualCode())
    {
        ChipLogError(SetupPayload, "Failed encoding invalid payload");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    static_assert(kManualSetupCodeChunk1CharLength + kManualSetupCodeChunk2CharLength + kManualSetupCodeChunk3CharLength ==
                      kManualSetupShortCodeCharLength,
                  "Manual code length mismatch");
    static_assert(kManualSetupChunk1DiscriminatorMsbitsLength + kManualSetupChunk2DiscriminatorLsbitsLength ==
                      kManualSetupDiscriminatorFieldLengthInBits,
                  "Discriminator won't fit");
    static_assert(kManualSetupChunk2PINCodeLsbitsLength + kManualSetupChunk3PINCodeMsbitsLength == kSetupPINCodeFieldLengthInBits,
                  "PIN code won't fit");

    uint32_t chunk1 = chunk1PayloadRepresentation(mSetupPayload);
    uint32_t chunk2 = chunk2PayloadRepresentation(mSetupPayload);
    uint32_t chunk3 = chunk3PayloadRepresentation(mSetupPayload);

    std::string decimalString = decimalStringWithPadding(chunk1, kManualSetupCodeChunk1CharLength);
    decimalString += decimalStringWithPadding(chunk2, kManualSetupCodeChunk2CharLength);
    decimalString += decimalStringWithPadding(chunk3, kManualSetupCodeChunk3CharLength);

    if (mSetupPayload.commissioningFlow == CommissioningFlow::kCustom)
    {
        decimalString += decimalStringWithPadding(mSetupPayload.vendorID, kManualSetupVendorIdCharLength);
        decimalString += decimalStringWithPadding(mSetupPayload.productID, kManualSetupProductIdCharLength);
    }
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());

    outDecimalString = decimalString;
    return CHIP_NO_ERROR;
}

} // namespace chip
