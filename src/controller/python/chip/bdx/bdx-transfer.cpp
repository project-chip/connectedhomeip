/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <controller/python/chip/bdx/bdx-transfer.h>

#include <protocols/bdx/BdxTransferSession.h>

namespace chip {
namespace bdx {

void BdxTransfer::SetDelegate(BdxTransfer::Delegate * delegate)
{
    mDelegate = delegate;
}

BdxTransfer::~BdxTransfer()
{
    if (mData != nullptr)
    {
        delete[] mData;
        mData = nullptr;
    }
}

CHIP_ERROR BdxTransfer::AcceptSend()
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    mAwaitingAccept = false;

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode = TransferControlFlags::kSenderDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset = mTransfer.GetStartOffset();
    acceptData.Length = mTransfer.GetTransferLength();
    return mTransfer.AcceptTransfer(acceptData);
}

CHIP_ERROR BdxTransfer::AcceptReceive(const ByteSpan & data_to_send)
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    mAwaitingAccept = false;
    mData = new uint8_t[data_to_send.size()];
    if (mData == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    memcpy(mData, data_to_send.data(), data_to_send.size());
    mDataCount = data_to_send.size();

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode = TransferControlFlags::kReceiverDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset = mTransfer.GetStartOffset();
    acceptData.Length = mTransfer.GetTransferLength();
    return mTransfer.AcceptTransfer(acceptData);
}

CHIP_ERROR BdxTransfer::Reject()
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    mAwaitingAccept = false;
    return mTransfer.RejectTransfer(StatusCode::kTransferFailedUnknownError);
}

void BdxTransfer::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    ChipLogDetail(BDX, "Received event %s", event.ToString(event.EventType));

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kInitReceived:
        mDelegate->InitMessageReceived(this, event.transferInitData);
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Received StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        EndSession(CHIP_ERROR_INTERNAL);
        break;
    case TransferSession::OutputEventType::kInternalError:
        EndSession(CHIP_ERROR_INTERNAL);
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        EndSession(CHIP_ERROR_TIMEOUT);
        break;
    case TransferSession::OutputEventType::kBlockReceived:
        if (mDelegate)
        {
            ByteSpan data(event.blockdata.Data, event.blockdata.Length);
            mDelegate->DataReceived(this, data);
        }
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        SendMessage(event);
        if (event.msgTypeData.HasMessageType(MessageType::BlockAckEOF))
        {
            // TODO: Completing the session here means the ACK for the BlockAckEOF message hasn't been received.
            EndSession(CHIP_NO_ERROR);
        }
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        EndSession(CHIP_NO_ERROR);
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
        mDataTransferredCount = std::min<size_t>(mDataTransferredCount + event.bytesToSkip.BytesToSkip, mDataCount);
        SendBlock();
        break;
    case TransferSession::OutputEventType::kQueryReceived:
        SendBlock();
        break;
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kNone:
        // Nothing to do.
        break;
    default:
        // Should never happen.
        chipDie();
        break;
    }
}

void BdxTransfer::EndSession(CHIP_ERROR result)
{
    mDelegate->TransferCompleted(this, result);
    ResetTransfer();
    if (mExchangeCtx)
    {
        mExchangeCtx->Close();
    }
}

void BdxTransfer::OnExchangeClosing(Messaging::ExchangeContext * exchangeContext)
{
    mExchangeCtx = nullptr;
}

CHIP_ERROR BdxTransfer::SendMessage(TransferSession::OutputEvent & event)
{
    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ::chip::Messaging::SendFlags sendFlags;
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport) &&
        !event.msgTypeData.HasMessageType(MessageType::BlockAckEOF))
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }
    return mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, event.MsgData.Retain(), sendFlags);
}

CHIP_ERROR BdxTransfer::SendBlock()
{
    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t dataRemaining = mDataCount - mDataTransferredCount;
    TransferSession::BlockData block;
    block.Data = mData + mDataTransferredCount;
    block.Length = std::min<size_t>(mTransfer.GetTransferBlockSize(), dataRemaining);
    block.IsEof = block.Length == dataRemaining;
    ReturnErrorOnFailure(mTransfer.PrepareBlock(block));
    mDataTransferredCount += block.Length;
    ScheduleImmediatePoll();
    return CHIP_NO_ERROR;
}

}  // namespace bdx
}  // namespace chip
