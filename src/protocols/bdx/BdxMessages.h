/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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
 *      This file defines structures and utility methods for working with BDX
 *      messages, mainly for writing to and reading from PacketBuffers.
 */

#pragma once

#include <lib/support/BitFlags.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace bdx {

constexpr uint16_t kMaxFileDesignatorLen = 0xFF;

constexpr const char * kProtocolName = "BDX";

enum class MessageType : uint8_t
{
    SendInit           = 0x01,
    SendAccept         = 0x02,
    ReceiveInit        = 0x04,
    ReceiveAccept      = 0x05,
    BlockQuery         = 0x10,
    Block              = 0x11,
    BlockEOF           = 0x12,
    BlockAck           = 0x13,
    BlockAckEOF        = 0x14,
    BlockQueryWithSkip = 0x15,
};

enum class StatusCode : uint16_t
{
    kLengthTooLarge             = 0x0012,
    kLengthTooShort             = 0x0013,
    kLengthMismatch             = 0x0014,
    kLengthRequired             = 0x0015,
    kBadMessageContents         = 0x0016,
    kBadBlockCounter            = 0x0017,
    kUnexpectedMessage          = 0x0018,
    kResponderBusy              = 0x0019,
    kTransferFailedUnknownError = 0x001F,
    kTransferMethodNotSupported = 0x0050,
    kFileDesignatorUnknown      = 0x0051,
    kStartOffsetNotSupported    = 0x0052,
    kVersionNotSupported        = 0x0053,
    kUnknown                    = 0x005F,
};

enum class TransferControlFlags : uint8_t
{
    // first 4 bits reserved for version
    kSenderDrive   = (1U << 4),
    kReceiverDrive = (1U << 5),
    kAsync         = (1U << 6),
};

enum class RangeControlFlags : uint8_t
{
    kDefLen      = (1U),
    kStartOffset = (1U << 1),
    kWiderange   = (1U << 4),
};

/**
 * @brief
 *   Interface for defining methods that apply to all BDX messages.
 */
struct BdxMessage
{
    /**
     * @brief
     *  Parse data from an PacketBuffer into a BdxMessage struct.
     *
     *  Note that this may store pointers that point into the passed PacketBuffer,
     *  so it is essential that the underlying PacketBuffer is not modified until after this
     *  struct is no longer needed.
     *
     * @param[in] aBuffer A PacketBufferHandle with a reference to the PacketBuffer containing the data.
     *
     * @return CHIP_ERROR Return an error if the message format is invalid and/or can't be parsed
     */
    CHECK_RETURN_VALUE
    virtual CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) = 0;

    /**
     * @brief
     *  Write the message fields to a buffer using the provided BufferWriter.
     *
     *  It is up to the caller to use BufferWriter::Fit() to verify that the write was
     *  successful. This method will also not check for correctness or completeness for
     *  any of the fields - it is the caller's responsibility to ensure that the fields
     *  align with BDX specifications.
     *
     * @param aBuffer A BufferWriter object that will be used to write the message
     */
    virtual Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const = 0;

    /**
     * @brief
     *  Returns the size of buffer needed to write the message.
     */
    virtual size_t MessageSize() const = 0;

#if CHIP_AUTOMATION_LOGGING
    /**
     * @brief
     * Log all parameters for this message.
     */
    virtual void LogMessage(bdx::MessageType messageType) const = 0;
#endif // CHIP_AUTOMATION_LOGGING

    virtual ~BdxMessage() = default;
};

/*
 * A structure for representing a SendInit or ReceiveInit message (both contain
 * identical parameters).
 */
struct TransferInit : public BdxMessage
{
    bool operator==(const TransferInit &) const;

    // Proposed Transfer Control (required)
    BitFlags<TransferControlFlags> TransferCtlOptions;
    uint8_t Version = 0; ///< The highest version supported by the sender

    // Range Control
    BitFlags<RangeControlFlags> mRangeCtlFlags;

    // All required
    uint16_t MaxBlockSize = 0; ///< Proposed max block size to use in transfer
    uint64_t StartOffset  = 0; ///< Proposed start offset of data. 0 for no offset
    uint64_t MaxLength    = 0; ///< Proposed max length of data in transfer, 0 for indefinite

    // File designator (required) and additional metadata (optional, TLV format)
    // WARNING: there is no guarantee at any point that these pointers will point to valid memory. The Buffer field should be used
    // to hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * FileDesignator = nullptr;
    uint16_t FileDesLength         = 0; ///< Length of file designator string (not including null-terminator)
    const uint8_t * Metadata       = nullptr;
    size_t MetadataLength          = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) override;
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const override;
    size_t MessageSize() const override;
#if CHIP_AUTOMATION_LOGGING
    void LogMessage(bdx::MessageType messageType) const override;
#endif // CHIP_AUTOMATION_LOGGING
};

using SendInit    = TransferInit;
using ReceiveInit = TransferInit;

