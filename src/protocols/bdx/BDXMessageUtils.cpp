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
 *      Implements utility methods for working with some complex BDX messages.
 */

#include <protocols/bdx/BDXMessageUtils.h>

#include <support/BufBound.h>
#include <support/BufferReader.h>
#include <support/CodeUtils.h>

#include <limits>

#define VERSION_MASK 0x0F
#define SENDER_DRIVE_MASK 0x10
#define RECEIVER_DRIVE_MASK 0x20
#define ASYNC_MASK 0x40

#define DEFLEN_MASK 0x01
#define START_OFFSET_MASK 0x02
#define WIDERANGE_MASK 0x10

using namespace chip;
using namespace chip::BDX;
using namespace chip::Encoding::LittleEndian;

CHIP_ERROR TransferInit::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    uint8_t rangeCtl            = 0;
    uint8_t proposedTransferCtl = 0;
    bool widerange              = ((mStartOffset | mMaxLength) > std::numeric_limits<uint32_t>::max());

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    proposedTransferCtl |= mSupportedVersions & VERSION_MASK;
    if (mSupportsSenderDrive)
        proposedTransferCtl |= SENDER_DRIVE_MASK;
    if (mSupportsReceiverDrive)
        proposedTransferCtl |= RECEIVER_DRIVE_MASK;
    if (mSupportsAsync)
        proposedTransferCtl |= ASYNC_MASK;

    if (mMaxLength > 0)
        rangeCtl |= DEFLEN_MASK;
    if (mStartOffset > 0)
        rangeCtl |= START_OFFSET_MASK;
    if (widerange)
        rangeCtl |= WIDERANGE_MASK;

    bbuf.Put(proposedTransferCtl);
    bbuf.Put(rangeCtl);
    bbuf.PutLE16(mMaxBlockSize);

    if (mStartOffset > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(mStartOffset);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(mStartOffset));
        }
    }

    if (mMaxLength > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(mMaxLength);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(mMaxLength));
        }
    }

    VerifyOrExit(mFileDesignator && mFileDesLength > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.PutLE16(mFileDesLength);
    bbuf.Put(mFileDesignator, static_cast<size_t>(mFileDesLength));

    // Metadata is optional
    if (mMetadata && mMetadataLength > 0)
    {
        bbuf.Put(mMetadata, static_cast<size_t>(mMetadataLength));
    }

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    aBuffer.SetDataLength(static_cast<uint16_t>(bbuf.Written()));

exit:
    return err;
}

