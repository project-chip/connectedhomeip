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
#include "DiagnosticLogsBDXTransferHandler.h"
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace app;
using namespace bdx;
using namespace app::Clusters::DiagnosticLogs;

// BDX transfer session polling interval.
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);

// Timeout for the BDX transfer session
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);

constexpr uint16_t kBdxMaxBlockSize = 1024;

StatusCode GetBdxStatusCodeFromChipError(CHIP_ERROR err)
{
    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        return StatusCode::kUnexpectedMessage;
    }
    if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        return StatusCode::kBadMessageContents;
    }
    return StatusCode::kUnknown;
}

CHIP_ERROR DiagnosticLogsBDXTransferHandler::InitializeTransfer(Messaging::ExchangeManager * exchangeMgr,
                                                                const SessionHandle sessionHandle, FabricIndex fabricIndex,
                                                                NodeId peerNodeId, LogProviderDelegate * delegate,
                                                                IntentEnum intent, CharSpan fileDesignator)
{
    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(peerNodeId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Create a new exchange context to use for the BDX transfer session
    // and send the SendInit message over the exchange.
    mBDXTransferExchangeCtx = exchangeMgr->NewContext(sessionHandle, this);
    VerifyOrReturnError(mBDXTransferExchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);

    mIntent   = intent;
    mDelegate = delegate;
    mFabricIndex.SetValue(fabricIndex);
    mPeerNodeId.SetValue(peerNodeId);
    mNumBytesSent     = 0;
    mIsAcceptReceived = false;

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kSenderDrive;
    initOptions.MaxBlockSize     = kBdxMaxBlockSize;
    initOptions.FileDesLength    = static_cast<uint16_t>(fileDesignator.size());
    initOptions.FileDesignator   = Uint8::from_const_char(fileDesignator.data());

    CHIP_ERROR err = Initiator::InitiateTransfer(&DeviceLayer::SystemLayer(), TransferRole::kSender, initOptions, kBdxTimeout,
                                                 kBdxPollIntervalMs);
    if (err != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(err);
        return err;
    }

    mInitialized = true;
    return CHIP_NO_ERROR;
}

void DiagnosticLogsBDXTransferHandler::ScheduleCleanUp()
{
    Reset();
    DeviceLayer::SystemLayer().ScheduleLambda([this] {
        delete this;
        DiagnosticLogsServer::Instance().HandleBDXTransferDone();
    });
}

void DiagnosticLogsBDXTransferHandler::HandleBDXError(CHIP_ERROR error)
{
    VerifyOrReturn(error != CHIP_NO_ERROR);
    LogErrorOnFailure(error);

    // If Send Accept was not received, send a response with status Denied to the RetrieveLogsRequest
    if (!mIsAcceptReceived)
    {
        DiagnosticLogsServer::Instance().SendCommandResponse(StatusEnum::kDenied);
    }
    // Call Reset to clean up state and schedule an asynchronous delete for the DiagnosticLogsBDXTransferHandler object.
    // Since an error occured during BDX, before we delete the DiagnosticLogsBDXTransferHandler object, we need the base class -
    // TransferFacilitator to stop polling for messages and clean up. Since the HandleBDXError is called only when BDX fails
    // and transfer is stopped, this will not be called more than once for a DiagnosticLogsBDXTransferHandler object.
    ScheduleCleanUp();
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
        // Call Reset to clean up state and schedule an asynchronous delete for the DiagnosticLogsBDXTransferHandler object.
        // Since BDX has completed successfully, we need the base class - TransferFacilitator to stop polling for messages and clean
        // up before we can delete the sub class. This will also be called once for a DiagnosticLogsBDXTransferHandler object.
        ScheduleCleanUp();
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", to_underlying(event.statusData.statusCode));
        [[fallthrough]];
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
        HandleBDXError(CHIP_ERROR_INTERNAL);
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        Messaging::SendFlags sendFlags;
        if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
        {
            // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and the
            // end of the transfer.
            sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
        }
        VerifyOrReturn(mBDXTransferExchangeCtx != nullptr);
        err = mBDXTransferExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                   std::move(event.MsgData), sendFlags);

        if (err == CHIP_NO_ERROR)
        {
            if (!sendFlags.Has(Messaging::SendMessageFlags::kExpectResponse))
            {
                // After sending the StatusReport, exchange context gets closed so, set mBDXTransferExchangeCtx to null
                mBDXTransferExchangeCtx = nullptr;
            }
        }
        else
        {
            ChipLogError(BDX, "SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
        }

        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived: {
        mIsAcceptReceived = true;
        mLogSessionHandle = mDelegate->StartLogCollection(mIntent);
        if (mLogSessionHandle == kInvalidLogSessionHandle)
        {
            ChipLogError(BDX, "No log available for intent enum %d", to_underlying(mIntent));
            DiagnosticLogsServer::Instance().SendCommandResponse(StatusEnum::kDenied);
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE));
            return;
        }
        // Send a response with status Success to the RetrieveLogsRequest, since we got a SendAccept message.
        DiagnosticLogsServer::Instance().SendCommandResponse(StatusEnum::kSuccess);

        [[fallthrough]];
    }
    case TransferSession::OutputEventType::kAckReceived: {
        uint16_t blockSize = mTransfer.GetTransferBlockSize();

        System::PacketBufferHandle blockBuf = System::PacketBufferHandle::New(blockSize);
        if (blockBuf.IsNull())
        {
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_NO_MEMORY));
            return;
        }

        MutableByteSpan buffer;

        buffer = MutableByteSpan(blockBuf->Start(), blockSize);

        bool isEOF = false;

        // Get the log next chunk and see if it fits i.e. if EOF is reported
        err = mDelegate->GetNextChunk(mLogSessionHandle, buffer, isEOF);

        if (err != CHIP_NO_ERROR)
        {
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
            return;
        }

        // If EOF is received, end the log collection session.
        if (isEOF)
        {
            mDelegate->EndLogCollection(mLogSessionHandle);
            mLogSessionHandle = kInvalidLogSessionHandle;
        }

        // Prepare the BDX block to send to the requestor
        TransferSession::BlockData blockData;
        blockData.Data   = blockBuf->Start();
        blockData.Length = static_cast<size_t>(buffer.size());
        blockData.IsEof  = isEOF;
        mNumBytesSent += static_cast<uint32_t>(blockData.Length);

        err = mTransfer.PrepareBlock(blockData);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "PrepareBlock failed: %" CHIP_ERROR_FORMAT, err.Format());
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
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
    mPeerNodeId.ClearValue();

    Initiator::ResetTransfer();

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
    mLogSessionHandle = kInvalidLogSessionHandle;
    mInitialized      = false;
    mNumBytesSent     = 0;
    mIsAcceptReceived = false;
}

#endif
