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

using namespace chip;

static uint32_t shortPayloadRepresentation(const SetupPayload & payload)
{
    constexpr int discriminatorOffset = kCustomFlowRequiredFieldLengthInBits;
    constexpr int pinCodeOffset       = discriminatorOffset + kManualSetupDiscriminatorFieldLengthInBits;
    uint32_t result                   = payload.requiresCustomFlow ? 1 : 0;

    static_assert(kManualSetupDiscriminatorFieldBitMask <= UINT32_MAX >> discriminatorOffset, "Discriminator won't fit");
    result |= static_cast<uint32_t>((payload.discriminator & kManualSetupDiscriminatorFieldBitMask) << discriminatorOffset);

    static_assert(pinCodeOffset + kSetupPINCodeFieldLengthInBits <= std::numeric_limits<uint32_t>::digits, "PIN code won't fit");
    result |= static_cast<uint32_t>(payload.setUpPINCode << pinCodeOffset);
    return result;
}

// TODO: issue #3663 - Unbounded stack in src/setup_payload
#if !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif

static std::string decimalStringWithPadding(uint32_t number, int minLength)
{
    char buf[minLength + 1];
    snprintf(buf, sizeof(buf), "%0*" PRIu32, minLength, number);
    return std::string(buf);
}

#if !defined(__clang__)
#pragma GCC diagnostic pop
#endif

CHIP_ERROR ManualSetupPayloadGenerator::payloadDecimalStringRepresentation(std::string & outDecimalString)
{
    if (!mSetupPayload.isValidManualCode())
    {
        ChipLogError(SetupPayload, "Failed encoding invalid payload");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint32_t shortDecimal     = shortPayloadRepresentation(mSetupPayload);
    std::string decimalString = decimalStringWithPadding(shortDecimal, kManualSetupShortCodeCharLength);

    if (mSetupPayload.requiresCustomFlow)
    {
        decimalString += decimalStringWithPadding(mSetupPayload.vendorID, kManualSetupVendorIdCharLength);
        decimalString += decimalStringWithPadding(mSetupPayload.productID, kManualSetupProductIdCharLength);
    }
    decimalString += Verhoeff10::ComputeCheckChar(decimalString.c_str());

    outDecimalString = decimalString;
    return CHIP_NO_ERROR;
}
