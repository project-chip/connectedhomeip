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
 * @file
 *   This file contains the implementation the chip message header
 *   encode/decode classes.
 */

#include "MessageHeader.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include <type_traits>

#include <core/CHIPError.h>
#include <support/BufferReader.h>
#include <support/CodeUtils.h>

/**********************************************
 * Header format (little endian):
 *
 * -------- Unencrypted header -----------------------------------------------------
 *  16 bit: | VERSION: 4 bit | FLAGS: 4 bit | ENCRYPTTYPE: 4 bit | RESERVED: 4 bit |
 *  32 bit: | MESSAGE_ID                                                           |
 *  64 bit: | SOURCE_NODE_ID (iff source node flag is set)                         |
 *  64 bit: | DEST_NODE_ID (iff destination node flag is set)                      |
 *  16 bit: | Encryption Key ID                                                    |
 *  16 bit: | Payload Length                                                       |
 * -------- Encrypted header -------------------------------------------------------
 *  8 bit:  | Exchange Header                                                      |
 *  8 bit:  | Message Type                                                         |
 *  16 bit: | Exchange ID                                                          |
 *  16 bit: | Optional Vendor ID                                                   |
 *  16 bit: | Protocol ID                                                          |
 *  32 bit: | Acknowledged Message Counter (if A flag in the Header is set)        |
 * -------- Encrypted Application Data Start ---------------------------------------
 *  <var>:  | Encrypted Data                                                       |
 * -------- Encrypted Application Data End -----------------------------------------
 *  <var>:  | (Unencrypted) Message Authentication Tag                             |
 *
 **********************************************/

