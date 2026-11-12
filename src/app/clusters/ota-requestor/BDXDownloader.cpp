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
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/core/OTAImageHeader.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
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

namespace {
constexpr uint8_t kMaxResumeAttemptsWithoutProgress = 3;
} // namespace

void TransferTimeoutCheckHandler(System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    BDXDownloader * bdxDownloader = static_cast<BDXDownloader *>(appState);

    if (bdxDownloader->HasTransferTimedOut())
    {
        // End download if transfer timeout has been detected
        bdxDownloader->OnDownloadTimeout();
    }
    else
    {
        // Else restart the timer
        TEMPORARY_RETURN_IGNORED systemLayer->StartTimer(bdxDownloader->GetTimeout(), TransferTimeoutCheckHandler, appState);
    }
}

System::Clock::Timeout BDXDownloader::GetTimeout()
{
    return mTimeout;
}

void BDXDownloader::Reset()
{
    mPrevBlockCounter = 0;
    DeviceLayer::SystemLayer().CancelTimer(TransferTimeoutCheckHandler, this);
}

bool BDXDownloader::HasTransferTimedOut()
{
    uint32_t curBlockCounter = mBdxTransfer.GetNextQueryNum();

    if (curBlockCounter > mPrevBlockCounter)
    {
        mPrevBlockCounter = curBlockCounter;
        return false;
    }

    ChipLogError(BDX, "BDX transfer timeout");
    return true;
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
    mTimeout          = timeout;
    mState            = State::kIdle;
    mRestartFromStart = false;
    mFinalized        = false;
    mBlockAckEOFSent  = false;
    mFinalizeResult.reset();
    mCheckFirstBlock = false;
    mBdxTransfer.Reset();

    VerifyOrReturnError(mState == State::kIdle, CHIP_ERROR_INCORRECT_STATE);

    // A partial image is resumed only when it was downloaded for the same version, provider and file designator.
    const std::optional<ImageIdentity> identity =
        ComputeImageIdentity(ByteSpan(bdxInitData.FileDesignator, bdxInitData.FileDesLength));
    if (!identity.has_value() || identity != mResumeIdentity)
    {
        DiscardPartialImage();
    }
    mResumeIdentity = identity;

    chip::bdx::TransferSession::TransferInitData initData = bdxInitData;
    initData.StartOffset                                  = ChooseStartOffset();
    mRequestedOffset                                      = initData.StartOffset;

    // Must call StartTransfer() here to store the the pointer data contained in bdxInitData in the TransferSession object.
    // Otherwise it could be freed before we can use it.
    ReturnErrorOnFailure(mBdxTransfer.StartTransfer(chip::bdx::TransferRole::kReceiver, initData,
                                                    /* TODO:(#12520) */ chip::System::Clock::Seconds16(30)));

    return CHIP_NO_ERROR;
}

void BDXDownloader::SetImageIdentity(uint32_t softwareVersion, const ScopedNodeId & provider)
{
    mImageVersion.emplace(softwareVersion);
    mImageProvider = provider;
}

std::optional<BDXDownloader::ImageIdentity> BDXDownloader::ComputeImageIdentity(ByteSpan fileDesignator) const
{
    VerifyOrReturnValue(mImageVersion.has_value(), std::nullopt);

    uint8_t fields[sizeof(uint32_t) + sizeof(NodeId) + sizeof(FabricIndex)];
    Encoding::LittleEndian::BufferWriter writer(fields, sizeof(fields));
    writer.Put32(*mImageVersion).Put64(mImageProvider.GetNodeId()).Put8(mImageProvider.GetFabricIndex());
    VerifyOrReturnValue(writer.Fit(), std::nullopt);

    ImageIdentity identity{};
    MutableByteSpan identitySpan(identity.data(), identity.size());
    Crypto::Hash_SHA256_stream hash;
    VerifyOrReturnValue(hash.Begin() == CHIP_NO_ERROR, std::nullopt);
    VerifyOrReturnValue(hash.AddData(ByteSpan(fields, writer.Needed())) == CHIP_NO_ERROR, std::nullopt);
    VerifyOrReturnValue(hash.AddData(fileDesignator) == CHIP_NO_ERROR, std::nullopt);
    VerifyOrReturnValue(hash.Finish(identitySpan) == CHIP_NO_ERROR, std::nullopt);
    return identity;
}

