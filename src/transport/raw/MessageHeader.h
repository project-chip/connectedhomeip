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
 * This file defines CHIP binary header encode/decode.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string.h>

#include <core/CHIPError.h>
#include <core/Optional.h>
#include <support/BitFlags.h>

namespace chip {

/// Convenience type to make it clear a number represents a node id.
typedef uint64_t NodeId;

static constexpr NodeId kUndefinedNodeId = 0ULL;
static constexpr NodeId kAnyNodeId       = 0xFFFFFFFFFFFFFFFFULL;
static constexpr size_t kMaxTagLen       = 16;

typedef int PacketHeaderFlags;

namespace Header {

enum class EncryptionType
{
    kAESCCMTagLen8  = 0,
    kAESCCMTagLen12 = 1,
    kAESCCMTagLen16 = 2,
};

/**
 *  @brief
 *    The CHIP Exchange header flag bits.
 */
enum class ExFlagValues : uint8_t
{
    /// Set when current message is sent by the initiator of an exchange.
    kExchangeFlag_Initiator = 0x01,
};

enum class FlagValues : uint16_t
{
    /// Header flag specifying that a destination node id is included in the header.
    kDestinationNodeIdPresent = 0x0100,

    /// Header flag specifying that a source node id is included in the header.
    kSourceNodeIdPresent = 0x0200,

    /// Header flag specifying that a source vendor id is included in the header.
    kVendorIdPresent = 0x0400,

    /// Header flag specifying that it is a control message for secure session.
    kSecureSessionControlMessage = 0x0800,

    /// Header flag specifying that it is a encrypted message.
    kSecure = 0x0001,

};

using Flags   = BitFlags<uint16_t, FlagValues>;
using ExFlags = BitFlags<uint8_t, ExFlagValues>;

// Header is a 16-bit value of the form
//  |  4 bit  | 4 bit |8 bit Security Flags|
//  +---------+-------+--------------------|
//  | version | Flags | P | C |Reserved| E |
//                      |   |            +---Encrypted
//                      |   +----------------Control message (TODO: Implement this)
//                      +--------------------Privacy enhancements (TODO: Implement this)
static constexpr uint16_t kFlagsMask = 0x0F01;

} // namespace Header

/**
 * Handles encoding/decoding of CHIP packet message headers.
 *
 * Packet headers are **UNENCRYPTED** and are placed at the start of
 * a message buffer.
 */
class PacketHeader
{
public:
    /**
     * Gets the message id set in the header.
     *
     * Message IDs are expecte to monotonically increase by one for each mesage
     * that has been sent.
     */
    uint32_t GetMessageId() const { return mMessageId; }

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

    uint16_t GetEncryptionKeyID() const { return mEncryptionKeyID; }

    /** Get the length of encrypted payload. */
    uint16_t GetPayloadLength() const { return mPayloadLength; }

    Header::Flags & GetFlags() { return mFlags; }
    const Header::Flags & GetFlags() const { return mFlags; }

    /** Check if it's a secure session control message. */
    bool IsSecureSessionControlMsg() const { return mFlags.Has(Header::FlagValues::kSecureSessionControlMessage); }

    Header::EncryptionType GetEncryptionType() const { return mEncryptionType; }

    PacketHeader & SetSecureSessionControlMsg(bool value)
    {
        mFlags.Set(Header::FlagValues::kSecureSessionControlMessage, value);
        return *this;
    }

    PacketHeader & SetSourceNodeId(NodeId id)
    {
        mSourceNodeId.SetValue(id);
        mFlags.Set(Header::FlagValues::kSourceNodeIdPresent);
        return *this;
    }

    PacketHeader & SetSourceNodeId(Optional<NodeId> id)
    {
        mSourceNodeId = id;
        mFlags.Set(Header::FlagValues::kSourceNodeIdPresent, id.HasValue());
        return *this;
    }

