/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BdxTransferDiagnosticLog.h"

#include <protocols/bdx/BdxTransferDiagnosticLogPool.h>

namespace chip {
namespace bdx {

namespace {
// Max block size for the BDX transfer.
constexpr uint32_t kMaxBdxBlockSize = 1024;

// How often we poll our transfer session.  Sadly, we get allocated on
// unsolicited message, which makes it hard for our clients to configure this.
// But the default poll interval is 500ms, which makes log downloads extremely
// slow.
constexpr System::Clock::Timeout kBdxPollInterval = System::Clock::Milliseconds32(50);

// Timeout for the BDX transfer session..
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);
constexpr TransferRole kBdxRole              = TransferRole::kReceiver;
} // namespace

void BdxTransferDiagnosticLog::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    if (event.EventType == TransferSession::OutputEventType::kNone)
    {
        // Because we are polling for output every 50ms on our transfer session,
        // we will get a lot of kNone events coming through here: one for every
        // time we poll but the other side has not sent anything new yet.  Just
        // ignore those here, for now, and make sure not to log them, because
        // that bloats the logs pretty quickly.
        return;
    }

    ChipLogDetail(BDX, "Got an event %s", event.ToString(event.EventType));

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kInitReceived:
        AbortTransferOnFailure(OnTransferSessionBegin(event));
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        LogErrorOnFailure(OnTransferSessionEnd(CHIP_ERROR_INTERNAL));
        break;
    case TransferSession::OutputEventType::kInternalError:
        LogErrorOnFailure(OnTransferSessionEnd(CHIP_ERROR_INTERNAL));
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        LogErrorOnFailure(OnTransferSessionEnd(CHIP_ERROR_TIMEOUT));
        break;
    case TransferSession::OutputEventType::kBlockReceived:
        AbortTransferOnFailure(OnBlockReceived(event));
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        LogErrorOnFailure(OnMessageToSend(event));

        if (event.msgTypeData.HasMessageType(MessageType::BlockAckEOF))
        {
            LogErrorOnFailure(OnTransferSessionEnd(CHIP_NO_ERROR));
        }
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
    // case TransferSession::OutputEventType::kNone: handled above.
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAcceptReceived:
        // Nothing to do.
        break;
    default:
        // Should never happen.
        chipDie();
        break;
    }
}

CHIP_ERROR BdxTransferDiagnosticLog::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                       System::PacketBufferHandle && payload)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // If we receive a SendInit message, then we prepare for transfer
    if (payloadHeader.HasMessageType(MessageType::SendInit))
    {
        FabricIndex fabricIndex = ec->GetSessionHandle()->GetFabricIndex();
        NodeId peerNodeId       = ec->GetSessionHandle()->GetPeer().GetNodeId();
        VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(peerNodeId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

        mTransferProxy.SetFabricIndex(fabricIndex);
        mTransferProxy.SetPeerNodeId(peerNodeId);
        auto flags(TransferControlFlags::kSenderDrive);
        ReturnLogErrorOnFailure(
            Responder::PrepareForTransfer(mSystemLayer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollInterval));
    }

    return TransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));
}

CHIP_ERROR BdxTransferDiagnosticLog::OnMessageToSend(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    auto & msgTypeData  = event.msgTypeData;
    bool isStatusReport = msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport);

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    Messaging::SendFlags sendFlags;
    VerifyOrDo(isStatusReport, sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse));

    // If there's an error sending the message, close the exchange by calling Reset.
    auto err = mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    VerifyOrDo(CHIP_NO_ERROR == err, OnTransferSessionEnd(err));

    return err;
}

CHIP_ERROR BdxTransferDiagnosticLog::OnTransferSessionBegin(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(nullptr != mDelegate, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(mTransferProxy.Init(&mTransfer));
    return mDelegate->OnTransferBegin(&mTransferProxy);
}

CHIP_ERROR BdxTransferDiagnosticLog::OnTransferSessionEnd(CHIP_ERROR error)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(nullptr != mDelegate, CHIP_ERROR_INCORRECT_STATE);

    LogErrorOnFailure(mDelegate->OnTransferEnd(&mTransferProxy, error));
    Reset();
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxTransferDiagnosticLog::OnBlockReceived(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(nullptr != mDelegate, CHIP_ERROR_INCORRECT_STATE);

    ByteSpan blockData(event.blockdata.Data, event.blockdata.Length);
    return mDelegate->OnTransferData(&mTransferProxy, blockData);
}

void BdxTransferDiagnosticLog::AbortTransferOnFailure(CHIP_ERROR error)
{
    VerifyOrReturn(CHIP_NO_ERROR != error);
    LogErrorOnFailure(error);
    LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(error)));
}

void BdxTransferDiagnosticLog::Reset()
{
    assertChipStackLockedByCurrentThread();

    Responder::ResetTransfer();

    if (mExchangeCtx)
    {
        mIsExchangeClosing = true;
        mExchangeCtx->Close();
        mIsExchangeClosing = false;
        mExchangeCtx       = nullptr;
    }

    mTransferProxy.Reset();
}

void BdxTransferDiagnosticLog::OnExchangeClosing(Messaging::ExchangeContext * ec)
{
    // The exchange can be closing while TransferFacilitator is still accessing us, so
    // the BdxTransferDiagnosticLog can not be released "right now".
    mSystemLayer->ScheduleWork(
        [](auto * systemLayer, auto * appState) -> void {
            auto * _this = static_cast<BdxTransferDiagnosticLog *>(appState);
            _this->mPoolDelegate->Release(_this);
        },
        this);

    // This block checks and handles the scenario where the exchange is closed externally (e.g., receiving a StatusReport).
    // Continuing to use it could lead to a use-after-free error and such an error might occur when the poll timer triggers and
    // OnTransferSessionEnd is called.
    // We know it's not just us normally closing the exchange if mIsExchangeClosing is false.
    VerifyOrReturn(!mIsExchangeClosing);
    mExchangeCtx = nullptr;
    LogErrorOnFailure(OnTransferSessionEnd(CHIP_ERROR_INTERNAL));
}

bool BdxTransferDiagnosticLog::IsForFabric(FabricIndex fabricIndex) const
{
    if (mExchangeCtx == nullptr || !mExchangeCtx->HasSessionHandle())
    {
        return false;
    }

    auto session = mExchangeCtx->GetSessionHandle();
    return session->GetFabricIndex() == fabricIndex;
}

void BdxTransferDiagnosticLog::AbortTransfer()
{
    // No need to mTransfer.AbortTransfer() here, since that just tries to async
    // send a StatusReport to the other side, but we are going away here.
    Reset();
}

} // namespace bdx
} // namespace chip