void BDXDownloader::DiscardPartialImage()
{
    VerifyOrReturn(mImageProcessor != nullptr && mImageProcessor->GetResumeOffset() > 0);
    LogErrorOnFailure(mImageProcessor->Abort());
    mDiscardPending = true;
}

uint64_t BDXDownloader::ChooseStartOffset()
{
    uint64_t offset = (mImageProcessor != nullptr && !mDiscardPending) ? mImageProcessor->GetResumeOffset() : 0;
    mDiscardPending = false;
    if (offset == 0)
    {
        mAttemptsWithoutProgress = 0;
        return 0;
    }
    mAttemptsWithoutProgress = (offset == mRequestedOffset) ? static_cast<uint8_t>(mAttemptsWithoutProgress + 1) : 0;
    if (mAttemptsWithoutProgress >= kMaxResumeAttemptsWithoutProgress)
    {
        ChipLogProgress(BDX, "No progress resuming at offset 0x" ChipLogFormatX64 ", starting anew", ChipLogValueX64(offset));
        LogErrorOnFailure(mImageProcessor->Abort());
        mAttemptsWithoutProgress = 0;
        return 0;
    }
    ChipLogProgress(BDX, "Resuming download at offset 0x" ChipLogFormatX64, ChipLogValueX64(offset));
    return offset;
}

