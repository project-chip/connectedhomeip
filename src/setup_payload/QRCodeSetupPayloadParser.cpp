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
 *      This file describes a QRCode Setup Payload parser based on the
 *      CHIP specification.
 */

#include "QRCodeSetupPayloadParser.h"
#include "Base41.h"

#include <core/CHIPError.h>
#include <iostream>
#include <string.h>
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

static string extractPayload(string inString)
{
    string chipSegment;
    char delimiter = '%';
    vector<int> delimiterIndices;
    delimiterIndices.push_back(-1);

    for (size_t i = 0; i < inString.length(); i++)
    {
        if (inString[i] == delimiter)
        {
            delimiterIndices.push_back(i);
        }
    }

    // Find the first string between delimiters that starts with kQRCodePrefix
    for (size_t i = 0; i < delimiterIndices.size(); i++)
    {
        size_t startIndex = delimiterIndices[i] + 1;
        size_t endIndex   = (i == delimiterIndices.size() - 1 ? string::npos : delimiterIndices[i + 1]);
        size_t length     = (endIndex != string::npos ? endIndex - startIndex : string::npos);
        string segment    = inString.substr(startIndex, length);

        // Find a segment that starts with kQRCodePrefix
        if (segment.find(kQRCodePrefix, 0) == 0 && segment.length() > strlen(kQRCodePrefix))
        {
            chipSegment = segment;
            break;
        }
    }

    if (chipSegment.length() > 0)
    {
        return chipSegment.substr(strlen(kQRCodePrefix)); // strip out prefix before returning
    }
    else
    {
        return chipSegment;
    }
}

CHIP_ERROR QRCodeSetupPayloadParser::populatePayload(SetupPayload & outPayload)
{
    vector<uint8_t> buf = vector<uint8_t>();

    string payload = extractPayload(mBase41Representation);
    if (payload.length() == 0)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR result = base41Decode(payload, buf);

    if (CHIP_NO_ERROR != result)
    {
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
