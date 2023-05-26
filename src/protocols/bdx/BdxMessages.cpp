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
    uint8_t proposedTransferCtl;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    ReturnErrorOnFailure(
        bufReader.Read8(&proposedTransferCtl).Read8(mRangeCtlFlags.RawStorage()).Read16(&MaxBlockSize).StatusCode());

    Version = proposedTransferCtl & kVersionMask;
    TransferCtlOptions.SetRaw(static_cast<uint8_t>(proposedTransferCtl & ~kVersionMask));

    StartOffset = 0;
    if (mRangeCtlFlags.Has(RangeControlFlags::kStartOffset))
    {
        if (mRangeCtlFlags.Has(RangeControlFlags::kWiderange))
        {
            ReturnErrorOnFailure(bufReader.Read64(&StartOffset).StatusCode());
        }
        else
        {
            ReturnErrorOnFailure(bufReader.Read32(&tmpUint32Value).StatusCode());
            StartOffset = tmpUint32Value;
        }
    }

    MaxLength = 0;
    if (mRangeCtlFlags.Has(RangeControlFlags::kDefLen))
    {
        if (mRangeCtlFlags.Has(RangeControlFlags::kWiderange))
        {
            ReturnErrorOnFailure(bufReader.Read64(&MaxLength).StatusCode());
        }
        else
        {
            ReturnErrorOnFailure(bufReader.Read32(&tmpUint32Value).StatusCode());
            MaxLength = tmpUint32Value;
        }
    }

    ReturnErrorOnFailure(bufReader.Read16(&FileDesLength).StatusCode());

    VerifyOrReturnError(bufReader.HasAtLeast(FileDesLength), CHIP_ERROR_MESSAGE_INCOMPLETE);
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

    return CHIP_NO_ERROR;
}

size_t TransferInit::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

#if CHIP_AUTOMATION_LOGGING
void TransferInit::LogMessage(bdx::MessageType messageType) const
{
    char fd[kMaxFileDesignatorLen];
    snprintf(fd, sizeof(fd), "%.*s", static_cast<int>(FileDesLength), FileDesignator);

    switch (messageType)
    {
    case MessageType::SendInit:
        ChipLogAutomation("SendInit");
        break;
    case MessageType::ReceiveInit:
        ChipLogAutomation("ReceiveInit");
        break;
    default:
        break;
    }

    ChipLogAutomation("  Proposed Transfer Control: 0x%X", static_cast<unsigned>(TransferCtlOptions.Raw() | Version));
    ChipLogAutomation("  Range Control: 0x%X", static_cast<unsigned>(mRangeCtlFlags.Raw()));
    ChipLogAutomation("  Proposed Max Block Size: %u", MaxBlockSize);
    ChipLogAutomation("  Start Offset: 0x" ChipLogFormatX64, ChipLogValueX64(StartOffset));
    ChipLogAutomation("  Proposed Max Length: 0x" ChipLogFormatX64, ChipLogValueX64(MaxLength));
    ChipLogAutomation("  File Designator Length: %u", FileDesLength);
    ChipLogAutomation("  File Designator: %s", fd);
}
#endif // CHIP_AUTOMATION_LOGGING

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
    uint8_t transferCtl = 0;
    uint8_t * bufStart  = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    ReturnErrorOnFailure(bufReader.Read8(&transferCtl).Read16(&MaxBlockSize).StatusCode());

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

    return CHIP_NO_ERROR;
}

size_t SendAccept::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

