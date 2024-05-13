/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
#include "BDXDiagnosticLogsProvider.h"
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::bdx;

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

// BDX transfer session polling interval.
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);

// Timeout for the BDX transfer session
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);

constexpr uint16_t kBdxMaxBlockSize = 1024;

CHIP_ERROR BDXDiagnosticLogsProvider::InitializeTransfer(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                                         DiagnosticLogsProviderDelegate * delegate, IntentEnum intent,
                                                         CharSpan fileDesignator)
{
    auto exchangeCtx = commandObj->GetExchangeContext();
    VerifyOrReturnError(nullptr != exchangeCtx, CHIP_ERROR_INVALID_ARGUMENT);

    auto exchangeMgr = exchangeCtx->GetExchangeMgr();
    VerifyOrReturnError(nullptr != exchangeMgr, CHIP_ERROR_INVALID_ARGUMENT);

    auto sessionHandle = exchangeCtx->GetSessionHandle();
    VerifyOrReturnError(sessionHandle->IsSecureSession(), CHIP_ERROR_INVALID_ARGUMENT);

    ScopedNodeId scopedPeerNodeId = sessionHandle->AsSecureSession()->GetPeer();
    auto fabricIndex              = scopedPeerNodeId.GetFabricIndex();
    VerifyOrReturnError(kUndefinedFabricIndex != fabricIndex, CHIP_ERROR_INVALID_ARGUMENT);

    auto peerNodeId = scopedPeerNodeId.GetNodeId();
    VerifyOrReturnError(kUndefinedNodeId != peerNodeId, CHIP_ERROR_INVALID_ARGUMENT);

    LogSessionHandle logSessionHandle;
    Optional<uint64_t> timeStamp;
    Optional<uint64_t> timeSinceBoot;
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(delegate->StartLogCollection(intent, logSessionHandle, timeStamp, timeSinceBoot));

    // Create a new exchange context to use for the BDX transfer session
    // and send the SendInit message over the exchange.
    auto transferExchangeCtx = exchangeMgr->NewContext(sessionHandle, this);
    VerifyOrReturnError(nullptr != transferExchangeCtx, CHIP_ERROR_NO_MEMORY);

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kSenderDrive;
    initOptions.MaxBlockSize     = kBdxMaxBlockSize;
    initOptions.FileDesLength    = static_cast<uint16_t>(fileDesignator.size());
    initOptions.FileDesignator   = Uint8::from_const_char(fileDesignator.data());

    CHIP_ERROR err = Initiator::InitiateTransfer(&DeviceLayer::SystemLayer(), TransferRole::kSender, initOptions, kBdxTimeout,
                                                 kBdxPollIntervalMs);
    if (CHIP_NO_ERROR != err)
    {
        LogErrorOnFailure(err);
        transferExchangeCtx->Close();
        return err;
    }

    mBDXTransferExchangeCtx = transferExchangeCtx;
    mDelegate               = delegate;
    mFabricIndex.SetValue(fabricIndex);
    mPeerNodeId.SetValue(peerNodeId);
    mIsAcceptReceived   = false;
    mLogSessionHandle   = logSessionHandle;
    mTimeStamp          = timeStamp;
    mTimeSinceBoot      = timeSinceBoot;
    mAsyncCommandHandle = CommandHandler::Handle(commandObj);
    mRequestPath        = path;
    mInitialized        = true;

    return CHIP_NO_ERROR;
}

void BDXDiagnosticLogsProvider::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kMsgToSend:
        OnMsgToSend(event);
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
        OnAcceptReceived();
        // Upon acceptance of the transfer, the OnAckReceived method initiates the process of sending logs.
        OnAckReceived();
        break;
    case TransferSession::OutputEventType::kAckReceived:
        OnAckReceived();
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        OnAckEOFReceived();
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        OnStatusReceived(event);
        break;
    case TransferSession::OutputEventType::kInternalError:
        OnInternalError();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        OnTimeout();
        break;
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kInitReceived:
        break;
    case TransferSession::OutputEventType::kBlockReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
        // TransferSession should prevent this case from happening.
        ChipLogError(BDX, "Unsupported event type");
        break;
    }
}

void BDXDiagnosticLogsProvider::OnMsgToSend(TransferSession::OutputEvent & event)
{
    VerifyOrReturn(mBDXTransferExchangeCtx != nullptr);

    auto & msgTypeData  = event.msgTypeData;
    bool isStatusReport = msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport);

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and the
    // end of the transfer.
    Messaging::SendFlags sendFlags;
    if (!isStatusReport)
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto err =
        mBDXTransferExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);

    VerifyOrDo(CHIP_NO_ERROR == err, Reset());
}