    PacketHeader & ClearSourceNodeId()
    {
        mSourceNodeId.ClearValue();
        mFlags.Clear(Header::FlagValues::kSourceNodeIdPresent);
        return *this;
    }

    /** Set the secure payload length for this header. */
    PacketHeader & SetPayloadLength(uint16_t len)
    {
        mPayloadLength = len;
        return *this;
    }

    PacketHeader & SetDestinationNodeId(NodeId id)
    {
        mDestinationNodeId.SetValue(id);
        mFlags.Set(Header::FlagValues::kDestinationNodeIdPresent);
        return *this;
    }

    PacketHeader & SetDestinationNodeId(Optional<NodeId> id)
    {
        mDestinationNodeId = id;
        mFlags.Set(Header::FlagValues::kDestinationNodeIdPresent, id.HasValue());
        return *this;
    }

    PacketHeader & ClearDestinationNodeId()
    {
        mDestinationNodeId.ClearValue();
        mFlags.Clear(Header::FlagValues::kDestinationNodeIdPresent);
        return *this;
    }

    PacketHeader & SetEncryptionKeyID(uint16_t id)
    {
        mEncryptionKeyID = id;
        return *this;
    }

    PacketHeader & SetMessageId(uint32_t id)
    {
        mMessageId = id;
        return *this;
    }

    PacketHeader & SetEncryptionType(Header::EncryptionType type)
    {
        mEncryptionType = type;
        return *this;
    }

    /**
     * A call to `Encode` will require at least this many bytes on the current
     * object to be successful.
     *
     * @return the number of bytes needed in a buffer to be able to Encode.
     */
    uint16_t EncodeSizeBytes() const;

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
    CHIP_ERROR Decode(const uint8_t * data, uint16_t size, uint16_t * decode_size);

    /**
     * Encodes a header into the given buffer.
     *
     * @param data - the buffer to write to
     * @param size - space available in the buffer (in bytes)
     * @param encode_size - number of bytes written to the buffer.
     * @param payloadFlags extra flags for packet header encoding
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     */
    CHIP_ERROR Encode(uint8_t * data, uint16_t size, uint16_t * encode_size, Header::Flags payloadFlags) const;

private:
    /// Represents the current encode/decode header version
    static constexpr int kHeaderVersion = 2;

    /// Value expected to be incremented for each message sent.
    uint32_t mMessageId = 0;

    /// What node the message originated from
    Optional<NodeId> mSourceNodeId;

    /// Intended recipient of the message.
    Optional<NodeId> mDestinationNodeId;

    /// Length of application data (payload)
    uint16_t mPayloadLength = 0;

    /// Encryption Key ID
    uint16_t mEncryptionKeyID = 0;

    /// Message flags read from the message.
    Header::Flags mFlags;

    /// Represents encryption type used for encrypting current packet
    Header::EncryptionType mEncryptionType = Header::EncryptionType::kAESCCMTagLen16;
};

/**
 * Handles encoding/decoding of CHIP payload headers.
 *
 * Payload headers are **ENCRYPTED** and are placed at the start of
 * an encrypted message payload.
 */
class PayloadHeader
{
public:
    /**
     * Gets the vendor id in the current message.
     *
     * NOTE: the vendor id is optional and may be missing.
     */
    const Optional<uint16_t> & GetVendorId() const { return mVendorId; }

    /** Get the Session ID from this header. */
    uint16_t GetExchangeID() const { return mExchangeID; }

    /** Get the Protocol ID from this header. */
    uint16_t GetProtocolID() const { return mProtocolID; }

    /** Get the secure msg type from this header. */
    uint8_t GetMessageType() const { return mMessageType; }

    /** Set the vendor id for this header. */
    PayloadHeader & SetVendorId(uint16_t id)
    {
        mVendorId.SetValue(id);

        return *this;
    }

