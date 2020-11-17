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
#include <support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace BDX {

enum TransferControlFlags : uint8_t
{
    // first 4 bits reserved for version
    kSenderDrive   = (1U << 4),
    kReceiverDrive = (1U << 5),
    kAsync         = (1U << 6),
};

enum RangeControlFlags : uint8_t
{
    kDefLen      = (1U),
    kStartOffset = (1U << 1),
    kWiderange   = (1U << 4),
};

struct TransferInit;
using SendInit    = TransferInit;
using ReceiveInit = TransferInit;

/*
 * A structure for representing a SendInit or ReceiveInit message (both contain
 * identical parameters).
 */
struct TransferInit
{
    /**
     * @brief
     *  Pack (write) the message into a PacketBuffer.
     *
     * @param[out] aBuffer a PacketBuffer to use to write the message
     *
     * @return CHIP_ERROR Any error that occurs when trying to write to the PacketBuffer
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Pack(System::PacketBuffer & aBuffer) const;

    /**
     * @brief
     *  Parse data from an PacketBuffer into a struct instance.
     *
     *  Note that this struct will store pointers that point into the passed PacketBuffer,
     *  so it is essential that the PacketBuffer is not modified or freed until after this
     *  struct is no longer needed.
     *
     * @param[in] aBuffer Pointer to a PacketBuffer containing the data.
     *
     *
     * @return CHIP_ERROR Any error that occurs when trying to read the message
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Parse(const System::PacketBuffer & aBuffer);

    /**
     * @brief
     *  Returns the size of buffer needed to write this message.
     */
    size_t PackedSize() const;

    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const TransferInit &) const;

    // Proposed Transfer Control (required)
    BitFlags<uint8_t, TransferControlFlags> TransferCtlOptions;
    uint8_t Version = 0; ///< This field represents

    // All required
    uint16_t MaxBlockSize = 0; ///< Proposed max block size to use in transfer
    uint64_t StartOffset  = 0; ///< Proposed start offset of data. 0 for no offset
    uint64_t MaxLength    = 0; ///< Proposed max length of data in transfer, 0 for indefinite

    // File designator (required)
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory.
    // It is up to the caller to ensure that the memory pointed to here has not been freed.
    const uint8_t * FileDesignator = nullptr;
    uint16_t FileDesLength         = 0; ///< Length of file designator string (not including null-terminator)

    // Additional metadata (optional, TLV format)
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory.
    // It is up to the caller to ensure that the memory pointed to here has not been freed.
    const uint8_t * Metadata = nullptr;
    uint16_t MetadataLength  = 0;
};

/*
 * A structure for representing a SendAccept message.
 */
struct SendAccept
{
    /**
     * @brief
     *  Pack (write) the message into a PacketBuffer.
     *
     * @param[out] aBuffer a PacketBuffer to use to write the message
     *
     * @return CHIP_ERROR Any error that occurs when trying to write to the PacketBuffer
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Pack(System::PacketBuffer & aBuffer) const;

    /**
     * @brief
     *  Parse data from an PacketBuffer into a struct instance
     *
     *  Note that this struct will store pointers that point into the passed PacketBuffer,
     *  so it is essential that the PacketBuffer is not modified or freed until after this
     *  struct is no longer needed.
     *
     * @param[in] aBuffer Pointer to a PacketBuffer containing the data
     *
     * @return CHIP_ERROR Any error that occurs when trying to read the message
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Parse(const System::PacketBuffer & aBuffer);

    /**
     * @brief
     *  Returns the size of buffer needed to write this message.
     */
    size_t PackedSize() const;

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
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory.
    // It is up to the caller to ensure that the memory pointed to here has not been freed.
    uint8_t * Metadata      = nullptr;
    uint16_t MetadataLength = 0;
};

/**
 * @class ReceiveAccept
 *
 * @brief
 *   The ReceiveAccept message is used to accept a proposed exchange when the
 *   receiver is the initiator.
 */
struct ReceiveAccept
{
    /**
     * @brief
     *  Pack (write) the message into a PacketBuffer.
     *
     * @param[out] aBuffer a PacketBuffer to use to write the message
     *
     * @return CHIP_ERROR Any error that occurs when trying to write to the PacketBuffer
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Pack(System::PacketBuffer & aBuffer) const;

    /**
     * @brief
     *  Parse data from an PacketBuffer into a struct instance
     *
     *  Note that this struct will store pointers that point into the passed PacketBuffer,
     *  so it is essential that the PacketBuffer is not modified or freed until after this
     *  struct is no longer needed.
     *
     * @param[in] aBuffer Pointer to a PacketBuffer containing the data
     *
     * @return CHIP_ERROR Any error that occurs when trying to read the message
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Parse(const System::PacketBuffer & aBuffer);

    /**
     * @brief
     *  Returns the size of buffer needed to write this message.
     */
    size_t PackedSize() const;

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
    // WARNING: there is no guarantee at any point that this pointer will point to valid memory.
    // It is up to the caller to ensure that the memory pointed to here has not been freed.
    uint8_t * Metadata      = nullptr;
    uint16_t MetadataLength = 0;
};

} // namespace BDX
} // namespace chip
