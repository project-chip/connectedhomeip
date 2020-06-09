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

#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>

#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace std;
using namespace chip::TLV;

// Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
static CHIP_ERROR populateBits(uint8_t * bits, int & offset, uint64_t input, size_t numberOfBits, size_t totalPayloadDataSizeInBits)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int index;

    VerifyOrExit(offset + numberOfBits <= totalPayloadDataSizeInBits, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(input < 1u << numberOfBits, err = CHIP_ERROR_INVALID_ARGUMENT);

    index = offset;
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
exit:
    return err;
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

CHIP_ERROR writeVendorOptionaData(TLVWriter & writer, vector<OptionalQRCodeInfo> optionalData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (OptionalQRCodeInfo info : optionalData)
    {
        if (info.type == optionalQRCodeInfoTypeString)
        {
            err = writer.PutString(ContextTag(info.tag), info.data.c_str());
            SuccessOrExit(err);
        }
        else if (info.type == optionalQRCodeInfoTypeInt)
        {
            err = writer.Put(ContextTag(info.tag), static_cast<int64_t>(info.integer));
            SuccessOrExit(err);
        }
    }
exit:
    return err;
}

CHIP_ERROR writeCHIPOptionaData(TLVWriter & writer, vector<CHIPQRCodeInfo> optionalData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (CHIPQRCodeInfo info : optionalData)
    {
        if (info.type == chipQRCodeInfoTypeString)
        {
            err = writer.PutString(ContextTag(info.tag), info.stringData.c_str());
            SuccessOrExit(err);
        }
        else if (info.type == chipQRCodeInfoTypeUInt32)
        {
            err = writer.Put(ContextTag(info.tag), info.unsignedInt32);
            SuccessOrExit(err);
        }
    }
exit:
    return err;
}

CHIP_ERROR generateTLVFromOptionalData(SetupPayload & outPayload, uint8_t * tlvDataStart, uint32_t maxLen,
                                       uint32_t & tlvDataLengthInBytes)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    vector<OptionalQRCodeInfo> vendorOptionalData = outPayload.getAllVendorOptionalData();
    vector<CHIPQRCodeInfo> chipOptionalData       = outPayload.getAllCHIPOptionalData();
    VerifyOrExit(vendorOptionalData.size() != 0 || chipOptionalData.size() != 0, err = CHIP_NO_ERROR);

    TLVWriter rootWriter;
    rootWriter.Init(tlvDataStart, maxLen);

    TLVWriter innerStructureWritter;

    err = rootWriter.OpenContainer(ProfileTag(2, 1), kTLVType_Structure,
                                   innerStructureWritter); // TODO: Remove outer nest of QR code TLV encoding #728
    SuccessOrExit(err);

    err = writeVendorOptionaData(innerStructureWritter, vendorOptionalData);
    SuccessOrExit(err);

    err = writeCHIPOptionaData(innerStructureWritter, chipOptionalData);
    SuccessOrExit(err);

    err = rootWriter.CloseContainer(innerStructureWritter);
    SuccessOrExit(err);

    err = rootWriter.Finalize();
    SuccessOrExit(err);

    tlvDataLengthInBytes = rootWriter.GetLengthWritten();

exit:
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
    err = populateBits(bits, offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits);
    err = populateTLVBits(bits, offset, tlvDataStart, tlvDataLengthInBytes, totalPayloadSizeInBits);
    return err;
}

static CHIP_ERROR payloadBase41RepresentationWithTLV(SetupPayload & setupPayload, string & base41Representation, size_t bitsetSize,
                                                     uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    uint8_t bits[bitsetSize];
    memset(bits, 0, bitsetSize);
    string encodedPayload;
    CHIP_ERROR err = generateBitSet(setupPayload, bits, tlvDataStart, tlvDataLengthInBytes);
    SuccessOrExit(err);

    encodedPayload = base41Encode(bits, ArraySize(bits));
    encodedPayload.insert(0, kQRCodePrefix);
    base41Representation = encodedPayload;
exit:
    return err;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase41Representation(string & base41Representation)
{
    // 6.1.2.2. Table: Packed Binary Data Structure
    // The TLV Data should be 0 length if TLV is not included.
    return payloadBase41Representation(base41Representation, NULL, 0);
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase41Representation(string & base41Representation, uint8_t * tlvDataStart,
                                                                    size_t tlvDataStartSize)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    uint32_t tlvDataLengthInBytes = 0;

    VerifyOrExit(mPayload.isValidQRCodePayload(), err = CHIP_ERROR_INVALID_ARGUMENT);
    err = generateTLVFromOptionalData(mPayload, tlvDataStart, tlvDataStartSize, tlvDataLengthInBytes);
    SuccessOrExit(err);

    err = payloadBase41RepresentationWithTLV(mPayload, base41Representation, kTotalPayloadDataSizeInBytes + tlvDataLengthInBytes,
                                             tlvDataStart, tlvDataLengthInBytes);
    SuccessOrExit(err);

exit:
    return err;
}
