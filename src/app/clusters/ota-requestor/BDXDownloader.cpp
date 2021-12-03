#include "BDXDownloader.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <protocols/bdx/BdxMessages.h>

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
    PollTransferSession();
}

CHIP_ERROR BDXDownloader::SetBDXParams(const chip::bdx::TransferSession::TransferInitData & bdxInitData)
{
    VerifyOrReturnError(mState == State::kIdle, CHIP_ERROR_INCORRECT_STATE);

    // Must call StartTransfer() here or otherwise the pointer data contained in bdxInitData could be freed before we can use it.
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
        mBdxTransfer.AbortTransfer(chip::bdx::StatusCode::kUnknown);
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
        mBdxTransfer.AbortTransfer(chip::bdx::StatusCode::kUnknown);
        if (mImageProcessor != nullptr)
        {
            mImageProcessor->Abort();
        }
    }
    else
    {
        ChipLogError(BDX, "no download in progress");
    }

    // Because AbortTransfer() will generate a StatusReport to send.
    PollTransferSession();
}

void BDXDownloader::PollTransferSession()
{
    TransferSession::OutputEvent outEvent;

    // TODO: Is this dangerous? What happens if the loop encounters two messages that need to be sent?
    do
    {
        mBdxTransfer.PollOutput(outEvent, /* TODO:(#12520) */ chip::System::Clock::Seconds16(0));
        CHIP_ERROR err = HandleBdxEvent(outEvent);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "HandleBDXEvent: %" CHIP_ERROR_FORMAT, err.Format()));
    } while (outEvent.EventType != TransferSession::OutputEventType::kNone);
}

CHIP_ERROR BDXDownloader::HandleBdxEvent(const chip::bdx::TransferSession::OutputEvent & outEvent)
{
    VerifyOrReturnError(mState == State::kInProgress, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mImageProcessor != nullptr, CHIP_ERROR_INCORRECT_STATE);

    switch (outEvent.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
        ReturnErrorOnFailure(mBdxTransfer.PrepareBlockQuery());
        // TODO: need to check ReceiveAccept parameters?
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        VerifyOrReturnError(mMsgDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
        ReturnErrorOnFailure(mMsgDelegate->SendMessage(outEvent));
        if (outEvent.msgTypeData.HasMessageType(chip::bdx::MessageType::BlockAckEOF))
        {
            mState = State::kComplete;
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

} // namespace chip
