/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <support/BitFlags.h>
#include <support/BufBound.h>
#include <support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>
namespace chip {
namespace bdx {

enum MessageType : uint8_t
{
    kBdxMsg_SendInit      = 0x01,
    kBdxMsg_SendAccept    = 0x02,
    kBdxMsg_ReceiveInit   = 0x04,
    kBdxMsg_ReceiveAccept = 0x05,
    kBdxMsg_BlockQuery    = 0x10,
    kBdxMsg_Block         = 0x11,
    kBdxMsg_BlockEOF      = 0x12,
    kBdxMsg_BlockAck      = 0x13,
    kBdxMsg_BlockAckEOF   = 0x14,
};

enum StatusCode : uint16_t
{
    kStatus_None                       = 0x0000,
    kStatus_Overflow                   = 0x0011,
    kStatus_LengthTooLarge             = 0x0012,
    kStatus_LengthTooShort             = 0x0013,
    kStatus_LengthMismatch             = 0x0014,
    kStatus_LengthRequired             = 0x0015,
    kStatus_BadMessageContents         = 0x0016,
    kStatus_BadBlockCounter            = 0x0017,
    kStatus_TransferFailedUnknownError = 0x001F,
    kStatus_ServerBadState             = 0x0020,
    kStatus_FailureToSend              = 0x0021,
    kStatus_TransferMethodNotSupported = 0x0050,
    kStatus_FileDesignatorUnknown      = 0x0051,
    kStatus_StartOffsetNotSupported    = 0x0052,
    kStatus_VersionNotSupported        = 0x0053,
    kStatus_Unknown                    = 0x005F,
};

enum TransferControlFlags : uint8_t
{
    // first 4 bits reserved for version
    kControl_SenderDrive   = (1U << 4),
    kControl_ReceiverDrive = (1U << 5),
    kControl_Async         = (1U << 6),
};

enum RangeControlFlags : uint8_t
{
    kRange_DefLen      = (1U),
    kRange_StartOffset = (1U << 1),
    kRange_Widerange   = (1U << 4),
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
     * @param[in] aBuffer A PacketBufferHandle with a refernce to the PacketBuffer containing the data.
     *
     * @return CHIP_ERROR Return an error if the message format is invalid and/or can't be parsed
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Parse(System::PacketBufferHandle aBuffer) { return DerivedParse(std::move(aBuffer)); }

    /**
     * @brief
     *  Write the message fields to a buffer using the provided BufBound.
     *
     *  It is up to the caller to use BufBound::Fit() to verify that the write was
     *  successful. This method will also not check for correctness or completeness for
     *  any of the fields - it is the caller's responsibility to ensure that the fields
     *  align with BDX specifications.
     *
     * @param aBuffer A BufBound object that will be used to write the message
     */
    BufBound & WriteToBuffer(BufBound & aBuffer) const { return DerivedWriteToBuffer(aBuffer); }

    /**
     * @brief
     *  Returns the size of buffer needed to write the message.
     */
    virtual size_t MessageSize() const { return DerivedMessageSize(); }

    virtual ~BdxMessage() = default;

private:
    virtual CHIP_ERROR DerivedParse(System::PacketBufferHandle aBuffer) = 0;
    virtual BufBound & DerivedWriteToBuffer(BufBound & aBuffer) const   = 0;
    virtual size_t DerivedMessageSize() const                           = 0;
};

/*
 * A structure for representing a SendInit or ReceiveInit message (both contain
 * identical parameters).
 */
struct TransferInit : public BdxMessage
{
    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const TransferInit &) const;

    // Proposed Transfer Control (required)
    BitFlags<uint8_t, TransferControlFlags> TransferCtlOptions;
    uint8_t Version = 0; ///< The highest version supported by the sender

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
    uint16_t MetadataLength        = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

private:
    CHIP_ERROR DerivedParse(System::PacketBufferHandle aBuffer) override;
    BufBound & DerivedWriteToBuffer(BufBound & aBuffer) const override;
    size_t DerivedMessageSize() const override;
};

using SendInit    = TransferInit;
using ReceiveInit = TransferInit;

/*
 * A structure for representing a SendAccept message.
 */
struct SendAccept : public BdxMessage
{
    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const SendAccept &) const;

    // Transfer Control (required, only one should be set)
    BitFlags<uint8_t, TransferControlFlags> TransferCtlFlags;

    uint8_t Version       = 0; ///< The agreed upon version for the transfer (required)
    uint16_t MaxBlockSize = 0; ///< Chosen max block size to use in transfer (required)

    // Additional metadata (optional, TLV format)
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory. The Buffer field should be used to
    // hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * Metadata = nullptr;
    uint16_t MetadataLength  = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

private:
    CHIP_ERROR DerivedParse(System::PacketBufferHandle aBuffer) override;
    BufBound & DerivedWriteToBuffer(BufBound & aBuffer) const override;
    size_t DerivedMessageSize() const override;
};

/**
 * A structure for representing ReceiveAccept messages.
 */
struct ReceiveAccept : public BdxMessage
{
    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const ReceiveAccept &) const;

    // Transfer Control (required, only one should be set)
    BitFlags<uint8_t, TransferControlFlags> TransferCtlFlags;

    // All required
    uint8_t Version       = 0; ///< The agreed upon version for the transfer
    uint16_t MaxBlockSize = 0; ///< Chosen max block size to use in transfer
    uint64_t StartOffset  = 0; ///< Chosen start offset of data. 0 for no offset.
    uint64_t Length       = 0; ///< Length of transfer. 0 if length is indefinite.

    // Additional metadata (optional, TLV format)
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory. The Buffer field should be used to
    // hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * Metadata = nullptr;
    uint16_t MetadataLength  = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

private:
    CHIP_ERROR DerivedParse(System::PacketBufferHandle aBuffer) override;
    BufBound & DerivedWriteToBuffer(BufBound & aBuffer) const override;
    size_t DerivedMessageSize() const override;
};

/**
 * A struct for representing messages contiaining just a counter field. Can be used to
 * represent BlockQuery, BlockAck, and BlockAckEOF.
 */
struct CounterMessage : public BdxMessage
{
    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const CounterMessage &) const;

    uint32_t BlockCounter = 0;

private:
    CHIP_ERROR DerivedParse(System::PacketBufferHandle aBuffer) override;
    BufBound & DerivedWriteToBuffer(BufBound & aBuffer) const override;
    size_t DerivedMessageSize() const override;
};

using BlockQuery  = CounterMessage;
using BlockAck    = CounterMessage;
using BlockAckEOF = CounterMessage;

/**
 * A struct that represents a message containing actual data (Block, BlockEOF).
 */
struct DataBlock : public BdxMessage
{
    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const DataBlock &) const;

    uint32_t BlockCounter = 0;

    // WARNING: there is no guarantee at any point that this pointer will point to valid memory. The Buffer field should be used to
    // hold a reference to the PacketBuffer containing the data in order to ensure the data is not freed.
    const uint8_t * Data = nullptr;
    uint16_t DataLength  = 0;

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    System::PacketBufferHandle Buffer;

private:
    CHIP_ERROR DerivedParse(System::PacketBufferHandle aBuffer) override;
    BufBound & DerivedWriteToBuffer(BufBound & aBuffer) const override;
    size_t DerivedMessageSize() const override;
};

using Block    = DataBlock;
using BlockEOF = DataBlock;

} // namespace bdx
} // namespace chip
