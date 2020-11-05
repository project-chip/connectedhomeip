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

#include <algorithm>
#include <inttypes.h>

#include <support/logging/CHIPLogging.h>
#include <support/verhoeff/Verhoeff.h>

using namespace chip;

static uint32_t shortPayloadRepresentation(const SetupPayload & payload)
{
    int offset      = 1;
    uint32_t result = payload.requiresCustomFlow ? 1 : 0;
    result |= (payload.discriminator & kManualSetupDiscriminatorFieldBitMask) << offset;
    offset += kManualSetupDiscriminatorFieldLengthInBits;
    result |= payload.setUpPINCode << offset;
    return result;
}

static inline int decimalStringWithPadding(char * buf, uint32_t number, int minLength)
{
    return snprintf(buf, minLength + 1, "%0*" PRIu32, minLength, number);
}

CHIP_ERROR ManualSetupPayloadGenerator::payloadDecimalStringRepresentation(std::string & outDecimalString)
{
    if (!mSetupPayload.isValidManualCode())
    {
        ChipLogError(SetupPayload, "Failed encoding invalid payload");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    constexpr size_t decimalStringLength =
        kManualSetupShortCodeCharLength + kManualSetupVendorIdCharLength + kManualSetupProductIdCharLength + 1;
    char decimalString[decimalStringLength];
    char * p = decimalString;

    uint32_t shortDecimal = shortPayloadRepresentation(mSetupPayload);
    p += decimalStringWithPadding(p, shortDecimal, kManualSetupShortCodeCharLength);

    if (mSetupPayload.requiresCustomFlow)
    {
        p += decimalStringWithPadding(p, mSetupPayload.vendorID, kManualSetupVendorIdCharLength);
        p += decimalStringWithPadding(p, mSetupPayload.productID, kManualSetupProductIdCharLength);
    }
    *p++ = Verhoeff10::ComputeCheckChar(decimalString);
    *p   = 0;

    outDecimalString = decimalString;
    return CHIP_NO_ERROR;
}