/*
 * A structure for representing a SendAccept message.
 */
struct SendAccept : public BdxMessage
{
    bool operator==(const SendAccept &) const;

    // Transfer Control (required, only one should be set)
    BitFlags<TransferControlFlags> TransferCtlFlags;

    uint8_t Version       = 0; ///< The agreed upon version for the transfer (required)
    uint16_t MaxBlockSize = 0; ///< Chosen max block size to use in transfer (required)

    // Additional metadata (optional, TLV format)
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory. The Buffer field should be used to
    // hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * Metadata = nullptr;
    size_t MetadataLength    = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) override;
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const override;
    size_t MessageSize() const override;
#if CHIP_AUTOMATION_LOGGING
    void LogMessage(bdx::MessageType messageType) const override;
#endif // CHIP_AUTOMATION_LOGGING
};

/**
 * A structure for representing ReceiveAccept messages.
 */
struct ReceiveAccept : public BdxMessage
{
    bool operator==(const ReceiveAccept &) const;

    // Transfer Control (required, only one should be set)
    BitFlags<TransferControlFlags> TransferCtlFlags;

    // Range Control
    BitFlags<RangeControlFlags> mRangeCtlFlags;

    // All required
    uint8_t Version       = 0; ///< The agreed upon version for the transfer
    uint16_t MaxBlockSize = 0; ///< Chosen max block size to use in transfer
    uint64_t StartOffset  = 0; ///< Chosen start offset of data. 0 for no offset.
    uint64_t Length       = 0; ///< Length of transfer. 0 if length is indefinite.

    // Additional metadata (optional, TLV format)
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory. The Buffer field should be used to
    // hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * Metadata = nullptr;
    size_t MetadataLength    = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) override;
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const override;
    size_t MessageSize() const override;
#if CHIP_AUTOMATION_LOGGING
    void LogMessage(bdx::MessageType messageType) const override;
#endif // CHIP_AUTOMATION_LOGGING
};

/**
 * A struct for representing messages contiaining just a counter field. Can be used to
 * represent BlockQuery, BlockAck, and BlockAckEOF.
 */
struct CounterMessage : public BdxMessage
{
    bool operator==(const CounterMessage &) const;

    uint32_t BlockCounter = 0;

    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) override;
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const override;
    size_t MessageSize() const override;
#if CHIP_AUTOMATION_LOGGING
    void LogMessage(bdx::MessageType messageType) const override;
#endif // CHIP_AUTOMATION_LOGGING
};

using BlockQuery  = CounterMessage;
using BlockAck    = CounterMessage;
using BlockAckEOF = CounterMessage;

/**
 * A struct that represents a message containing actual data (Block, BlockEOF).
 */
struct DataBlock : public BdxMessage
{
    bool operator==(const DataBlock &) const;

    uint32_t BlockCounter = 0;

    // WARNING: there is no guarantee at any point that this pointer will point to valid memory. The Buffer field should be used to
    // hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * Data = nullptr;
    size_t DataLength    = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) override;
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const override;
    size_t MessageSize() const override;
#if CHIP_AUTOMATION_LOGGING
    void LogMessage(bdx::MessageType messageType) const override;
#endif // CHIP_AUTOMATION_LOGGING
};

using Block    = DataBlock;
using BlockEOF = DataBlock;

struct BlockQueryWithSkip : public BdxMessage
{
    bool operator==(const BlockQueryWithSkip &) const;

    uint32_t BlockCounter = 0;
    uint64_t BytesToSkip  = 0;

    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) override;
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const override;
    size_t MessageSize() const override;
#if CHIP_AUTOMATION_LOGGING
    void LogMessage(bdx::MessageType messageType) const override;
#endif // CHIP_AUTOMATION_LOGGING
};

} // namespace bdx

namespace Protocols {
template <>
struct MessageTypeTraits<bdx::MessageType>
{
    static constexpr const Protocols::Id & ProtocolId() { return BDX::Id; }

    static auto GetTypeToNameTable()
    {
        static const std::array<MessageTypeNameLookup, 10> typeToNameTable = {
            {
                { bdx::MessageType::SendInit, "SendInit" },
                { bdx::MessageType::SendAccept, "SendAccept" },
                { bdx::MessageType::ReceiveInit, "ReceiveInit" },
                { bdx::MessageType::ReceiveAccept, "ReceiveAccept" },
                { bdx::MessageType::BlockQuery, "BlockQuery" },
                { bdx::MessageType::Block, "Block" },
                { bdx::MessageType::BlockEOF, "BlockEOF" },
                { bdx::MessageType::BlockAck, "BlockAck" },
                { bdx::MessageType::BlockAckEOF, "BlockAckEOF" },
                { bdx::MessageType::BlockQueryWithSkip, "BlockQueryWithSkip" },
            },
        };

        return &typeToNameTable;
    }
};
} // namespace Protocols

} // namespace chip