#if CHIP_AUTOMATION_LOGGING
void SendAccept::LogMessage(bdx::MessageType messageType) const
{
    (void) messageType;
    ChipLogAutomation("SendAccept");
    ChipLogAutomation("  Transfer Control: 0x%X", static_cast<unsigned>(TransferCtlFlags.Raw() | Version));
    ChipLogAutomation("  Max Block Size: %u", MaxBlockSize);
}
#endif // CHIP_AUTOMATION_LOGGING

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
    uint8_t transferCtl     = 0;
    uint32_t tmpUint32Value = 0; // Used for reading non-wide length and offset fields
    uint8_t * bufStart      = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    ReturnErrorOnFailure(bufReader.Read8(&transferCtl).Read8(mRangeCtlFlags.RawStorage()).Read16(&MaxBlockSize).StatusCode());

    Version = transferCtl & kVersionMask;

    // Only one of these values should be set. It is up to the caller to verify this.
    TransferCtlFlags.SetRaw(static_cast<uint8_t>(transferCtl & ~kVersionMask));

    StartOffset = 0;
    if (mRangeCtlFlags.Has(RangeControlFlags::kStartOffset))
    {
        if (mRangeCtlFlags.Has(RangeControlFlags::kWiderange))
        {
            ReturnErrorOnFailure(bufReader.Read64(&StartOffset).StatusCode());
        }
        else
        {
            ReturnErrorOnFailure(bufReader.Read32(&tmpUint32Value).StatusCode());
            StartOffset = tmpUint32Value;
        }
    }

    Length = 0;
    if (mRangeCtlFlags.Has(RangeControlFlags::kDefLen))
    {
        if (mRangeCtlFlags.Has(RangeControlFlags::kWiderange))
        {
            ReturnErrorOnFailure(bufReader.Read64(&Length).StatusCode());
        }
        else
        {
            ReturnErrorOnFailure(bufReader.Read32(&tmpUint32Value).StatusCode());
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

    return CHIP_NO_ERROR;
}

size_t ReceiveAccept::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

#if CHIP_AUTOMATION_LOGGING
void ReceiveAccept::LogMessage(bdx::MessageType messageType) const
{
    (void) messageType;
    ChipLogAutomation("ReceiveAccept");
    ChipLogAutomation("  Transfer Control: 0x%X", TransferCtlFlags.Raw() | Version);
    ChipLogAutomation("  Range Control: 0x%X", mRangeCtlFlags.Raw());
    ChipLogAutomation("  Max Block Size: %u", MaxBlockSize);
    ChipLogAutomation("  Length: 0x" ChipLogFormatX64, ChipLogValueX64(Length));
}
#endif // CHIP_AUTOMATION_LOGGING

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

#if CHIP_AUTOMATION_LOGGING
void CounterMessage::LogMessage(bdx::MessageType messageType) const
{
    switch (messageType)
    {
    case MessageType::BlockQuery:
        ChipLogAutomation("BlockQuery");
        break;
    case MessageType::BlockAck:
        ChipLogAutomation("BlockAck");
        break;
    case MessageType::BlockAckEOF:
        ChipLogAutomation("BlockAckEOF");
        break;
    default:
        break;
    }

    ChipLogAutomation("  Block Counter: %" PRIu32, BlockCounter);
}
#endif // CHIP_AUTOMATION_LOGGING

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
    uint8_t * bufStart = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    ReturnErrorOnFailure(bufReader.Read32(&BlockCounter).StatusCode());

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

    return CHIP_NO_ERROR;
}

size_t DataBlock::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

#if CHIP_AUTOMATION_LOGGING
void DataBlock::LogMessage(bdx::MessageType messageType) const
{
    switch (messageType)
    {
    case MessageType::Block:
        ChipLogAutomation("Block");
        break;
    case MessageType::BlockEOF:
        ChipLogAutomation("BlockEOF");
        break;
    default:
        break;
    }

    ChipLogAutomation("  Block Counter: %" PRIu32, BlockCounter);
    ChipLogAutomation("  Data Length: %u", static_cast<unsigned int>(DataLength));
}
#endif // CHIP_AUTOMATION_LOGGING

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

// WARNING: this function should never return early, since MessageSize() relies on it to calculate
// the size of the message (even if the message is incomplete or filled out incorrectly).
Encoding::LittleEndian::BufferWriter & BlockQueryWithSkip::WriteToBuffer(Encoding::LittleEndian::BufferWriter & aBuffer) const
{
    aBuffer.Put32(BlockCounter);
    aBuffer.Put64(BytesToSkip);
    return aBuffer;
}

CHIP_ERROR BlockQueryWithSkip::Parse(System::PacketBufferHandle aBuffer)
{
    uint8_t * bufStart = aBuffer->Start();
    Reader bufReader(bufStart, aBuffer->DataLength());

    return bufReader.Read32(&BlockCounter).Read64(&BytesToSkip).StatusCode();
}

size_t BlockQueryWithSkip::MessageSize() const
{
    BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

bool BlockQueryWithSkip::operator==(const BlockQueryWithSkip & another) const
{
    return (BlockCounter == another.BlockCounter && BytesToSkip == another.BytesToSkip);
}

#if CHIP_AUTOMATION_LOGGING
void BlockQueryWithSkip::LogMessage(bdx::MessageType messageType) const
{
    ChipLogAutomation("BlockQueryWithSkip");
    ChipLogAutomation("  Block Counter: %" PRIu32, BlockCounter);
    ChipLogAutomation("  Bytes To Skip: %" PRIu64, BytesToSkip);
}
#endif // CHIP_AUTOMATION_LOGGING
