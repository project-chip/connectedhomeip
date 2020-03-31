/**
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
 *      This file describes a QRCode Setup Payload parser based on the
 *      CHIP specification.
 */

#include "QRCodeSetupPayloadParser.h"
#include "Base45.h"

#include <core/CHIPError.h>
#include <iostream>
#include <vector>

using namespace chip;
using namespace std;

// Populate numberOfBits into dest from buf starting at startIndex
static CHIP_ERROR readBits(vector<uint8_t> buf, int & index, uint64_t & dest, size_t numberOfBitsToRead)
{
    dest = 0;
    if (index + numberOfBitsToRead > buf.size() * 8 || numberOfBitsToRead > sizeof(uint64_t) * 8)
    {
        fprintf(stderr, "Error parsing QR code. startIndex %d numberOfBitsToLoad %zu buf_len %zu ", index, numberOfBitsToRead,
                buf.size());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    int currentIndex = index;
    for (size_t bitsRead = 0; bitsRead < numberOfBitsToRead; bitsRead++)
    {
        if (buf[currentIndex / 8] & (1 << (currentIndex % 8)))
        {
            dest |= (1 << bitsRead);
        }
        currentIndex++;
    }
    index += numberOfBitsToRead;
    return CHIP_NO_ERROR;
}

CHIP_ERROR QRCodeSetupPayloadParser::populatePayload(SetupPayload & outPayload)
{
    vector<uint8_t> buf = vector<uint8_t>();

    CHIP_ERROR result = base45Decode(mBase45Representation, buf);

    if (CHIP_NO_ERROR != result)
    {
        fprintf(stderr, "Decoding of base45 string failed");
        return result;
    }

    int indexToReadFrom = 0;
    uint64_t dest;

    result = readBits(buf, indexToReadFrom, dest, kVersionFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.version = dest;

    result = readBits(buf, indexToReadFrom, dest, kVendorIDFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.vendorID = dest;

    result = readBits(buf, indexToReadFrom, dest, kProductIDFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.productID = dest;

    result = readBits(buf, indexToReadFrom, dest, kCustomFlowRequiredFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.requiresCustomFlow = dest;

    result = readBits(buf, indexToReadFrom, dest, kRendezvousInfoFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.rendezvousInformation = dest;

    result = readBits(buf, indexToReadFrom, dest, kPayloadDiscriminatorFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.discriminator = dest;

    result = readBits(buf, indexToReadFrom, dest, kSetupPINCodeFieldLengthInBits);
    if (result != CHIP_NO_ERROR)
    {
        return result;
    }
    outPayload.setUpPINCode = dest;

    return result;
}
