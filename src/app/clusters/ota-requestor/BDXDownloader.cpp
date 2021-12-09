/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "BDXDownloader.h"

#include <lib/core/CHIPError.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <protocols/bdx/BdxMessages.h>
#include <system/SystemClock.h> /* TODO:(#12520) remove */
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

using chip::OTADownloader;
using chip::bdx::TransferSession;

namespace chip {

void BDXDownloader::OnMessageReceived(const chip::PayloadHeader & payloadHeader, chip::System::PacketBufferHandle msg)
{
    VerifyOrReturn(mState == State::kInProgress, ChipLogError(BDX, "Can't accept messages, no transfer in progress"));
    CHIP_ERROR err =
        mBdxTransfer.HandleMessageReceived(payloadHeader, std::move(msg), /* TODO:(#12520) */ chip::System::Clock::Seconds16(0));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "unable to handle message: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // HandleMessageReceived() will only decode/parse the message. Need to Poll() in order to do the message handling work in
    // HandleBdxEvent().
    PollTransferSession();
}

CHIP_ERROR BDXDownloader::SetBDXParams(const chip::bdx::TransferSession::TransferInitData & bdxInitData)
{
    VerifyOrReturnError(mState == State::kIdle, CHIP_ERROR_INCORRECT_STATE);

    // Must call StartTransfer() here to store the the pointer data contained in bdxInitData in the TransferSession object.
    // Otherwise it could be freed before we can use it.
    ReturnErrorOnFailure(mBdxTransfer.StartTransfer(chip::bdx::TransferRole::kReceiver, bdxInitData,
                                                    /* TODO:(#12520) */ chip::System::Clock::Seconds16(30)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDownloader::BeginPrepareDownload()
{
    VerifyOrReturnError(mState == State::kIdle, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mImageProcessor->PrepareDownload());

    mState = State::kPreparing;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDownloader::OnPreparedForDownload(CHIP_ERROR status)
{
    VerifyOrReturnError(mState == State::kPreparing, CHIP_ERROR_INCORRECT_STATE);

    if (status == CHIP_NO_ERROR)
    {
        mState = State::kInProgress;

        // Must call here because StartTransfer() should have prepared a ReceiveInit message, and now we should send it.
        PollTransferSession();
    }
    else
    {
        ChipLogError(BDX, "failed to prepare download: %" CHIP_ERROR_FORMAT, status.Format());
        mBdxTransfer.Reset();
        mState = State::kIdle;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDownloader::FetchNextData()
{
    VerifyOrReturnError(mState == State::kInProgress, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQuery());
    PollTransferSession();

    return CHIP_NO_ERROR;
}

void BDXDownloader::OnDownloadTimeout()
{
    if (mState == State::kInProgress)
    {
        ChipLogDetail(BDX, "aborting due to timeout");
        mBdxTransfer.Reset();
        if (mImageProcessor != nullptr)
        {
            mImageProcessor->Abort();
        }
        mState = State::kIdle;
    }
    else
    {
        ChipLogError(BDX, "no download in progress");
    }
}

void BDXDownloader::EndDownload(CHIP_ERROR reason)
{
    if (mState == State::kInProgress)
    {
        // There's no method for a BDX receiving driver to cleanly end a transfer
        mBdxTransfer.AbortTransfer(chip::bdx::StatusCode::kUnknown);
        if (mImageProcessor != nullptr)
        {
            mImageProcessor->Abort();
        }
        mState = State::kIdle;

        // Because AbortTransfer() will generate a StatusReport to send.
        PollTransferSession();
    }
    else
    {
        ChipLogError(BDX, "no download in progress");
    }
}

void BDXDownloader::PollTransferSession()
{
    TransferSession::OutputEvent outEvent;

    // WARNING: Is this dangerous? What happens if the loop encounters two messages that need to be sent? Does the ExchangeContext
    // allow that?
    do
    {
        mBdxTransfer.PollOutput(outEvent, /* TODO:(#12520) */ chip::System::Clock::Seconds16(0));
        CHIP_ERROR err = HandleBdxEvent(outEvent);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "HandleBDXEvent: %" CHIP_ERROR_FORMAT, err.Format()));
    } while (outEvent.EventType != TransferSession::OutputEventType::kNone);
}

CHIP_ERROR BDXDownloader::HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent)
{
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);

    switch (outEvent.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
        ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQuery());
        // TODO: need to check ReceiveAccept parameters
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        VerifyOrReturnError(mMsgDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mMsgDelegate->SendMessage(outEvent));
        if (outEvent.msgTypeData.HasMessageType(chip::bdx::MessageType::BlockAckEOF))
        {
            // BDX transfer is not complete until BlockAckEOF has been sent
            mState = State::kComplete;

            // TODO: how/when to reset the BDXDownloader to be ready to handle another download
        }
        break;
    }
    case TransferSession::OutputEventType::kBlockReceived: {
        chip::ByteSpan blockData(outEvent.blockdata.Data, outEvent.blockdata.Length);
        ReturnErrorOnFailure(mImageProcessor->ProcessBlock(blockData));

        // TODO: this will cause problems if Finalize() is not guaranteed to do its work after ProcessBlock().
        if (outEvent.blockdata.IsEof)
        {
            mBdxTransfer.PrepareBlockAck();
            ReturnErrorOnFailure(mImageProcessor->Finalize());
        }

        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "BDX StatusReport %x", static_cast<uint16_t>(outEvent.statusData.statusCode));
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "TransferSession error");
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "Unexpected BDX event: %u", static_cast<uint16_t>(outEvent.EventType));
        break;
    }

    return CHIP_NO_ERROR;
}

