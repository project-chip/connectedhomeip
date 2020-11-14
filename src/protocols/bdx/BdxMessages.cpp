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

#include <protocols/bdx/BdxMessages.h>

#include <support/BufBound.h>
#include <support/BufferReader.h>
#include <support/CodeUtils.h>

#include <limits>

namespace {
constexpr uint8_t kVersionMask = 0x0F;
} // namespace

using namespace chip;
using namespace chip::BDX;
using namespace chip::Encoding::LittleEndian;

CHIP_ERROR TransferInit::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    uint8_t proposedTransferCtl = 0;
    bool widerange              = ((mStartOffset | mMaxLength) > std::numeric_limits<uint32_t>::max());
    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    proposedTransferCtl |= mSupportedVersions & kVersionMask;
    proposedTransferCtl = proposedTransferCtl | mTransferCtlFlags.Raw();

    BitFlags<uint8_t, RangeControlFlags> rangeCtlFlags;
    rangeCtlFlags.Set(kDefLen, mMaxLength > 0);
    rangeCtlFlags.Set(kStartOffset, mStartOffset > 0);
    rangeCtlFlags.Set(kWiderange, widerange);

    bbuf.Put(proposedTransferCtl);
    bbuf.Put(rangeCtlFlags.Raw());
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

CHIP_ERROR TransferInit::Parse(const System::PacketBuffer & aBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t proposedTransferCtl;
    uint8_t rangeCtl;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());
    BitFlags<uint8_t, RangeControlFlags> rangeCtlFlags;

    SuccessOrExit(bufReader.Read8(&proposedTransferCtl).Read8(&rangeCtl).Read16(&mMaxBlockSize).StatusCode());

    mSupportedVersions = proposedTransferCtl & kVersionMask;
    mTransferCtlFlags.SetRaw(static_cast<uint8_t>(proposedTransferCtl & ~kVersionMask));
    rangeCtlFlags.SetRaw(rangeCtl);

    if (rangeCtlFlags.Has(kStartOffset))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&mStartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            mStartOffset = tmpUint32Value;
        }
    }

    if (rangeCtlFlags.Has(kDefLen))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&mMaxLength).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            mMaxLength = tmpUint32Value;
        }
    }

    SuccessOrExit(bufReader.Read16(&mFileDesLength).StatusCode());

    // WARNING: this struct will store a pointer to the start of the file designator in the PacketBuffer,
    // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
    // or there is risk of unsafe memory access.
    VerifyOrExit(bufReader.HasAtLeast(mFileDesLength), err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    mFileDesignator = &bufStart[bufReader.OctetsRead()];

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > mFileDesLength)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        uint16_t metadataStartIndex = static_cast<uint16_t>(bufReader.OctetsRead() + mFileDesLength);
        mMetadata                   = &bufStart[metadataStartIndex];
        mMetadataLength             = static_cast<uint16_t>(aBuffer.DataLength() - metadataStartIndex);
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
    if (mMetadataLength != another.mMetadataLength || mFileDesLength != another.mFileDesLength)
    {
        return false;
    }
    bool fileDesMatches  = memcmp(mFileDesignator, another.mFileDesignator, mFileDesLength) == 0;
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mSupportedVersions == another.mSupportedVersions && mTransferCtlFlags.Raw() == another.mTransferCtlFlags.Raw() &&
            mStartOffset == another.mStartOffset && mMaxLength == another.mMaxLength && mMaxBlockSize == another.mMaxBlockSize &&
            fileDesMatches && metadataMatches);
}

CHIP_ERROR SendAccept::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    transferCtl |= mVersion & kVersionMask;
    transferCtl = transferCtl | mTransferCtlFlags.Raw();

    bbuf.Put(transferCtl);
    bbuf.PutLE16(mMaxBlockSize);
    bbuf.Put(mMetadata, static_cast<size_t>(mMetadataLength));

    VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_BUFFER_TOO_SMALL);
    aBuffer.SetDataLength(static_cast<uint16_t>(bbuf.Written()));

exit:
    return err;
}

CHIP_ERROR SendAccept::Parse(const System::PacketBuffer & aBuffer)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;
    uint8_t * bufStart  = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());

    SuccessOrExit(bufReader.Read8(&transferCtl).Read16(&mMaxBlockSize).StatusCode());

    mVersion = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    mTransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > 0)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        mMetadata       = &bufStart[bufReader.OctetsRead()];
        mMetadataLength = bufReader.Remaining();
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
    if (mMetadataLength != another.mMetadataLength)
    {
        return false;
    }
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mVersion == another.mVersion && mTransferCtlFlags.Raw() == another.mTransferCtlFlags.Raw() &&
            mMaxBlockSize == another.mMaxBlockSize && metadataMatches);
}

CHIP_ERROR ReceiveAccept::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;
    bool widerange      = ((mStartOffset | mLength) > std::numeric_limits<uint32_t>::max());

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    transferCtl |= mVersion & kVersionMask;
    transferCtl = transferCtl | mTransferCtlFlags.Raw();

    BitFlags<uint8_t, RangeControlFlags> rangeCtlFlags;
    rangeCtlFlags.Set(kDefLen, mLength > 0);
    rangeCtlFlags.Set(kStartOffset, mStartOffset > 0);
    rangeCtlFlags.Set(kWiderange, widerange);

    bbuf.Put(transferCtl);
    bbuf.Put(rangeCtlFlags.Raw());
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

CHIP_ERROR ReceiveAccept::Parse(const System::PacketBuffer & aBuffer)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint8_t transferCtl     = 0;
    uint8_t rangeCtl        = 0;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer.Start();
    Reader bufReader(bufStart, aBuffer.DataLength());
    BitFlags<uint8_t, RangeControlFlags> rangeCtlFlags;

    SuccessOrExit(bufReader.Read8(&transferCtl).Read8(&rangeCtl).Read16(&mMaxBlockSize).StatusCode());

    mVersion = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    mTransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    rangeCtlFlags.SetRaw(rangeCtl);

    if (rangeCtlFlags.Has(kStartOffset))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&mStartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            mStartOffset = tmpUint32Value;
        }
    }

    if (rangeCtlFlags.Has(kDefLen))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&mLength).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            mLength = tmpUint32Value;
        }
    }

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > 0)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        mMetadata       = &bufStart[bufReader.OctetsRead()];
        mMetadataLength = bufReader.Remaining();
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
    if (mMetadataLength != another.mMetadataLength)
    {
        return false;
    }
    bool metadataMatches = memcmp(mMetadata, another.mMetadata, mMetadataLength) == 0;

    return (mVersion == another.mVersion && mTransferCtlFlags.Raw() == another.mTransferCtlFlags.Raw() &&
            mStartOffset == another.mStartOffset && mMaxBlockSize == another.mMaxBlockSize && mLength == another.mLength &&
            metadataMatches);
}
