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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/GroupId.h>
#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BufferReader.h>
#include <lib/support/TypeTraits.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

namespace chip {

static constexpr size_t kMaxTagLen = 16;

static constexpr size_t kMaxAppMessageLen = 1200;

static constexpr uint16_t kMsgUnicastSessionIdUnsecured = 0x0000;

typedef int PacketHeaderFlags;

namespace Header {

enum class SessionType : uint8_t
{
    kUnicastSession = 0,
    kGroupSession   = 1,
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

    /// Secured Extension block is present.
    kExchangeFlag_SecuredExtension = 0x08,

    /// Set when a vendor id is prepended to the Message Protocol Id field.
    kExchangeFlag_VendorIdPresent = 0x10,
};

// Message flags 8-bit value of the form
//  |  4 bits | 1 | 1 | 2 bits |
//  +---------+-------+--------|
//  | version | - | S | DSIZ
//                  |   |
//                  |   +---------------- Destination Id field
//                  +-------------------- Source node Id present

enum class MsgFlagValues : uint8_t
{
    /// Header flag specifying that a source node id is included in the header.
    kSourceNodeIdPresent       = 0b00000100,
    kDestinationNodeIdPresent  = 0b00000001,
    kDestinationGroupIdPresent = 0b00000010,
    kDSIZReserved              = 0b00000011,

};

// Security flags 8-bit value of the form
//  | 1 | 1 | 1  | 3 | 2 bits |
//  +------------+---+--------|
//  | P | C | MX | - | SessionType
//
// With :
// P  = Privacy flag
// C  = Control Msg flag
// MX = Message Extension

enum class SecFlagValues : uint8_t
{
    kPrivacyFlag      = 0b10000000,
    kControlMsgFlag   = 0b01000000,
    kMsgExtensionFlag = 0b00100000,
};

enum SecFlagMask
{
    kSessionTypeMask = 0b00000011, ///< Mask to extract sessionType
};

using MsgFlags = BitFlags<MsgFlagValues>;
using SecFlags = BitFlags<SecFlagValues>;

using ExFlags = BitFlags<ExFlagValues>;

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
    enum
    {
        kHeaderMinLength        = 8,
        kPrivacyHeaderMinLength = 4,
        kPrivacyHeaderOffset    = 4,
    };

    /**
     * Gets the message counter set in the header.
     *
     * Message IDs are expecte to monotonically increase by one for each mesage
     * that has been sent.
     */
    uint32_t GetMessageCounter() const { return mMessageCounter; }

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
     * Gets the destination group id in the current message.
     *
     * NOTE: the destination group id is optional and may be missing.
     */
    const Optional<GroupId> & GetDestinationGroupId() const { return mDestinationGroupId; }

    uint16_t GetSessionId() const { return mSessionId; }
    Header::SessionType GetSessionType() const { return mSessionType; }

    uint8_t GetMessageFlags() const { return mMsgFlags.Raw(); }

    uint8_t GetSecurityFlags() const { return mSecFlags.Raw(); }

    bool HasPrivacyFlag() const { return mSecFlags.Has(Header::SecFlagValues::kPrivacyFlag); }

    bool HasSourceNodeId() const { return mMsgFlags.Has(Header::MsgFlagValues::kSourceNodeIdPresent); }
    bool HasDestinationNodeId() const { return mMsgFlags.Has(Header::MsgFlagValues::kDestinationNodeIdPresent); }
    bool HasDestinationGroupId() const { return mMsgFlags.Has(Header::MsgFlagValues::kDestinationGroupIdPresent); }

    void SetFlags(Header::SecFlagValues value) { mSecFlags.Set(value); }
    void SetFlags(Header::MsgFlagValues value) { mMsgFlags.Set(value); }

    void SetMessageFlags(uint8_t flags) { mMsgFlags.SetRaw(flags); }

    void SetSecurityFlags(uint8_t securityFlags)
    {
        mSecFlags.SetRaw(securityFlags);
        mSessionType = static_cast<Header::SessionType>(securityFlags & Header::SecFlagMask::kSessionTypeMask);
    }

    bool IsGroupSession() const { return mSessionType == Header::SessionType::kGroupSession; }
    bool IsUnicastSession() const { return mSessionType == Header::SessionType::kUnicastSession; }

    bool IsSessionTypeValid() const
    {
        switch (mSessionType)
        {
        case Header::SessionType::kUnicastSession:
            return true;
        case Header::SessionType::kGroupSession:
            return true;
        default:
            return false;
        }
    }

