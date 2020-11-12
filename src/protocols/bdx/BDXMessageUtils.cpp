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

#define VERSION_MASK 0x0F
#define SENDER_DRIVE_MASK 0x10
#define RECEIVER_DRIVE_MASK 0x20
#define ASYNC_MASK 0x40
#define CONTROL_MODE_MASK 0xF0

#define DEFLEN_MASK 0x01
#define START_OFFSET_MASK 0x02
#define WIDERANGE_MASK 0x10

using namespace chip;
using namespace chip::BDX;
using namespace chip::Encoding::LittleEndian;

CHIP_ERROR TransferInit::Pack(System::PacketBuffer & aBuffer)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint8_t rangeCtl = 0;
    uint8_t ptcByte  = 0;

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    ptcByte |= mSupportedVersions & VERSION_MASK;
    if (mSupportsSenderDrive)
        ptcByte |= SENDER_DRIVE_MASK;
    if (mSupportsReceiverDrive)
        ptcByte |= RECEIVER_DRIVE_MASK;
    if (mSupportsAsync)
        ptcByte |= ASYNC_MASK;

    if (mDefLen)
        rangeCtl |= DEFLEN_MASK;
    if (mStartOffset)
        rangeCtl |= START_OFFSET_MASK;
    if (mWideRange)
        rangeCtl |= WIDERANGE_MASK;

    bbuf.Put(ptcByte);
    bbuf.Put(rangeCtl);
    bbuf.PutLE16(mMaxBlockSize);

    if (mStartOffset > 0)
    {
        if (mWideRange)
        {
            bbuf.PutLE64(mStartOffset);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(mStartOffset));
        }
    }

    if (mDefLen)
    {
        if (mWideRange)
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

CHIP_ERROR TransferInit::Parse(System::PacketBuffer & aBuffer, TransferInit & aParsedMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t ptcByte;
    uint8_t rangeCtl;
    bool hasStartOffset     = false;
    uint32_t tmpUint32Value = 0;
    uint8_t * bufStart      = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&ptcByte).Read8(&rangeCtl).Read16(&aParsedMessage.mMaxBlockSize).StatusCode());

    aParsedMessage.mSupportedVersions     = ptcByte & VERSION_MASK;
    aParsedMessage.mSupportsSenderDrive   = ((ptcByte & SENDER_DRIVE_MASK) != 0);
    aParsedMessage.mSupportsReceiverDrive = ((ptcByte & RECEIVER_DRIVE_MASK) != 0);
    aParsedMessage.mSupportsAsync         = ((ptcByte & ASYNC_MASK) != 0);

    aParsedMessage.mDefLen    = (rangeCtl & DEFLEN_MASK) != 0;
    hasStartOffset            = (rangeCtl & START_OFFSET_MASK) != 0;
    aParsedMessage.mWideRange = (rangeCtl & WIDERANGE_MASK) != 0;

    if (hasStartOffset)
    {
        if (aParsedMessage.mWideRange)
        {
            SuccessOrExit(bufReader.Read64(&aParsedMessage.mStartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            aParsedMessage.mStartOffset = tmpUint32Value;
        }
    }

    if (aParsedMessage.mDefLen)
    {
        if (aParsedMessage.mWideRange)
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

    VerifyOrExit(bufReader.OctetsRead() + aParsedMessage.mFileDesLength <= aBuffer.DataLength(),
                 err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    aParsedMessage.mFileDesignator = &bufStart[bufReader.OctetsRead()];

    // Rest of message is metadata (could be empty)
    if (bufReader.OctetsRead() + aParsedMessage.mFileDesLength < aBuffer.DataLength())
    {
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

size_t TransferInit::PackedSize()
{
    size_t offsetSize = mWideRange ? 8 : 4;
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
            mWideRange == another.mWideRange && mStartOffset == another.mStartOffset && mDefLen == another.mDefLen &&
            mMaxLength == another.mMaxLength && mMaxBlockSize == another.mMaxBlockSize && fileDesMatches && metadataMatches);
}

CHIP_ERROR SendAccept::Pack(System::PacketBuffer & aBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t tcByte = 0;

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    tcByte |= mVersion & VERSION_MASK;
    tcByte |= mControlMode;

    bbuf.Put(tcByte);
    bbuf.PutLE16(mMaxBlockSize);
    bbuf.Put(mMetadata, static_cast<size_t>(mMetadataLength));

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    aBuffer.SetDataLength(static_cast<uint16_t>(bbuf.Written()));

exit:
    return err;
}

CHIP_ERROR SendAccept::Parse(System::PacketBuffer & aBuffer, SendAccept & aParsedMessage)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    uint8_t tcByte         = 0;
    uint8_t tmpControlMode = 0;
    uint8_t * bufStart     = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&tcByte).Read16(&aParsedMessage.mMaxBlockSize).StatusCode());

    aParsedMessage.mVersion = tcByte & VERSION_MASK;
    tmpControlMode          = tcByte & CONTROL_MODE_MASK;

    // Determine transfer control mode. Only one mode should be selected.
    // TODO: should this verification happen here?
    VerifyOrExit(tmpControlMode == kSenderDrive || tmpControlMode == kReceiverDrive || tmpControlMode == kAsync,
                 err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    aParsedMessage.mControlMode = static_cast<ControlMode>(tmpControlMode);

    // Rest of message is metadata (could be empty)
    if (bufReader.OctetsRead() < aBuffer.DataLength())
    {
        aParsedMessage.mMetadata       = &bufStart[bufReader.OctetsRead()];
        aParsedMessage.mMetadataLength = static_cast<uint16_t>(aBuffer.DataLength() - bufReader.OctetsRead());
    }

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t SendAccept::PackedSize()
{
    // First byte is Transfer Control byte
    return (1 + sizeof(mMaxBlockSize) + mMetadataLength);
}

bool SendAccept::operator==(const SendAccept & another) const
{
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mVersion == another.mVersion && mControlMode == another.mControlMode && mMaxBlockSize == another.mMaxBlockSize &&
            metadataMatches);
}

CHIP_ERROR ReceiveAccept::Pack(System::PacketBuffer & aBuffer)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    uint8_t tcByte   = 0;
    uint8_t rangeCtl = 0;

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    tcByte |= mVersion & VERSION_MASK;
    tcByte |= mControlMode;

    if (mDefLen)
        rangeCtl |= DEFLEN_MASK;
    if (mStartOffset)
        rangeCtl |= START_OFFSET_MASK;
    if (mWideRange)
        rangeCtl |= WIDERANGE_MASK;

    bbuf.Put(tcByte);
    bbuf.Put(rangeCtl);
    bbuf.PutLE16(mMaxBlockSize);

    if (mStartOffset > 0)
    {
        if (mWideRange)
        {
            bbuf.PutLE64(mStartOffset);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(mStartOffset));
        }
    }

    if (mDefLen)
    {
        if (mWideRange)
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

CHIP_ERROR ReceiveAccept::Parse(System::PacketBuffer & aBuffer, ReceiveAccept & aParsedMessage)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    bool hasStartOffset     = false;
    uint8_t tcByte          = 0;
    uint8_t rangeCtl        = 0;
    uint8_t tmpControlMode  = 0;
    uint32_t tmpUint32Value = 0;
    uint8_t * bufStart      = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&tcByte).Read8(&rangeCtl).Read16(&aParsedMessage.mMaxBlockSize).StatusCode());

    aParsedMessage.mVersion = tcByte & VERSION_MASK;
    tmpControlMode          = tcByte & CONTROL_MODE_MASK;

    // Determine transfer control mode. Only one mode should be selected.
    // TODO: should this verification happen here?
    VerifyOrExit(tmpControlMode == kSenderDrive || tmpControlMode == kReceiverDrive || tmpControlMode == kAsync,
                 err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    aParsedMessage.mControlMode = static_cast<ControlMode>(tmpControlMode);

    aParsedMessage.mDefLen    = (rangeCtl & DEFLEN_MASK) != 0;
    hasStartOffset            = (rangeCtl & START_OFFSET_MASK) != 0;
    aParsedMessage.mWideRange = (rangeCtl & WIDERANGE_MASK) != 0;

    if (hasStartOffset)
    {
        if (aParsedMessage.mWideRange)
        {
            SuccessOrExit(bufReader.Read64(&aParsedMessage.mStartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            aParsedMessage.mStartOffset = tmpUint32Value;
        }
    }

    if (aParsedMessage.mDefLen)
    {
        if (aParsedMessage.mWideRange)
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
    if (bufReader.OctetsRead() < aBuffer.DataLength())
    {
        aParsedMessage.mMetadata       = &bufStart[bufReader.OctetsRead()];
        aParsedMessage.mMetadataLength = static_cast<uint16_t>(aBuffer.DataLength() - bufReader.OctetsRead());
    }

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t ReceiveAccept::PackedSize()
{
    size_t offsetSize = mWideRange ? 8 : 4;
    size_t lengthSize = offsetSize;

    // First 2 bytes are Transfer Control and Range Control bytes
    return (2 + sizeof(mMaxBlockSize) + offsetSize + lengthSize + mMetadataLength);
}

bool ReceiveAccept::operator==(const ReceiveAccept & another) const
{
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mVersion == another.mVersion && mControlMode == another.mControlMode && mWideRange == another.mWideRange &&
            mStartOffset == another.mStartOffset && mDefLen == another.mDefLen && mMaxBlockSize == another.mMaxBlockSize &&
            mLength == another.mLength && metadataMatches);
}