CHIP_ERROR TransferInit::Parse(const System::PacketBuffer & aBuffer, TransferInit & aParsedMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t proposedTransferCtl;
    uint8_t rangeCtl;
    bool hasStartOffset     = false;
    bool hasDefLen          = false;
    bool widerange          = false;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&proposedTransferCtl).Read8(&rangeCtl).Read16(&aParsedMessage.mMaxBlockSize).StatusCode());

    aParsedMessage.mSupportedVersions     = proposedTransferCtl & VERSION_MASK;
    aParsedMessage.mSupportsSenderDrive   = ((proposedTransferCtl & SENDER_DRIVE_MASK) != 0);
    aParsedMessage.mSupportsReceiverDrive = ((proposedTransferCtl & RECEIVER_DRIVE_MASK) != 0);
    aParsedMessage.mSupportsAsync         = ((proposedTransferCtl & ASYNC_MASK) != 0);

    hasDefLen      = (rangeCtl & DEFLEN_MASK) != 0;
    hasStartOffset = (rangeCtl & START_OFFSET_MASK) != 0;
    widerange      = (rangeCtl & WIDERANGE_MASK) != 0;

    if (hasStartOffset)
    {
        if (widerange)
        {
            SuccessOrExit(bufReader.Read64(&aParsedMessage.mStartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            aParsedMessage.mStartOffset = tmpUint32Value;
        }
    }

    if (hasDefLen)
    {
        if (widerange)
        {
            SuccessOrExit(bufReader.Read64(&aParsedMessage.mMaxLength).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            aParsedMessage.mMaxLength = tmpUint32Value;
        }
    }

    SuccessOrExit(bufReader.Read16(&aParsedMessage.mFileDesLength).StatusCode());

    // WARNING: this struct will store a pointer to the start of the file designator in the PacketBuffer,
    // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
    // or there is risk of unsafe memory access.
    VerifyOrExit(bufReader.HasAtLeast(aParsedMessage.mFileDesLength), err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    aParsedMessage.mFileDesignator = &bufStart[bufReader.OctetsRead()];

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > aParsedMessage.mFileDesLength)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        uint16_t metadataStartIndex    = static_cast<uint16_t>(bufReader.OctetsRead() + aParsedMessage.mFileDesLength);
        aParsedMessage.mMetadata       = &bufStart[metadataStartIndex];
        aParsedMessage.mMetadataLength = static_cast<uint16_t>(aBuffer.DataLength() - metadataStartIndex);
    }

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t TransferInit::PackedSize() const
{
    bool widerange    = ((mStartOffset | mMaxLength) > std::numeric_limits<uint32_t>::max());
    size_t offsetSize = widerange ? 8 : 4;
    size_t lengthSize = offsetSize;

    // First 2 bytes are Transfer Control and Range Control bytes
    return (2 + sizeof(mMaxBlockSize) + offsetSize + lengthSize + sizeof(mFileDesLength) + mFileDesLength + mMetadataLength);
}

bool TransferInit::operator==(const TransferInit & another) const
{
    bool fileDesMatches  = memcmp(mFileDesignator, another.mFileDesignator, mFileDesLength) == 0;
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mSupportedVersions == another.mSupportedVersions && mSupportsSenderDrive == another.mSupportsSenderDrive &&
            mSupportsReceiverDrive == another.mSupportsReceiverDrive && mSupportsAsync == another.mSupportsAsync &&
            mStartOffset == another.mStartOffset && mMaxLength == another.mMaxLength && mMaxBlockSize == another.mMaxBlockSize &&
            fileDesMatches && metadataMatches);
}

CHIP_ERROR SendAccept::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    transferCtl |= mVersion & VERSION_MASK;
    if (mUseSenderDrive)
        transferCtl |= SENDER_DRIVE_MASK;
    else if (mUseReceiverDrive)
        transferCtl |= RECEIVER_DRIVE_MASK;
    else if (mUseAsync)
        transferCtl |= ASYNC_MASK;

    bbuf.Put(transferCtl);
    bbuf.PutLE16(mMaxBlockSize);
    bbuf.Put(mMetadata, static_cast<size_t>(mMetadataLength));

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    aBuffer.SetDataLength(static_cast<uint16_t>(bbuf.Written()));

exit:
    return err;
}

CHIP_ERROR SendAccept::Parse(const System::PacketBuffer & aBuffer, SendAccept & aParsedMessage)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;
    uint8_t * bufStart  = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&transferCtl).Read16(&aParsedMessage.mMaxBlockSize).StatusCode());

    aParsedMessage.mVersion = transferCtl & VERSION_MASK;

    // Only one of these values should be set. It is up to the caller to verify this.
    aParsedMessage.mUseAsync         = (transferCtl & ASYNC_MASK) != 0;
    aParsedMessage.mUseReceiverDrive = (transferCtl & RECEIVER_DRIVE_MASK) != 0;
    aParsedMessage.mUseSenderDrive   = (transferCtl & SENDER_DRIVE_MASK) != 0;

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > 0)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        aParsedMessage.mMetadata       = &bufStart[bufReader.OctetsRead()];
        aParsedMessage.mMetadataLength = bufReader.Remaining();
    }

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t SendAccept::PackedSize() const
{
    // First byte is Transfer Control byte
    return (1 + sizeof(mMaxBlockSize) + mMetadataLength);
}

bool SendAccept::operator==(const SendAccept & another) const
{
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mVersion == another.mVersion && mUseAsync == another.mUseAsync && mUseReceiverDrive == another.mUseReceiverDrive &&
            mUseSenderDrive == another.mUseSenderDrive && mMaxBlockSize == another.mMaxBlockSize && metadataMatches);
}

