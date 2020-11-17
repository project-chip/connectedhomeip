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
    bool widerange = (StartOffset > std::numeric_limits<uint32_t>::max()) || (MaxLength > std::numeric_limits<uint32_t>::max());
    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    proposedTransferCtl |= Version & kVersionMask;
    proposedTransferCtl = proposedTransferCtl | TransferCtlOptions.Raw();

    BitFlags<uint8_t, RangeControlFlags> rangeCtlFlags;
    rangeCtlFlags.Set(kDefLen, MaxLength > 0);
    rangeCtlFlags.Set(kStartOffset, StartOffset > 0);
    rangeCtlFlags.Set(kWiderange, widerange);

    bbuf.Put(proposedTransferCtl);
    bbuf.Put(rangeCtlFlags.Raw());
    bbuf.PutLE16(MaxBlockSize);

    if (StartOffset > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(StartOffset);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(StartOffset));
        }
    }

    if (MaxLength > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(MaxLength);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(MaxLength));
        }
    }

    VerifyOrExit(FileDesignator && FileDesLength > 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.PutLE16(FileDesLength);
    bbuf.Put(FileDesignator, static_cast<size_t>(FileDesLength));

    // Metadata is optional
    if (Metadata && MetadataLength > 0)
    {
        bbuf.Put(Metadata, static_cast<size_t>(MetadataLength));
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

    SuccessOrExit(bufReader.Read8(&proposedTransferCtl).Read8(&rangeCtl).Read16(&MaxBlockSize).StatusCode());

    Version = proposedTransferCtl & kVersionMask;
    TransferCtlOptions.SetRaw(static_cast<uint8_t>(proposedTransferCtl & ~kVersionMask));
    rangeCtlFlags.SetRaw(rangeCtl);

    if (rangeCtlFlags.Has(kStartOffset))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&StartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            StartOffset = tmpUint32Value;
        }
    }

    if (rangeCtlFlags.Has(kDefLen))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&MaxLength).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            MaxLength = tmpUint32Value;
        }
    }

    SuccessOrExit(bufReader.Read16(&FileDesLength).StatusCode());

    // WARNING: this struct will store a pointer to the start of the file designator in the PacketBuffer,
    // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
    // or there is risk of unsafe memory access.
    VerifyOrExit(bufReader.HasAtLeast(FileDesLength), err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    FileDesignator = &bufStart[bufReader.OctetsRead()];

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > FileDesLength)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        uint16_t metadataStartIndex = static_cast<uint16_t>(bufReader.OctetsRead() + FileDesLength);
        Metadata                    = &bufStart[metadataStartIndex];
        MetadataLength              = static_cast<uint16_t>(aBuffer.DataLength() - metadataStartIndex);
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
    bool widerange    = ((StartOffset | MaxLength) > std::numeric_limits<uint32_t>::max());
    size_t offsetSize = widerange ? 8 : 4;
    size_t lengthSize = offsetSize;

    // First 2 bytes are Transfer Control and Range Control bytes
    return (2 + sizeof(MaxBlockSize) + offsetSize + lengthSize + sizeof(FileDesLength) + FileDesLength + MetadataLength);
}

bool TransferInit::operator==(const TransferInit & another) const
{
    if (MetadataLength != another.MetadataLength || FileDesLength != another.FileDesLength)
    {
        return false;
    }
    bool fileDesMatches  = memcmp(FileDesignator, another.FileDesignator, FileDesLength) == 0;
    bool metadataMatches = memcmp(Metadata, another.Metadata, MetadataLength) == 0;

    return (Version == another.Version && TransferCtlOptions.Raw() == another.TransferCtlOptions.Raw() &&
            StartOffset == another.StartOffset && MaxLength == another.MaxLength && MaxBlockSize == another.MaxBlockSize &&
            fileDesMatches && metadataMatches);
}

