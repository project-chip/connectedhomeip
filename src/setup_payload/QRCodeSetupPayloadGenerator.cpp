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
#include <math.h>

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <core/CHIPTLVData.hpp>
#include <support/RandUtils.h>

using namespace chip;
using namespace std;
using namespace chip::TLV;

// Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
static void populateBits(uint8_t * bits, int & offset, uint64_t input, size_t numberOfBits, size_t totalPayloadDataSizeInBits)
{
    // do nothing in the case where we've overflowed. should never happen
    if (offset + numberOfBits > totalPayloadDataSizeInBits || input >= 1u << numberOfBits)
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

static void populateTLVBits(uint8_t * bits, int & offset, uint8_t * tlvBuf, size_t tlvBufSizeInBytes,
                            int totalPayloadDataSizeInBits)
{
    for (size_t i = 0; i < tlvBufSizeInBytes; i++)
    {
        uint8_t value = tlvBuf[i];
        populateBits(bits, offset, value, 8, totalPayloadDataSizeInBits);
    }
}

void addCHIPInfoToOptionalData(SetupPayload & outPayload)
{
    if (outPayload.serialNumber.length() > 0)
    {
        optionalQRCodeInfo info;
        info.type = optionalQRCodeInfoTypeString;
        info.tag  = ContextTag(kSerialNumberTag);
        info.data = outPayload.serialNumber;
        outPayload.addOptionalData(info);
    }
}

CHIP_ERROR writeOptionaData(TLVWriter & writer, vector<optionalQRCodeInfo> optionalData)
{
    CHIP_ERROR err;
    for (optionalQRCodeInfo info : optionalData)
    {
        if (info.type == optionalQRCodeInfoTypeString)
        {
            err = writer.PutString(info.tag, info.data.c_str());
        }
        else if (info.type == optionalQRCodeInfoTypeInt)
        {
            err = writer.Put(info.tag, info.integer);
        }
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR generateTLVFromOptionalData(SetupPayload & outPayload, uint8_t * tlvDataStart, uint32_t maxLen,
                                       uint32_t & tlvDataLengthInBytes)
{
    addCHIPInfoToOptionalData(outPayload);
    vector<optionalQRCodeInfo> optionalData = outPayload.getAllOptionalData();
    if (optionalData.size() == 0)
    {
        return CHIP_NO_ERROR;
    }

    TLVWriter writer;
    writer.Init(tlvDataStart, maxLen);

    CHIP_ERROR err;
    TLVWriter writer2;

    err = writer.OpenContainer(ProfileTag(2, 1), kTLVType_Structure, writer2); // TODO: How to remove outer nest
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    err = writeOptionaData(writer2, optionalData);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    err = writer.CloseContainer(writer2);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    err = writer.Finalize();
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    tlvDataLengthInBytes = writer.GetLengthWritten();
    return CHIP_NO_ERROR;
}

static CHIP_ERROR generateBitSet(SetupPayload & payload, uint8_t * bits, uint8_t * tlvDataStart, uint32_t tlvDataLengthInBytes)
{
    int offset                 = 0;
    int totalPayloadSizeInBits = kTotalPayloadDataSizeInBits + (tlvDataLengthInBytes * 8);
    populateBits(bits, offset, payload.version, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, payload.vendorID, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, payload.productID, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, payload.requiresCustomFlow, kCustomFlowRequiredFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, payload.rendezvousInformation, kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, payload.discriminator, kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, payload.setUpPINCode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateBits(bits, offset, 0, kReservedFieldLengthInBits, kTotalPayloadDataSizeInBits);
    populateTLVBits(bits, offset, tlvDataStart, tlvDataLengthInBytes, totalPayloadSizeInBits);
    return CHIP_NO_ERROR;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBinaryRepresentation(string & binaryRepresentation)
{
    if (!mPayload.isValidQRCodePayload())
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t tlvDataStart[2048];
    uint32_t tlvDataLengthInBytes = 0;
    CHIP_ERROR err                = generateTLVFromOptionalData(mPayload, tlvDataStart, sizeof(tlvDataStart), tlvDataLengthInBytes);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    uint8_t bits[kTotalPayloadDataSizeInBytes + tlvDataLengthInBytes] = { 0 };
    err = generateBitSet(mPayload, bits, tlvDataStart, tlvDataLengthInBytes);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    string binary;
    for (int i = sizeof(bits) / sizeof(bits[0]) - 1; i >= 0; i--)
    {
        string miniBinary;
        for (unsigned j = 1 << 7; j != 0;)
        {
            binary += bits[i] & j ? "1" : "0";
            miniBinary += bits[i] & j ? "1" : "0";
            j >>= 1;
        }
    }
    binaryRepresentation = binary;
    return CHIP_NO_ERROR;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase41Representation(string & base41Representation)
{
    if (!mPayload.isValidQRCodePayload())
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint8_t tlvDataStart[2048];
    uint32_t tlvDataLengthInBytes = 0;
    CHIP_ERROR err                = generateTLVFromOptionalData(mPayload, tlvDataStart, sizeof(tlvDataStart), tlvDataLengthInBytes);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    uint8_t bits[kTotalPayloadDataSizeInBytes + tlvDataLengthInBytes] = { 0 };
    err = generateBitSet(mPayload, bits, tlvDataStart, tlvDataLengthInBytes);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    string encodedPayload = base41Encode(bits, sizeof(bits) / sizeof(bits[0]));
    encodedPayload.insert(0, kQRCodePrefix);
    base41Representation = encodedPayload;

    return CHIP_NO_ERROR;
}
