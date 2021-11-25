#include "bdx-downloader.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

using chip::bdx::TransferSession;
using chip::bdx::TransferSession::OutputEventType;

void BdxDownloader::OnMessageReceived(const PayloadHeader & payloadHeader, System::PacketBufferHandle msg)
{
    VerifyOrReturnError(mState == kInProgress, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err = mBdxTransfer.HandleMessageReceived(payloadHeader, msg, /* TODO:(#12520) */ 0);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "unable to handle message: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

CHIP_ERROR BdxDownloader::SetBDXParams(const chip::bdx::TransferSession::TransferInitData & bdxInitData)
{
    VerifyOrReturnError(mState == kIdle, CHIP_ERROR_INCORRECT_STATE);

    // Must call StartTransfer() here or otherwise the pointer data contained in bdxInitData could be freed before we can use it.
    ReturnErrorOnFailure(mBdxTransfer.StartTransfer(chip::bdx::TransferRole::kReceiver, bdxInitData, 0));
}

CHIP_ERROR BdxDownloader::BeginPrepareDownload()
{
    VerifyOrReturnError(mState == kIdle, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mImageProcessor->PrepareDownload());

    mState = kPreparing;
}

CHIP_ERROR BdxDownloader::OnPreparedForDownload()
{
    VerifyOrReturnError(mState == kPreparing, CHIP_ERROR_INCORRECT_STATE);
    PollTransferSession();

    mState = kInProgress;
}

CHIP_ERROR BdxDownloader::FetchNextData()
{
    VerifyOrReturnError(mState == kInProgress, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQuery());
    PollTransferSession();
}

void BdxDownloader::OnDownloadTimedOut()
{
    if (mState == kInProgress)
    {
        mBdxTransfer.AbortTransfer(chip::bdx::StatusCode::kUnknown);
    }
    else
    {
        ChipLogError(BDX, "no download in progress");
    }
}

void BdxDownloader::EndDownload(CHIP_ERROR reason = CHIP_NO_ERROR)
{
    if (mState == kInProgress)
    {
        mBdxTransfer.AbortTransfer(chip::bdx::StatusCode::kUnknown);
        if (mImageProcessor != nullptr)
        {
            ReturnErrorOnFailure(mImageProcessor->Abort());
        }
    }
    else
    {
        ChipLogError(BDX, "no download in progress");
    }

    // Because AbortTransfer() will generate a StatusReport to send.
    PollTransferSession();
}

void BdxDownloader::PollTransferSession()
{
    TransferSession::OutputEvent outEvent;

    // TODO: Is this dangerous? What happens if the loop encounters two messages that need to be sent?
    while (outEvent.EventType != TransferSession::OutputEventType::kNone)
    {
        mBdxTransfer.PollOutput(outEvent, 0);
        CHIP_ERROR err = HandleBdxEvent(outEvent);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "HandleBDXEvent: %" CHIP_ERROR_FORMAT, err.Format()));
    }
}

CHIP_ERROR BdxDownloader::HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent)
{
    VerifyOrReturnError(mState == kInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);

    switch (outEvent.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        // TODO:
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
        // TODO:
        break;
    case OutputEventType::kMsgToSend:
        VerifyOrReturnError(mMsgDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
        CHIP_ERROR err = mMsgDelegate->SendMessage(outEvent);
        break;
    case OutputEventType::kBlockReceived: {
        chip::ByteSpan blockData(outEvent.blockdata.Data, outEvent.blockdata.Length);
        CHIP_ERROR err = mImageProcessor->ProcessBlock(blockData);
        break;
    }
    case OutputEventType::kStatusReceived:
        ChipLogError(BDX, "BDX StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case OutputEventType::kInternalError:
        ChipLogError(BDX, "TransferSession error");
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        mBdxTransfer.Reset();
        ReturnErrorOnFailure(mImageProcessor->Abort());
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "Unexpected BDX event: %u", outEvent.EventType);
        break;
    }
