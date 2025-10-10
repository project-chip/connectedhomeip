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
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTROTAUnsolicitedBDXMessageHandler.h"
#import "NSStringSpanConversion.h"

#include <chrono>
#include <platform/Darwin/UserDefaults.h>

using namespace chip;
using namespace chip::bdx;
using namespace chip::app;
using namespace chip::Tracing::DarwinFramework;

constexpr uint16_t kMaxBdxBlockSize = 1024;

// For Thread networks we want to reduce the BDX block size a bit, to reduce
// the number of frames per BDX block. To pick a BDX block size for Thread
// networks we came up with a number based on:
//
//  IEEE 802.15.4 frame size = 127 bytes
//
//  First fragment:
//    IEEE 802.15.4 Header         = 21 bytes
//    Fragmentation Header         = 4 bytes
//    IPHC & UDPHC                 = 23 bytes
//    Mesh Header                  = 5 bytes (applies only for multi hop)
//  Subsequent fragments:
//    IEEE 802.15.4 Header         = 21 bytes
//    Fragmentation Header         = 6 bytes
//    Mesh Header                  = 5 bytes (applies only for multi hop)
//
// That leaves 74 bytes in the first fragment and 95 bytes in following fragments.
//
//  Matter Message Header          = 8 bytes
//  Matter Protocol Header         = 10 bytes (including an Acknowledged Message Counter)
//  Matter Message Footer          = 16 bytes
//
//  BDX Counter                    = 4 bytes
//
// So the overhead per BDX block is 38 bytes.
//
// Given that, a BDX block of 511 bytes would fit into 6 frames, which is the
// current default value.
//
// The number of Thread frames that we'll use per BDX block can be overridden
// with `defaults write <domain> BDXThreadFramesPerBlock <numberOfFrames>`.

constexpr uint8_t kMaxThreadFramesPerBdxBlock = 6;

static constexpr uint16_t ComputeBDXBlockSizeForThread(uint8_t framesPerBlock)
{
    return 74 + (framesPerBlock - 1) * 95 - 38;
}

// For now, don't use kMaxThreadFramesPerBdxBlock unless explicitly opted in.
// The user default will still be respected if it's set.
constexpr bool kUseSmartBlockSizingForThread = false;

// Timeout for the BDX transfer session. The OTA Spec mandates this should be >= 5 minutes.
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60);

// For thread devices, we may want to throttle sending Blocks in response to BlockQuery messages
// to avoid spamming the network with too many BDX messages.  For now, default to the old 50ms
// polling interval we used to have.
// To override the throttle interval,
// use ` defaults write <domain> BDXThrottleIntervalForThreadDevicesInMSecs <throttleIntervalinMsecs>`
// See UserDefaults.mm for details.
constexpr auto kBdxThrottleDefaultInterval = System::Clock::Milliseconds32(50);

constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kSender;

// An ARC-managed object that lets us do weak references to a MTROTAImageTransferHandler
// (which is a C++ object).
@interface MTROTAImageTransferHandlerWrapper : NSObject
- (instancetype)initWithMTROTAImageTransferHandler:(MTROTAImageTransferHandler *)otaImageTransferHandler;

// Non-atomic property since it's read or written only on the Matter queue.
@property (nonatomic, nullable, readwrite, assign) MTROTAImageTransferHandler * otaImageTransferHandler;
@end

@implementation MTROTAImageTransferHandlerWrapper

- (instancetype)initWithMTROTAImageTransferHandler:(MTROTAImageTransferHandler *)otaImageTransferHandler
{
    assertChipStackLockedByCurrentThread();
    if (self = [super init]) {
        _otaImageTransferHandler = otaImageTransferHandler;
    }
    return self;
}

- (MTROTAImageTransferHandler *)otaImageTransferHandler
{
    assertChipStackLockedByCurrentThread();
    return _otaImageTransferHandler;
}

- (void)SetOtaImageTransferHandler:(MTROTAImageTransferHandler *)otaImageTransferHandler
{
    assertChipStackLockedByCurrentThread();
    _otaImageTransferHandler = otaImageTransferHandler;
}
@end

MTROTAImageTransferHandler::MTROTAImageTransferHandler(System::Layer * layer)
{
    assertChipStackLockedByCurrentThread();

    mSystemLayer = layer;

    MTROTAUnsolicitedBDXMessageHandler::GetInstance()->OnTransferHandlerCreated(this);
    mOTAImageTransferHandlerWrapper = [[MTROTAImageTransferHandlerWrapper alloc] initWithMTROTAImageTransferHandler:this];
}

