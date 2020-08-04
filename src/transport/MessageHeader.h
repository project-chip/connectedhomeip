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
 * @file  This file defines the chip::MessageHeader class.
 */

#ifndef MESSAGEHEADER_H_
#define MESSAGEHEADER_H_

#include <cstddef>
#include <cstdint>
#include <string.h>

#include <core/CHIPError.h>
#include <core/Optional.h>

namespace chip {

/// Convenience type to make it clear a number represents a node id.
typedef uint64_t NodeId;

constexpr NodeId kUndefinedNodeId = 0xFFFFFFFFFFFFFFFFll;
constexpr size_t kMaxTagLen       = 16;

/** Handles encoding/decoding of CHIP message headers */
class MessageHeader
{
public:
    enum class EncryptionType
    {
        kAESCCMTagLen8  = 0,
        kAESCCMTagLen12 = 1,
        kAESCCMTagLen16 = 2,
    };

    /**
     * Gets the source node id in the current message.
     *
     * NOTE: the source node id is optional and may be missing.
     */
    const Optional<NodeId> & GetSourceNodeId() const { return mSourceNodeId; }

    /**
     * Gets the destination node id in the current message.
     *
     * NOTE: the destination node id is optional and may be missing.
     */
    const Optional<NodeId> & GetDestinationNodeId() const { return mDestinationNodeId; }

    /**
     * Gets the vendor id in the current message.
     *
     * NOTE: the vendor id is optional and may be missing.
     */
    const Optional<uint16_t> & GetVendorId() const { return mVendorId; }

    /**
     * Gets the message id set in the header.
     *
     * Message IDs are expecte to monotonically increase by one for each mesage
     * that has been sent.
     */
    uint32_t GetMessageId() const { return mMessageId; }

    /** Get the length of encrypted payload. */
    uint16_t GetPayloadLength() const { return mPayloadLength; }

    /** Get the secure msg type from this header. */
    uint8_t GetMessageType(void) const { return mMessageType; }

    /** Get the Session ID from this header. */
    uint16_t GetExchangeID(void) const { return mExchangeID; }

    /** Get the Protocol ID from this header. */
    uint16_t GetProtocolID(void) const { return mProtocolID; }

    /** Get the MAC tag length. */
    size_t GetTagLength(void) const { return TagLenForEncryptionType(mEncryptionType); }

    const uint8_t * GetTag(void) const { return &mTag[0]; }

    /** Set the message id for this header. */
    MessageHeader & SetMessageId(uint32_t id)
    {
        mMessageId = id;

        return *this;
    }

    /** Set the source node id for this header. */
    MessageHeader & SetSourceNodeId(NodeId id)
    {
        mSourceNodeId.SetValue(id);

        return *this;
    }

    /** Set the source node id for this header. */
    MessageHeader & SetSourceNodeId(Optional<NodeId> id)
    {
        mSourceNodeId = id;

        return *this;
    }

    /** Clear the source node id for this header. */
    MessageHeader & ClearSourceNodeId()
    {
        mSourceNodeId.ClearValue();

        return *this;
    }

    /** Set the destination node id for this header. */
    MessageHeader & SetDestinationNodeId(NodeId id)
    {
        mDestinationNodeId.SetValue(id);

        return *this;
    }

    /** Set the destination node id for this header. */
    MessageHeader & SetDestinationNodeId(Optional<NodeId> id)
    {
        mDestinationNodeId = id;
        return *this;
    }

    /** Clear the destination node id for this header. */
    MessageHeader & ClearDestinationNodeId()
    {
        mDestinationNodeId.ClearValue();

        return *this;
    }

    /** Set the vendor id for this header. */
    MessageHeader & SetVendorId(uint16_t id)
    {
        mVendorId.SetValue(id);

        return *this;
    }

    /** Set the vendor id for this header. */
    MessageHeader & SetVendorId(Optional<uint16_t> id)
    {
        mVendorId = id;

        return *this;
    }

    /** Clear the vendor id for this header. */
    MessageHeader & ClearVendorId()
    {
        mVendorId.ClearValue();

        return *this;
    }

    /** Set the secure message type for this header. */
    MessageHeader & SetPayloadLength(uint16_t len)
    {
        mPayloadLength = len;
        return *this;
    }

    /** Set the secure message type for this header. */
    MessageHeader & SetMessageType(uint8_t type)
    {
        mMessageType = type;
        return *this;
    }

    /** Set the security session ID for this header. */
    MessageHeader & SetExchangeID(uint16_t id)
    {
        mExchangeID = id;
        return *this;
    }

