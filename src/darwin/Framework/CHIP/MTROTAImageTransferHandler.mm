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

#import "MTROTAImageTransferHandler.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTROTAUnsolicitedBDXMessageHandler.h"
#import "NSStringSpanConversion.h"

#include <MTRError_Internal.h>

using namespace chip;
using namespace chip::bdx;
using namespace chip::app;

// TODO Expose a method onto the delegate to make that configurable.
constexpr uint32_t kMaxBdxBlockSize = 1024;

// Since the BDX timeout is 5 minutes and we are starting this after query image is available and before the BDX init comes,
// we just double the timeout to give enough time for the BDX init to come in a reasonable amount of time.
constexpr System::Clock::Timeout kBdxInitReceivedTimeout = System::Clock::Seconds16(10 * 60);

constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kSender;

MTROTAImageTransferHandler::MTROTAImageTransferHandler()
{
    // Increment the number of delegates by 1.
    MTROTAUnsolicitedBDXMessageHandler::IncrementNumberOfDelegates();
}

CHIP_ERROR MTROTAImageTransferHandler::PrepareForTransfer(
    Messaging::ExchangeContext * exchangeCtx, FabricIndex fabricIndex, NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);

    return AsyncResponder::PrepareForTransfer(exchangeCtx, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout);
}

MTROTAImageTransferHandler::~MTROTAImageTransferHandler() { ResetState(); }

void MTROTAImageTransferHandler::ResetState()
{
    assertChipStackLockedByCurrentThread();
    if (mNodeId.HasValue() && mFabricIndex.HasValue()) {
        ChipLogProgress(Controller,
            "Resetting state for OTA Provider; no longer providing an update for node id 0x" ChipLogFormatX64 ", fabric index %u",
            ChipLogValueX64(mNodeId.Value()), mFabricIndex.Value());
    } else {
        ChipLogProgress(Controller, "Resetting state for OTA Provider");
    }
    chip::DeviceLayer::SystemLayer().CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);

    AsyncResponder::ResetTransfer();
    mFabricIndex.ClearValue();
    mNodeId.ClearValue();
    mDelegate = nil;
    mDelegateNotificationQueue = nil;
    MTROTAUnsolicitedBDXMessageHandler::DecrementNumberOfDelegates();
}
/**
 * Timer callback called when we don't receive a BDX init within a reasonable time after a successful QueryImage response.
 */
void MTROTAImageTransferHandler::HandleBdxInitReceivedTimeoutExpired(chip::System::Layer * systemLayer, void * state)
{
    VerifyOrReturn(state != nullptr);
    static_cast<MTROTAImageTransferHandler *>(state)->ResetState();
}

CHIP_ERROR MTROTAImageTransferHandler::OnMessageToSend(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(AsyncTransferFacilitator::GetExchangeContext() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto & msgTypeData = event.msgTypeData;
    ChipLogError(BDX, "OnMessageToSend msgTypeData.MessageType = %hu", msgTypeData.MessageType);
    Messaging::ExchangeContext * ec = AsyncTransferFacilitator::GetExchangeContext();
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (ec != nullptr) {
        // If there's an error sending the message, call ResetState.
        err = ec->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
        if (err != CHIP_NO_ERROR) {
            ResetState();
        } else if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
            // If the send was successful for a status report, since we are not expecting a response the exchange context is
            // already closed. We need to null out the reference to avoid having a dangling pointer.
            ec = nullptr;
            ResetState();
        }
    }
    return err;
}

