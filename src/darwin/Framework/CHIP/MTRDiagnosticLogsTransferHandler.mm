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

// TODO Expose a method onto the delegate to make that configurable.
constexpr uint32_t kMaxBdxBlockSize = 1024;

// Timeout for the BDX transfer session. The OTA Spec mandates this should be >= 5 minutes.
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);
constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kReceiver;

// Need DiagnosticLogsProcessorInterface handle to process block which will be implemented in darwin/linux

// TODO: need to check how to handle MTDeviceController being shutdown.

CHIP_ERROR MTRDiagnosticLogsTransferHandler::PrepareForTransfer(System::Layer * _Nonnull layer, FabricIndex fabricIndex, NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);

    return Responder::PrepareForTransfer(layer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout);
}

MTRDiagnosticLogsTransferHandler::~MTRDiagnosticLogsTransferHandler()
{
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
        // TODO: Map NSError to BDX error
        LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE)));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode = bdx::TransferControlFlags::kSenderDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset = mTransfer.GetStartOffset();
    acceptData.Length = mTransfer.GetTransferLength();

    mTransfer.AcceptTransfer(acceptData);
    mIsInBDXSession = true;
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
    } else if (event.EventType != TransferSession::OutputEventType::kBlockReceived || !event.blockdata.IsEof) {
        error = CHIP_ERROR_INTERNAL;
    }

    if (error != CHIP_NO_ERROR) {
        // Notify the MTRDevice via the callback that the BDX transfer has completed with error
        if (mCallback) {
            mCallback(NO);
        }
        Reset();
        return error;
    }

    // Notify the MTRDevice via the callback that the BDX transfer has completed with success
    if (mCallback) {
        mCallback(YES);
    }
    Reset();
    return CHIP_NO_ERROR;
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
            // TODO: map nserror to status code
            LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE)));
            return CHIP_ERROR_INCORRECT_STATE;
        }
    } else {
        LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE)));
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = mTransfer.PrepareBlockAck();
    LogErrorOnFailure(err);
    if (err != CHIP_NO_ERROR) {
        LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(CHIP_ERROR_INCORRECT_STATE)));
        return err;
    }
    if (event.blockdata.IsEof) {
        if (mFileHandle != nil) {
            [mFileHandle closeFile];
            mFileHandle = nullptr;
        }
        OnTransferSessionEnd(event);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDiagnosticLogsTransferHandler::OnMessageToSend(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    // VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto & msgTypeData = event.msgTypeData;
    // If there's an error sending the message, close the exchange and call ResetState.
    // TODO: If we can remove the !mInitialized check in ResetState(), just calling ResetState() will suffice here.
    CHIP_ERROR err
        = mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    if (err != CHIP_NO_ERROR) {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
        Reset();
    } else if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
        // If the send was successful for a status report, since we are not expecting a response the exchange context is
        // already closed. We need to null out the reference to avoid having a dangling pointer.
        mExchangeCtx = nullptr;
        Reset();
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
        ChipLogError(BDX, "HandleTransferSessionOutput called");
        err = OnTransferSessionBegin(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err)));
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
    case TransferSession::OutputEventType::kMsgToSend:
        err = OnMessageToSend(event);
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kNone:
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

void MTRDiagnosticLogsTransferHandler::Reset()
{
    mIsInBDXSession = false;
    mFileURL = nullptr;
    mCallback = nullptr;

    if (mFileHandle != nil) {
        [mFileHandle closeFile];
        mFileHandle = nullptr;
    }
    if (mExchangeCtx) {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }
    mFabricIndex.ClearValue();
    mNodeId.ClearValue();

    mTransfer.Reset();
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
    CHIP_ERROR err;
    mExchangeCtx = ec;

    // If we receive a ReceiveInit message, then we prepare for transfer
    if (payloadHeader.HasMessageType(MessageType::SendInit)) {
        NodeId nodeId = ec->GetSessionHandle()->GetPeer().GetNodeId();
        FabricIndex fabricIndex = ec->GetSessionHandle()->GetFabricIndex();

        if (nodeId != kUndefinedNodeId && fabricIndex != kUndefinedFabricIndex) {
            err = PrepareForTransfer(&(DeviceLayer::SystemLayer()), fabricIndex, nodeId);
            if (err != CHIP_NO_ERROR) {
                ChipLogError(Controller, "Failed to prepare for transfer for BDX");
                return err;
            }
        }
    }

    TransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));

    return err;
}
