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
 * @file  This file contains the implementation the chip::MessageHeader class.
 */

#include "MessageHeader.h"

#include <assert.h>

#include <core/CHIPEncoding.h>
#include <core/CHIPError.h>
#include <support/CodeUtils.h>

/**********************************************
 * Header format (little endian):
 *
 *  16 bit: | VERSION: 4 bit | FLAGS: 4 bit | RESERVED: 8 bit |
 *  16 bit: | Secure message type                             |
 *  32 bit: | MESSAGE_ID                                      |
 *  32 bit: | Secure Session ID                               |
 *  16 bit: | Profile ID                                      |
 *  64 bit: | Message Authentication Tag                      |
 *  64 bit: | SOURCE_NODE_ID (iff source node flag is set)    |
 *  64 bit: | DEST_NODE_ID (iff destination node flag is set) |
 *
 **********************************************/

namespace chip {
namespace {

using namespace chip::Encoding;

/// size of the fixed portion of the header
constexpr size_t kFixedHeaderSizeBytes = 20;

/// size of a serialized node id inside a header
constexpr size_t kNodeIdSizeBytes = 8;

/// Header flag specifying that a destination node id is included in the header.
constexpr uint16_t kFlagDestinationNodeIdPresent = 0x0100;
/// Header flag specifying that a source node id is included in the header.
constexpr uint16_t kFlagSourceNodeIdPresent = 0x0200;

/// Mask to extract just the version part from a 16bit header prefix.
constexpr uint16_t kVersionMask = 0xF000;
/// Shift to convert to/from a masked version 16bit value to a 4bit version.
constexpr int kVersionShift = 12;

} // namespace

size_t MessageHeader::EncodeSizeBytes() const
{
    size_t size = kFixedHeaderSizeBytes;

    if (mSourceNodeId.HasValue())
    {
        size += kNodeIdSizeBytes;
    }

    if (mDestinationNodeId.HasValue())
    {
        size += kNodeIdSizeBytes;
    }

    return size;
}

CHIP_ERROR MessageHeader::Decode(const uint8_t * data, size_t size, size_t * decode_len)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    const uint8_t * p = data;
    uint16_t header;
    int version;

    VerifyOrExit(size >= kFixedHeaderSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);

    header  = LittleEndian::Read16(p);
    version = ((header & kVersionMask) >> kVersionShift);
    VerifyOrExit(version == kHeaderVersion, err = CHIP_ERROR_VERSION_MISMATCH);

    mSecureMsgType   = LittleEndian::Read16(p);
    mMessageId       = LittleEndian::Read32(p);
    mSecureSessionID = LittleEndian::Read32(p);
    mProfileID       = LittleEndian::Read16(p);
    mTag             = LittleEndian::Read64(p);

    assert(p - data == kFixedHeaderSizeBytes);
    size -= kFixedHeaderSizeBytes;

    if (header & kFlagSourceNodeIdPresent)
    {
        VerifyOrExit(size >= kNodeIdSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);
        mSourceNodeId.SetValue(LittleEndian::Read64(p));
        size -= kNodeIdSizeBytes;
    }
    else
    {
        mSourceNodeId.ClearValue();
    }

    if (header & kFlagDestinationNodeIdPresent)
    {
        VerifyOrExit(size >= kNodeIdSizeBytes, err = CHIP_ERROR_INVALID_ARGUMENT);
        mDestinationNodeId.SetValue(LittleEndian::Read64(p));
        size -= kNodeIdSizeBytes;
    }
    else
    {
        mDestinationNodeId.ClearValue();
    }

    *decode_len = p - data;

exit:

    return err;
}

CHIP_ERROR MessageHeader::Encode(uint8_t * data, size_t size, size_t * encode_size) const
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    uint8_t * p     = data;
    uint16_t header = kHeaderVersion << kVersionShift;

    VerifyOrExit(size >= EncodeSizeBytes(), err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mSourceNodeId.HasValue())
    {
        header |= kFlagSourceNodeIdPresent;
    }
    if (mDestinationNodeId.HasValue())
    {
        header |= kFlagDestinationNodeIdPresent;
    }

    LittleEndian::Write16(p, header);
    LittleEndian::Write16(p, mSecureMsgType);
    LittleEndian::Write32(p, mMessageId);
    LittleEndian::Write32(p, mSecureSessionID);
    LittleEndian::Write16(p, mProfileID);
    LittleEndian::Write64(p, mTag);
    if (mSourceNodeId.HasValue())
    {
        LittleEndian::Write64(p, mSourceNodeId.Value());
    }
    if (mDestinationNodeId.HasValue())
    {
        LittleEndian::Write64(p, mDestinationNodeId.Value());
    }

    // Written data size provided to caller on success
    *encode_size = p - data;

exit:
    return err;
}

} // namespace chip
