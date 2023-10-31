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

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
#include "DiagnosticLogsBDXTransferHandler.h"
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>

using chip::BitFlags;
using chip::ByteSpan;
using chip::CharSpan;
using chip::FabricIndex;
using chip::FabricInfo;
using chip::MutableCharSpan;
using chip::NodeId;
using chip::Span;
using chip::bdx::TransferControlFlags;
using chip::Protocols::InteractionModel::Status;

using namespace chip;
using namespace chip::app;
using namespace chip::bdx;
using namespace chip::app::Clusters::DiagnosticLogs;

// BDX Transfer Params
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);

// Timeout for the BDX transfer session. The OTA Spec mandates this should be >= 5 minutes.
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);

constexpr uint16_t kBdxMaxBlockSize = 1024;

CHIP_ERROR DiagnosticLogsBDXTransferHandler::InitializeTransfer(chip::Messaging::ExchangeContext * exchangeCtx,
                                                                FabricIndex fabricIndex, NodeId nodeId,
                                                                LogProviderDelegate * delegate, IntentEnum intent,
                                                                CharSpan fileDesignator)
{
    if (mInitialized)
    {
        // Reset stale connection from the same Node if exists.
        Reset();
    }

    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mExchangeCtx = exchangeCtx->GetExchangeMgr()->NewContext(exchangeCtx->GetSessionHandle(), this);
    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);

    mIntent   = intent;
    mDelegate = delegate;
    mFabricIndex.SetValue(fabricIndex);
    mNodeId.SetValue(nodeId);
    mNumBytesSent = 0;

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = bdx::TransferControlFlags::kSenderDrive;
    initOptions.MaxBlockSize     = kBdxMaxBlockSize;
    initOptions.FileDesLength    = static_cast<uint16_t>(fileDesignator.size());
    initOptions.FileDesignator   = reinterpret_cast<const uint8_t *>(fileDesignator.data());

    CHIP_ERROR err = Initiator::InitiateTransfer(&DeviceLayer::SystemLayer(), bdx::TransferRole::kSender, initOptions, kBdxTimeout,
                                                 kBdxPollIntervalMs);
    if (err != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(err);
        return err;
    }

    mInitialized = true;
    return CHIP_NO_ERROR;
}

void DiagnosticLogsBDXTransferHandler::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kAckEOFReceived:
        mStopPolling = true;
        Reset();
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        DiagnosticLogsServer::Instance().HandleBDXResponse(CHIP_ERROR_INTERNAL);
        Reset();
        break;
    case TransferSession::OutputEventType::kInternalError:
        DiagnosticLogsServer::Instance().HandleBDXResponse(CHIP_ERROR_INTERNAL);
        Reset();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        DiagnosticLogsServer::Instance().HandleBDXResponse(CHIP_ERROR_TIMEOUT);
        Reset();
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        Messaging::SendFlags sendFlags;
        if (!event.msgTypeData.HasMessageType(chip::Protocols::SecureChannel::MsgType::StatusReport))
        {
            // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and the
            // end of the transfer.
            sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
        }
        VerifyOrReturn(mExchangeCtx != nullptr);
        err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                        sendFlags);

        if (err == CHIP_NO_ERROR)
        {
            if (!sendFlags.Has(chip::Messaging::SendMessageFlags::kExpectResponse))
            {
                // After sending the StatusReport, exchange context gets closed so, set mExchangeCtx to null
                mExchangeCtx = nullptr;
            }
        }
        else
        {
            ChipLogError(BDX, "SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            DiagnosticLogsServer::Instance().HandleBDXResponse(err);
            Reset();
        }

        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived: {
        mLogSessionHandle = mDelegate->StartLogCollection(mIntent);

        if (mLogSessionHandle == kInvalidLogSessionHandle)
        {
            ChipLogError(BDX, "Invalid log session handle");
            DiagnosticLogsServer::Instance().HandleBDXResponse(CHIP_ERROR_INCORRECT_STATE);
            mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }
        // Send a response to the RetreiveLogRequest since we got a SendAccept message.
        DiagnosticLogsServer::Instance().HandleBDXResponse(CHIP_NO_ERROR);
    }
    // Fallthrough
    case TransferSession::OutputEventType::kAckReceived: {
        uint16_t blockSize   = mTransfer.GetTransferBlockSize();
        uint16_t bytesToRead = blockSize;

        if (mTransfer.GetTransferLength() > 0 && mNumBytesSent + blockSize > mTransfer.GetTransferLength())
        {
            // cast should be safe because of condition above
            bytesToRead = static_cast<uint16_t>(mTransfer.GetTransferLength() - mNumBytesSent);
        }

        chip::System::PacketBufferHandle blockBuf = chip::System::PacketBufferHandle::New(bytesToRead);
        if (blockBuf.IsNull())
        {
            mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }

        MutableByteSpan buffer;

        buffer = MutableByteSpan(blockBuf->Start(), bytesToRead);

        bool isEOF = false;

        // Get the log next chunk and see if it fits i.e. if EOF is reported
        uint64_t bytesRead = mDelegate->GetNextChunk(mLogSessionHandle, buffer, isEOF);

        if (bytesRead == 0)
        {
            mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }

        if (isEOF)
        {
            mDelegate->EndLogCollection(mLogSessionHandle);
            mLogSessionHandle = kInvalidLogSessionHandle;
        }

        TransferSession::BlockData blockData;
        blockData.Data   = blockBuf->Start();
        blockData.Length = static_cast<size_t>(bytesRead);
        blockData.IsEof  = isEOF;
        mNumBytesSent += static_cast<uint32_t>(blockData.Length);

        err = mTransfer.PrepareBlock(blockData);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "PrepareBlock failed: %" CHIP_ERROR_FORMAT, err.Format());
            mTransfer.AbortTransfer(StatusCode::kUnknown);
        }
        break;
    }
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kQueryReceived:
        break;
    default:
        // TransferSession should prevent this case from happening.
        ChipLogError(BDX, "Unsupported event type");
        break;
    }
}

void DiagnosticLogsBDXTransferHandler::Reset()
{
    assertChipStackLockedByCurrentThread();

    mFabricIndex.ClearValue();
    mNodeId.ClearValue();

    Initiator::ResetTransfer();

    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }

    if (mDelegate != nullptr)
    {
        mDelegate->EndLogCollection(mLogSessionHandle);
        mDelegate = nullptr;
    }
    mLogSessionHandle = kInvalidLogSessionHandle;
    mInitialized      = false;
    mNumBytesSent     = 0;
}

#endif