// TODO: Add unit tests for parsing BDX URI
CHIP_ERROR BDXDownloader::ParseBdxUri(CharSpan uri, NodeId & nodeId, MutableCharSpan fileDesignator)
{
    // Check against minimum length of a valid BDX URI
    if (uri.size() < kValidBdxUriMinLen)
    {
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    uint8_t readValue[kUriMaxLen];
    Encoding::LittleEndian::Reader uriReader(reinterpret_cast<const uint8_t *>(uri.data()), uri.size());

    // Check the scheme field matches the BDX prefix
    memset(readValue, 0, sizeof(readValue));
    ReturnErrorOnFailure(uriReader.ReadBytes(readValue, sizeof(bdxPrefix)).StatusCode());
    ByteSpan expectedScheme(bdxPrefix, sizeof(bdxPrefix));
    ByteSpan actualScheme(readValue, sizeof(bdxPrefix));
    if (!expectedScheme.data_equal(actualScheme))
    {
        return CHIP_ERROR_INVALID_SCHEME_PREFIX;
    }

    // Extract the node ID from the authority field
    memset(readValue, 0, sizeof(readValue));
    ReturnErrorOnFailure(uriReader.ReadBytes(readValue, kNodeIdHexStringLen).StatusCode());
    uint8_t buffer[kNodeIdHexStringLen];
    if (Encoding::HexToBytes(reinterpret_cast<const char *>(readValue), kNodeIdHexStringLen, buffer, kNodeIdHexStringLen) == 0)
    {
        return CHIP_ERROR_INVALID_DESTINATION_NODE_ID;
    }
    nodeId = Encoding::BigEndian::Get64(buffer);
    if (!IsOperationalNodeId(nodeId))
    {
        return CHIP_ERROR_INVALID_DESTINATION_NODE_ID;
    }

    // Verify the separator between authority and path fields
    memset(readValue, 0, sizeof(readValue));
    ReturnErrorOnFailure(uriReader.ReadBytes(readValue, sizeof(bdxSeparator)).StatusCode());
    ByteSpan expectedSeparator(bdxSeparator, sizeof(bdxSeparator));
    ByteSpan actualSeparator(readValue, sizeof(bdxSeparator));
    if (!expectedSeparator.data_equal(actualSeparator))
    {
        return CHIP_ERROR_MISSING_URI_SEPARATOR;
    }

    // Extract file designator from the path field
    size_t fileDesignatorLength = uriReader.Remaining();
    memset(readValue, 0, sizeof(readValue));
    ReturnErrorOnFailure(uriReader.ReadBytes(readValue, fileDesignatorLength).StatusCode());
    size_t written = static_cast<size_t>(snprintf(fileDesignator.data(), fileDesignator.size(), "%s", readValue));
    if (written != fileDesignatorLength)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
