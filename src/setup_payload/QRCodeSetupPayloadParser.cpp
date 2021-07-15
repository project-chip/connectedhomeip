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
#include "Base38.h"

#include <math.h>
#include <memory>
#include <string.h>
#include <vector>

#include <core/CHIPCore.h>
#include <core/CHIPError.h>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
#include <support/SafeInt.h>
#include <support/ScopedBuffer.h>

namespace chip {

// Populate numberOfBits into dest from buf starting at startIndex
static CHIP_ERROR readBits(std::vector<uint8_t> buf, size_t & index, uint64_t & dest, size_t numberOfBitsToRead)
{
    dest = 0;
    if (index + numberOfBitsToRead > buf.size() * 8 || numberOfBitsToRead > sizeof(uint64_t) * 8)
    {
        ChipLogError(SetupPayload, "Error parsing QR code. startIndex %zu numberOfBitsToLoad %zu buf_len %zu ", index,
                     numberOfBitsToRead, buf.size());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    size_t currentIndex = index;
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

static CHIP_ERROR openTLVContainer(TLV::TLVReader & reader, TLV::TLVType type, uint64_t tag, TLV::TLVReader & containerReader)
{
    VerifyOrReturnError(reader.GetType() == type, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(reader.GetTag() == tag, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(reader.GetLength() == 0, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(reader.OpenContainer(containerReader));

    VerifyOrReturnError(containerReader.GetContainerType() == type, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR retrieveOptionalInfoString(TLV::TLVReader & reader, OptionalQRCodeInfo & info)
{
    const uint32_t valLength = reader.GetLength();
    chip::Platform::ScopedMemoryBuffer<char> value;
    value.Alloc(valLength + 1);
    VerifyOrReturnError(value, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(reader.GetString(value.Get(), valLength + 1));

    info.type = optionalQRCodeInfoTypeString;
    info.data = std::string(value.Get());

    return CHIP_NO_ERROR;
}

static CHIP_ERROR retrieveOptionalInfoInt32(TLV::TLVReader & reader, OptionalQRCodeInfo & info)
{
    int32_t value;
    ReturnErrorOnFailure(reader.Get(value));

    info.type  = optionalQRCodeInfoTypeInt32;
    info.int32 = value;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR retrieveOptionalInfoInt64(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info)
{
    int64_t value;
    ReturnErrorOnFailure(reader.Get(value));

    info.type  = optionalQRCodeInfoTypeInt64;
    info.int64 = value;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR retrieveOptionalInfoUInt32(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info)
{
    uint32_t value;
    ReturnErrorOnFailure(reader.Get(value));

    info.type   = optionalQRCodeInfoTypeUInt32;
    info.uint32 = value;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR retrieveOptionalInfoUInt64(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info)
{
    uint64_t value;
    ReturnErrorOnFailure(reader.Get(value));

    info.type   = optionalQRCodeInfoTypeUInt64;
    info.uint64 = value;

    return CHIP_NO_ERROR;
}

static CHIP_ERROR retrieveOptionalInfo(TLV::TLVReader & reader, OptionalQRCodeInfo & info, optionalQRCodeInfoType type)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (type == optionalQRCodeInfoTypeString)
    {
        err = retrieveOptionalInfoString(reader, info);
    }
    else if (type == optionalQRCodeInfoTypeInt32)
    {
        err = retrieveOptionalInfoInt32(reader, info);
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

static CHIP_ERROR retrieveOptionalInfo(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info, optionalQRCodeInfoType type)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (type == optionalQRCodeInfoTypeString || type == optionalQRCodeInfoTypeInt32)
    {
        err = retrieveOptionalInfo(reader, static_cast<OptionalQRCodeInfo &>(info), type);
    }
    else if (type == optionalQRCodeInfoTypeInt64)
    {
        err = retrieveOptionalInfoInt64(reader, info);
    }
    else if (type == optionalQRCodeInfoTypeUInt32)
    {
        err = retrieveOptionalInfoUInt32(reader, info);
    }
    else if (type == optionalQRCodeInfoTypeUInt64)
    {
        err = retrieveOptionalInfoUInt64(reader, info);
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

CHIP_ERROR QRCodeSetupPayloadParser::retrieveOptionalInfos(SetupPayload & outPayload, TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (err == CHIP_NO_ERROR)
    {
        const TLV::TLVType type = reader.GetType();
        if (type != TLV::kTLVType_UTF8String && type != TLV::kTLVType_SignedInteger && type != TLV::kTLVType_UnsignedInteger)
        {
            err = reader.Next();
            continue;
        }

        const uint8_t tag = static_cast<uint8_t>(TLV::TagNumFromTag(reader.GetTag()));
        VerifyOrReturnError(TLV::IsContextTag(tag) == true || TLV::IsProfileTag(tag) == true, CHIP_ERROR_INVALID_TLV_TAG);

        optionalQRCodeInfoType elemType = optionalQRCodeInfoTypeUnknown;
        if (type == TLV::kTLVType_UTF8String)
        {
            elemType = optionalQRCodeInfoTypeString;
        }
        if (type == TLV::kTLVType_SignedInteger || type == TLV::kTLVType_UnsignedInteger)
        {
            elemType = outPayload.getNumericTypeFor(tag);
        }

        if (IsCHIPTag(tag))
        {
            OptionalQRCodeInfoExtension info;
            info.tag = tag;
            ReturnErrorOnFailure(retrieveOptionalInfo(reader, info, elemType));

            ReturnErrorOnFailure(outPayload.addOptionalExtensionData(info));
        }
        else
        {
            OptionalQRCodeInfo info;
            info.tag = tag;
            ReturnErrorOnFailure(retrieveOptionalInfo(reader, info, elemType));

            ReturnErrorOnFailure(outPayload.addOptionalVendorData(info));
        }
        err = reader.Next();
    }
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR QRCodeSetupPayloadParser::parseTLVFields(SetupPayload & outPayload, uint8_t * tlvDataStart, size_t tlvDataLengthInBytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!CanCastTo<uint32_t>(tlvDataLengthInBytes))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    TLV::TLVReader rootReader;
    rootReader.Init(tlvDataStart, static_cast<uint32_t>(tlvDataLengthInBytes));
    ReturnErrorOnFailure(rootReader.Next());

    if (rootReader.GetType() != TLV::kTLVType_Structure)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLV::TLVReader innerStructureReader;
    ReturnErrorOnFailure(openTLVContainer(rootReader, TLV::kTLVType_Structure, TLV::AnonymousTag, innerStructureReader));
    ReturnErrorOnFailure(innerStructureReader.Next());
    err = retrieveOptionalInfos(outPayload, innerStructureReader);

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR QRCodeSetupPayloadParser::populateTLV(SetupPayload & outPayload, const std::vector<uint8_t> & buf, size_t & index)
{
    size_t bitsLeftToRead = (buf.size() * 8) - index;
    size_t tlvBytesLength = (bitsLeftToRead + 7) / 8; // ceil(bitsLeftToRead/8)
    chip::Platform::ScopedMemoryBuffer<uint8_t> tlvArray;

    ReturnErrorCodeIf(tlvBytesLength == 0, CHIP_NO_ERROR);

    tlvArray.Alloc(tlvBytesLength);
    ReturnErrorCodeIf(!tlvArray, CHIP_ERROR_NO_MEMORY);

    for (size_t i = 0; i < tlvBytesLength; i++)
    {
        uint64_t dest;
        readBits(buf, index, dest, 8);
        tlvArray[i] = static_cast<uint8_t>(dest);
    }

    return parseTLVFields(outPayload, tlvArray.Get(), tlvBytesLength);
}

static std::string extractPayload(std::string inString)
{
    std::string chipSegment;
    char delimiter = '%';
    std::vector<size_t> startIndices;
    startIndices.push_back(0);

    for (size_t i = 0; i < inString.length(); i++)
    {
        if (inString[i] == delimiter)
        {
            startIndices.push_back(i + 1);
        }
    }

    // Find the first string between delimiters that starts with kQRCodePrefix
    for (size_t i = 0; i < startIndices.size(); i++)
    {
        size_t startIndex   = startIndices[i];
        size_t endIndex     = (i == startIndices.size() - 1 ? std::string::npos : startIndices[i + 1] - 1);
        size_t length       = (endIndex != std::string::npos ? endIndex - startIndex : std::string::npos);
        std::string segment = inString.substr(startIndex, length);

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

    return chipSegment;
}

CHIP_ERROR QRCodeSetupPayloadParser::populatePayload(SetupPayload & outPayload)
{
    std::vector<uint8_t> buf;
    size_t indexToReadFrom = 0;
    uint64_t dest;

    std::string payload = extractPayload(mBase38Representation);
    VerifyOrReturnError(payload.length() != 0, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(base38Decode(payload, buf));

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kVersionFieldLengthInBits));
    static_assert(kVersionFieldLengthInBits <= 8, "Won't fit in uint8_t");
    outPayload.version = static_cast<uint8_t>(dest);

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kVendorIDFieldLengthInBits));
    static_assert(kVendorIDFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.vendorID = static_cast<uint16_t>(dest);

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kProductIDFieldLengthInBits));
    static_assert(kProductIDFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.productID = static_cast<uint16_t>(dest);

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kCommissioningFlowFieldLengthInBits));
    static_assert(kCommissioningFlowFieldLengthInBits <= std::numeric_limits<std::underlying_type_t<CommissioningFlow>>::digits,
                  "Won't fit in CommissioningFlow");
    outPayload.commissioningFlow = static_cast<CommissioningFlow>(dest);

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kRendezvousInfoFieldLengthInBits));
    static_assert(kRendezvousInfoFieldLengthInBits <= 8 * sizeof(RendezvousInformationFlag),
                  "Won't fit in RendezvousInformationFlags");
    outPayload.rendezvousInformation = RendezvousInformationFlags(static_cast<RendezvousInformationFlag>(dest));

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kPayloadDiscriminatorFieldLengthInBits));
    static_assert(kPayloadDiscriminatorFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.discriminator = static_cast<uint16_t>(dest);

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kSetupPINCodeFieldLengthInBits));
    static_assert(kSetupPINCodeFieldLengthInBits <= 32, "Won't fit in uint32_t");
    outPayload.setUpPINCode = static_cast<uint32_t>(dest);

    ReturnErrorOnFailure(readBits(buf, indexToReadFrom, dest, kPaddingFieldLengthInBits));

    return populateTLV(outPayload, buf, indexToReadFrom);
}

} // namespace chip