CHIP_ERROR MTROTAImageTransferHandler::Init(Messaging::ExchangeContext * exchangeCtx)
{
    assertChipStackLockedByCurrentThread();

    mPeer = GetPeerScopedNodeId(exchangeCtx);
    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mPeer.GetFabricIndex()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

    mDelegate = controller.otaProviderDelegate;
    mDelegateNotificationQueue = controller.otaProviderDelegateQueue;

    // We should have already checked that this controller supports OTA.
    VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDelegateNotificationQueue != nil, CHIP_ERROR_INCORRECT_STATE);

    mIsPeerNodeAKnownThreadDevice = [controller definitelyUsesThreadForDevice:mPeer.GetNodeId()];

    mBDXThrottleIntervalForThreadDevices = Platform::GetUserDefaultBDXThrottleIntervalForThread().value_or(kBdxThrottleDefaultInterval);

    BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);

    uint16_t blockSize;
    if (mIsPeerNodeAKnownThreadDevice) {
        auto framesPerBlock = Platform::GetUserDefaultBDXThreadFramesPerBlock();
        if (framesPerBlock.has_value()) {
            blockSize = ComputeBDXBlockSizeForThread(Platform::GetUserDefaultBDXThreadFramesPerBlock().value());
        } else if (kUseSmartBlockSizingForThread) {
            blockSize = ComputeBDXBlockSizeForThread(kMaxThreadFramesPerBdxBlock);
        } else {
            blockSize = kMaxBdxBlockSize;
        }
    } else {
        blockSize = kMaxBdxBlockSize;
    }
    return AsyncResponder::Init(mSystemLayer, exchangeCtx, kBdxRole, flags, blockSize, kBdxTimeout);
}

MTROTAImageTransferHandler::~MTROTAImageTransferHandler()
{
    assertChipStackLockedByCurrentThread();

    if (mNeedToCallTransferSessionEnd) {
        // TODO: Store the actual error involved in error cases, so we can pass the right thing here.
        InvokeTransferSessionEndCallback(CHIP_ERROR_INTERNAL);
    }

    MTROTAUnsolicitedBDXMessageHandler::GetInstance()->OnTransferHandlerDestroyed(this);
    mOTAImageTransferHandlerWrapper.otaImageTransferHandler = nullptr;
}

CHIP_ERROR MTROTAImageTransferHandler::OnTransferSessionBegin(const TransferSession::OutputEventType eventType)
{
    assertChipStackLockedByCurrentThread();

    uint16_t fdl = 0;
    auto fd = mTransfer.GetFileDesignator(fdl);
    VerifyOrReturnError(fdl <= bdx::kMaxFileDesignatorLen, CHIP_ERROR_INCORRECT_STATE);
    CharSpan fileDesignatorSpan(Uint8::to_const_char(fd), fdl);

    auto fileDesignator = AsString(fileDesignatorSpan);
    if (fileDesignator == nil) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    auto offset = @(mTransfer.GetStartOffset());

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mPeer.GetFabricIndex()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

    mNumBytesProcessed = 0;

    MATTER_LOG_METRIC_BEGIN(kMetricOTATransfer);
    MATTER_LOG_METRIC(kMetricOTATransferOffset, uint32_t(mTransfer.GetStartOffset()));

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
                    NotifyEventHandled(eventType, err);
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
                NotifyEventHandled(eventType, err);
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here, but if the controller is shut down we
                              // should already have been destroyed anyway.
                          }];
    };

    auto nodeId = @(mPeer.GetNodeId());

    auto strongDelegate = mDelegate;
    auto delegateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delegateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
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
    mNeedToCallTransferSessionEnd = true;
    return CHIP_NO_ERROR;
}

void MTROTAImageTransferHandler::InvokeTransferSessionEndCallback(CHIP_ERROR error)
{
    assertChipStackLockedByCurrentThread();

    mNeedToCallTransferSessionEnd = false;

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mPeer.GetFabricIndex()];
    VerifyOrReturn(controller != nil);
    auto nodeId = @(mPeer.GetNodeId());

    auto strongDelegate = mDelegate;
    auto delegateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delegateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    auto * device = [MTRDevice deviceWithNodeID:nodeId controller:controller];

    MATTER_LOG_METRIC(kMetricOTADeviceVendorID, device.vendorID.unsignedIntValue);
    MATTER_LOG_METRIC(kMetricOTADeviceProductID, device.productID.unsignedIntValue);
    MATTER_LOG_METRIC(kMetricOTADeviceUsesThread, mIsPeerNodeAKnownThreadDevice);
    MATTER_LOG_METRIC(kMetricOTATNumBytesProcessed, uint32_t(mNumBytesProcessed));
    MATTER_LOG_METRIC_END(kMetricOTATransfer, error);

    // Always collect the metrics to avoid unbounded growth of the stats in the collector
    MTRMetrics * metrics = [[MTRMetricsCollector sharedInstance] metricSnapshotForCategory:@("ota") removeMetrics:YES];
    auto nsError = [MTRError errorForCHIPErrorCode:error];
    if ([strongDelegate respondsToSelector:@selector(handleBDXTransferSessionEndForNodeID:controller:metrics:error:)]) {
        dispatch_async(delegateQueue, ^{
            [strongDelegate handleBDXTransferSessionEndForNodeID:nodeId controller:controller
                                                         metrics:metrics
                                                           error:nsError];
        });
    } else if ([strongDelegate respondsToSelector:@selector(handleBDXTransferSessionEndForNodeID:controller:error:)]) {
        dispatch_async(delegateQueue, ^{
            [strongDelegate handleBDXTransferSessionEndForNodeID:nodeId
                                                      controller:controller
                                                           error:nsError];
        });
    }
}