CHIP_ERROR MTROTAImageTransferHandler::OnTransferSessionBegin(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();
    // Once we receive the BDX init, cancel the BDX Init timeout and start the BDX session
    chip::DeviceLayer::SystemLayer().CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);

    VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    uint16_t fdl = 0;
    auto fd = mTransfer.GetFileDesignator(fdl);
    VerifyOrReturnError(fdl <= bdx::kMaxFileDesignatorLen, CHIP_ERROR_INVALID_ARGUMENT);
    CharSpan fileDesignatorSpan(Uint8::to_const_char(fd), fdl);

    auto fileDesignator = AsString(fileDesignatorSpan);
    if (fileDesignator == nil) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto offset = @(mTransfer.GetStartOffset());

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

    auto completionHandler = ^(NSError * _Nullable error) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                if (error != nil) {
                    CHIP_ERROR err = [MTRError errorToCHIPErrorCode:error];
                    LogErrorOnFailure(err);
                    ResetState();
                    AsyncResponder::NotifyEventHandledWithError(err);
                    return;
                }

                // bdx::TransferSession will automatically reject a transfer if there are no
                // common supported control modes. It will also default to the smaller
                // block size.
                TransferSession::TransferAcceptData acceptData;
                acceptData.ControlMode = bdx::TransferControlFlags::kReceiverDrive;
                acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
                acceptData.StartOffset = mTransfer.GetStartOffset();
                acceptData.Length = mTransfer.GetTransferLength();

                LogErrorOnFailure(mTransfer.AcceptTransfer(acceptData));
                AsyncResponder::NotifyEventHandledWithError([MTRError errorToCHIPErrorCode:error]);
                return;
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    auto nodeId = @(mNodeId.Value());

    auto strongDelegate = mDelegate;
    auto delagateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delagateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        AsyncResponder::NotifyEventHandledWithError(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    dispatch_async(delagateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector
                            (handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completion:)]) {
            [strongDelegate handleBDXTransferSessionBeginForNodeID:nodeId
                                                        controller:controller
                                                    fileDesignator:fileDesignator
                                                            offset:offset
                                                        completion:completionHandler];
        } else {
            [strongDelegate handleBDXTransferSessionBeginForNodeID:nodeId
                                                        controller:controller
                                                    fileDesignator:fileDesignator
                                                            offset:offset
                                                 completionHandler:completionHandler];
        }
    });

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROTAImageTransferHandler::OnTransferSessionEnd(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR error = CHIP_NO_ERROR;
    if (event.EventType == TransferSession::OutputEventType::kTransferTimeout) {
        error = CHIP_ERROR_TIMEOUT;
    } else if (event.EventType != TransferSession::OutputEventType::kAckEOFReceived
        || event.EventType != TransferSession::OutputEventType::kAckEOFReceived) {
        error = CHIP_ERROR_INTERNAL;
    }

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);
    auto nodeId = @(mNodeId.Value());

    auto strongDelegate = mDelegate;
    auto delagateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delagateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        AsyncResponder::NotifyEventHandledWithError(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if ([strongDelegate respondsToSelector:@selector(handleBDXTransferSessionEndForNodeID:controller:error:)]) {
        dispatch_async(delagateQueue, ^{
            [strongDelegate handleBDXTransferSessionEndForNodeID:nodeId
                                                      controller:controller
                                                           error:[MTRError errorForCHIPErrorCode:error]];
        });
    }

    ResetState();
    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROTAImageTransferHandler::OnBlockQuery(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    auto blockSize = @(mTransfer.GetTransferBlockSize());
    auto blockIndex = @(mTransfer.GetNextBlockNum());

    auto bytesToSkip = @(0);
    if (event.EventType == TransferSession::OutputEventType::kQueryWithSkipReceived) {
        bytesToSkip = @(event.bytesToSkip.BytesToSkip);
    }

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

    auto completionHandler = ^(NSData * _Nullable data, BOOL isEOF) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                if (data == nil) {
                    AsyncResponder::NotifyEventHandledWithError(CHIP_ERROR_INCORRECT_STATE);
                    return;
                }

                TransferSession::BlockData blockData;
                blockData.Data = static_cast<const uint8_t *>([data bytes]);
                blockData.Length = static_cast<size_t>([data length]);
                blockData.IsEof = isEOF;

                CHIP_ERROR err = mTransfer.PrepareBlock(blockData);
                if (CHIP_NO_ERROR != err) {
                    LogErrorOnFailure(err);
                }
                AsyncResponder::NotifyEventHandledWithError(err);
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    // TODO Handle MaxLength

    auto nodeId = @(mNodeId.Value());

    auto strongDelegate = mDelegate;
    auto delagateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delagateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        AsyncResponder::NotifyEventHandledWithError(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    dispatch_async(delagateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleBDXQueryForNodeID:
                                                                      controller:blockSize:blockIndex:bytesToSkip:completion:)]) {
            [strongDelegate handleBDXQueryForNodeID:nodeId
                                         controller:controller
                                          blockSize:blockSize
                                         blockIndex:blockIndex
                                        bytesToSkip:bytesToSkip
                                         completion:completionHandler];
        } else {
            [strongDelegate handleBDXQueryForNodeID:nodeId
                                         controller:controller
                                          blockSize:blockSize
                                         blockIndex:blockIndex
                                        bytesToSkip:bytesToSkip
                                  completionHandler:completionHandler];
        }
    });

    return CHIP_NO_ERROR;
}

