/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "NSDataSpanConversion.h"

#include "MTRDiagnosticLogsTransferHandler.h"
#include <MTRError_Internal.h>

using namespace chip;
using namespace chip::bdx;
using namespace chip::app;

constexpr uint32_t kMaxBdxBlockSize = 1024;

// Timeout for the BDX transfer session. The OTA Spec mandates this should be >= 5 minutes.
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);
constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kReceiver;

CHIP_ERROR MTRDiagnosticLogsTransferHandler::PrepareForTransfer(System::Layer * _Nonnull layer, FabricIndex fabricIndex, NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kSenderDrive);

    return Responder::PrepareForTransfer(layer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout);
}

bdx::StatusCode GetBdxStatusCodeFromChipError(CHIP_ERROR err)
{
    if (err == CHIP_ERROR_INCORRECT_STATE) {
        return bdx::StatusCode::kUnexpectedMessage;
    }
    if (err == CHIP_ERROR_INVALID_ARGUMENT) {
        return bdx::StatusCode::kBadMessageContents;
    }
    return bdx::StatusCode::kUnknown;
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::OnTransferSessionBegin(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    NSError * error = nil;

    mFileHandle = [NSFileHandle fileHandleForWritingToURL:mFileURL error:&error];

    if (mFileHandle == nil || error != nil) {
        CHIP_ERROR err = [MTRError errorToCHIPErrorCode:error];
        LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err)));
        return err;
    }

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode = bdx::TransferControlFlags::kSenderDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset = mTransfer.GetStartOffset();
    acceptData.Length = mTransfer.GetTransferLength();

    mTransfer.AcceptTransfer(acceptData);
    mInitialized = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::OnTransferSessionEnd(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (event.EventType == TransferSession::OutputEventType::kTransferTimeout) {
        error = CHIP_ERROR_TIMEOUT;
    } else if (event.EventType != TransferSession::OutputEventType::kMsgToSend || !event.msgTypeData.HasMessageType(MessageType::BlockAckEOF)) {
        error = CHIP_ERROR_INTERNAL;
    }
    Reset();

    // Notify the MTRDevice via the callback that the BDX transfer has completed with error or success.
    if (mCallback) {
        mCallback(error != CHIP_NO_ERROR ? NO : YES);
    }
    return error;
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::OnBlockReceived(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    chip::ByteSpan blockData(event.blockdata.Data, event.blockdata.Length);

    if (mFileHandle != nil) {
        [mFileHandle seekToEndOfFile];
        NSError * error = nil;
        [mFileHandle writeData:AsData(blockData) error:&error];

        if (error != nil) {
            CHIP_ERROR err = [MTRError errorToCHIPErrorCode:error];
            return mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
        }
    } else {
        return mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE));
    }

    CHIP_ERROR err = mTransfer.PrepareBlockAck();
    LogErrorOnFailure(err);
    if (err != CHIP_NO_ERROR) {
        return mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
    }
    if (event.blockdata.IsEof) {
        mFileHandle = nil;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::OnMessageToSend(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mExchangeCtx != nil, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto & msgTypeData = event.msgTypeData;
    // If there's an error sending the message, close the exchange and call Reset.
    CHIP_ERROR err
        = mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    if (err != CHIP_NO_ERROR) {
        mExchangeCtx->Close();
        mExchangeCtx = nil;
        Reset();
    } else if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
        // If the send was successful for a status report, since we are not expecting a response the exchange context is
        // already closed. We need to null out the reference to avoid having a dangling pointer.
        mExchangeCtx = nil;
    }
    return err;
}

void MTRDiagnosticLogsTransferHandler::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    ChipLogError(BDX, "Got an event %s", event.ToString(event.EventType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.EventType) {
    case TransferSession::OutputEventType::kInitReceived:
        err = OnTransferSessionBegin(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
        }
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        [[fallthrough]];
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
        err = OnTransferSessionEnd(event);
        break;
    case TransferSession::OutputEventType::kBlockReceived:
        err = OnBlockReceived(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
        }
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        err = OnMessageToSend(event);
        if (event.msgTypeData.HasMessageType(MessageType::BlockAckEOF)) {
            err = OnTransferSessionEnd(event);
        }
        break;
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAcceptReceived:
        // Nothing to do.
        break;
    default:
        // Should never happens.
        chipDie();
        break;
    }
}

void MTRDiagnosticLogsTransferHandler::AbortTransfer(chip::bdx::StatusCode reason)
{
    assertChipStackLockedByCurrentThread();
    mTransfer.AbortTransfer(reason);
}

void MTRDiagnosticLogsTransferHandler::Reset()
{
    assertChipStackLockedByCurrentThread();
    mInitialized = false;
    mFileURL = nil;
    mFileHandle = nil;

    Responder::ResetTransfer();
    if (mExchangeCtx) {
        mExchangeCtx->Close();
        mExchangeCtx = nil;
    }
    mFabricIndex.ClearValue();
    mNodeId.ClearValue();
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::ConfigureState(chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    mFabricIndex.SetValue(fabricIndex);
    mNodeId.SetValue(nodeId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::OnMessageReceived(
    Messaging::ExchangeContext * _Nonnull ec, const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    VerifyOrReturnError(ec != nil, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err;
    mExchangeCtx = ec;

    // If we receive a ReceiveInit message, then we prepare for transfer
    if (payloadHeader.HasMessageType(MessageType::SendInit)) {
        NodeId nodeId = ec->GetSessionHandle()->GetPeer().GetNodeId();
        FabricIndex fabricIndex = ec->GetSessionHandle()->GetFabricIndex();

        if (nodeId != kUndefinedNodeId && fabricIndex != kUndefinedFabricIndex) {
            err = PrepareForTransfer(&(DeviceLayer::SystemLayer()), fabricIndex, nodeId);
            if (err != CHIP_NO_ERROR) {
                ChipLogError(BDX, "Failed to prepare for transfer for BDX");
                return err;
            }
        }
    }

    TransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));

    return err;
}
