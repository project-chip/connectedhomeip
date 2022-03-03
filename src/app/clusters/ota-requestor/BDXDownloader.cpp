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

#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/BdxMessages.h>
#include <system/SystemClock.h> /* TODO:(#12520) remove */
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

using chip::OTADownloader;
using chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAChangeReasonEnum;
using chip::app::DataModel::Nullable;
using chip::bdx::TransferSession;

namespace chip {

// Timeout value in seconds to abort the download if there's no progress in the transfer session.
System::Clock::Timeout mTimeout = System::Clock::kZero;

static uint8_t prevPercentageComplete = 0;

void StartTimeoutTimerHandler(System::Layer * systemLayer, void * appState)
{
    if (static_cast<BDXDownloader *>(appState)->CheckTransferTimeout())
    {
        // End download if transfer timeout has been detected
        static_cast<BDXDownloader *>(appState)->OnDownloadTimeout();
    }
    else
    {
        // Else restart the timer
        systemLayer->StartTimer(mTimeout, StartTimeoutTimerHandler, appState);
    }
}

bool BDXDownloader::CheckTransferTimeout()
{
    uint8_t curPercentageComplete =
        mImageProcessor->GetPercentComplete().IsNull() ? 0 : mImageProcessor->GetPercentComplete().Value();

    if (curPercentageComplete > prevPercentageComplete)
    {
        prevPercentageComplete = curPercentageComplete;
        return false;
    }
    else
    {
        ChipLogError(BDX, "BDX transfer timeout");
        return true;
    }
}

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

CHIP_ERROR BDXDownloader::SetBDXParams(const chip::bdx::TransferSession::TransferInitData & bdxInitData,
                                       System::Clock::Timeout timeout)
{
    mTimeout = timeout;
    mState   = State::kIdle;
    mBdxTransfer.Reset();

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

    prevPercentageComplete = 0;
    DeviceLayer::SystemLayer().StartTimer(mTimeout, StartTimeoutTimerHandler, this);

    ReturnErrorOnFailure(mImageProcessor->PrepareDownload());

    SetState(State::kPreparing, OTAChangeReasonEnum::kSuccess);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDownloader::OnPreparedForDownload(CHIP_ERROR status)
{
    VerifyOrReturnError(mState == State::kPreparing, CHIP_ERROR_INCORRECT_STATE);

    if (status == CHIP_NO_ERROR)
    {
        SetState(State::kInProgress, OTAChangeReasonEnum::kSuccess);

        // Must call here because StartTransfer() should have prepared a ReceiveInit message, and now we should send it.
        PollTransferSession();
    }
    else
    {
        ChipLogError(BDX, "failed to prepare download: %" CHIP_ERROR_FORMAT, status.Format());
        mBdxTransfer.Reset();
        SetState(State::kIdle, OTAChangeReasonEnum::kFailure);
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
    prevPercentageComplete = 0;
    DeviceLayer::SystemLayer().CancelTimer(StartTimeoutTimerHandler, this);

    if (mState == State::kInProgress)
    {
        ChipLogDetail(BDX, "aborting due to timeout");
        mBdxTransfer.Reset();
        if (mImageProcessor != nullptr)
        {
            mImageProcessor->Abort();
        }
        SetState(State::kIdle, OTAChangeReasonEnum::kTimeOut);
    }
    else
    {
        ChipLogError(BDX, "no download in progress");
    }
}

void BDXDownloader::EndDownload(CHIP_ERROR reason)
{
    prevPercentageComplete = 0;
    DeviceLayer::SystemLayer().CancelTimer(StartTimeoutTimerHandler, this);

    if (mState == State::kInProgress)
    {
        bdx::StatusCode status = bdx::StatusCode::kUnknown;
        if (reason == CHIP_ERROR_INVALID_FILE_IDENTIFIER)
        {
            status = bdx::StatusCode::kBadMessageContents;
        }
        else if (reason == CHIP_ERROR_WRITE_FAILED)
        {
            status = bdx::StatusCode::kTransferFailedUnknownError;
        }

        // There is no method for a BDX receiving driver to cleanly end a transfer
        mBdxTransfer.AbortTransfer(status);
        if (mImageProcessor != nullptr)
        {
            mImageProcessor->Abort();
        }
        SetState(State::kIdle, OTAChangeReasonEnum::kSuccess);

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
            prevPercentageComplete = 0;
            DeviceLayer::SystemLayer().CancelTimer(StartTimeoutTimerHandler, this);

            // BDX transfer is not complete until BlockAckEOF has been sent
            SetState(State::kComplete, OTAChangeReasonEnum::kSuccess);
        }
        break;
    }
    case TransferSession::OutputEventType::kBlockReceived: {
        chip::ByteSpan blockData(outEvent.blockdata.Data, outEvent.blockdata.Length);
        ReturnErrorOnFailure(mImageProcessor->ProcessBlock(blockData));
        mStateDelegate->OnUpdateProgressChanged(mImageProcessor->GetPercentComplete());

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
        prevPercentageComplete = 0;
        DeviceLayer::SystemLayer().CancelTimer(StartTimeoutTimerHandler, this);
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        prevPercentageComplete = 0;
        DeviceLayer::SystemLayer().CancelTimer(StartTimeoutTimerHandler, this);
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

void BDXDownloader::SetState(State state, OTAChangeReasonEnum reason)
{
    mState = state;

    if (mStateDelegate)
    {
        mStateDelegate->OnDownloadStateChanged(state, reason);
    }
}

} // namespace chip
