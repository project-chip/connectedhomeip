/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <type_traits>

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TypeTraits.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

namespace chip {

static constexpr size_t kMaxTagLen = 16;

static constexpr size_t kMaxAppMessageLen = 1200;

typedef int PacketHeaderFlags;

namespace Header {

enum class EncryptionType
{
    kEncryptionTypeNone = 0,
    kAESCCMTagLen16     = 1,
};

/**
 *  @brief
 *    The CHIP Exchange header flag bits.
 */
enum class ExFlagValues : uint8_t
{
    /// Set when current message is sent by the initiator of an exchange.
    kExchangeFlag_Initiator = 0x01,

    /// Set when current message is an acknowledgment for a previously received message.
    kExchangeFlag_AckMsg = 0x02,

    /// Set when current message is requesting an acknowledgment from the recipient.
    kExchangeFlag_NeedsAck = 0x04,

    /// Set when a vendor id is prepended to the Message Protocol Id field.
    kExchangeFlag_VendorIdPresent = 0x10,
};

enum class FlagValues : uint16_t
{
    /// Header flag specifying that a destination node id is included in the header.
    kDestinationNodeIdPresent = 0x0001,

    /// Header flag specifying that a destination group id is included in the header.
    kDestinationGroupIdPresent = 0x0002,

    /// Header flag specifying that a source node id is included in the header.
    kSourceNodeIdPresent = 0x0004,

    /// Header flag specifying that it is a control message for secure session.
    kSecureSessionControlMessage = 0x4000,

    /// Header flag specifying that it is a encrypted message.
    kEncryptedMessage = 0x0100,

};

using Flags   = BitFlags<FlagValues>;
using ExFlags = BitFlags<ExFlagValues>;

// Header is a 16-bit value of the form
//  |  4 bit  | 4 bit |8 bit Security Flags|
//  +---------+-------+--------------------|
//  | version | Flags | P | C |Reserved| E |
//                      |   |            +---Encrypted
//                      |   +----------------Control message (TODO: Implement this)
//                      +--------------------Privacy enhancements (TODO: Implement this)

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
     * A version of Decode that uses the type system to determine available
     * space.
     */
    template <uint16_t N>
    inline CHIP_ERROR Decode(const uint8_t (&data)[N], uint16_t * decode_size)
    {
        return Decode(data, N, decode_size);
    }

    /**
     * A version of Decode that decodes from the start of a PacketBuffer and
     * consumes the bytes we decoded from.
     */
    CHIP_ERROR DecodeAndConsume(const System::PacketBufferHandle & buf);

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
    CHIP_ERROR Encode(uint8_t * data, uint16_t size, uint16_t * encode_size) const;

    /**
     * A version of Encode that uses the type system to determine available
     * space.
     */
    template <int N>
    inline CHIP_ERROR Encode(uint8_t (&data)[N], uint16_t * encode_size) const
    {
        return Encode(data, N, encode_size);
    }

    /**
     * A version of Encode that encodes into a PacketBuffer before the
     * PacketBuffer's current data.
     */
    CHIP_ERROR EncodeBeforeData(const System::PacketBufferHandle & buf) const;

    /**
     * A version of Encode that encodes into a PacketBuffer at the start of the
     * current data space.  This assumes that someone has already preallocated
     * space for the header.
     */
    inline CHIP_ERROR EncodeAtStart(const System::PacketBufferHandle & buf, uint16_t * encode_size) const
    {
        return Encode(buf->Start(), buf->DataLength(), encode_size);
    }

private:
    /// Represents the current encode/decode header version
    static constexpr int kHeaderVersion = 2;

    /// Value expected to be incremented for each message sent.
    uint32_t mMessageId = 0;

    /// What node the message originated from
    Optional<NodeId> mSourceNodeId;

    /// Intended recipient of the message.
    Optional<NodeId> mDestinationNodeId;

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
    constexpr PayloadHeader() { SetProtocol(Protocols::NotSpecified); }
    PayloadHeader & operator=(const PayloadHeader &) = default;

    /** Get the Session ID from this header. */
    uint16_t GetExchangeID() const { return mExchangeID; }

    /** Get the Protocol ID from this header. */
    Protocols::Id GetProtocolID() const { return mProtocolID; }

    /** Check whether the header has a given protocol */
    bool HasProtocol(Protocols::Id protocol) const { return mProtocolID == protocol; }

    /** Get the secure msg type from this header. */
    uint8_t GetMessageType() const { return mMessageType; }

    /** Check whether the header has a given secure message type */
    bool HasMessageType(uint8_t type) const { return mMessageType == type; }
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    bool HasMessageType(MessageType type) const
    {
        return HasProtocol(Protocols::MessageTypeTraits<MessageType>::ProtocolId()) && HasMessageType(to_underlying(type));
    }