CHIP_ERROR MTROTAImageTransferHandler::OnTransferSessionEnd(const TransferSession::OutputEventType eventType)
{
    assertChipStackLockedByCurrentThread();

    CHIP_ERROR error = CHIP_NO_ERROR;
    if (eventType == TransferSession::OutputEventType::kTransferTimeout) {
        error = CHIP_ERROR_TIMEOUT;
    } else if (eventType != TransferSession::OutputEventType::kAckEOFReceived) {
        error = CHIP_ERROR_INTERNAL;
    }

    InvokeTransferSessionEndCallback(error);

    if (error == CHIP_NO_ERROR) {
        NotifyEventHandled(eventType, error);
    }
    return error;
}

CHIP_ERROR MTROTAImageTransferHandler::OnBlockQuery(const TransferSession::OutputEventType eventType, uint64_t bytesToSkip)
{
    assertChipStackLockedByCurrentThread();

    // For thread devices, we need to throttle sending the response to
    // BlockQuery, so need to track when we started handling BlockQuery.
    auto startBlockQueryHandlingTimestamp = System::SystemClock().GetMonotonicTimestamp();

    auto blockSize = @(mTransfer.GetTransferBlockSize());
    auto blockIndex = @(mTransfer.GetNextBlockNum());

    auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:mPeer.GetFabricIndex()];
    VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

    MTROTAImageTransferHandlerWrapper * __weak weakWrapper = mOTAImageTransferHandlerWrapper;

    auto respondWithBlock = ^(NSData * _Nullable data, BOOL isEOF) {
        if (data) {
            mNumBytesProcessed += data.length;
        }
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                // Check if the OTA image transfer handler is still valid. If not, return from the completion handler.
                MTROTAImageTransferHandlerWrapper * strongWrapper = weakWrapper;
                if (!strongWrapper || !strongWrapper.otaImageTransferHandler) {
                    return;
                }

                if (data == nil) {
                    MTR_LOG_ERROR("Nil OTA data block when updating " ChipLogFormatScopedNodeId,
                        ChipLogValueScopedNodeId(strongWrapper.otaImageTransferHandler->mPeer));
                    NotifyEventHandled(eventType, CHIP_ERROR_INCORRECT_STATE);
                    return;
                }

                if (data.length != blockSize.unsignedLongLongValue && !isEOF) {
                    // "Transfer of OTA Software Update images" in the spec says:
                    //
                    // Actual Block Size used over all transports SHALL be the negotiated Maximum
                    // Block Size for every block except the last one, which may be of any size less
                    // or equal to the Maximum Block Size (including zero).
                    MTR_LOG_ERROR("Invalid OTA block size %lu for non-final block when updating " ChipLogFormatScopedNodeId ".  Expected block of size %@",
                        static_cast<unsigned long>(data.length), ChipLogValueScopedNodeId(strongWrapper.otaImageTransferHandler->mPeer), blockSize);
                    NotifyEventHandled(eventType, CHIP_ERROR_INCORRECT_STATE);
                    return;
                }

                TransferSession::BlockData blockData;
                blockData.Data = static_cast<const uint8_t *>([data bytes]);
                blockData.Length = static_cast<size_t>([data length]);
                blockData.IsEof = isEOF;

                CHIP_ERROR err = mTransfer.PrepareBlock(blockData);
                LogErrorOnFailure(err);
                NotifyEventHandled(eventType, err);
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here, but if the controller is shut down we
                              // should already have been destroyed anyway.
                          }];
    };

    // TODO Handle MaxLength

    auto nodeId = @(mPeer.GetNodeId());

    auto strongDelegate = mDelegate;
    auto delegateQueue = mDelegateNotificationQueue;
    if (strongDelegate == nil || delegateQueue == nil) {
        LogErrorOnFailure(CHIP_ERROR_INCORRECT_STATE);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    void (^completionHandler)(NSData * _Nullable data, BOOL isEOF) = nil;

    // If the peer node is a Thread device, check how much time has elapsed since we started processing the BlockQuery,
    // round it up to the nearest multiple of kBdxThrottleDefaultInterval, and respond with the block at that point.
    if (mIsPeerNodeAKnownThreadDevice) {
        completionHandler = ^(NSData * _Nullable data, BOOL isEOF) {
            auto timeElapsed = System::SystemClock().GetMonotonicTimestamp() - startBlockQueryHandlingTimestamp;
            // Integer division rounds down, so dividing by mBDXThrottleIntervalForThreadDevices and then multiplying
            // by it again rounds down to the nearest multiple of mBDXThrottleIntervalForThreadDevices.
            auto remainder = timeElapsed - (timeElapsed / mBDXThrottleIntervalForThreadDevices) * mBDXThrottleIntervalForThreadDevices;

            if (remainder == System::Clock::Milliseconds32(0)) {
                respondWithBlock(data, isEOF);
            } else {
                auto nsRemaining = std::chrono::duration_cast<std::chrono::nanoseconds>(remainder);
                dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, nsRemaining.count());
                dispatch_after(time, delegateQueue, ^{
                    respondWithBlock(data, isEOF);
                });
            }
        };
    } else {
        completionHandler = respondWithBlock;
    }

    dispatch_async(delegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleBDXQueryForNodeID:
                                                                      controller:blockSize:blockIndex:bytesToSkip:completion:)]) {
            [strongDelegate handleBDXQueryForNodeID:nodeId
                                         controller:controller
                                          blockSize:blockSize
                                         blockIndex:blockIndex
                                        bytesToSkip:@(bytesToSkip)
                                         completion:completionHandler];
        } else {
            [strongDelegate handleBDXQueryForNodeID:nodeId
                                         controller:controller
                                          blockSize:blockSize
                                         blockIndex:blockIndex
                                        bytesToSkip:@(bytesToSkip)
                                  completionHandler:completionHandler];
        }
    });
    return CHIP_NO_ERROR;
}