CHIP_ERROR SendAccept::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    transferCtl |= Version & kVersionMask;
    transferCtl = transferCtl | TransferCtlFlags.Raw();

    bbuf.Put(transferCtl);
    bbuf.PutLE16(MaxBlockSize);
    bbuf.Put(Metadata, static_cast<size_t>(MetadataLength));

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

    SuccessOrExit(bufReader.Read8(&transferCtl).Read16(&MaxBlockSize).StatusCode());

    Version = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    TransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > 0)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        Metadata       = &bufStart[bufReader.OctetsRead()];
        MetadataLength = bufReader.Remaining();
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
    return (1 + sizeof(MaxBlockSize) + MetadataLength);
}

bool SendAccept::operator==(const SendAccept & another) const
{
    if (MetadataLength != another.MetadataLength)
    {
        return false;
    }
    bool metadataMatches = memcmp(Metadata, another.Metadata, MetadataLength) == 0;

    return (Version == another.Version && TransferCtlFlags.Raw() == another.TransferCtlFlags.Raw() &&
            MaxBlockSize == another.MaxBlockSize && metadataMatches);
}

CHIP_ERROR ReceiveAccept::Pack(System::PacketBuffer & aBuffer) const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;
    bool widerange      = (StartOffset > std::numeric_limits<uint32_t>::max()) || (Length > std::numeric_limits<uint32_t>::max());

    BufBound bbuf(aBuffer.Start(), aBuffer.AvailableDataLength());

    transferCtl |= Version & kVersionMask;
    transferCtl = transferCtl | TransferCtlFlags.Raw();

    BitFlags<uint8_t, RangeControlFlags> rangeCtlFlags;
    rangeCtlFlags.Set(kDefLen, Length > 0);
    rangeCtlFlags.Set(kStartOffset, StartOffset > 0);
    rangeCtlFlags.Set(kWiderange, widerange);

    bbuf.Put(transferCtl);
    bbuf.Put(rangeCtlFlags.Raw());
    bbuf.PutLE16(MaxBlockSize);

    if (StartOffset > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(StartOffset);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(StartOffset));
        }
    }

    if (Length > 0)
    {
        if (widerange)
        {
            bbuf.PutLE64(Length);
        }
        else
        {
            bbuf.PutLE32(static_cast<uint32_t>(Length));
        }
    }

    bbuf.Put(Metadata, static_cast<size_t>(MetadataLength));

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

    SuccessOrExit(bufReader.Read8(&transferCtl).Read8(&rangeCtl).Read16(&MaxBlockSize).StatusCode());

    Version = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    TransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    rangeCtlFlags.SetRaw(rangeCtl);

    if (rangeCtlFlags.Has(kStartOffset))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&StartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            StartOffset = tmpUint32Value;
        }
    }

    if (rangeCtlFlags.Has(kDefLen))
    {
        if (rangeCtlFlags.Has(kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&Length).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            Length = tmpUint32Value;
        }
    }

    // Rest of message is metadata (could be empty)
    if (bufReader.Remaining() > 0)
    {
        // WARNING: this struct will store a pointer to the start of metadata in the PacketBuffer,
        // but will not make a copy. It is essential that this struct not outlive the PacketBuffer,
        // or there is risk of unsafe memory access.
        Metadata       = &bufStart[bufReader.OctetsRead()];
        MetadataLength = bufReader.Remaining();
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
    bool widerange    = ((StartOffset | Length) > std::numeric_limits<uint32_t>::max());
    size_t offsetSize = widerange ? 8 : 4;
    size_t lengthSize = offsetSize;

    // First 2 bytes are Transfer Control and Range Control bytes
    return (2 + sizeof(MaxBlockSize) + offsetSize + lengthSize + MetadataLength);
}

bool ReceiveAccept::operator==(const ReceiveAccept & another) const
{
    if (MetadataLength != another.MetadataLength)
    {
        return false;
    }
    bool metadataMatches = memcmp(Metadata, another.Metadata, MetadataLength) == 0;

    return (Version == another.Version && TransferCtlFlags.Raw() == another.TransferCtlFlags.Raw() &&
            StartOffset == another.StartOffset && MaxBlockSize == another.MaxBlockSize && Length == another.Length &&
            metadataMatches);
}