    /**
     * Gets the Acknowledged Message Counter from this header.
     *
     * NOTE: the Acknowledged Message Counter is optional and may be missing.
     */
    const Optional<uint32_t> & GetAckId() const { return mAckId; }

    /**
     * Set the message type for this header.  This requires setting the protocol
     * id as well, because the meaning of a message type is only relevant given
     * a specific protocol.
     *
     * This should only be used for cases when we don't have a strongly typed
     * message type and hence can't automatically determine the protocol from
     * the message type.
     */
    PayloadHeader & SetMessageType(Protocols::Id protocol, uint8_t type)
    {
        SetProtocol(protocol);
        mMessageType = type;
        return *this;
    }

    /** Set the secure message type, with the protocol id derived from the
        message type. */
    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    PayloadHeader & SetMessageType(MessageType type)
    {
        SetMessageType(Protocols::MessageTypeTraits<MessageType>::ProtocolId(), to_underlying(type));
        return *this;
    }

    /** Set the security session ID for this header. */
    PayloadHeader & SetExchangeID(uint16_t id)
    {
        mExchangeID = id;
        return *this;
    }

    /** Set the Initiator flag bit. */
    PayloadHeader & SetInitiator(bool inInitiator)
    {
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_Initiator, inInitiator);
        return *this;
    }

    PayloadHeader & SetAckId(uint32_t id)
    {
        mAckId.SetValue(id);
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_AckMsg);
        return *this;
    }

    /** Set the AckMsg flag bit. */
    PayloadHeader & SetAckId(Optional<uint32_t> id)
    {
        mAckId = id;
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_AckMsg, id.HasValue());
        return *this;
    }

    /** Set the NeedsAck flag bit. */
    PayloadHeader & SetNeedsAck(bool inNeedsAck)
    {
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_NeedsAck, inNeedsAck);
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
     *  Determine whether the current message is an acknowledgment for a previously received message.
     *
     *  @return Returns 'true' if current message is an acknowledgment, else 'false'.
     *
     */
    bool IsAckMsg() const { return mExchangeFlags.Has(Header::ExFlagValues::kExchangeFlag_AckMsg); }

    /**
     *  Determine whether current message is requesting an acknowledgment from the recipient.
     *
     *  @return Returns 'true' if the current message is requesting an acknowledgment from the recipient, else 'false'.
     *
     */
    bool NeedsAck() const { return mExchangeFlags.Has(Header::ExFlagValues::kExchangeFlag_NeedsAck); }

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
     * A version of Decode that uses the type system to determine available
     * space.
     */
    template <uint16_t N>
    inline CHIP_ERROR Decode(const uint8_t (&data)[N], uint16_t * decode_size)
    {
        return Decode(data, N, decode_size);
    }

    /**
     * A version of Decode that decodes from the start of a PacketBuffer and
     * consumes the bytes we decoded from.
     */
    CHIP_ERROR DecodeAndConsume(const System::PacketBufferHandle & buf);

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

    /**
     * A version of Encode that uses the type system to determine available
     * space.
     */
    template <uint16_t N>
    inline CHIP_ERROR Encode(uint8_t (&data)[N], uint16_t * decode_size) const
    {
        return Encode(data, N, decode_size);
    }

    /**
     * A version of Encode that encodes into a PacketBuffer before the
     * PacketBuffer's current data.
     */
    CHIP_ERROR EncodeBeforeData(const System::PacketBufferHandle & buf) const;

    /**
     * A version of Encode that encodes into a PacketBuffer at the start of the
     * current data space.  This assumes that someone has already preallocated
     * space for the header.
     */
    inline CHIP_ERROR EncodeAtStart(const System::PacketBufferHandle & buf, uint16_t * encode_size) const
    {
        return Encode(buf->Start(), buf->DataLength(), encode_size);
    }

private:
    constexpr void SetProtocol(Protocols::Id protocol)
    {
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_VendorIdPresent, protocol.GetVendorId() != VendorId::Common);
        mProtocolID = protocol;
    }

    constexpr bool HaveVendorId() const { return mExchangeFlags.Has(Header::ExFlagValues::kExchangeFlag_VendorIdPresent); }

    /// Packet type (application data, security control packets, e.g. pairing,
    /// configuration, rekey etc)
    uint8_t mMessageType = 0;

    /// Security session identifier
    uint16_t mExchangeID = 0;

    /// Protocol identifier
    Protocols::Id mProtocolID = Protocols::NotSpecified;

    /// Bit flag indicators for CHIP Exchange header
    Header::ExFlags mExchangeFlags;

    /// Message counter of a previous message that is being acknowledged by the current message
    Optional<uint32_t> mAckId;
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
