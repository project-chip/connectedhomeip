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
 *      Implements utility methods for working with some complex BDX messages.
 */

#include <protocols/bdx/BdxMessages.h>

#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>

#include <limits>
#include <utility>

namespace {
constexpr uint8_t kVersionMask = 0x0F;
} // namespace

using namespace chip;
using namespace chip::bdx;
using namespace chip::Encoding::LittleEndian;

// WARNING: this function should never return early, since MessageSize() relies on it to calculate
// the size of the message (even if the message is incomplete or filled out incorrectly).
BufferWriter & TransferInit::WriteToBuffer(BufferWriter & aBuffer) const
{
    const BitFlags<TransferControlFlags> proposedTransferCtl(Version & kVersionMask, TransferCtlOptions);
    const bool widerange =
        (StartOffset > std::numeric_limits<uint32_t>::max()) || (MaxLength > std::numeric_limits<uint32_t>::max());

    BitFlags<RangeControlFlags> rangeCtlFlags;
    rangeCtlFlags.Set(RangeControlFlags::kDefLen, MaxLength > 0);
    rangeCtlFlags.Set(RangeControlFlags::kStartOffset, StartOffset > 0);
    rangeCtlFlags.Set(RangeControlFlags::kWiderange, widerange);

    aBuffer.Put(proposedTransferCtl.Raw());
    aBuffer.Put(rangeCtlFlags.Raw());
    aBuffer.Put16(MaxBlockSize);

    if (StartOffset > 0)
    {
        if (widerange)
        {
            aBuffer.Put64(StartOffset);
        }
        else
        {
            aBuffer.Put32(static_cast<uint32_t>(StartOffset));
        }
    }

    if (MaxLength > 0)
    {
        if (widerange)
        {
            aBuffer.Put64(MaxLength);
        }
        else
        {
            aBuffer.Put32(static_cast<uint32_t>(MaxLength));
        }
    }

    aBuffer.Put16(FileDesLength);
    if (FileDesignator != nullptr)
    {
        aBuffer.Put(FileDesignator, static_cast<size_t>(FileDesLength));
    }

    if (Metadata != nullptr)
    {
        aBuffer.Put(Metadata, static_cast<size_t>(MetadataLength));
    }
    return aBuffer;
}