void MTROTAImageTransferHandler::HandleAsyncTransferSessionOutput(TransferSession::OutputEvent & event)
{
    VerifyOrReturn(mDelegate != nil);

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.EventType) {
    case TransferSession::OutputEventType::kInitReceived:
        err = OnTransferSessionBegin(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            AsyncResponder::NotifyEventHandledWithError(err);
            ResetState();
        }
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        [[fallthrough]];
    case TransferSession::OutputEventType::kAckEOFReceived:
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
        err = OnTransferSessionEnd(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            AsyncResponder::NotifyEventHandledWithError(err);
            ResetState();
        }
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
        err = OnBlockQuery(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            AsyncResponder::NotifyEventHandledWithError(err);
            ResetState();
        }
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        err = OnMessageToSend(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            AsyncResponder::NotifyEventHandledWithError(err);
            ResetState();
        }
        break;
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kAckReceived:
        // Nothing to do.
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        // Should never happens.
        chipDie();
        break;
    }
    LogErrorOnFailure(err);
}

CHIP_ERROR MTROTAImageTransferHandler::ConfigureState(chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:fabricIndex];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

    mDelegate = controller.otaProviderDelegate;
    mDelegateNotificationQueue = controller.otaProviderDelegateQueue;

    // We should have already checked that this controller supports OTA.
    VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDelegateNotificationQueue != nil, CHIP_ERROR_INCORRECT_STATE);

    // Start a timer to track whether we receive a BDX init after a successful query image in a reasonable amount of time
    CHIP_ERROR err
        = chip::DeviceLayer::SystemLayer().StartTimer(kBdxInitReceivedTimeout, HandleBdxInitReceivedTimeoutExpired, this);
    LogErrorOnFailure(err);

    ReturnErrorOnFailure(err);

    mFabricIndex.SetValue(fabricIndex);
    mNodeId.SetValue(nodeId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROTAImageTransferHandler::OnMessageReceived(
    chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader, chip::System::PacketBufferHandle && payload)
{
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err;
    ChipLogProgress(BDX, "%s: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId, __FUNCTION__,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    // If we receive a ReceiveInit message, then we prepare for transfer
    if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
        NodeId nodeId = ec->GetSessionHandle()->GetPeer().GetNodeId();
        FabricIndex fabricIndex = ec->GetSessionHandle()->GetFabricIndex();

        if (nodeId != kUndefinedNodeId && fabricIndex != kUndefinedFabricIndex) {
            err = PrepareForTransfer(ec, fabricIndex, nodeId);
            if (err != CHIP_NO_ERROR) {
                ChipLogError(Controller, "Failed to prepare for transfer for BDX");
            }
        }
    }

    // Send the message to the AsyncFacilitator to drive the BDX session state machine
    AsyncTransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));
    return err;
}
