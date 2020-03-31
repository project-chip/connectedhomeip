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

/**
 *    @file
 *      This file describes a Manual Entry Code Generator.
 *
 */

#include "ManualSetupPayloadGenerator.h"

using namespace chip;

static uint32_t shortPayloadRepresentation(SetupPayload payload)
{
    int offset = 1;
    uint32_t result = payload.vendorID ? 1 : 0;
    result |= payload.setUpPINCode << offset;
    offset += kSetupPINCodeFieldLengthInBits;

    result |= payload.discriminator << offset;
    return result;
}

static string decimalStringWithPadding(uint32_t number, int minLength)
{
    char buf[minLength];
    sprintf(buf, "%0*u", minLength, number);
    return string(buf);
}

string ManualSetupPayloadGenerator::payloadDecimalStringRepresentation() 
{
    mSetupPayload.isManualPayloadSetup = true;
    if (!mSetupPayload.isValid())
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return string();
    }
    
    uint32_t shortDecimal = shortPayloadRepresentation(mSetupPayload);
    string decimalString = decimalStringWithPadding(shortDecimal, SetupPayload::manualSetupShortCodeCharLength());

    if (mSetupPayload.vendorID) {
        decimalString += decimalStringWithPadding(mSetupPayload.vendorID, SetupPayload::manualSetupVendorIdCharLength());
        decimalString += decimalStringWithPadding(mSetupPayload.productID, SetupPayload::manualSetupProductIdCharLength());
    }
    return decimalString;
}