CHIP_ERROR ReceiveAccept::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;
    uint8_t rangeCtl    = 0;
    bool widerange      = ((mStartOffset | mLength) > std::numeric_limits<uint32_t>::max());

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    transferCtl |= mVersion & VERSION_MASK;
    if (mUseSenderDrive)
        transferCtl |= SENDER_DRIVE_MASK;
    else if (mUseReceiverDrive)
        transferCtl |= RECEIVER_DRIVE_MASK;
    else if (mUseAsync)
        transferCtl |= ASYNC_MASK;

    if (mLength > 0)
        rangeCtl |= DEFLEN_MASK;
    if (mStartOffset > 0)
        rangeCtl |= START_OFFSET_MASK;
    if (widerange)
        rangeCtl |= WIDERANGE_MASK;

    bbuf.Put(transferCtl);
    bbuf.Put(rangeCtl);
    bbuf.PutLE16(mMaxBlockSize);

    if (mStartOffset > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(mStartOffset);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(mStartOffset));
        }
    }

    if (mLength > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(mLength);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(mLength));
        }
    }

    bbuf.Put(mMetadata, static_cast<size_t>(mMetadataLength));

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    aBuffer.SetDataLength(static_cast<uint16_t>(bbuf.Written()));

exit:
    return err;
}

CHIP_ERROR ReceiveAccept::Parse(const System::PacketBuffer & aBuffer, ReceiveAccept & aParsedMessage)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool hasDefLen          = false;
    bool hasStartOffset     = false;
    bool widerange          = false;
    uint8_t transferCtl     = 0;
    uint8_t rangeCtl        = 0;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&transferCtl).Read8(&rangeCtl).Read16(&aParsedMessage.mMaxBlockSize).StatusCode());

    aParsedMessage.mVersion = transferCtl & VERSION_MASK;

    // Only one of these values should be set. It is up to the caller to verify this.
    aParsedMessage.mUseAsync         = (transferCtl & ASYNC_MASK) != 0;
    aParsedMessage.mUseReceiverDrive = (transferCtl & RECEIVER_DRIVE_MASK) != 0;
    aParsedMessage.mUseSenderDrive   = (transferCtl & SENDER_DRIVE_MASK) != 0;

    hasDefLen      = (rangeCtl & DEFLEN_MASK) != 0;
    hasStartOffset = (rangeCtl & START_OFFSET_MASK) != 0;
    widerange      = (rangeCtl & WIDERANGE_MASK) != 0;

    if (hasStartOffset)
    {
        if (widerange)
        {
            SuccessOrExit(bufReader.Read64(&aParsedMessage.mStartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            aParsedMessage.mStartOffset = tmpUint32Value;
        }
    }

    if (hasDefLen)
    {
        if (widerange)
        {
            SuccessOrExit(bufReader.Read64(&aParsedMessage.mLength).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            aParsedMessage.mLength = tmpUint32Value;
        }
    }

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > 0)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        aParsedMessage.mMetadata       = &bufStart[bufReader.OctetsRead()];
        aParsedMessage.mMetadataLength = bufReader.Remaining();
    }

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t ReceiveAccept::PackedSize() const
{
    bool widerange    = ((mStartOffset | mLength) > std::numeric_limits<uint32_t>::max());
    size_t offsetSize = widerange ? 8 : 4;
    size_t lengthSize = offsetSize;

    // First 2 bytes are Transfer Control and Range Control bytes
    return (2 + sizeof(mMaxBlockSize) + offsetSize + lengthSize + mMetadataLength);
}

bool ReceiveAccept::operator==(const ReceiveAccept & another) const
{
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mVersion == another.mVersion && mUseAsync == another.mUseAsync && mUseReceiverDrive == another.mUseReceiverDrive &&
            mUseSenderDrive == another.mUseSenderDrive && mStartOffset == another.mStartOffset &&
            mMaxBlockSize == another.mMaxBlockSize && mLength == another.mLength && metadataMatches);
}
