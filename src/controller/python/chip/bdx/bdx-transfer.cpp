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

CHIP_ERROR BdxTransfer::AcceptSend(BdxTransfer::DataCallback callback)
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(callback != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mAwaitingAccept = false;
    mDataCallback = callback;

    TransferSession::TransferAcceptData accept_data;
    accept_data.ControlMode = mInitData.TransferCtlFlags;  // TODO: Is this value correct?
    accept_data.MaxBlockSize = mInitData.MaxBlockSize;
    // TODO: Check the acceptable control modes for a Diagnostic Logs cluster transfer.
    return mTransfer.AcceptTransfer(accept_data);
}

CHIP_ERROR BdxTransfer::AcceptReceive(const ByteSpan data_to_send)
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

    TransferSession::TransferAcceptData accept_data;
    accept_data.ControlMode = mInitData.TransferCtlFlags;  // TODO: Is this value correct?
    accept_data.MaxBlockSize = mInitData.MaxBlockSize;
    return mTransfer.AcceptTransfer(accept_data);
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
        mInitData = event.transferInitData;
        mDelegate->InitMessageReceived(this, mInitData);
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Received StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        // TODO: Not a great error type. The issue isn't internal, it's external. We can check the status code, but I don't know
        // if that would produce a better error type.
        EndSession(CHIP_ERROR_INTERNAL);
        break;
    case TransferSession::OutputEventType::kInternalError:
        EndSession(CHIP_ERROR_INTERNAL);
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        EndSession(CHIP_ERROR_TIMEOUT);
        break;
    case TransferSession::OutputEventType::kBlockReceived:
        ByteSpan data(event.blockdata.Data, event.blockdata.Length);
        mDataCallback(data);
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
        mDataTransferredCount += event.bytesToSkip.BytesToSkip;
        mDataTransferredCount = std::min(mDataTransferredCount, mDataCount);
        // Fallthrough intentional.
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
    ReturnOnFailure(mTransfer.PrepareBlock(block));
    mDataTransferredCount += block.Length;
    ScheduleImmediatePoll();
    return CHIP_NO_ERROR;
}

}  // namespace bdx
}  // namespace chip