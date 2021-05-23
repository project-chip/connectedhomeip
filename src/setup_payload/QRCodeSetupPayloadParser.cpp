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
        ChipLogError(SetupPayload, "Error parsing QR code. startIndex %d numberOfBitsToLoad %zu buf_len %zu ", index,
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

static CHIP_ERROR retrieveOptionalInfoString(TLV::TLVReader & reader, OptionalQRCodeInfo & info)
{
    CHIP_ERROR err;
    uint32_t valLength = reader.GetLength();
    chip::Platform::ScopedMemoryBuffer<char> value;
    value.Alloc(valLength + 1);
    VerifyOrExit(value, err = CHIP_ERROR_NO_MEMORY);

    err = reader.GetString(value.Get(), valLength + 1);
    SuccessOrExit(err);

    info.type = optionalQRCodeInfoTypeString;
    info.data = std::string(value.Get());

exit:
    return err;
}

static CHIP_ERROR retrieveOptionalInfoInt32(TLV::TLVReader & reader, OptionalQRCodeInfo & info)
{
    int32_t value;
    CHIP_ERROR err = reader.Get(value);
    SuccessOrExit(err);

    info.type  = optionalQRCodeInfoTypeInt32;
    info.int32 = value;

exit:
    return err;
}

static CHIP_ERROR retrieveOptionalInfoInt64(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info)
{
    int64_t value;
    CHIP_ERROR err = reader.Get(value);
    SuccessOrExit(err);

    info.type  = optionalQRCodeInfoTypeInt64;
    info.int64 = value;

exit:
    return err;
}

static CHIP_ERROR retrieveOptionalInfoUInt32(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info)
{
    uint32_t value;
    CHIP_ERROR err = reader.Get(value);
    SuccessOrExit(err);

    info.type   = optionalQRCodeInfoTypeUInt32;
    info.uint32 = value;

exit:
    return err;
}

static CHIP_ERROR retrieveOptionalInfoUInt64(TLV::TLVReader & reader, OptionalQRCodeInfoExtension & info)
{
    uint64_t value;
    CHIP_ERROR err = reader.Get(value);
    SuccessOrExit(err);

    info.type   = optionalQRCodeInfoTypeUInt64;
    info.uint64 = value;

exit:
    return err;
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
    TLV::TLVType type;
    uint8_t tag;
    while (err == CHIP_NO_ERROR)
    {
        type = reader.GetType();
        if (type != TLV::kTLVType_UTF8String && type != TLV::kTLVType_SignedInteger && type != TLV::kTLVType_UnsignedInteger)
        {
            err = reader.Next();
            continue;
        }

        tag = static_cast<uint8_t>(TLV::TagNumFromTag(reader.GetTag()));
        VerifyOrExit(TLV::IsContextTag(tag) == true || TLV::IsProfileTag(tag) == true, err = CHIP_ERROR_INVALID_TLV_TAG);

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
            err      = retrieveOptionalInfo(reader, info, elemType);
            SuccessOrExit(err);

            err = outPayload.addOptionalExtensionData(info);
            SuccessOrExit(err);
        }
        else
        {
            OptionalQRCodeInfo info;
            info.tag = tag;
            err      = retrieveOptionalInfo(reader, info, elemType);
            SuccessOrExit(err);

            err = outPayload.addOptionalVendorData(info);
            SuccessOrExit(err);
        }
        err = reader.Next();
    }
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }

exit:
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
    err = rootReader.Next();
    SuccessOrExit(err);

    if (rootReader.GetType() != TLV::kTLVType_Structure)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLV::TLVReader innerStructureReader;
    err = openTLVContainer(rootReader, TLV::kTLVType_Structure, TLV::AnonymousTag, innerStructureReader);
    SuccessOrExit(err);
    err = innerStructureReader.Next();
    SuccessOrExit(err);
    err = retrieveOptionalInfos(outPayload, innerStructureReader);

    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
exit:
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

std::string extractPayload(std::string inString)
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
    CHIP_ERROR err         = CHIP_NO_ERROR;
    size_t indexToReadFrom = 0;
    uint64_t dest;

    std::string payload = extractPayload(mBase41Representation);
    VerifyOrExit(payload.length() != 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    err = base41Decode(payload, buf);
    SuccessOrExit(err);

    err = readBits(buf, indexToReadFrom, dest, kVersionFieldLengthInBits);
    SuccessOrExit(err);
    static_assert(kVersionFieldLengthInBits <= 8, "Won't fit in uint8_t");
    outPayload.version = static_cast<uint8_t>(dest);

    err = readBits(buf, indexToReadFrom, dest, kVendorIDFieldLengthInBits);
    SuccessOrExit(err);
    static_assert(kVendorIDFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.vendorID = static_cast<uint16_t>(dest);

    err = readBits(buf, indexToReadFrom, dest, kProductIDFieldLengthInBits);
    SuccessOrExit(err);
    static_assert(kProductIDFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.productID = static_cast<uint16_t>(dest);

    err = readBits(buf, indexToReadFrom, dest, kCustomFlowRequiredFieldLengthInBits);
    SuccessOrExit(err);
    static_assert(kCustomFlowRequiredFieldLengthInBits <= 8, "Won't fit in uint8_t");
    outPayload.requiresCustomFlow = static_cast<uint8_t>(dest);

    err = readBits(buf, indexToReadFrom, dest, kRendezvousInfoFieldLengthInBits);
    SuccessOrExit(err);
    outPayload.rendezvousInformation = RendezvousInformationFlags(static_cast<RendezvousInformationFlag>(dest));

    err = readBits(buf, indexToReadFrom, dest, kPayloadDiscriminatorFieldLengthInBits);
    SuccessOrExit(err);
    static_assert(kPayloadDiscriminatorFieldLengthInBits <= 16, "Won't fit in uint16_t");
    outPayload.discriminator = static_cast<uint16_t>(dest);

    err = readBits(buf, indexToReadFrom, dest, kSetupPINCodeFieldLengthInBits);
    SuccessOrExit(err);
    static_assert(kSetupPINCodeFieldLengthInBits <= 32, "Won't fit in uint32_t");
    outPayload.setUpPINCode = static_cast<uint32_t>(dest);

    err = readBits(buf, indexToReadFrom, dest, kPaddingFieldLengthInBits);
    SuccessOrExit(err);

    err = populateTLV(outPayload, buf, indexToReadFrom);
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace chip
