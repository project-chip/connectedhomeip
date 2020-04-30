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
 *      This file implements a QRCode Setup Payload generator in accordance
 *      with the CHIP specification.
 *
 */

#include "QRCodeSetupPayloadGenerator.h"
#include "Base41.h"

#include <iostream>
#include <stdlib.h>

using namespace chip;
using namespace std;

// Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
static void populateBits(uint8_t * bits, int & offset, uint64_t input, size_t numberOfBits)
{
    // do nothing in the case where we've overflowed. should never happen
    if (offset + numberOfBits > kTotalPayloadDataSizeInBits || input >= 1u << numberOfBits)
    {
        fprintf(stderr, "Overflow while trying to generate a QR Code. Bailing.");
        return;
    }

    int index = offset;
    offset += numberOfBits;
    while (input != 0)
    {
        if (input & 1)
        {
            bits[index / 8] |= 1 << index % 8;
        }
        index++;
        input >>= 1;
    }
}

static void generateBitSet(SetupPayload & payload, uint8_t * bits)
{
    int offset = 0;

    populateBits(bits, offset, payload.version, kVersionFieldLengthInBits);
    populateBits(bits, offset, payload.vendorID, kVendorIDFieldLengthInBits);
    populateBits(bits, offset, payload.productID, kProductIDFieldLengthInBits);
    populateBits(bits, offset, payload.requiresCustomFlow, kCustomFlowRequiredFieldLengthInBits);
    populateBits(bits, offset, payload.rendezvousInformation, kRendezvousInfoFieldLengthInBits);
    populateBits(bits, offset, payload.discriminator, kPayloadDiscriminatorFieldLengthInBits);
    populateBits(bits, offset, payload.setUpPINCode, kSetupPINCodeFieldLengthInBits);
    populateBits(bits, offset, 0, kReservedFieldLengthInBits);
}

string QRCodeSetupPayloadGenerator::payloadBinaryRepresentation()
{
    if (mPayload.isValidQRCodePayload())
    {
        uint8_t bits[kTotalPayloadDataSizeInBytes] = { 0 };

        generateBitSet(mPayload, bits);

        string binary;
        for (int i = sizeof(bits) / sizeof(bits[0]) - 1; i >= 0; i--)
        {
            for (unsigned j = 1 << 8; j != 0;)
            {
                j >>= 1;
                binary += bits[i] & j ? "1" : "0";
            }
        }
        return binary;
    }
    else
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return string();
    }
}

string QRCodeSetupPayloadGenerator::payloadBase41Representation()
{
    if (mPayload.isValidQRCodePayload())
    {
        uint8_t bits[kTotalPayloadDataSizeInBytes] = { 0 };

        generateBitSet(mPayload, bits);

        string encodedPayload = base41Encode(bits, sizeof(bits) / sizeof(bits[0]));
        encodedPayload.insert(0, kQRCodePrefix);
        return encodedPayload;
    }
    else
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return string();
    }
}
