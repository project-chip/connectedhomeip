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
 * @file This file contains the implementation the chip message header
 *       encode/decode classes.
 */

#include "MessageHeader.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>

#include <type_traits>

#include <core/CHIPEncoding.h>
#include <core/CHIPError.h>
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

    static_assert(kEncryptedHeaderSizeBytes + kVendorIdSizeBytes <= UINT16_MAX, "Header size does not fit in uint16_t");
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

CHIP_ERROR PacketHeader::Decode(const uint8_t * const data, size_t size, uint16_t * decode_len)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    const uint8_t * p = data;
    int version;

    VerifyOrExit(size >= kFixedUnencryptedHeaderSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t header;
    header  = LittleEndian::Read16(p);
    version = ((header & kVersionMask) >> kVersionShift);
    VerifyOrExit(version == kHeaderVersion, err = CHIP_ERROR_VERSION_MISMATCH);

    mEncryptionType = (Header::EncryptionType)((header & kEncryptionTypeMask) >> kEncryptionTypeShift);
    mFlags.value    = header & Header::Flags::kMask;

    mMessageId = LittleEndian::Read32(p);

    if (mFlags.value & Header::Flags::kSourceNodeIdPresent)
    {
        static_assert(kNodeIdSizeBytes == 8, "Read64 might read more bytes than we have in the buffer");
        VerifyOrExit(size >= static_cast<size_t>(p - data) + kNodeIdSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);
        mSourceNodeId.SetValue(LittleEndian::Read64(p));
    }
    else
    {
        mSourceNodeId.ClearValue();
    }

    if (mFlags.value & Header::Flags::kDestinationNodeIdPresent)
    {
        VerifyOrExit(size >= static_cast<size_t>(p - data) + kNodeIdSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);
        mDestinationNodeId.SetValue(LittleEndian::Read64(p));
    }
    else
    {
        mDestinationNodeId.ClearValue();
    }

    VerifyOrExit(size >= static_cast<size_t>(p - data) + sizeof(uint16_t) * 2, err = CHIP_ERROR_INVALID_ARGUMENT);
    mEncryptionKeyID = LittleEndian::Read16(p);
    mPayloadLength   = LittleEndian::Read16(p);

    VerifyOrExit(p - data == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *decode_len = static_cast<uint16_t>(p - data);

exit:

    return err;
}

CHIP_ERROR PayloadHeader::Decode(Header::Flags flags, const uint8_t * const data, size_t size, uint16_t * decode_len)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    const uint8_t * p = data;

    VerifyOrExit(size >= kEncryptedHeaderSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);

    mExchangeHeader = Read8(p);
    mMessageType    = Read8(p);
    mExchangeID     = LittleEndian::Read16(p);

    if (flags.value & Header::Flags::kVendorIdPresent)
    {
        VerifyOrExit(size >= static_cast<size_t>(p - data) + kVendorIdSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);
        mVendorId.SetValue(LittleEndian::Read16(p));
    }
    else
    {
        mVendorId.ClearValue();
    }

    VerifyOrExit(size >= static_cast<size_t>(p - data) + sizeof(uint16_t), err = CHIP_ERROR_INVALID_ARGUMENT);
    mProtocolID = LittleEndian::Read16(p);

    VerifyOrExit(p - data == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *decode_len = static_cast<uint16_t>(p - data);

exit:

    return err;
}

CHIP_ERROR PacketHeader::Encode(uint8_t * data, size_t size, uint16_t * encode_size, Header::Flags payloadFlags) const
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    uint8_t * p     = data;
    uint16_t header = kHeaderVersion << kVersionShift;

    VerifyOrExit(size >= EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Payload flags are restricted.
    VerifyOrExit((payloadFlags.value & Header::Flags::kValidPayloadFlags) == payloadFlags.value, err = CHIP_ERROR_INVALID_ARGUMENT);

    header |= mFlags.value;
    header |= payloadFlags.value;

    if (mSourceNodeId.HasValue())
    {
        header |= Header::Flags::kSourceNodeIdPresent;
    }
    if (mDestinationNodeId.HasValue())
    {
        header |= Header::Flags::kDestinationNodeIdPresent;
    }

    header |= (static_cast<uint16_t>((uint16_t) mEncryptionType << kEncryptionTypeShift) & kEncryptionTypeMask);

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

CHIP_ERROR PayloadHeader::Encode(uint8_t * data, size_t size, uint16_t * encode_size) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t * p    = data;
    VerifyOrExit(size >= EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    Write8(p, mExchangeHeader);
    Write8(p, mMessageType);
    LittleEndian::Write16(p, mExchangeID);
    if (mVendorId.HasValue())
    {
        LittleEndian::Write16(p, mVendorId.Value());
    }
    LittleEndian::Write16(p, mProtocolID);

    // Written data size provided to caller on success
    VerifyOrExit(p - data == EncodeSizeBytes(), err = CHIP_ERROR_INTERNAL);
    *encode_size = static_cast<uint16_t>(p - data);

exit:
    return err;
}

Header::Flags PayloadHeader::GetEncodePacketFlags() const
{
    Header::Flags result;
    if (mVendorId.HasValue())
    {
        result.value |= Header::Flags::kVendorIdPresent;
    }
    return result;
}

CHIP_ERROR MessageAuthenticationCode::Decode(const PacketHeader & packetHeader, const uint8_t * const data, size_t size,
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

CHIP_ERROR MessageAuthenticationCode::Encode(const PacketHeader & packetHeader, uint8_t * data, size_t size,
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