void BDXDiagnosticLogsProvider::OnAcceptReceived()
{
    mIsAcceptReceived = true;

    // On reception of a BDX SendAccept message the Node SHALL send a RetrieveLogsResponse command with a Status field set to
    // Success and proceed with the log transfer over BDX.
    SendCommandResponse(StatusEnum::kSuccess);
}

void BDXDiagnosticLogsProvider::OnAckReceived()
{
    uint16_t blockSize = mTransfer.GetTransferBlockSize();

    auto blockBuf = System::PacketBufferHandle::New(blockSize);
    VerifyOrReturn(!blockBuf.IsNull(), mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_NO_MEMORY)));

    auto buffer     = MutableByteSpan(blockBuf->Start(), blockSize);
    bool isEndOfLog = false;

    // Get the log next chunk and see if it fits i.e. if is end of log is reported
    auto err = mDelegate->CollectLog(mLogSessionHandle, buffer, isEndOfLog);
    VerifyOrReturn(CHIP_NO_ERROR == err, mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err)));

    // If the buffer has empty space, end the log collection session.
    if (isEndOfLog)
    {
        mDelegate->EndLogCollection(mLogSessionHandle);
        mLogSessionHandle = kInvalidLogSessionHandle;
    }

    // Prepare the BDX block to send to the requestor
    TransferSession::BlockData blockData;
    blockData.Data   = blockBuf->Start();
    blockData.Length = static_cast<size_t>(buffer.size());
    blockData.IsEof  = isEndOfLog;

    err = mTransfer.PrepareBlock(blockData);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "PrepareBlock failed: %" CHIP_ERROR_FORMAT, err.Format());
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
    }
}

void BDXDiagnosticLogsProvider::OnAckEOFReceived()
{
    ChipLogProgress(BDX, "Diagnostic logs transfer: Success");

    Reset();
}

void BDXDiagnosticLogsProvider::OnStatusReceived(TransferSession::OutputEvent & event)
{
    ChipLogError(BDX, "Diagnostic logs transfer: StatusReport Error %x", to_underlying(event.statusData.statusCode));

    // If a failure StatusReport is received in response to the SendInit message, the Node SHALL send a RetrieveLogsResponse command
    // with a Status of Denied.
    VerifyOrDo(mIsAcceptReceived, SendCommandResponse(StatusEnum::kDenied));
    Reset();
}

void BDXDiagnosticLogsProvider::OnInternalError()
{
    ChipLogError(BDX, "Internal Error");
    VerifyOrDo(mIsAcceptReceived, SendCommandResponse(StatusEnum::kDenied));
    Reset();
}

void BDXDiagnosticLogsProvider::OnTimeout()
{
    ChipLogError(BDX, "Timeout");
    VerifyOrDo(mIsAcceptReceived, SendCommandResponse(StatusEnum::kDenied));
    Reset();
}

void BDXDiagnosticLogsProvider::SendCommandResponse(StatusEnum status)
{
    auto commandHandleRef = std::move(mAsyncCommandHandle);
    auto commandHandle    = commandHandleRef.Get();

    VerifyOrReturn(nullptr != commandHandle, ChipLogError(Zcl, "SendCommandResponse - commandHandler is null"));

    Commands::RetrieveLogsResponse::Type response;
    response.status = status;
    if (status == StatusEnum::kSuccess)
    {
        if (mTimeStamp.HasValue())
        {
            response.UTCTimeStamp = mTimeStamp;
        }

        if (mTimeSinceBoot.HasValue())
        {
            response.timeSinceBoot = mTimeSinceBoot;
        }
    }
    commandHandle->AddResponse(mRequestPath, response);
}

void BDXDiagnosticLogsProvider::Reset()
{
    assertChipStackLockedByCurrentThread();

    Initiator::ResetTransfer();
    mExchangeCtx = nullptr;

    if (mBDXTransferExchangeCtx != nullptr)
    {
        mBDXTransferExchangeCtx->Close();
        mBDXTransferExchangeCtx = nullptr;
    }

    if (mDelegate != nullptr)
    {
        mDelegate->EndLogCollection(mLogSessionHandle);
        mDelegate = nullptr;
    }

    mFabricIndex.ClearValue();
    mPeerNodeId.ClearValue();
    mIsAcceptReceived = false;
    mLogSessionHandle = kInvalidLogSessionHandle;
    mTimeStamp.ClearValue();
    mTimeSinceBoot.ClearValue();
    mAsyncCommandHandle = nullptr;
    mRequestPath        = ConcreteCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    mInitialized        = false;
}

void BDXDiagnosticLogsProvider::OnExchangeClosing(Messaging::ExchangeContext * ec)
{
    mBDXTransferExchangeCtx = nullptr;
}

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip

#endif
