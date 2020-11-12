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
 *      messages.
 */

#pragma once

#include <support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace BDX {

enum ControlMode : uint8_t
{
    kNotSpecified  = 0x00,
    kSenderDrive   = 0x10,
    kReceiverDrive = 0x20,
    kAsync         = 0x40,
};

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
    CHIP_ERROR Pack(System::PacketBuffer & aBuffer);

    /**
     * @brief
     *  Parse data from an PacketBuffer into a struct instance
     *
     * @param[in] aBuffer Pointer to a PacketBuffer containing the data.
     * @param[out] aParsedMessage Reference to a struct instance where results will be stored.
     *             Note that this struct will store pointers into the passed PacketBuffer, so it
     *             is essential that the PacketBuffer is not modified or freed until after the
     *             TransferInit struct is no longer needed.
     *
     * @return CHIP_ERROR Any error that occurs when trying to read the message
     */
    static CHIP_ERROR Parse(System::PacketBuffer & aBuffer, TransferInit & aParsedMessage);

    /**
     * @brief
     *  Get the size of the message once it is written to a buffer.
     */
    size_t PackedSize();

    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const TransferInit &) const;

    // Proposed Transfer Control (required)
    bool mSupportsAsync;
    bool mSupportsReceiverDrive;
    bool mSupportsSenderDrive;
    uint8_t mSupportedVersions;

    // Range Control (required)
    bool mWideRange;       ///< Set true to indicate mStartOffset and mDefLen are 64-bit, else 32-bit.
    uint64_t mStartOffset; ///< Proposed start offset of data. 0 for no offset. Will be truncated if mWideRange is false.
    bool mDefLen;          ///< True if transfer has a definite length.

    uint64_t mMaxLength;    ///< Proposed max length of data in transfer, 0 for indefinite (required if mDefLen is set).
                            ///< Will be truncated if mWideRange is false.
    uint16_t mMaxBlockSize; ///< Proposed max block size to use in transfer (required)

    // File designator (required)
    uint8_t * mFileDesignator;
    uint16_t mFileDesLength; ///< Length of file designator string (not including null-terminator)

    // Additional metadata (optional, TLV format)
    uint8_t * mMetadata;
    uint16_t mMetadataLength;
};

struct SendInit : public TransferInit
{
};

struct ReceiveInit : public TransferInit
{
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
    CHIP_ERROR Pack(System::PacketBuffer & aBuffer);

    /**
     * @brief
     *  Parse data from an PacketBuffer into a struct instance
     *
     * @param[in] aBuffer Pointer to a PacketBuffer containing the data
     * @param[out] aParsedMessage Reference to a struct instance where results will be stored.
     *             Note that this struct will store pointers into the passed PacketBuffer, so it
     *             is essential that the PacketBuffer is not modified or freed until after the
     *             SendAccept struct is no longer needed.
     *
     * @return CHIP_ERROR Any error that occurs when trying to read the message
     */
    static CHIP_ERROR Parse(System::PacketBuffer & aBuffer, SendAccept & aResponse);

    /**
     * @brief
     *  Get the size of the message once it is written to a buffer.
     */
    size_t PackedSize();

    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const SendAccept &) const;

    // All required
    uint8_t mVersion;         ///< The agreed upon version for the transfer.
    ControlMode mControlMode; ///< Agreed upon transfer control method
    uint16_t mMaxBlockSize;   ///< Chosen max block size to use in transfer

    // Additional metadata (optional, TLV format)
    uint8_t * mMetadata;
    uint16_t mMetadataLength;
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
    CHIP_ERROR Pack(System::PacketBuffer & aBuffer);

    /**
     * @brief
     *  Parse data from an PacketBuffer into a struct instance
     *
     * @param[in] aBuffer Pointer to a PacketBuffer containing the data
     * @param[out] aParsedMessage Reference to a struct instance where results will be stored.
     *             Note that this struct will store pointers into the passed PacketBuffer, so it
     *             is essential that the PacketBuffer is not modified or freed until after the
     *             ReceiveAccept struct is no longer needed.
     *
     * @return CHIP_ERROR Any error that occurs when trying to read the message
     */
    static CHIP_ERROR Parse(System::PacketBuffer & aBuffer, ReceiveAccept & aResponse);

    /**
     * @brief
     *  Get the size of the message once it is written to a buffer.
     */
    size_t PackedSize();

    /**
     * @brief
     *  Equality check method.
     */
    bool operator==(const ReceiveAccept &) const;

    // All required
    uint8_t mVersion;         ///< The agreed upon version for the transfer
    ControlMode mControlMode; ///< Agreed upon transfer control method

    // Range Control (must fill in)
    bool mWideRange;       ///< Set true to indicate mStartOffset and mDefLen are 64-bit, else 32-bit
    uint64_t mStartOffset; ///< Chosen start offset of data. 0 for no offset. Will be truncated if
                           ///< mWideRange is false.
    bool mDefLen;          ///< Set to true if transfer has definite length

    uint16_t mMaxBlockSize; ///< Chosen max block size to use in transfer (required)
    uint64_t mLength;       ///< Length of transfer (only valid/required if mDefLen is set). Will be
                            ///< truncated if mWideRange is false.

    // Additional metadata (optional, TLV format)
    uint8_t * mMetadata;
    uint16_t mMetadataLength;
};

} // namespace BDX
} // namespace chip