CHIP_ERROR BDXDownloader::BeginPrepareDownload()
{
    VerifyOrReturnError(mState == State::kIdle, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mPrevBlockCounter = 0;

    // Note that due to the nature of this timer function, the actual time taken to detect a stalled BDX connection might be
    // anywhere in the range of [mTimeout, 2*mTimeout)
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(mTimeout, TransferTimeoutCheckHandler, this);

    // The processor keeps its partial image only when told to continue it from the offset requested from the provider, so the
    // two cannot disagree even when an earlier SuspendDownload() or Abort() has not completed yet.
    ReturnErrorOnFailure(mImageProcessor->PrepareDownloadFrom(mRequestedOffset));

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
        Reset();
        mBdxTransfer.Reset();
        SetState(State::kIdle, OTAChangeReasonEnum::kFailure);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDownloader::OnFinalized(CHIP_ERROR status)
{
    VerifyOrReturnError(mState == State::kInProgress && mFinalized && mImageProcessor != nullptr &&
                            mImageProcessor->ReportsFinalizeResult(),
                        CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(!mFinalizeResult.has_value(), CHIP_ERROR_INCORRECT_STATE);

    // Only record the result here. FinishIfFinalized() acts on it once BlockAckEOF has also been sent, so the order in which
    // the result and BlockAckEOF arrive does not matter, and a failure reported inside Finalize() still lets BlockAckEOF go out.
    mFinalizeResult = status;
    FinishIfFinalized();
    return CHIP_NO_ERROR;
}

void BDXDownloader::FinishIfFinalized()
{
    VerifyOrReturn(mState == State::kInProgress && mBlockAckEOFSent);

    const bool reportsResult = mImageProcessor != nullptr && mImageProcessor->ReportsFinalizeResult();
    VerifyOrReturn(!reportsResult || mFinalizeResult.has_value());

    Reset();
    // A finished image is not a partial one, so a later EndDownload() leaves it alone. The next SetBDXParams() sets
    // mRequestedOffset again, and the processor reports no resume offset, which clears mAttemptsWithoutProgress.
    mResumeIdentity.reset();

    if (!reportsResult || *mFinalizeResult == CHIP_NO_ERROR)
    {
        SetState(State::kComplete, OTAChangeReasonEnum::kSuccess);
        return;
    }

    // The image failed the processor's checks, e.g. its digest does not match its header. End the download as failed, which
    // discards the image, before the requestor is told it is downloaded and asks the provider to apply it.
    ChipLogError(BDX, "Downloaded image failed its checks: %" CHIP_ERROR_FORMAT, mFinalizeResult->Format());
    mBdxTransfer.Reset();
    SetState(State::kIdle, OTAChangeReasonEnum::kFailure);
    LogErrorOnFailure(mImageProcessor->Abort());
}

CHIP_ERROR BDXDownloader::FetchNextData()
{
    VerifyOrReturnError(mState == State::kInProgress, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQuery());
    PollTransferSession();

    return CHIP_NO_ERROR;
}

CHIP_ERROR BDXDownloader::SkipData(uint32_t numBytes)
{
    VerifyOrReturnError(mState == State::kInProgress, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQueryWithSkip(numBytes));
    PollTransferSession();

    return CHIP_NO_ERROR;
}

void BDXDownloader::OnDownloadTimeout()
{
    Reset();

    if (mState == State::kInProgress)
    {
        ChipLogDetail(BDX, "aborting due to timeout");
        mBdxTransfer.Reset();
        if (mImageProcessor != nullptr && mFinalized)
        {
            // The whole image was received and finalized, but the transfer did not complete, e.g. because BlockAckEOF could not
            // be sent. A finalized image is not a partial one, so it is discarded instead of being resumed past its end.
            mResumeIdentity.reset();
            LogErrorOnFailure(mImageProcessor->Abort());
        }
        else if (mImageProcessor != nullptr)
        {
            LogErrorOnFailure(mImageProcessor->SuspendDownload());
        }
        SetState(State::kIdle, OTAChangeReasonEnum::kTimeOut);
    }
    else
    {
        ChipLogError(BDX, "No download in progress");
    }
}

void BDXDownloader::EndDownload(CHIP_ERROR reason)
{
    Reset();

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
        TEMPORARY_RETURN_IGNORED mBdxTransfer.AbortTransfer(status);
        if (mImageProcessor != nullptr)
        {
            TEMPORARY_RETURN_IGNORED mImageProcessor->Abort();
        }
        // Because AbortTransfer() will generate a StatusReport to send.
        PollTransferSession();

        // Now that we've sent our report, we're idle.
        SetState(State::kIdle, OTAChangeReasonEnum::kSuccess);
    }
    else if (mState == State::kPreparing)
    {
        // The prepare is still queued with the processor. Leaving the state at kPreparing would let OnPreparedForDownload() start a
        // transfer into a file that the abort queued here is about to remove, so go idle: it then reports the prepare as stale.
        mBdxTransfer.Reset();
        mResumeIdentity.reset();
        if (mImageProcessor != nullptr)
        {
            LogErrorOnFailure(mImageProcessor->Abort());
        }
        SetState(State::kIdle, OTAChangeReasonEnum::kSuccess);
    }
    else if (mResumeIdentity.has_value() && mImageProcessor != nullptr)
    {
        // Discard a suspended download. The suspend may not have completed yet, so abort without checking GetResumeOffset(); the
        // abort completes after it. Forgetting the identity keeps the next download from resuming either way.
        mResumeIdentity.reset();
        LogErrorOnFailure(mImageProcessor->Abort());
    }
    else
    {
        ChipLogError(BDX, "No download in progress");
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

void BDXDownloader::CleanupOnError(OTAChangeReasonEnum reason)
{
    // A restart is not an error of this attempt: the provider cannot continue the partial image, so the same image is downloaded
    // again from offset 0 instead of reporting a failure and waiting for the next query.
    const bool restart = mRestartFromStart;
    mRestartFromStart  = false;

    Reset();
    mBdxTransfer.Reset();
    SetState(State::kIdle, restart ? OTAChangeReasonEnum::kSuccess : reason);
    if (mImageProcessor)
    {
        TEMPORARY_RETURN_IGNORED mImageProcessor->Abort();
    }
    if (restart && mStateDelegate != nullptr)
    {
        mStateDelegate->OnDownloadRestartRequested();
    }
}

CHIP_ERROR BDXDownloader::HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent)
{
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);

    switch (outEvent.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
        // A ReceiveAccept has no start offset field, so accepting the transfer means accepting the requested start offset. The
        // StartOffset of the accept data is not checked: a provider that follows the specification leaves it out (it parses as
        // 0), and an SDK provider echoes the request whether or not it serves data from there. A provider that serves data from
        // elsewhere is caught by the check of the first block below, or by the size and digest in the image header.
        mCheckFirstBlock = mRequestedOffset > 0;
        ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQuery());
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        VerifyOrReturnError(mMsgDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mMsgDelegate->SendMessage(outEvent));
        if (outEvent.msgTypeData.HasMessageType(chip::bdx::MessageType::BlockAckEOF))
        {
            // BDX transfer is not complete until BlockAckEOF has been sent. A processor that reports the result of Finalize() may
            // not have done so yet; the download then finishes when it reports. Until then the timeout keeps running, so a result
            // that never arrives ends the download like a stalled transfer.
            mBlockAckEOFSent = true;
            FinishIfFinalized();
        }
        break;
    }
    case TransferSession::OutputEventType::kBlockReceived: {
        chip::ByteSpan blockData(outEvent.blockdata.Data, outEvent.blockdata.Length);
        if (mCheckFirstBlock && StartsWithImageHeader(blockData))
        {
            // Data from a non-zero offset starts with the file identifier of an OTA image header: the provider accepted the
            // offset but serves the image from its start. Stop at the first block, before the processor appends it to the
            // partial image, and download again from offset 0. If the data at the offset happens to look like a header, this
            // costs a download from 0 and nothing else. The abort ends in a kInternalError event of the same poll, which runs
            // CleanupOnError(); the restart requests offset 0, which is never checked, so it cannot loop.
            ChipLogError(BDX,
                         "Provider serves the image from its start instead of offset 0x" ChipLogFormatX64 ", restarting from 0",
                         ChipLogValueX64(mRequestedOffset));
            mCheckFirstBlock = false;
            LogErrorOnFailure(mBdxTransfer.AbortTransfer(bdx::StatusCode::kBadMessageContents));
            mDiscardPending   = true;
            mRestartFromStart = true;
            break;
        }
        mCheckFirstBlock = false;
        ReturnErrorOnFailure(mImageProcessor->ProcessBlock(blockData));
        mStateDelegate->OnUpdateProgressChanged(mImageProcessor->GetPercentComplete());

        // TODO: this will cause problems if Finalize() is not guaranteed to do its work after ProcessBlock().
        if (outEvent.blockdata.IsEof)
        {
            TEMPORARY_RETURN_IGNORED mBdxTransfer.PrepareBlockAck();
            // Set before Finalize(), which may report its result to OnFinalized() before it returns.
            mFinalized = true;
            ReturnErrorOnFailure(mImageProcessor->Finalize());
        }

        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "BDX StatusReport %x", static_cast<uint16_t>(outEvent.statusData.statusCode));
        if (outEvent.statusData.statusCode == bdx::StatusCode::kStartOffsetNotSupported && mRequestedOffset > 0)
        {
            // The provider cannot continue the partial image, e.g. it does not support a start offset. Fall back to offset 0.
            ChipLogProgress(BDX, "Provider rejected start offset 0x" ChipLogFormatX64 ", restarting from offset 0",
                            ChipLogValueX64(mRequestedOffset));
            mDiscardPending   = true;
            mRestartFromStart = true;
        }
        CleanupOnError(OTAChangeReasonEnum::kFailure);
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "TransferSession error");
        CleanupOnError(OTAChangeReasonEnum::kFailure);
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        CleanupOnError(OTAChangeReasonEnum::kTimeOut);
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

bool BDXDownloader::StartsWithImageHeader(ByteSpan data)
{
    return data.size() >= sizeof(uint32_t) && Encoding::LittleEndian::Get32(data.data()) == kOTAImageFileIdentifier;
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