    bool IsValidGroupMsg() const
    {
        // Check is based on spec 4.11.2
        return (IsGroupSession() && HasSourceNodeId() && HasDestinationGroupId() && !IsSecureSessionControlMsg());
    }

    bool IsValidMCSPMsg() const
    {
        // Check is based on spec 4.9.2.4
        return (IsGroupSession() && HasSourceNodeId() && HasDestinationNodeId() && IsSecureSessionControlMsg());
    }

    bool IsEncrypted() const { return !((mSessionId == kMsgUnicastSessionIdUnsecured) && IsUnicastSession()); }

    uint16_t MICTagLength() const { return (IsEncrypted()) ? chip::Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES : 0; }

    /** Check if it's a secure session control message. */
    bool IsSecureSessionControlMsg() const { return mSecFlags.Has(Header::SecFlagValues::kControlMsgFlag); }

    PacketHeader & SetSecureSessionControlMsg(bool value)
    {
        mSecFlags.Set(Header::SecFlagValues::kControlMsgFlag, value);
        return *this;
    }

    PacketHeader & SetSourceNodeId(NodeId id)
    {
        mSourceNodeId.SetValue(id);
        mMsgFlags.Set(Header::MsgFlagValues::kSourceNodeIdPresent);
        return *this;
    }

    PacketHeader & SetSourceNodeId(Optional<NodeId> id)
    {
        mSourceNodeId = id;
        mMsgFlags.Set(Header::MsgFlagValues::kSourceNodeIdPresent, id.HasValue());
        return *this;
    }

    PacketHeader & ClearSourceNodeId()
    {
        mSourceNodeId.ClearValue();
        mMsgFlags.Clear(Header::MsgFlagValues::kSourceNodeIdPresent);
        return *this;
    }

    PacketHeader & SetDestinationNodeId(NodeId id)
    {
        mDestinationNodeId.SetValue(id);
        mMsgFlags.Set(Header::MsgFlagValues::kDestinationNodeIdPresent);
        return *this;
    }

    PacketHeader & SetDestinationNodeId(Optional<NodeId> id)
    {
        mDestinationNodeId = id;
        mMsgFlags.Set(Header::MsgFlagValues::kDestinationNodeIdPresent, id.HasValue());
        return *this;
    }

    PacketHeader & ClearDestinationNodeId()
    {
        mDestinationNodeId.ClearValue();
        mMsgFlags.Clear(Header::MsgFlagValues::kDestinationNodeIdPresent);
        return *this;
    }

    PacketHeader & SetDestinationGroupId(GroupId id)
    {
        mDestinationGroupId.SetValue(id);
        mMsgFlags.Set(Header::MsgFlagValues::kDestinationGroupIdPresent);
        return *this;
    }

    PacketHeader & SetDestinationGroupId(Optional<GroupId> id)
    {
        mDestinationGroupId = id;
        mMsgFlags.Set(Header::MsgFlagValues::kDestinationGroupIdPresent, id.HasValue());
        return *this;
    }

    PacketHeader & ClearDestinationGroupId()
    {
        mDestinationGroupId.ClearValue();
        mMsgFlags.Clear(Header::MsgFlagValues::kDestinationGroupIdPresent);
        return *this;
    }

    PacketHeader & SetSessionType(Header::SessionType type)
    {
        mSessionType     = type;
        uint8_t typeMask = to_underlying(Header::kSessionTypeMask);
        mSecFlags.SetRaw(static_cast<uint8_t>((mSecFlags.Raw() & ~typeMask) | (to_underlying(type) & typeMask)));
        return *this;
    }

    PacketHeader & SetSessionId(uint16_t id)
    {
        mSessionId = id;
        return *this;
    }

    PacketHeader & SetMessageCounter(uint32_t id)
    {
        mMessageCounter = id;
        return *this;
    }

    PacketHeader & SetUnsecured()
    {
        mSessionId   = kMsgUnicastSessionIdUnsecured;
        mSessionType = Header::SessionType::kUnicastSession;
        return *this;
    }

    /**
     * Returns a pointer to the start of the privacy header
     * given a pointer to the start of the message.
     */
    uint8_t * PrivacyHeader(uint8_t * msgBuf) const { return msgBuf + PacketHeader::kPrivacyHeaderOffset; }

