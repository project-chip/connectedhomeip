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

#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>

#include <core/CHIPCore.h>
#include <core/CHIPError.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>

using namespace chip;
using namespace std;
using namespace chip::TLV;
using namespace chip::TLV::Utilities;

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

static CHIP_ERROR openTLVContainer(TLVReader & reader, TLVType type, uint64_t tag, TLVReader & containerReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(reader.GetType() == type, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(reader.GetTag() == tag, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(reader.GetLength() == 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = reader.OpenContainer(containerReader);
    SuccessOrExit(err);

    VerifyOrExit(containerReader.GetContainerType() == type, err = CHIP_ERROR_INVALID_ARGUMENT);
exit:
    return err;
}

static CHIP_ERROR retrieveStringOptionalInfo(TLVReader & reader, OptionalQRCodeInfo & info)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint32_t valLength = reader.GetLength();
    char * val         = (char *) malloc(valLength + 1);
    err                = reader.GetString(val, valLength + 1);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    info.type = optionalQRCodeInfoTypeString;
    info.tag  = reader.GetTag();
    info.data = string(val);
    free(val);
    return err;
}

static CHIP_ERROR retrieveIntegerOptionalInfo(TLVReader & reader, OptionalQRCodeInfo & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int64_t storedInteger;
    err = reader.Get(storedInteger);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    info.type    = optionalQRCodeInfoTypeInt;
    info.tag     = reader.GetTag();
    info.integer = storedInteger;
    return err;
}

static void populatePayloadTLVField(SetupPayload & outPayload, OptionalQRCodeInfo info)
{
    if (info.tag == ContextTag(kSerialNumberTag))
    {
        outPayload.serialNumber = info.data;
    }
    else
    {
        outPayload.addOptionalData(info);
    }
}

static CHIP_ERROR parseTLVFields(SetupPayload & outPayload, uint8_t * tlvDataStart, uint32_t tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVReader rootReader;
    rootReader.Init(tlvDataStart, tlvDataLengthInBytes);
    rootReader.ImplicitProfileId = outPayload.productID;
    err                          = rootReader.Next();
    SuccessOrExit(err);
    {
        TLVReader innerStructureReader;
        err = openTLVContainer(rootReader, kTLVType_Structure, ProfileTag(2, 1), innerStructureReader);
        SuccessOrExit(err);
        err = innerStructureReader.Next();
        while (err == CHIP_NO_ERROR)
        {
            TLVType type = innerStructureReader.GetType();
            OptionalQRCodeInfo info;
            if (type != kTLVType_UTF8String && type != kTLVType_SignedInteger)
            {
                err = innerStructureReader.Next();
                continue;
            }
            if (type == kTLVType_UTF8String)
            {
                err = retrieveStringOptionalInfo(innerStructureReader, info);
            }
            else if (type == kTLVType_SignedInteger)
            {
                err = retrieveIntegerOptionalInfo(innerStructureReader, info);
            }
            SuccessOrExit(err);
            populatePayloadTLVField(outPayload, info);
            err = innerStructureReader.Next();
        }
    }
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
exit:
    return err;
}

static CHIP_ERROR populateTLV(SetupPayload & outPayload, const vector<uint8_t> & buf, int & index)
{
    if (buf.size() * 8 == (uint) index)
    {
        return CHIP_NO_ERROR;
    }
    size_t bitsLeftToRead = (buf.size() * 8) - index;
    size_t tlvBytesLength = ceil(double(bitsLeftToRead) / 8);
    uint8_t * tlvArray    = new uint8_t[tlvBytesLength];
    for (size_t i = 0; i < tlvBytesLength; i++)
    {
        uint64_t dest;
        readBits(buf, index, dest, 8);
        tlvArray[i] = static_cast<uint8_t>(dest);
    }

    return parseTLVFields(outPayload, tlvArray, tlvBytesLength);
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
    vector<uint8_t> buf;
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int indexToReadFrom = 0;
    uint64_t dest;

    string payload = extractPayload(mBase41Representation);
    VerifyOrExit(payload.length() != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = base41Decode(payload, buf);
    SuccessOrExit(err);

    err = readBits(buf, indexToReadFrom, dest, kVersionFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.version = dest;

    err = readBits(buf, indexToReadFrom, dest, kVendorIDFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.vendorID = dest;

    err = readBits(buf, indexToReadFrom, dest, kProductIDFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.productID = dest;

    err = readBits(buf, indexToReadFrom, dest, kCustomFlowRequiredFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.requiresCustomFlow = dest;

    err = readBits(buf, indexToReadFrom, dest, kRendezvousInfoFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.rendezvousInformation = dest;

    err = readBits(buf, indexToReadFrom, dest, kPayloadDiscriminatorFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.discriminator = dest;

    err = readBits(buf, indexToReadFrom, dest, kSetupPINCodeFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.setUpPINCode = dest;

    err = readBits(buf, indexToReadFrom, dest, kReservedFieldLengthInBits);
    SuccessOrExit(err);

    err = populateTLV(outPayload, buf, indexToReadFrom);
    SuccessOrExit(err);

exit:
    return err;
}
