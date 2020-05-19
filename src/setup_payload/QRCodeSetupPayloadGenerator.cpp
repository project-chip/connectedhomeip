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
#include <support/CodeUtils.h>

using namespace chip;
using namespace std;
using namespace chip::TLV;

// Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
static CHIP_ERROR populateBits(uint8_t * bits, int & offset, uint64_t input, size_t numberOfBits, size_t totalPayloadDataSizeInBits)
{
    // do nothing in the case where we've overflowed. should never happen
    if (offset + numberOfBits > totalPayloadDataSizeInBits || input >= 1u << numberOfBits)
    {
        fprintf(stderr, "Overflow while trying to generate a QR Code. Bailing.");
        return CHIP_ERROR_INVALID_ARGUMENT;
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
    return CHIP_NO_ERROR;
}

static CHIP_ERROR populateTLVBits(uint8_t * bits, int & offset, uint8_t * tlvBuf, size_t tlvBufSizeInBytes,
                                  int totalPayloadDataSizeInBits)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (size_t i = 0; i < tlvBufSizeInBytes; i++)
    {
        uint8_t value = tlvBuf[i];
        err           = populateBits(bits, offset, value, 8, totalPayloadDataSizeInBits);
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }
    return err;
}

static void addCHIPInfoToOptionalData(SetupPayload & outPayload)
{
    if (outPayload.serialNumber.length() > 0)
    {
        OptionalQRCodeInfo info;
        info.type = optionalQRCodeInfoTypeString;
        info.tag  = ContextTag(kSerialNumberTag);
        info.data = outPayload.serialNumber;
        outPayload.addOptionalData(info);
    }
}

CHIP_ERROR writeOptionaData(TLVWriter & writer, vector<OptionalQRCodeInfo> optionalData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (OptionalQRCodeInfo info : optionalData)
    {
        if (info.type == optionalQRCodeInfoTypeString)
        {
            err = writer.PutString(info.tag, info.data.c_str());
        }
        else if (info.type == optionalQRCodeInfoTypeInt)
        {
            err = writer.Put(info.tag, info.integer);
        }
    }
    return err;
}

CHIP_ERROR generateTLVFromOptionalData(SetupPayload & outPayload, uint8_t * tlvDataStart, uint32_t maxLen,
                                       uint32_t & tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    addCHIPInfoToOptionalData(outPayload);
    vector<OptionalQRCodeInfo> optionalData = outPayload.getAllOptionalData();
    if (optionalData.size() == 0)
    {
        return CHIP_NO_ERROR;
    }

    TLVWriter rootWriter;
    rootWriter.Init(tlvDataStart, maxLen);

    TLVWriter innerStructureWritter;

    err = rootWriter.OpenContainer(ProfileTag(2, 1), kTLVType_Structure,
                                   innerStructureWritter); // TODO: Remove outer nest of QR code TLV encoding #728
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    err = writeOptionaData(innerStructureWritter, optionalData);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    err = rootWriter.CloseContainer(innerStructureWritter);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    err = rootWriter.Finalize();
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    tlvDataLengthInBytes = rootWriter.GetLengthWritten();
    return err;
}

static CHIP_ERROR generateBitSet(SetupPayload & payload, uint8_t * bits, uint8_t * tlvDataStart, uint32_t tlvDataLengthInBytes)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    int offset                 = 0;
    int totalPayloadSizeInBits = kTotalPayloadDataSizeInBits + (tlvDataLengthInBytes * 8);
    err = populateBits(bits, offset, payload.version, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, payload.vendorID, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, payload.productID, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, payload.requiresCustomFlow, kCustomFlowRequiredFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, payload.rendezvousInformation, kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, payload.discriminator, kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, payload.setUpPINCode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateBits(bits, offset, 0, kReservedFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateTLVBits(bits, offset, tlvDataStart, tlvDataLengthInBytes, totalPayloadSizeInBits);
    return err;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBinaryRepresentation(string & binaryRepresentation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!mPayload.isValidQRCodePayload())
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    uint8_t tlvDataStart[2048];
    uint32_t tlvDataLengthInBytes = 0;
    err                           = generateTLVFromOptionalData(mPayload, tlvDataStart, sizeof(tlvDataStart), tlvDataLengthInBytes);
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
    for (int i = ArraySize(bits) - 1; i >= 0; i--)
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
    return err;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase41Representation(string & base41Representation)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!mPayload.isValidQRCodePayload())
    {
        fprintf(stderr, "\nFailed encoding invalid payload\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint8_t tlvDataStart[2048];
    uint32_t tlvDataLengthInBytes = 0;
    err                           = generateTLVFromOptionalData(mPayload, tlvDataStart, sizeof(tlvDataStart), tlvDataLengthInBytes);
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

    return err;
}
