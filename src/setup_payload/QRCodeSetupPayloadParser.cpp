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
#include "SetupCodeUtils.h"

#include <iostream>
#include <vector>

using namespace chip;
using namespace std;

// Populate numberOfBits into dest from buf starting at startIndex
static uint64_t readBits(vector <uint8_t> buf, int &index, size_t numberOfBitsToRead)
{
    uint64_t dest = 0;
    if (index + numberOfBitsToRead > buf.size() * 8 || numberOfBitsToRead > sizeof(uint64_t) * 8)
    {
        fprintf(stderr, "Error parsing QR code. startIndex %d numberOfBitsToLoad %zu buf_len %zu ", index, numberOfBitsToRead,
                buf.size());
        return 0;
    }

    int currentIndex  = index;
    for (size_t bitsRead = 0; bitsRead < numberOfBitsToRead; bitsRead++)
    {
        if (buf[currentIndex / 8] & (1 << (currentIndex % 8)))
        {
            dest |= (1 << bitsRead);
        }
        currentIndex++;
    }
    index += numberOfBitsToRead;
    return dest;
}

SetupPayload QRCodeSetupPayloadParser::payload() 
{
    vector <uint8_t> buf = base45Decode(mBase45StringRepresentation);
    if (buf.size() == 0) {
        fprintf(stderr, "Decoding of base45 string failed");
        return SetupPayload();
    }

    SetupPayload payload;

    int indexToReadFrom = 0;
    
    payload.version = readBits(buf, indexToReadFrom, kVersionFieldLengthInBits);
    payload.vendorID = readBits(buf, indexToReadFrom, kVendorIDFieldLengthInBits);
    payload.productID = readBits(buf, indexToReadFrom, kProductIDFieldLengthInBits);
    payload.requiresCustomFlow = readBits(buf, indexToReadFrom, kCustomFlowRequiredFieldLengthInBits);
    payload.rendezvousInformation = readBits(buf, indexToReadFrom, kRendezvousInfoFieldLengthInBits);
    payload.discriminator = readBits(buf, indexToReadFrom, kPayloadDiscriminatorFieldLengthInBits);
    payload.setUpPINCode = readBits(buf, indexToReadFrom, kSetupPINCodeFieldLengthInBits);

    return payload;
}
