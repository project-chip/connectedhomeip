/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "Decoder.h"

#include "../logging/Log.h"

#include <lib/core/CHIPSafeCasts.h>
#include <protocols/bdx/BdxMessages.h>

namespace {
constexpr char kProtocolName[] = "Bulk Data Exchange";

constexpr char kUnknown[]            = "Unknown";
constexpr char kSendInit[]           = "Send Init";
constexpr char kSendAccept[]         = "Send Accept";
constexpr char kReceiveInit[]        = "Receive Init";
constexpr char kReceiveAccept[]      = "Receive Accept";
constexpr char kBlockQuery[]         = "Block Query";
constexpr char kBlock[]              = "Block";
constexpr char kBlockEOF[]           = "Block End Of File";
constexpr char kBlockAck[]           = "Block Ack";
constexpr char kBlockAckEOF[]        = "Block Ack End Of File";
constexpr char kBlockQueryWithSkip[] = "Block Query With Skip";

constexpr char kDataHeader[] = "Data";
} // namespace

using MessageType          = chip::bdx::MessageType;
using RangeControlFlags    = chip::bdx::RangeControlFlags;
using TransferControlFlags = chip::bdx::TransferControlFlags;

namespace chip {
namespace trace {
namespace bdx {

using namespace logging;

CHIP_ERROR DecodeTransferInit(System::PacketBufferHandle msgData);
CHIP_ERROR DecodeSendAccept(System::PacketBufferHandle msgData);
CHIP_ERROR DecodeReceiveAccept(System::PacketBufferHandle msgData);
CHIP_ERROR DecodeBlockCounter(System::PacketBufferHandle msgData);
CHIP_ERROR DecodeDataBlock(System::PacketBufferHandle msgData);
CHIP_ERROR DecodeBlockQueryWithSkip(System::PacketBufferHandle msgData);
void DecodeAndPrintTransferControl(const char * header, BitFlags<TransferControlFlags> & flags);
void DecodeAndPrintRangeControl(const char * header, BitFlags<RangeControlFlags> & flags);
void DecodeAndPrintMetadata(const ByteSpan & data);

const char * ToProtocolName()
{
    return kProtocolName;
}

const char * ToProtocolMessageTypeName(uint8_t protocolCode)
{
    switch (protocolCode)
    {
    case to_underlying(MessageType::SendInit):
        return kSendInit;
    case to_underlying(MessageType::SendAccept):
        return kSendAccept;
    case to_underlying(MessageType::ReceiveInit):
        return kReceiveInit;
    case to_underlying(MessageType::ReceiveAccept):
        return kReceiveAccept;
    case to_underlying(MessageType::BlockQuery):
        return kBlockQuery;
    case to_underlying(MessageType::Block):
        return kBlock;
    case to_underlying(MessageType::BlockEOF):
        return kBlockEOF;
    case to_underlying(MessageType::BlockAck):
        return kBlockAck;
    case to_underlying(MessageType::BlockAckEOF):
        return kBlockAckEOF;
    case to_underlying(MessageType::BlockQueryWithSkip):
        return kBlockQueryWithSkip;
    }

    return kUnknown;
}

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len)
{
    auto msgData = System::PacketBufferHandle::NewWithData(data, len);

    switch (protocolCode)
    {
    case to_underlying(MessageType::SendInit):
        return DecodeTransferInit(std::move(msgData));
    case to_underlying(MessageType::SendAccept):
        return DecodeSendAccept(std::move(msgData));
    case to_underlying(MessageType::ReceiveInit):
        return DecodeTransferInit(std::move(msgData));
    case to_underlying(MessageType::ReceiveAccept):
        return DecodeReceiveAccept(std::move(msgData));
    case to_underlying(MessageType::BlockQuery):
        return DecodeBlockCounter(std::move(msgData));
    case to_underlying(MessageType::Block):
        return DecodeDataBlock(std::move(msgData));
    case to_underlying(MessageType::BlockEOF):
        return DecodeDataBlock(std::move(msgData));
    case to_underlying(MessageType::BlockAck):
        return DecodeBlockCounter(std::move(msgData));
    case to_underlying(MessageType::BlockAckEOF):
        return DecodeBlockCounter(std::move(msgData));
    case to_underlying(MessageType::BlockQueryWithSkip):
        return DecodeBlockQueryWithSkip(std::move(msgData));
    }

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DecodeTransferInit(System::PacketBufferHandle msgData)
{
    auto scopedIndent = ScopedLogIndentWithSize(kDataHeader, msgData->DataLength());

    chip::bdx::TransferInit msg;
    ReturnErrorOnFailure(msg.Parse(msgData.Retain()));

    auto ptc      = msg.TransferCtlOptions;
    auto rc       = msg.mRangeCtlFlags;
    auto pmbs     = msg.MaxBlockSize;
    auto startofs = msg.StartOffset;
    auto len      = msg.MaxLength;
    auto fdl      = msg.FileDesLength;
    auto fd       = CharSpan(Uint8::to_const_char(msg.FileDesignator), msg.FileDesLength);
    auto mdata    = ByteSpan(msg.Metadata, msg.MetadataLength);

    DecodeAndPrintTransferControl("Proposed Transfer Control", ptc);
    DecodeAndPrintRangeControl("Range Control", rc);
    Log("Proposed Max Block Size", pmbs);

    if (rc.Has(RangeControlFlags::kStartOffset))
    {
        LogAsHex("Start Offset", startofs);
    }

    if (rc.Has(RangeControlFlags::kDefLen))
    {
        LogAsHex("Proposed Max Length", len);
    }

    Log("File Designator Length", fdl);
    Log("File Designator", fd);
    DecodeAndPrintMetadata(mdata);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeSendAccept(System::PacketBufferHandle msgData)
{
    auto scopedIndent = ScopedLogIndentWithSize(kDataHeader, msgData->DataLength());

    chip::bdx::SendAccept msg;
    ReturnErrorOnFailure(msg.Parse(msgData.Retain()));

    auto tc    = msg.TransferCtlFlags;
    auto mbs   = msg.MaxBlockSize;
    auto mdata = ByteSpan(msg.Metadata, msg.MetadataLength);

    DecodeAndPrintTransferControl("Transfer Control", tc);
    Log("Max Block Size", mbs);
    DecodeAndPrintMetadata(mdata);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeReceiveAccept(System::PacketBufferHandle msgData)
{
    auto scopedIndent = ScopedLogIndentWithSize(kDataHeader, msgData->DataLength());

    chip::bdx::ReceiveAccept msg;
    ReturnErrorOnFailure(msg.Parse(msgData.Retain()));

    auto tc    = msg.TransferCtlFlags;
    auto rc    = msg.mRangeCtlFlags;
    auto mbs   = msg.MaxBlockSize;
    auto len   = msg.Length;
    auto mdata = ByteSpan(msg.Metadata, msg.MetadataLength);

    DecodeAndPrintTransferControl("Transfer Control", tc);
    DecodeAndPrintRangeControl("Range Control", rc);
    Log("Max Block Size", mbs);

    if (rc.Has(RangeControlFlags::kDefLen))
    {
        LogAsHex("Length", len);
    }

    DecodeAndPrintMetadata(mdata);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeBlockCounter(System::PacketBufferHandle msgData)
{
    auto scopedIndent = ScopedLogIndent(kDataHeader);

    chip::bdx::CounterMessage msg;
    ReturnErrorOnFailure(msg.Parse(msgData.Retain()));

    Log("BlockCounter", msg.BlockCounter);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeDataBlock(System::PacketBufferHandle msgData)
{
    auto scopedIndent = ScopedLogIndentWithSize(kDataHeader, msgData->DataLength());

    chip::bdx::DataBlock msg;
    ReturnErrorOnFailure(msg.Parse(msgData.Retain()));

    Log("BlockCounter", msg.BlockCounter);

    auto data = ByteSpan(msg.Data, msg.DataLength);
    Log("Data", data);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeBlockQueryWithSkip(System::PacketBufferHandle msgData)
{
    auto scopedIndent = ScopedLogIndent(kDataHeader);

    chip::bdx::BlockQueryWithSkip msg;
    ReturnErrorOnFailure(msg.Parse(msgData.Retain()));

    Log("BlockCounter", msg.BlockCounter);
    LogAsHex("BytesToSkip", msg.BytesToSkip);

    return CHIP_NO_ERROR;
}

void DecodeAndPrintTransferControl(const char * header, BitFlags<TransferControlFlags> & flags)
{
    auto scopedIndent = ScopedLogIndentWithFlags(header, flags.Raw());

    if (flags.Has(TransferControlFlags::kSenderDrive))
    {
        Log("SenderDrive");
    }

    if (flags.Has(TransferControlFlags::kReceiverDrive))
    {
        Log("ReceivedDrive");
    }

    if (flags.Has(TransferControlFlags::kAsync))
    {
        Log("Async");
    }
}

void DecodeAndPrintRangeControl(const char * header, BitFlags<RangeControlFlags> & flags)
{
    auto scopedIndent = ScopedLogIndentWithFlags(header, flags.Raw());

    if (flags.Has(chip::bdx::RangeControlFlags::kDefLen))
    {
        Log("DefLen");
    }

    if (flags.Has(RangeControlFlags::kStartOffset))
    {
        Log("StartOffset");
    }

    if (flags.Has(RangeControlFlags::kWiderange))
    {
        Log("Widerange");
    }
}

void DecodeAndPrintMetadata(const ByteSpan & data)
{
    if (data.size())
    {
        // TODO Make metadata decoding prettier.
        Log("HasMetadata");
    }
}

} // namespace bdx
} // namespace trace
} // namespace chip