void MTROTAImageTransferHandler::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    VerifyOrReturn(mDelegate != nil);

    TransferSession::OutputEventType eventType = event.EventType;

    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.EventType) {
    case TransferSession::OutputEventType::kInitReceived:
        err = OnTransferSessionBegin(eventType);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            NotifyEventHandled(eventType, err);
        }
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        [[fallthrough]];
    case TransferSession::OutputEventType::kAckEOFReceived:
    case TransferSession::OutputEventType::kInternalError:
    case TransferSession::OutputEventType::kTransferTimeout:
        err = OnTransferSessionEnd(eventType);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            NotifyEventHandled(eventType, err);
        }
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kQueryReceived: {
        uint64_t bytesToSkip = (eventType == TransferSession::OutputEventType::kQueryWithSkipReceived ? event.bytesToSkip.BytesToSkip : 0);
        err = OnBlockQuery(eventType, bytesToSkip);
        if (err != CHIP_NO_ERROR) {
            LogErrorOnFailure(err);
            NotifyEventHandled(eventType, err);
        }
        break;
    }
    case TransferSession::OutputEventType::kNone:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        // Should never happen since this implements an OTA provider in BDX Receiver role.
        chipDie();
        break;
    }
}

void MTROTAImageTransferHandler::DestroySelf()
{
    assertChipStackLockedByCurrentThread();

    delete this;
}

ScopedNodeId MTROTAImageTransferHandler::GetPeerScopedNodeId(Messaging::ExchangeContext * ec)
{
    auto sessionHandle = ec->GetSessionHandle();

    if (sessionHandle->IsSecureSession()) {
        return sessionHandle->AsSecureSession()->GetPeer();
    } else if (sessionHandle->IsGroupSession()) {
        return sessionHandle->AsIncomingGroupSession()->GetPeer();
    }
    return ScopedNodeId();
}

CHIP_ERROR MTROTAImageTransferHandler::OnMessageReceived(
    Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader, System::PacketBufferHandle && payload)
{
    assertChipStackLockedByCurrentThread();

    ChipLogProgress(BDX, "MTROTAImageTransferHandler: OnMessageReceived: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);
    CHIP_ERROR err;

    // If we receive a ReceiveInit message, then we prepare for transfer.
    //
    // If init succeeds, or is not needed, we send the message to the AsyncTransferFacilitator for processing.
    if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
        err = Init(ec);
        if (err != CHIP_NO_ERROR) {
            ChipLogError(Controller, "OnMessageReceived: Failed to prepare for transfer for BDX: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
    }

    // Send the message to the AsyncFacilitator to drive the BDX session state machine.
    AsyncTransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));
    return err;
}
