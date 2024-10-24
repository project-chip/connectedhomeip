/**
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

#import "MTROTAImageTransferHandler.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTROTAUnsolicitedBDXMessageHandler.h"
#import "NSStringSpanConversion.h"

using namespace chip;
using namespace chip::bdx;
using namespace chip::app;

constexpr uint32_t kMaxBdxBlockSize = 1024;

// Timeout for the BDX transfer session. The OTA Spec mandates this should be >= 5 minutes.
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);

constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kSender;

@interface MTROTAImageTransferHandlerWrapper : NSObject
- (instancetype)initWithMTROTAImageTransferHandler:(MTROTAImageTransferHandler *)otaImageTransferHandler;
@property (nonatomic, nullable, readwrite, assign) MTROTAImageTransferHandler * otaImageTransferHandler;
@end

@implementation MTROTAImageTransferHandlerWrapper

- (instancetype)initWithMTROTAImageTransferHandler:(MTROTAImageTransferHandler *)otaImageTransferHandler
{
    if (self = [super init]) {
        _otaImageTransferHandler = otaImageTransferHandler;
    }
    return self;
}
@end

MTROTAImageTransferHandlerWrapper * mOTAImageTransferHandlerWrapper;

MTROTAImageTransferHandler::MTROTAImageTransferHandler()
{
    assertChipStackLockedByCurrentThread();

    MTROTAUnsolicitedBDXMessageHandler::GetInstance()->OnDelegateCreated(this);
    mOTAImageTransferHandlerWrapper = [[MTROTAImageTransferHandlerWrapper alloc] initWithMTROTAImageTransferHandler:this];
}

CHIP_ERROR MTROTAImageTransferHandler::Init(System::Layer * layer,
    Messaging::ExchangeContext * exchangeCtx, FabricIndex fabricIndex, NodeId nodeId)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(layer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = layer;

    ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);

    return AsyncResponder::Init(mSystemLayer, exchangeCtx, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout);
}

MTROTAImageTransferHandler::~MTROTAImageTransferHandler()
{
    assertChipStackLockedByCurrentThread();
    mFabricIndex.ClearValue();
    mNodeId.ClearValue();
    mDelegate = nil;
    mDelegateNotificationQueue = nil;
    mSystemLayer = nil;

    MTROTAUnsolicitedBDXMessageHandler::GetInstance()->OnDelegateDestroyed(this);
    mOTAImageTransferHandlerWrapper.otaImageTransferHandler = nullptr;
}

CHIP_ERROR MTROTAImageTransferHandler::OnTransferSessionBegin(TransferSession::OutputEvent & event)
{
    assertChipStackLockedByCurrentThread();

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

    MTROTAImageTransferHandlerWrapper * __weak weakWrapper = mOTAImageTransferHandlerWrapper;

    auto completionHandler = ^(NSError * _Nullable error) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                // Check if the OTA image transfer handler is still valid. If not, return from the completion handler.
                MTROTAImageTransferHandlerWrapper * strongWrapper = weakWrapper;
                if (!strongWrapper || !strongWrapper.otaImageTransferHandler) {
                    return;
                }

                if (error != nil) {
                    CHIP_ERROR err = [MTRError errorToCHIPErrorCode:error];
                    LogErrorOnFailure(err);
                    OnTransferSessionEnd(event);
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

                CHIP_ERROR err = mTransfer.AcceptTransfer(acceptData);
                LogErrorOnFailure(err);
                AsyncResponder::NotifyEventHandled(event, err);
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    auto nodeId = @(mNodeId.Value());

    auto strongDelegate = mDelegate;
    auto delegateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delegateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        AsyncResponder::NotifyEventHandled(event, CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    dispatch_async(delegateQueue, ^{
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
    } else if (event.EventType != TransferSession::OutputEventType::kAckEOFReceived) {
        error = CHIP_ERROR_INTERNAL;
    }

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);
    auto nodeId = @(mNodeId.Value());

    auto strongDelegate = mDelegate;
    auto delegateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delegateQueue == nil) {
        error = CHIP_ERROR_INCORRECT_STATE;
        LogErrorOnFailure(error);
        AsyncResponder::NotifyEventHandled(event, error);
        return error;
    }

    if ([strongDelegate respondsToSelector:@selector(handleBDXTransferSessionEndForNodeID:controller:error:)]) {
        dispatch_async(delegateQueue, ^{
            [strongDelegate handleBDXTransferSessionEndForNodeID:nodeId
                                                      controller:controller
                                                           error:[MTRError errorForCHIPErrorCode:error]];
        });
    }
    AsyncResponder::NotifyEventHandled(event, error);
    return error;
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

    MTROTAImageTransferHandlerWrapper * __weak weakWrapper = mOTAImageTransferHandlerWrapper;

    auto completionHandler = ^(NSData * _Nullable data, BOOL isEOF) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                // Check if the OTA image transfer handler is still valid. If not, return from the completion handler.
                MTROTAImageTransferHandlerWrapper * strongWrapper = weakWrapper;
                if (!strongWrapper || !strongWrapper.otaImageTransferHandler) {
                    return;
                }

                if (data == nil) {
                    AsyncResponder::NotifyEventHandled(event, CHIP_ERROR_INCORRECT_STATE);
                    return;
                }

                TransferSession::BlockData blockData;
                blockData.Data = static_cast<const uint8_t *>([data bytes]);
                blockData.Length = static_cast<size_t>([data length]);
                blockData.IsEof = isEOF;

                CHIP_ERROR err = mTransfer.PrepareBlock(blockData);
                LogErrorOnFailure(err);
                AsyncResponder::NotifyEventHandled(event, err);
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    // TODO Handle MaxLength

    auto nodeId = @(mNodeId.Value());

    auto strongDelegate = mDelegate;
    auto delegateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delegateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        AsyncResponder::NotifyEventHandled(event, CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    dispatch_async(delegateQueue, ^{
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

void MTROTAImageTransferHandler::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    VerifyOrReturn(mDelegate != nil);

    ChipLogError(BDX, "OutputEvent type: %s", event.ToString(event.EventType));

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.EventType) {
    case TransferSession::OutputEventType::kInitReceived:
        err = OnTransferSessionBegin(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            AsyncResponder::NotifyEventHandled(event, err);
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
            AsyncResponder::NotifyEventHandled(event, err);
        }
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived:
        err = OnBlockQuery(event);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            AsyncResponder::NotifyEventHandled(event, err);
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
}

void MTROTAImageTransferHandler::DestroySelf()
{
    assertChipStackLockedByCurrentThread();

    delete this;
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

    mFabricIndex.SetValue(fabricIndex);
    mNodeId.SetValue(nodeId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTROTAImageTransferHandler::OnMessageReceived(
    Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    assertChipStackLockedByCurrentThread();

    ChipLogProgress(BDX, "MTROTAImageTransferHandler: OnMessageReceived: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err;

    // If we receive a ReceiveInit message, then we prepare for transfer. Otherwise we send the message
    // received to the AsyncTransferFacilitator for processing.
    if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
        NodeId nodeId = ec->GetSessionHandle()->GetPeer().GetNodeId();
        FabricIndex fabricIndex = ec->GetSessionHandle()->GetFabricIndex();

        if (nodeId != kUndefinedNodeId && fabricIndex != kUndefinedFabricIndex) {
            err = Init(&DeviceLayer::SystemLayer(), ec, fabricIndex, nodeId);
            if (err != CHIP_NO_ERROR) {
                ChipLogError(Controller, "OnMessageReceived: Failed to prepare for transfer for BDX");
                return err;
            }
        }
    }

    // Send the message to the AsyncFacilitator to drive the BDX session state machine.
    AsyncTransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));
    return err;
}