    size_t PrivacyHeaderLength() const
    {
        size_t length = kPrivacyHeaderMinLength;
        if (mMsgFlags.Has(Header::MsgFlagValues::kSourceNodeIdPresent))
        {
            length += sizeof(NodeId);
        }
        if (mMsgFlags.Has(Header::MsgFlagValues::kDestinationNodeIdPresent))
        {
            length += sizeof(NodeId);
        }
        else if (mMsgFlags.Has(Header::MsgFlagValues::kDestinationGroupIdPresent))
        {
            length += sizeof(GroupId);
        }
        return length;
    }

    size_t PayloadOffset() const
    {
        size_t offset = kPrivacyHeaderMinLength;
        offset += PrivacyHeaderLength();
        return offset;
    }

    /**
     * A call to `Encode` will require at least this many bytes on the current
     * object to be successful.
     *
     * @return the number of bytes needed in a buffer to be able to Encode.
     */
    uint16_t EncodeSizeBytes() const;

    /**
     * Decodes the fixed portion of the header fields from the given buffer.
     * The fixed header includes: message flags, session id, and security flags.
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     *    CHIP_ERROR_VERSION_MISMATCH if header version is not supported.
     */
    CHIP_ERROR DecodeFixed(const System::PacketBufferHandle & buf);

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
    /**
     * Decodes the fixed portion of the header fields from the stream reader.
     * The fixed header includes: message flags, session id, and security flags.
     *
     * @return CHIP_NO_ERROR on success.
     *
     * Possible failures:
     *    CHIP_ERROR_INVALID_ARGUMENT on insufficient buffer size
     *    CHIP_ERROR_VERSION_MISMATCH if header version is not supported.
     */
    CHIP_ERROR DecodeFixedCommon(Encoding::LittleEndian::Reader & reader);

    /// Represents the current encode/decode header version (4 bits)
    static constexpr uint8_t kMsgHeaderVersion = 0x00;

    /// Value expected to be incremented for each message sent.
    uint32_t mMessageCounter = 0;

    /// What node the message originated from
    Optional<NodeId> mSourceNodeId;

    /// Intended recipient of the message.
    Optional<NodeId> mDestinationNodeId;
    Optional<GroupId> mDestinationGroupId;

    /// Session ID
    uint16_t mSessionId = kMsgUnicastSessionIdUnsecured;

    Header::SessionType mSessionType = Header::SessionType::kUnicastSession;

    /// Flags read from the message.
    Header::MsgFlags mMsgFlags;
    Header::SecFlags mSecFlags;
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
    constexpr PayloadHeader(const PayloadHeader &) = default;
    PayloadHeader & operator=(const PayloadHeader &) = default;

    /** Get the Session ID from this header. */
    uint16_t GetExchangeID() const { return mExchangeID; }

    /** Get the Protocol ID from this header. */
    Protocols::Id GetProtocolID() const { return mProtocolID; }

    /** Check whether the header has a given protocol */
    bool HasProtocol(Protocols::Id protocol) const { return mProtocolID == protocol; }

    /** Get the secure msg type from this header. */
    uint8_t GetMessageType() const { return mMessageType; }

    /** Get the raw exchange flags from this header. */
    uint8_t GetExhangeFlags() const { return mExchangeFlags.Raw(); }

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
    const Optional<uint32_t> & GetAckMessageCounter() const { return mAckMessageCounter; }

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

    PayloadHeader & SetAckMessageCounter(uint32_t id)
    {
        mAckMessageCounter.SetValue(id);
        mExchangeFlags.Set(Header::ExFlagValues::kExchangeFlag_AckMsg);
        return *this;
    }

    /** Set the AckMsg flag bit. */
    PayloadHeader & SetAckMessageCounter(Optional<uint32_t> id)
    {
        mAckMessageCounter = id;
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
    Optional<uint32_t> mAckMessageCounter;
};

/** Handles encoding/decoding of CHIP message headers */
class MessageAuthenticationCode
{
public:
    const uint8_t * GetTag() const { return &mTag[0]; }

    /** Set the message auth tag for this header. */
    MessageAuthenticationCode & SetTag(PacketHeader * header, const uint8_t * tag, size_t len)
    {
        const size_t tagLen = chip::Crypto::CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES;
        if (tagLen > 0 && tagLen <= kMaxTagLen && len == tagLen)
        {
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

private:
    /// Message authentication tag generated at encryption of the message.
    uint8_t mTag[kMaxTagLen];
};

} // namespace chip