namespace chip {
namespace {

using namespace chip::Encoding;

/// size of the fixed portion of the header
constexpr size_t kFixedUnencryptedHeaderSizeBytes = 10;

/// size of the encrypted portion of the header
constexpr size_t kEncryptedHeaderSizeBytes = 6;

/// size of a serialized node id inside a header
constexpr size_t kNodeIdSizeBytes = 8;

/// size of a serialized vendor id inside a header
constexpr size_t kVendorIdSizeBytes = 2;

/// size of a serialized ack id inside a header
constexpr size_t kAckIdSizeBytes = 4;

/// Mask to extract just the version part from a 16bit header prefix.
constexpr uint16_t kVersionMask = 0xF000;
/// Shift to convert to/from a masked version 16bit value to a 4bit version.
constexpr int kVersionShift = 12;

/// Mask to extract just the encryption type part from a 16bit header prefix.
constexpr uint16_t kEncryptionTypeMask = 0xF0;
/// Shift to convert to/from a masked encryption type 16bit value to a 4bit encryption type.
constexpr int kEncryptionTypeShift = 4;

} // namespace

uint16_t PacketHeader::EncodeSizeBytes() const
{
    size_t size = kFixedUnencryptedHeaderSizeBytes;

    if (mSourceNodeId.HasValue())
    {
        size += kNodeIdSizeBytes;
    }

    if (mDestinationNodeId.HasValue())
    {
        size += kNodeIdSizeBytes;
    }

    static_assert(kFixedUnencryptedHeaderSizeBytes + kNodeIdSizeBytes + kNodeIdSizeBytes <= UINT16_MAX,
                  "Header size does not fit in uint16_t");
    return static_cast<uint16_t>(size);
}

uint16_t PayloadHeader::EncodeSizeBytes() const
{
    size_t size = kEncryptedHeaderSizeBytes;

    if (mVendorId.HasValue())
    {
        size += kVendorIdSizeBytes;
    }

    if (mAckId.HasValue())
    {
        size += kAckIdSizeBytes;
    }

    static_assert(kEncryptedHeaderSizeBytes + kVendorIdSizeBytes + kAckIdSizeBytes <= UINT16_MAX,
                  "Header size does not fit in uint16_t");
    return static_cast<uint16_t>(size);
}

uint16_t MessageAuthenticationCode::TagLenForEncryptionType(Header::EncryptionType encType)
{
    switch (encType)
    {
    case Header::EncryptionType::kAESCCMTagLen8:
        return 8;

    case Header::EncryptionType::kAESCCMTagLen12:
        return 12;

    case Header::EncryptionType::kAESCCMTagLen16:
        return 16;

    default:
        return 0;
    }
}

CHIP_ERROR PacketHeader::Decode(const uint8_t * const data, uint16_t size, uint16_t * decode_len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    LittleEndian::Reader reader(data, size);
    int version;
    uint16_t octets_read;

    uint16_t header;
    err = reader.Read16(&header).StatusCode();
    SuccessOrExit(err);
    version = ((header & kVersionMask) >> kVersionShift);
    VerifyOrExit(version == kHeaderVersion, err = CHIP_ERROR_VERSION_MISMATCH);

    mEncryptionType = static_cast<Header::EncryptionType>((header & kEncryptionTypeMask) >> kEncryptionTypeShift);
    mFlags.SetRaw(header & Header::kFlagsMask);

    err = reader.Read32(&mMessageId).StatusCode();
    SuccessOrExit(err);

    if (mFlags.Has(Header::FlagValues::kSourceNodeIdPresent))
    {
        uint64_t sourceNodeId;
        err = reader.Read64(&sourceNodeId).StatusCode();
        SuccessOrExit(err);
        mSourceNodeId.SetValue(sourceNodeId);
    }
    else
    {
        mSourceNodeId.ClearValue();
    }

    if (mFlags.Has(Header::FlagValues::kDestinationNodeIdPresent))
    {
        uint64_t destinationNodeId;
        err = reader.Read64(&destinationNodeId).StatusCode();
        SuccessOrExit(err);
        mDestinationNodeId.SetValue(destinationNodeId);
    }
    else
    {
        mDestinationNodeId.ClearValue();
    }

    err = reader.Read16(&mEncryptionKeyID).Read16(&mPayloadLength).StatusCode();
    SuccessOrExit(err);

    octets_read = reader.OctetsRead();
    VerifyOrExit(octets_read == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *decode_len = octets_read;

exit:

    return err;
}

CHIP_ERROR PayloadHeader::Decode(const uint8_t * const data, uint16_t size, uint16_t * decode_len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    LittleEndian::Reader reader(data, size);
    uint8_t header;
    uint16_t octets_read;

    err = reader.Read8(&header).Read8(&mMessageType).Read16(&mExchangeID).StatusCode();
    SuccessOrExit(err);

    mExchangeFlags.SetRaw(header);

    if (mExchangeFlags.Has(Header::ExFlagValues::kExchangeFlag_VendorIdPresent))
    {
        uint16_t vendor_id;
        err = reader.Read16(&vendor_id).StatusCode();
        SuccessOrExit(err);
        mVendorId.SetValue(vendor_id);
    }
    else
    {
        mVendorId.ClearValue();
    }

    err = reader.Read16(&mProtocolID).StatusCode();
    SuccessOrExit(err);

    if (mExchangeFlags.Has(Header::ExFlagValues::kExchangeFlag_AckMsg))
    {
        uint32_t ack_id;
        err = reader.Read32(&ack_id).StatusCode();
        SuccessOrExit(err);
        mAckId.SetValue(ack_id);
    }
    else
    {
        mAckId.ClearValue();
    }

    octets_read = reader.OctetsRead();
    VerifyOrExit(octets_read == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *decode_len = octets_read;

exit:

    return err;
}

CHIP_ERROR PacketHeader::Encode(uint8_t * data, uint16_t size, uint16_t * encode_size) const
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    uint8_t * p     = data;
    uint16_t header = kHeaderVersion << kVersionShift;

    VerifyOrExit(size >= EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    {
        Header::Flags encodeFlags = mFlags;
        encodeFlags.Set(Header::FlagValues::kSourceNodeIdPresent, mSourceNodeId.HasValue())
            .Set(Header::FlagValues::kDestinationNodeIdPresent, mDestinationNodeId.HasValue());

        header = header | encodeFlags.Raw();
    }

    header |= (static_cast<uint16_t>(static_cast<uint16_t>(mEncryptionType) << kEncryptionTypeShift) & kEncryptionTypeMask);

    LittleEndian::Write16(p, header);
    LittleEndian::Write32(p, mMessageId);
    if (mSourceNodeId.HasValue())
    {
        LittleEndian::Write64(p, mSourceNodeId.Value());
    }
    if (mDestinationNodeId.HasValue())
    {
        LittleEndian::Write64(p, mDestinationNodeId.Value());
    }

    LittleEndian::Write16(p, mEncryptionKeyID);
    LittleEndian::Write16(p, mPayloadLength);

    // Written data size provided to caller on success
    VerifyOrExit(p - data == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *encode_size = static_cast<uint16_t>(p - data);

exit:
    return err;
}

CHIP_ERROR PayloadHeader::Encode(uint8_t * data, uint16_t size, uint16_t * encode_size) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t * p    = data;
    uint8_t header = mExchangeFlags.Raw();

    VerifyOrExit(size >= EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    Write8(p, header);
    Write8(p, mMessageType);
    LittleEndian::Write16(p, mExchangeID);
    if (mVendorId.HasValue())
    {
        LittleEndian::Write16(p, mVendorId.Value());
    }
    LittleEndian::Write16(p, mProtocolID);
    if (mAckId.HasValue())
    {
        LittleEndian::Write32(p, mAckId.Value());
    }

    // Written data size provided to caller on success
    VerifyOrExit(p - data == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *encode_size = static_cast<uint16_t>(p - data);

exit:
    return err;
}

CHIP_ERROR MessageAuthenticationCode::Decode(const PacketHeader & packetHeader, const uint8_t * const data, uint16_t size,
                                             uint16_t * decode_len)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    const uint8_t * p     = data;
    const uint16_t taglen = TagLenForEncryptionType(packetHeader.GetEncryptionType());

    VerifyOrExit(taglen != 0, err = CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER);
    VerifyOrExit(size >= taglen, err = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(&mTag[0], p, taglen);

    *decode_len = taglen;

exit:

    return err;
}

CHIP_ERROR MessageAuthenticationCode::Encode(const PacketHeader & packetHeader, uint8_t * data, uint16_t size,
                                             uint16_t * encode_size) const
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    uint8_t * p           = data;
    const uint16_t taglen = TagLenForEncryptionType(packetHeader.GetEncryptionType());

    VerifyOrExit(taglen != 0, err = CHIP_ERROR_WRONG_ENCRYPTION_TYPE);
    VerifyOrExit(size >= taglen, err = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(p, &mTag[0], taglen);

    // Written data size provided to caller on success
    *encode_size = taglen;

exit:
    return err;
}

} // namespace chip