CHIP_ERROR TransferInit::Parse(System::PacketBufferHandle aBuffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t proposedTransferCtl;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());
    BitFlags<RangeControlFlags> rangeCtlFlags;

    SuccessOrExit(bufReader.Read8(&proposedTransferCtl).Read8(rangeCtlFlags.RawStorage()).Read16(&MaxBlockSize).StatusCode());

    Version = proposedTransferCtl & kVersionMask;
    TransferCtlOptions.SetRaw(static_cast<uint8_t>(proposedTransferCtl & ~kVersionMask));

    StartOffset = 0;
    if (rangeCtlFlags.Has(RangeControlFlags::kStartOffset))
    {
        if (rangeCtlFlags.Has(RangeControlFlags::kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&StartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            StartOffset = tmpUint32Value;
        }
    }

    MaxLength = 0;
    if (rangeCtlFlags.Has(RangeControlFlags::kDefLen))
    {
        if (rangeCtlFlags.Has(RangeControlFlags::kWiderange))
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

    VerifyOrExit(bufReader.HasAtLeast(FileDesLength), err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    FileDesignator = &bufStart[bufReader.OctetsRead()];

    // Rest of message is metadata (could be empty)
    Metadata       = nullptr;
    MetadataLength = 0;
    if (bufReader.Remaining() > FileDesLength)
    {
        uint16_t metadataStartIndex = static_cast<uint16_t>(bufReader.OctetsRead() + FileDesLength);
        Metadata                    = &bufStart[metadataStartIndex];
        MetadataLength              = static_cast<uint16_t>(aBuffer->DataLength() - metadataStartIndex);
    }

    // Retain ownership of the packet buffer so that the FileDesignator and Metadata pointers remain valid.
    Buffer = std::move(aBuffer);

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t TransferInit::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

bool TransferInit::operator==(const TransferInit & another) const
{
    if ((MetadataLength != another.MetadataLength) || (FileDesLength != another.FileDesLength))
    {
        return false;
    }

    bool fileDesMatches = true;
    if (FileDesLength > 0)
    {
        fileDesMatches = (memcmp(FileDesignator, another.FileDesignator, FileDesLength) == 0);
    }

    bool metadataMatches = true;
    if (MetadataLength > 0)
    {
        metadataMatches = (memcmp(Metadata, another.Metadata, MetadataLength) == 0);
    }

    return ((Version == another.Version) && (TransferCtlOptions == another.TransferCtlOptions) &&
            (StartOffset == another.StartOffset) && (MaxLength == another.MaxLength) && (MaxBlockSize == another.MaxBlockSize) &&
            fileDesMatches && metadataMatches);
}

// WARNING: this function should never return early, since MessageSize() relies on it to calculate
// the size of the message (even if the message is incomplete or filled out incorrectly).
Encoding::LittleEndian::BufferWriter & SendAccept::WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const
{
    const BitFlags<TransferControlFlags> transferCtl(Version & kVersionMask, TransferCtlFlags);

    aBuffer.Put(transferCtl.Raw());
    aBuffer.Put16(MaxBlockSize);

    if (Metadata != nullptr)
    {
        aBuffer.Put(Metadata, static_cast<size_t>(MetadataLength));
    }
    return aBuffer;
}

CHIP_ERROR SendAccept::Parse(System::PacketBufferHandle aBuffer)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    uint8_t transferCtl = 0;
    uint8_t * bufStart  = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    SuccessOrExit(bufReader.Read8(&transferCtl).Read16(&MaxBlockSize).StatusCode());

    Version = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    TransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    // Rest of message is metadata (could be empty)
    Metadata       = nullptr;
    MetadataLength = 0;
    if (bufReader.Remaining() > 0)
    {
        Metadata       = &bufStart[bufReader.OctetsRead()];
        MetadataLength = bufReader.Remaining();
    }

    // Retain ownership of the packet buffer so that the Metadata pointer remains valid.
    Buffer = std::move(aBuffer);

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t SendAccept::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

bool SendAccept::operator==(const SendAccept & another) const
{
    if (MetadataLength != another.MetadataLength)
    {
        return false;
    }

    bool metadataMatches = true;
    if (MetadataLength > 0)
    {
        metadataMatches = (memcmp(Metadata, another.Metadata, MetadataLength) == 0);
    }

    return ((Version == another.Version) && (TransferCtlFlags == another.TransferCtlFlags) &&
            (MaxBlockSize == another.MaxBlockSize) && metadataMatches);
}

// WARNING: this function should never return early, since MessageSize() relies on it to calculate
// the size of the message (even if the message is incomplete or filled out incorrectly).
Encoding::LittleEndian::BufferWriter & ReceiveAccept::WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const
{
    const BitFlags<TransferControlFlags> transferCtlFlags(Version & kVersionMask, TransferCtlFlags);
    const bool widerange = (StartOffset > std::numeric_limits<uint32_t>::max()) || (Length > std::numeric_limits<uint32_t>::max());

    BitFlags<RangeControlFlags> rangeCtlFlags;
    rangeCtlFlags.Set(RangeControlFlags::kDefLen, Length > 0);
    rangeCtlFlags.Set(RangeControlFlags::kStartOffset, StartOffset > 0);
    rangeCtlFlags.Set(RangeControlFlags::kWiderange, widerange);

    aBuffer.Put(transferCtlFlags.Raw());
    aBuffer.Put(rangeCtlFlags.Raw());
    aBuffer.Put16(MaxBlockSize);

    if (StartOffset > 0)
    {
        if (widerange)
        {
            aBuffer.Put64(StartOffset);
        }
        else
        {
            aBuffer.Put32(static_cast<uint32_t>(StartOffset));
        }
    }

    if (Length > 0)
    {
        if (widerange)
        {
            aBuffer.Put64(Length);
        }
        else
        {
            aBuffer.Put32(static_cast<uint32_t>(Length));
        }
    }

    if (Metadata != nullptr)
    {
        aBuffer.Put(Metadata, static_cast<size_t>(MetadataLength));
    }
    return aBuffer;
}

CHIP_ERROR ReceiveAccept::Parse(System::PacketBufferHandle aBuffer)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint8_t transferCtl     = 0;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());
    BitFlags<RangeControlFlags> rangeCtlFlags;

    SuccessOrExit(bufReader.Read8(&transferCtl).Read8(rangeCtlFlags.RawStorage()).Read16(&MaxBlockSize).StatusCode());

    Version = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    TransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    StartOffset = 0;
    if (rangeCtlFlags.Has(RangeControlFlags::kStartOffset))
    {
        if (rangeCtlFlags.Has(RangeControlFlags::kWiderange))
        {
            SuccessOrExit(bufReader.Read64(&StartOffset).StatusCode());
        }
        else
        {
            SuccessOrExit(bufReader.Read32(&tmpUint32Value).StatusCode());
            StartOffset = tmpUint32Value;
        }
    }

    Length = 0;
    if (rangeCtlFlags.Has(RangeControlFlags::kDefLen))
    {
        if (rangeCtlFlags.Has(RangeControlFlags::kWiderange))
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
    Metadata       = nullptr;
    MetadataLength = 0;
    if (bufReader.Remaining() > 0)
    {
        Metadata       = &bufStart[bufReader.OctetsRead()];
        MetadataLength = bufReader.Remaining();
    }

    // Retain ownership of the packet buffer so that the Metadata pointer remains valid.
    Buffer = std::move(aBuffer);

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t ReceiveAccept::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

bool ReceiveAccept::operator==(const ReceiveAccept & another) const
{
    if (MetadataLength != another.MetadataLength)
    {
        return false;
    }

    bool metadataMatches = true;
    if (MetadataLength > 0)
    {
        metadataMatches = (memcmp(Metadata, another.Metadata, MetadataLength) == 0);
    }

    return ((Version == another.Version) && (TransferCtlFlags == another.TransferCtlFlags) &&
            (StartOffset == another.StartOffset) && (MaxBlockSize == another.MaxBlockSize) && (Length == another.Length) &&
            metadataMatches);
}

// WARNING: this function should never return early, since MessageSize() relies on it to calculate
// the size of the message (even if the message is incomplete or filled out incorrectly).
Encoding::LittleEndian::BufferWriter & CounterMessage::WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const
{
    return aBuffer.Put32(BlockCounter);
}

CHIP_ERROR CounterMessage::Parse(System::PacketBufferHandle aBuffer)
{
    uint8_t * bufStart = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());
    return bufReader.Read32(&BlockCounter).StatusCode();
}

size_t CounterMessage::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

bool CounterMessage::operator==(const CounterMessage & another) const
{
    return (BlockCounter == another.BlockCounter);
}

// WARNING: this function should never return early, since MessageSize() relies on it to calculate
// the size of the message (even if the message is incomplete or filled out incorrectly).
Encoding::LittleEndian::BufferWriter & DataBlock::WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const
{
    aBuffer.Put32(BlockCounter);
    if (Data != nullptr)
    {
        aBuffer.Put(Data, DataLength);
    }
    return aBuffer;
}

CHIP_ERROR DataBlock::Parse(System::PacketBufferHandle aBuffer)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint8_t * bufStart = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    SuccessOrExit(bufReader.Read32(&BlockCounter).StatusCode());

    // Rest of message is data
    Data       = nullptr;
    DataLength = 0;
    if (bufReader.Remaining() > 0)
    {
        Data       = &bufStart[bufReader.OctetsRead()];
        DataLength = bufReader.Remaining();
    }

    // Retain ownership of the packet buffer so that the Data pointer remains valid.
    Buffer = std::move(aBuffer);

exit:
    if (bufReader.StatusCode() != CHIP_NO_ERROR)
    {
        err = bufReader.StatusCode();
    }
    return err;
}

size_t DataBlock::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

bool DataBlock::operator==(const DataBlock & another) const
{
    if (DataLength != another.DataLength)
    {
        return false;
    }

    bool dataMatches = true;
    if (DataLength > 0)
    {
        dataMatches = memcmp(Data, another.Data, DataLength) == 0;
    }

    return ((BlockCounter == another.BlockCounter) && dataMatches);
}