    /** Set the vendor id for this header. */
    PayloadHeader & SetVendorId(Optional<uint16_t> id)
    {
        mVendorId = id;

        return *this;
    }

    /** Clear the vendor id for this header. */
    PayloadHeader & ClearVendorId()
    {
        mVendorId.ClearValue();

        return *this;
    }

    /** Set the secure message type for this header. */
    PayloadHeader & SetMessageType(uint8_t type)
    {
        mMessageType = type;
        return *this;
    }

    /** Set the security session ID for this header. */
    PayloadHeader & SetExchangeID(uint16_t id)
    {
        mExchangeID = id;
        return *this;
    }

    /** Set the Protocol ID for this header. */
    PayloadHeader & SetProtocolID(uint16_t id)
    {
        mProtocolID = id;
        return *this;
    }

    /** Set the Initiator flag bit. */
    PayloadHeader & SetInitiator(bool inInitiator)
    {
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_Initiator, inInitiator);
        return *this;
    }

    /**
     *  Determine whether the initiator of the exchange.
     *
     *  @return Returns 'true' if it is the initiator, else 'false'.
     *
     */
    bool IsInitiator() const { return mExchangeFlags.Has(Header::ExFlagValues::kExchangeFlag_Initiator); }

    /**
     * A call to `Encode` will require at least this many bytes on the current
     * object to be successful.
     *
     * @return the number of bytes needed in a buffer to be able to Encode.
     */
    uint16_t EncodeSizeBytes() const;

    /**
     * Decodes the encrypted header fields from the given buffer.
     *
     * @param flags - decode flags
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
    CHIP_ERROR Decode(Header::Flags flags, const uint8_t * data, uint16_t size, uint16_t * decode_size);

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
    CHIP_ERROR Encode(uint8_t * data, uint16_t size, uint16_t * encode_size) const;

    /** Flags required for encoding this payload. */
    Header::Flags GetEncodePacketFlags() const;

private:
    /// Packet type (application data, security control packets, e.g. pairing,
    /// configuration, rekey etc)
    uint8_t mMessageType = 0;

    /// Security session identifier
    uint16_t mExchangeID = 0;

    /// Vendor identifier
    Optional<uint16_t> mVendorId;

    /// Protocol identifier
    uint16_t mProtocolID = 0;

    /// Bit flag indicators for CHIP Exchange header
    Header::ExFlags mExchangeFlags;
};

/** Handles encoding/decoding of CHIP message headers */
class MessageAuthenticationCode
{
public:
    const uint8_t * GetTag() const { return &mTag[0]; }

    /** Set the message auth tag for this header. */
    MessageAuthenticationCode & SetTag(PacketHeader * header, Header::EncryptionType encType, uint8_t * tag, size_t len)
    {
        const size_t tagLen = TagLenForEncryptionType(encType);
        if (tagLen > 0 && tagLen <= kMaxTagLen && len == tagLen)
        {
            header->SetEncryptionType(encType);
            memcpy(&mTag, tag, tagLen);
        }

        return *this;
    }

    /**
     * Decodes the Message Authentication Tag from the given buffer.
     *
     * @param packetHeader - header containing encryption information
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
    CHIP_ERROR Decode(const PacketHeader & packetHeader, const uint8_t * data, uint16_t size, uint16_t * decode_size);

    /**
     * Encodes the Messae Authentication Tag into the given buffer.
     *
     * @param packetHeader - header containing encryption information
     * @param data - the buffer to write to
     * @param size - space available in the buffer (in bytes)
     * @param encode_size - number of bytes written to the buffer.
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     */
    CHIP_ERROR Encode(const PacketHeader & packetHeader, uint8_t * data, uint16_t size, uint16_t * encode_size) const;

    static uint16_t TagLenForEncryptionType(Header::EncryptionType encType);

private:
    /// Message authentication tag generated at encryption of the message.
    uint8_t mTag[kMaxTagLen];
};

} // namespace chip