    /** Set the Protocol ID for this header. */
    MessageHeader & SetProtcolID(uint16_t id)
    {
        mProtocolID = id;
        return *this;
    }

    /** Set the message auth tag for this header. */
    MessageHeader & SetTag(EncryptionType encType, uint8_t * tag, size_t len)
    {
        const size_t tagLen = TagLenForEncryptionType(encType);
        if (tagLen > 0 && tagLen <= kMaxTagLen && len == tagLen)
        {
            mEncryptionType = encType;
            memcpy(&mTag, tag, tagLen);
        }

        return *this;
    }

    /**
     * A call to `Encode` will require at least this many bytes on the current
     * object to be successful.
     *
     * @return the number of bytes needed in a buffer to be able to Encode.
     */
    size_t EncodeSizeBytes() const;

    /**
     * A call to `Encode` will require at least this many bytes on the current
     * object to be successful.
     *
     * @return the number of bytes needed in a buffer to be able to Encode.
     */
    size_t EncryptedHeaderSizeBytes() const;

    /**
     * Decodes a header from the given buffer.
     *
     * @param data - the buffer to read from
     * @param size - bytes available in the buffer
     * @param decode_size - number of bytes read from the buffer to decode the
     *                      object
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     *    CHIP_ERROR_VERSION_MISMATCH if header version is not supported.
     */
    CHIP_ERROR Decode(const uint8_t * data, size_t size, size_t * decode_size);

    /**
     * Encodes a header into the given buffer.
     *
     * @param data - the buffer to write to
     * @param size - space available in the buffer (in bytes)
     * @param encode_size - number of bytes written to the buffer.
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     */
    CHIP_ERROR Encode(uint8_t * data, size_t size, size_t * encode_size) const;

    /**
     * Decodes the encrypted header fields from the given buffer.
     *
     * @param data - the buffer to read from
     * @param size - bytes available in the buffer
     * @param decode_size - number of bytes read from the buffer to decode the
     *                      object
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     *    CHIP_ERROR_VERSION_MISMATCH if header version is not supported.
     */
    CHIP_ERROR DecodeEncryptedHeader(const uint8_t * data, size_t size, size_t * decode_size);

    /**
     * Decodes the Message Authentication Tag from the given buffer.
     *
     * @param data - the buffer to read from
     * @param size - bytes available in the buffer
     * @param decode_size - number of bytes read from the buffer to decode the
     *                      object
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     *    CHIP_ERROR_VERSION_MISMATCH if header version is not supported.
     */
    CHIP_ERROR DecodeMACTag(const uint8_t * data, size_t size, size_t * decode_size);

    /**
     * Encodes the encrypted part of the header into the given buffer.
     *
     * @param data - the buffer to write to
     * @param size - space available in the buffer (in bytes)
     * @param encode_size - number of bytes written to the buffer.
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     */
    CHIP_ERROR EncodeEncryptedHeader(uint8_t * data, size_t size, size_t * encode_size) const;

    /**
     * Encodes the Messae Authentication Tag into the given buffer.
     *
     * @param data - the buffer to write to
     * @param size - space available in the buffer (in bytes)
     * @param encode_size - number of bytes written to the buffer.
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     */
    CHIP_ERROR EncodeMACTag(uint8_t * data, size_t size, size_t * encode_size) const;

private:
    size_t TagLenForEncryptionType(EncryptionType encType) const;

    /// Represents the current encode/decode header version
    static constexpr int kHeaderVersion = 2;

    /// Represents encryption type used for encrypting current packet
    EncryptionType mEncryptionType = EncryptionType::kAESCCMTagLen8;

    /// Value expected to be incremented for each message sent.
    uint32_t mMessageId = 0;

    /// What node the message originated from
    Optional<NodeId> mSourceNodeId;

    /// Intended recipient of the message.
    Optional<NodeId> mDestinationNodeId;

    /// Encryption Key ID
    uint16_t mEncryptionKeyID = 0;

    /// Length of application data (payload)
    uint16_t mPayloadLength = 0;

    /// Header structure for exchange information
    uint8_t mExchangeHeader = 0;

    /// Packet type (application data, security control packets, e.g. pairing,
    /// configuration, rekey etc)
    uint8_t mMessageType = 0;

    /// Security session identifier
    uint16_t mExchangeID = 0;

    /// Vendor identifier
    Optional<uint16_t> mVendorId;

    /// Protocol identifier
    uint16_t mProtocolID = 0;

    /// Message authentication tag generated at encryption of the message.
    uint8_t mTag[kMaxTagLen];

    /// Message header read from the message.
    uint16_t mHeader = 0;
};

} // namespace chip

#endif // MESSAGEHEADER_H_
