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
#include <protocols/CHIPProtocols.h>
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

CHIP_ERROR writeTag(TLVWriter & writer, uint64_t tag, OptionalQRCodeInfo & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (info.type == optionalQRCodeInfoTypeString)
    {
        err = writer.PutString(tag, info.data.c_str());
    }
    else if (info.type == optionalQRCodeInfoTypeInt32)
    {
        err = writer.Put(tag, info.int32);
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

CHIP_ERROR writeTag(TLVWriter & writer, uint64_t tag, OptionalQRCodeInfoExtension & info)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (info.type == optionalQRCodeInfoTypeString || info.type == optionalQRCodeInfoTypeInt32)
    {
        err = writeTag(writer, tag, static_cast<OptionalQRCodeInfo &>(info));
    }
    else if (info.type == optionalQRCodeInfoTypeInt64)
    {
        err = writer.Put(tag, info.int64);
    }
    else if (info.type == optionalQRCodeInfoTypeUInt32)
    {
        err = writer.Put(tag, info.uint32);
    }
    else if (info.type == optionalQRCodeInfoTypeUInt64)
    {
        err = writer.Put(tag, info.uint64);
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::generateTLVFromOptionalData(SetupPayload & outPayload, uint8_t * tlvDataStart,
                                                                    uint32_t maxLen, uint32_t & tlvDataLengthInBytes)
{
    CHIP_ERROR err                                            = CHIP_NO_ERROR;
    vector<OptionalQRCodeInfo> optionalData                   = outPayload.getAllOptionalVendorData();
    vector<OptionalQRCodeInfoExtension> optionalExtensionData = outPayload.getAllOptionalExtensionData();
    VerifyOrExit(optionalData.size() != 0 || optionalExtensionData.size() != 0, err = CHIP_NO_ERROR);

    TLVWriter rootWriter;
    rootWriter.Init(tlvDataStart, maxLen);
    rootWriter.ImplicitProfileId = chip::Protocols::kChipProtocol_ServiceProvisioning;

    // The cost (in bytes) of the top-level container is amortized as soon as there is at least 4 optionals elements.
    if ((optionalData.size() + optionalExtensionData.size()) >= 4)
    {

        TLVWriter innerStructureWriter;

        err = rootWriter.OpenContainer(ProfileTag(rootWriter.ImplicitProfileId, kTag_QRCodeExensionDescriptor), kTLVType_Structure,
                                       innerStructureWriter);
        SuccessOrExit(err);

        for (OptionalQRCodeInfo info : optionalData)
        {
            err = writeTag(innerStructureWriter, ContextTag(info.tag), info);
            SuccessOrExit(err);
        }

        for (OptionalQRCodeInfoExtension info : optionalExtensionData)
        {
            err = writeTag(innerStructureWriter, ContextTag(info.tag), info);
            SuccessOrExit(err);
        }

        err = rootWriter.CloseContainer(innerStructureWriter);
        SuccessOrExit(err);
    }
    else
    {
        for (OptionalQRCodeInfo info : optionalData)
        {
            err = writeTag(rootWriter, ProfileTag(rootWriter.ImplicitProfileId, info.tag), info);
            SuccessOrExit(err);
        }

        for (OptionalQRCodeInfoExtension info : optionalExtensionData)
        {
            err = writeTag(rootWriter, ProfileTag(rootWriter.ImplicitProfileId, info.tag), info);
            SuccessOrExit(err);
        }
    }
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
    err = populateBits(bits, offset, (uint16_t) payload.rendezvousInformation, kRendezvousInfoFieldLengthInBits,
                       kTotalPayloadDataSizeInBits);
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
