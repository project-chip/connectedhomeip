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
#include "Base38Encode.h"

#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVData.h>
#include <lib/core/TLVDebug.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ScopedBuffer.h>
#include <protocols/Protocols.h>

#include <stdlib.h>
#include <string.h>

namespace chip {

// Populates numberOfBits starting from LSB of input into bits, which is assumed to be zero-initialized
static CHIP_ERROR populateBits(uint8_t * bits, size_t & offset, uint64_t input, size_t numberOfBits,
                               size_t totalPayloadDataSizeInBits)
{
    VerifyOrReturnError(offset + numberOfBits <= totalPayloadDataSizeInBits, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(input < 1u << numberOfBits, CHIP_ERROR_INVALID_ARGUMENT);

    size_t index = offset;
    offset += numberOfBits;
    while (input != 0)
    {
        if (input & 1)
        {
            const uint8_t mask = static_cast<uint8_t>(1 << index % 8);
            bits[index / 8]    = static_cast<uint8_t>(bits[index / 8] | mask);
        }
        index++;
        input >>= 1;
    }
    return CHIP_NO_ERROR;
}

static CHIP_ERROR populateTLVBits(uint8_t * bits, size_t & offset, const uint8_t * tlvBuf, size_t tlvBufSizeInBytes,
                                  size_t totalPayloadDataSizeInBits)
{
    for (size_t i = 0; i < tlvBufSizeInBytes; i++)
    {
        const uint8_t value = tlvBuf[i];
        ReturnErrorOnFailure(populateBits(bits, offset, value, 8, totalPayloadDataSizeInBits));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR writeTag(TLV::TLVWriter & writer, TLV::Tag tag, OptionalQRCodeInfo & info)
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

CHIP_ERROR writeTag(TLV::TLVWriter & writer, TLV::Tag tag, OptionalQRCodeInfoExtension & info)
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
                                                                    uint32_t maxLen, size_t & tlvDataLengthInBytes)
{
    std::vector<OptionalQRCodeInfo> optionalData                   = outPayload.getAllOptionalVendorData();
    std::vector<OptionalQRCodeInfoExtension> optionalExtensionData = outPayload.getAllOptionalExtensionData();
    VerifyOrReturnError(!optionalData.empty() || !optionalExtensionData.empty(), CHIP_NO_ERROR);

    TLV::TLVWriter rootWriter;
    rootWriter.Init(tlvDataStart, maxLen);

    TLV::TLVWriter innerStructureWriter;

    ReturnErrorOnFailure(rootWriter.OpenContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, innerStructureWriter));

    for (OptionalQRCodeInfo info : optionalData)
    {
        ReturnErrorOnFailure(writeTag(innerStructureWriter, TLV::ContextTag(info.tag), info));
    }

    for (OptionalQRCodeInfoExtension info : optionalExtensionData)
    {
        ReturnErrorOnFailure(writeTag(innerStructureWriter, TLV::ContextTag(info.tag), info));
    }

    ReturnErrorOnFailure(rootWriter.CloseContainer(innerStructureWriter));

    ReturnErrorOnFailure(rootWriter.Finalize());

    tlvDataLengthInBytes = rootWriter.GetLengthWritten();

    return CHIP_NO_ERROR;
}

static CHIP_ERROR generateBitSet(PayloadContents & payload, MutableByteSpan & bits, uint8_t * tlvDataStart,
                                 size_t tlvDataLengthInBytes)
{
    size_t offset                 = 0;
    size_t totalPayloadSizeInBits = kTotalPayloadDataSizeInBits + (tlvDataLengthInBytes * 8);
    VerifyOrReturnError(bits.size() * 8 >= totalPayloadSizeInBits, CHIP_ERROR_BUFFER_TOO_SMALL);

    ReturnErrorOnFailure(
        populateBits(bits.data(), offset, payload.version, kVersionFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(
        populateBits(bits.data(), offset, payload.vendorID, kVendorIDFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(
        populateBits(bits.data(), offset, payload.productID, kProductIDFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(populateBits(bits.data(), offset, static_cast<uint64_t>(payload.commissioningFlow),
                                      kCommissioningFlowFieldLengthInBits, kTotalPayloadDataSizeInBits));
    VerifyOrReturnError(payload.rendezvousInformation.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(populateBits(bits.data(), offset, payload.rendezvousInformation.Value().Raw(),
                                      kRendezvousInfoFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(populateBits(bits.data(), offset, payload.discriminator.GetLongValue(),
                                      kPayloadDiscriminatorFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(
        populateBits(bits.data(), offset, payload.setUpPINCode, kSetupPINCodeFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(populateBits(bits.data(), offset, 0, kPaddingFieldLengthInBits, kTotalPayloadDataSizeInBits));
    ReturnErrorOnFailure(populateTLVBits(bits.data(), offset, tlvDataStart, tlvDataLengthInBytes, totalPayloadSizeInBits));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR payloadBase38RepresentationWithTLV(PayloadContents & payload, MutableCharSpan & outBuffer, MutableByteSpan bits,
                                                     uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    memset(bits.data(), 0, bits.size());
    ReturnErrorOnFailure(generateBitSet(payload, bits, tlvDataStart, tlvDataLengthInBytes));

    CHIP_ERROR err   = CHIP_NO_ERROR;
    size_t prefixLen = strlen(kQRCodePrefix);

    if (outBuffer.size() < prefixLen + 1)
    {
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else
    {
        MutableCharSpan subSpan = outBuffer.SubSpan(prefixLen, outBuffer.size() - prefixLen);
        memcpy(outBuffer.data(), kQRCodePrefix, prefixLen);
        err = base38Encode(bits, subSpan);
        // Reduce output span size to be the size of written data
        outBuffer.reduce_size(subSpan.size() + prefixLen);
    }

    return err;
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase38Representation(std::string & base38Representation)
{
    // 6.1.2.2. Table: Packed Binary Data Structure
    // The TLV Data should be 0 length if TLV is not included.
    return payloadBase38Representation(base38Representation, nullptr, 0);
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase38RepresentationWithAutoTLVBuffer(std::string & base38Representation)
{
    // Estimate the size of the needed buffer.
    size_t estimate = 0;

    auto dataItemSizeEstimate = [](const OptionalQRCodeInfo & item) {
        // Each data item needs a control byte and a context tag.
        size_t size = 2;

        if (item.type == optionalQRCodeInfoTypeString)
        {
            // We'll need to encode the string length and then the string data.
            // Length is at most 8 bytes.
            size += 8;
            size += item.data.size();
        }
        else
        {
            // Integer.  Assume it might need up to 8 bytes, for simplicity.
            size += 8;
        }
        return size;
    };

    auto vendorData = mPayload.getAllOptionalVendorData();
    for (auto & data : vendorData)
    {
        estimate += dataItemSizeEstimate(data);
    }

    auto extensionData = mPayload.getAllOptionalExtensionData();
    for (auto & data : extensionData)
    {
        estimate += dataItemSizeEstimate(data);
    }

    estimate = TLV::EstimateStructOverhead(estimate);

    VerifyOrReturnError(CanCastTo<uint32_t>(estimate), CHIP_ERROR_NO_MEMORY);

    Platform::ScopedMemoryBuffer<uint8_t> buf;
    VerifyOrReturnError(buf.Alloc(estimate), CHIP_ERROR_NO_MEMORY);

    return payloadBase38Representation(base38Representation, buf.Get(), static_cast<uint32_t>(estimate));
}

CHIP_ERROR QRCodeSetupPayloadGenerator::payloadBase38Representation(std::string & base38Representation, uint8_t * tlvDataStart,
                                                                    uint32_t tlvDataStartSize)
{
    size_t tlvDataLengthInBytes = 0;

    VerifyOrReturnError(mAllowInvalidPayload || mPayload.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(generateTLVFromOptionalData(mPayload, tlvDataStart, tlvDataStartSize, tlvDataLengthInBytes));

    std::vector<uint8_t> bits(kTotalPayloadDataSizeInBytes + tlvDataLengthInBytes);
    MutableByteSpan bitsSpan(bits.data(), bits.capacity());
    std::vector<char> buffer(base38EncodedLength(bits.capacity()) + strlen(kQRCodePrefix));
    MutableCharSpan bufferSpan(buffer.data(), buffer.capacity());

    ReturnErrorOnFailure(payloadBase38RepresentationWithTLV(mPayload, bufferSpan, bitsSpan, tlvDataStart, tlvDataLengthInBytes));

    base38Representation.assign(bufferSpan.data());
    return CHIP_NO_ERROR;
}

CHIP_ERROR QRCodeBasicSetupPayloadGenerator::payloadBase38Representation(MutableCharSpan & outBuffer)
{
    uint8_t bits[kTotalPayloadDataSizeInBytes];
    VerifyOrReturnError(mPayload.isValidQRCodePayload(), CHIP_ERROR_INVALID_ARGUMENT);

    return payloadBase38RepresentationWithTLV(mPayload, outBuffer, MutableByteSpan(bits), nullptr, 0);
}

} // namespace chip
