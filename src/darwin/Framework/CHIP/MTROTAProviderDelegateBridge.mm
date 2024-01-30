/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTROTAProviderDelegateBridge.h"
#import "MTRBaseClusters.h"
#import "MTRCommandPayloadsObjC.h"
#import "MTRDeviceControllerFactory_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app/clusters/ota-provider/ota-provider.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/Global.h>
#include <lib/support/TypeTraits.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/Constants.h>

#include <MTRError_Internal.h>
#include <messaging/ExchangeMgr.h>
#include <platform/LockTracker.h>
#include <protocols/bdx/BdxUri.h>
#include <protocols/bdx/TransferFacilitator.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace chip::bdx;
using Protocols::InteractionModel::Status;

// TODO Expose a method onto the delegate to make that configurable.
constexpr uint32_t kMaxBdxBlockSize = 1024;
constexpr uint32_t kMaxBDXURILen = 256;

// Since the BDX timeout is 5 minutes and we are starting this after query image is available and before the BDX init comes,
// we just double the timeout to give enough time for the BDX init to come in a reasonable amount of time.
constexpr System::Clock::Timeout kBdxInitReceivedTimeout = System::Clock::Seconds16(10 * 60);

// Time in seconds after which the requestor should retry calling query image if
// busy status is receieved.  The spec minimum is 2 minutes, but in practice OTA
// generally takes a lot longer than that and devices only retry a few times
// before giving up.  Default to 10 minutes for now, until we have a better
// system of computing an expected completion time for the currently-running
// OTA.
constexpr uint32_t kDelayedActionTimeSeconds = 600;

constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);
constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kSender;

class BdxOTASender : public bdx::Responder {
public:
    BdxOTASender() {};

    CHIP_ERROR PrepareForTransfer(FabricIndex fabricIndex, NodeId nodeId)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

        ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

        BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);
        return Responder::PrepareForTransfer(mSystemLayer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollIntervalMs);
    }

    CHIP_ERROR Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mSystemLayer == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mExchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

        exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);

        mSystemLayer = systemLayer;
        mExchangeMgr = exchangeMgr;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Shutdown()
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
        ResetState();

        mExchangeMgr = nullptr;
        mSystemLayer = nullptr;

        return CHIP_NO_ERROR;
    }

    void ControllerShuttingDown(MTRDeviceController * controller)
    {
        assertChipStackLockedByCurrentThread();

        if (mInitialized && mFabricIndex.Value() == controller.fabricIndex) {
            ResetState();
        }
    }

    void ResetState()
    {
        assertChipStackLockedByCurrentThread();
        if (mNodeId.HasValue() && mFabricIndex.HasValue()) {
            ChipLogProgress(Controller,
                "Resetting state for OTA Provider; no longer providing an update for node id 0x" ChipLogFormatX64
                ", fabric index %u",
                ChipLogValueX64(mNodeId.Value()), mFabricIndex.Value());
        } else {
            ChipLogProgress(Controller, "Resetting state for OTA Provider");
        }
        if (mSystemLayer) {
            mSystemLayer->CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);
        }
        // TODO: Check if this can be removed. It seems like we can close the exchange context and reset transfer regardless.
        if (!mInitialized) {
            return;
        }
        Responder::ResetTransfer();
        ++mTransferGeneration;
        mFabricIndex.ClearValue();
        mNodeId.ClearValue();

        if (mExchangeCtx != nullptr) {
            mExchangeCtx->Close();
            mExchangeCtx = nullptr;
        }

        mDelegate = nil;
        mDelegateNotificationQueue = nil;

        mInitialized = false;
    }

private:
    /**
     * Timer callback called when we don't receive a BDX init within a reasonable time after a successful QueryImage response.
     */
    static void HandleBdxInitReceivedTimeoutExpired(chip::System::Layer * systemLayer, void * state)
    {
        VerifyOrReturn(state != nullptr);
        static_cast<BdxOTASender *>(state)->ResetState();
    }

    CHIP_ERROR OnMessageToSend(TransferSession::OutputEvent & event)
    {
        assertChipStackLockedByCurrentThread();

        VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);

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
            ResetState();
        } else if (event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
            // If the send was successful for a status report, since we are not expecting a response the exchange context is
            // already closed. We need to null out the reference to avoid having a dangling pointer.
            mExchangeCtx = nullptr;
            ResetState();
        }
        return err;
    }

    CHIP_ERROR OnTransferSessionBegin(TransferSession::OutputEvent & event)
    {
        assertChipStackLockedByCurrentThread();
        // Once we receive the BDX init, cancel the BDX Init timeout and start the BDX session
        if (mSystemLayer) {
            mSystemLayer->CancelTimer(HandleBdxInitReceivedTimeoutExpired, this);
        }

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

        auto transferGeneration = mTransferGeneration;

        auto completionHandler = ^(NSError * _Nullable error) {
            [controller
                asyncDispatchToMatterQueue:^() {
                    assertChipStackLockedByCurrentThread();

                    if (!mInitialized || mTransferGeneration != transferGeneration) {
                        // Callback for a stale transfer.
                        return;
                    }

                    if (error != nil) {
                        CHIP_ERROR err = [MTRError errorToCHIPErrorCode:error];
                        LogErrorOnFailure(err);
                        LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err)));
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
                }
                              errorHandler:^(NSError *) {
                                  // Not much we can do here
                              }];
        };

        auto nodeId = @(mNodeId.Value());

        auto strongDelegate = mDelegate;
        dispatch_async(mDelegateNotificationQueue, ^{
            if ([strongDelegate respondsToSelector:@selector(handleBDXTransferSessionBeginForNodeID:controller:fileDesignator:offset:completion:)]) {
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

    CHIP_ERROR OnTransferSessionEnd(TransferSession::OutputEvent & event)
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
        if ([strongDelegate respondsToSelector:@selector(handleBDXTransferSessionEndForNodeID:controller:error:)]) {
            dispatch_async(mDelegateNotificationQueue, ^{
                [strongDelegate handleBDXTransferSessionEndForNodeID:nodeId
                                                          controller:controller
                                                               error:[MTRError errorForCHIPErrorCode:error]];
            });
        }

        ResetState();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnBlockQuery(TransferSession::OutputEvent & event)
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

        auto transferGeneration = mTransferGeneration;

        auto completionHandler = ^(NSData * _Nullable data, BOOL isEOF) {
            [controller
                asyncDispatchToMatterQueue:^() {
                    assertChipStackLockedByCurrentThread();

                    if (!mInitialized || mTransferGeneration != transferGeneration) {
                        // Callback for a stale transfer.
                        return;
                    }

                    if (data == nil) {
                        LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
                        return;
                    }

                    TransferSession::BlockData blockData;
                    blockData.Data = static_cast<const uint8_t *>([data bytes]);
                    blockData.Length = static_cast<size_t>([data length]);
                    blockData.IsEof = isEOF;

                    CHIP_ERROR err = mTransfer.PrepareBlock(blockData);
                    if (CHIP_NO_ERROR != err) {
                        LogErrorOnFailure(err);
                        LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
                    }
                }
                              errorHandler:^(NSError *) {
                                  // Not much we can do here
                              }];
        };

        // TODO Handle MaxLength

        auto nodeId = @(mNodeId.Value());

        auto strongDelegate = mDelegate;
        dispatch_async(mDelegateNotificationQueue, ^{
            if ([strongDelegate respondsToSelector:@selector(handleBDXQueryForNodeID:controller:blockSize:blockIndex:bytesToSkip:completion:)]) {
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

    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override
    {
        VerifyOrReturn(mDelegate != nil);

        CHIP_ERROR err = CHIP_NO_ERROR;
        switch (event.EventType) {
        case TransferSession::OutputEventType::kInitReceived:
            err = OnTransferSessionBegin(event);
            if (err != CHIP_NO_ERROR) {
                LogErrorOnFailure(mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err)));
            }
            break;
        case TransferSession::OutputEventType::kStatusReceived:
            ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
            [[fallthrough]];
        case TransferSession::OutputEventType::kAckEOFReceived:
        case TransferSession::OutputEventType::kInternalError:
        case TransferSession::OutputEventType::kTransferTimeout:
            err = OnTransferSessionEnd(event);
            break;
        case TransferSession::OutputEventType::kQueryWithSkipReceived:
        case TransferSession::OutputEventType::kQueryReceived:
            err = OnBlockQuery(event);
            break;
        case TransferSession::OutputEventType::kMsgToSend:
            err = OnMessageToSend(event);
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

    CHIP_ERROR ConfigureState(chip::FabricIndex fabricIndex, chip::NodeId nodeId)
    {
        assertChipStackLockedByCurrentThread();

        if (mInitialized) {
            // Prevent a new node connection since another is active.
            VerifyOrReturnError(mFabricIndex.Value() == fabricIndex && mNodeId.Value() == nodeId, CHIP_ERROR_BUSY);

            // Reset stale connection from the same Node if exists.
            ResetState();
        }

        auto * controller = [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:fabricIndex];
        VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);

        mDelegate = controller.otaProviderDelegate;
        mDelegateNotificationQueue = controller.otaProviderDelegateQueue;

        // We should have already checked that this controller supports OTA.
        VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDelegateNotificationQueue != nil, CHIP_ERROR_INCORRECT_STATE);

        // Start a timer to track whether we receive a BDX init after a successful query image in a reasonable amount of time
        CHIP_ERROR err = mSystemLayer->StartTimer(kBdxInitReceivedTimeout, HandleBdxInitReceivedTimeoutExpired, this);
        LogErrorOnFailure(err);

        ReturnErrorOnFailure(err);

        mFabricIndex.SetValue(fabricIndex);
        mNodeId.SetValue(nodeId);

        mInitialized = true;

        return CHIP_NO_ERROR;
    }

    bool mInitialized = false;
    Optional<FabricIndex> mFabricIndex;
    Optional<NodeId> mNodeId;
    id<MTROTAProviderDelegate> mDelegate = nil;
    dispatch_queue_t mDelegateNotificationQueue = nil;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;

    // Since we are a singleton, we get reused across transfers, but also have
    // async calls that can happen.  The transfer generation keeps track of
    // which transfer we are currently doing, so we can ignore async calls
    // attached to no-longer-running transfers.
    uint64_t mTransferGeneration = 0;
};

namespace {
Global<BdxOTASender> gOtaSender;

NSInteger constexpr kOtaProviderEndpoint = 0;
} // anonymous namespace

MTROTAProviderDelegateBridge::MTROTAProviderDelegateBridge() { Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this); }

MTROTAProviderDelegateBridge::~MTROTAProviderDelegateBridge()
{
    gOtaSender->ResetState();
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, nullptr);
}

CHIP_ERROR MTROTAProviderDelegateBridge::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeManager)
{
    return gOtaSender->Init(systemLayer, exchangeManager);
}

void MTROTAProviderDelegateBridge::Shutdown() { gOtaSender->Shutdown(); }

void MTROTAProviderDelegateBridge::ControllerShuttingDown(MTRDeviceController * controller)
{
    gOtaSender->ControllerShuttingDown(controller);
}

namespace {
// Return false if we could not get peer node info (a running controller for
// the fabric and a node id).  In that case we will have already added an
// error status to the CommandHandler.
//
// Otherwise set outNodeId and outController to values that identify the source
// node for the command.
bool GetPeerNodeInfo(CommandHandler * commandHandler, const ConcreteCommandPath & commandPath, NodeId * outNodeId,
    MTRDeviceController * __autoreleasing _Nonnull * _Nonnull outController)
{
    auto desc = commandHandler->GetSubjectDescriptor();
    if (desc.authMode != Access::AuthMode::kCase) {
        commandHandler->AddStatus(commandPath, Status::Failure);
        return false;
    }

    auto * controller =
        [[MTRDeviceControllerFactory sharedInstance] runningControllerForFabricIndex:commandHandler->GetAccessingFabricIndex()];
    if (controller == nil) {
        commandHandler->AddStatus(commandPath, Status::Failure);
        return false;
    }

    if (!controller.otaProviderDelegate) {
        // This controller does not support OTA.
        commandHandler->AddStatus(commandPath, Status::UnsupportedCommand);
        return false;
    }

    *outController = controller;
    *outNodeId = desc.subject;
    return true;
}

// Ensures we have a usable CommandHandler and do not have an error.
//
// When this function returns non-null, it's safe to go ahead and use the return
// value to send a response.
//
// When this function returns null, the CommandHandler::Handle should not be
// used anymore.
CommandHandler * _Nullable EnsureValidState(
    CommandHandler::Handle & handle, const ConcreteCommandPath & cachedCommandPath, const char * prefix, NSError * _Nullable error)
{
    CommandHandler * handler = handle.Get();
    if (handler == nullptr) {
        ChipLogError(Controller, "%s: no CommandHandler to send response", prefix);
        return nullptr;
    }

    if (error != nil) {
        auto * desc = [error description];
        auto err = [MTRError errorToCHIPErrorCode:error];
        ChipLogError(
            Controller, "%s: application returned error: '%s', sending error: '%s'", prefix, desc.UTF8String, err.AsString());

        handler->AddStatus(cachedCommandPath, StatusIB(err).mStatus);
        handle.Release();
        return nullptr;
    }

    return handler;
}

// Ensures we have a usable CommandHandler and that our args don't involve any
// errors, for the case when we have data to send back.
//
// When this function returns non-null, it's safe to go ahead and use whatever
// object "data" points to to add a response to the command.
//
// When this function returns null, the CommandHandler::Handle should not be
// used anymore.
CommandHandler * _Nullable EnsureValidState(CommandHandler::Handle & handle, const ConcreteCommandPath & cachedCommandPath,
    const char * prefix, NSObject * _Nullable data, NSError * _Nullable error)
{
    CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, prefix, error);
    VerifyOrReturnValue(handler != nullptr, nullptr);

    if (data == nil) {
        ChipLogError(Controller, "%s: no data to send as a response", prefix);
        handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Failure);
        handle.Release();
        return nullptr;
    }

    return handler;
}
} // anonymous namespace

void MTROTAProviderDelegateBridge::HandleQueryImage(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const Commands::QueryImage::DecodableType & commandData)
{
    assertChipStackLockedByCurrentThread();

    NodeId nodeId;
    MTRDeviceController * controller;
    if (!GetPeerNodeInfo(commandObj, commandPath, &nodeId, &controller)) {
        return;
    }

    auto fabricIndex = commandObj->GetAccessingFabricIndex();
    auto ourNodeId = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetLocalScopedNodeId();

    auto * commandParams = [[MTROTASoftwareUpdateProviderClusterQueryImageParams alloc] init];
    CHIP_ERROR err = ConvertToQueryImageParams(commandData, commandParams);
    if (err != CHIP_NO_ERROR) {
        commandObj->AddStatus(commandPath, StatusIB(err).mStatus);
        return;
    }

    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    auto completionHandler = ^(
        MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, "QueryImage", data, error);
                VerifyOrReturn(handler != nullptr);

                ChipLogDetail(Controller, "QueryImage: application responded with: %s", [[data description] UTF8String]);

                auto hasUpdate = [data.status isEqual:@(MTROTASoftwareUpdateProviderStatusUpdateAvailable)];
                auto isBDXProtocolSupported =
                    [commandParams.protocolsSupported containsObject:@(MTROTASoftwareUpdateProviderDownloadProtocolBDXSynchronous)];

                // The logic we are following here is if none of the protocols supported by the requestor are supported by us, we
                // can't transfer the image even if we had an image available and we would return a Protocol Not Supported status.
                // Assumption here is the requestor would send us a list of all the protocols it supports. If one/more of the
                // protocols supported by the requestor are supported by us, we check if an image is not available due to various
                // reasons - image not available, delegate reporting busy, we will respond with the status in the delegate response.
                // If update is available, we try to prepare for transfer and build the uri in the response with a status of Image
                // Available

                // If the protocol requested is not supported, return status - Protocol Not Supported
                if (!isBDXProtocolSupported) {
                    Commands::QueryImageResponse::Type response;
                    response.status = static_cast<StatusEnum>(MTROTASoftwareUpdateProviderStatusDownloadProtocolNotSupported);
                    handler->AddResponse(cachedCommandPath, response);
                    handle.Release();
                    return;
                }

                Commands::QueryImageResponse::Type delegateResponse;
                ConvertFromQueryImageResponseParams(data, delegateResponse);

                // If update is not available, return the delegate response
                if (!hasUpdate) {
                    handler->AddResponse(cachedCommandPath, delegateResponse);
                    handle.Release();
                    return;
                }

                // If there is an update available, try to prepare for a transfer.
                CHIP_ERROR err = gOtaSender->PrepareForTransfer(fabricIndex, nodeId);
                if (CHIP_NO_ERROR != err) {

                    // Handle busy error separately as we have a query image response status that maps to busy
                    if (err == CHIP_ERROR_BUSY) {
                        ChipLogError(
                            Controller, "Responding with Busy due to being in the middle of handling another BDX transfer");
                        Commands::QueryImageResponse::Type response;
                        response.status = static_cast<StatusEnum>(MTROTASoftwareUpdateProviderStatusBusy);
                        response.delayedActionTime.SetValue(delegateResponse.delayedActionTime.ValueOr(kDelayedActionTimeSeconds));
                        handler->AddResponse(cachedCommandPath, response);
                        handle.Release();
                        // We do not reset state when we get the busy error because that means we are locked in a BDX transfer
                        // session with another requestor when we get this query image request. We do not want to interrupt the
                        // ongoing transfer instead just respond to the second requestor with a busy status and a delayedActionTime
                        // in which the requestor can retry.
                        return;
                    }
                    LogErrorOnFailure(err);
                    handler->AddStatus(cachedCommandPath, StatusIB(err).mStatus);
                    handle.Release();
                    // We need to reset state here to clean up any initialization we might have done including starting the BDX
                    // timeout timer while preparing for transfer if any failure occurs afterwards.
                    gOtaSender->ResetState();
                    return;
                }

                char uriBuffer[kMaxBDXURILen];
                MutableCharSpan uri(uriBuffer);
                err = bdx::MakeURI(ourNodeId.GetNodeId(), AsCharSpan(data.imageURI), uri);
                if (CHIP_NO_ERROR != err) {
                    LogErrorOnFailure(err);
                    handler->AddStatus(cachedCommandPath, StatusIB(err).mStatus);
                    handle.Release();
                    gOtaSender->ResetState();
                    return;
                }
                delegateResponse.imageURI.SetValue(uri);
                handler->AddResponse(cachedCommandPath, delegateResponse);
                handle.Release();
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    auto strongDelegate = controller.otaProviderDelegate;
    dispatch_async(controller.otaProviderDelegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleQueryImageForNodeID:controller:params:completion:)]) {
            [strongDelegate handleQueryImageForNodeID:@(nodeId)
                                           controller:controller
                                               params:commandParams
                                           completion:completionHandler];
        } else {
            // Cast is safe because subclass does not add any selectors.
            [strongDelegate
                handleQueryImageForNodeID:@(nodeId)
                               controller:controller
                                   params:static_cast<MTROtaSoftwareUpdateProviderClusterQueryImageParams *>(commandParams)
                        completionHandler:^(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                            NSError * _Nullable error) {
                            completionHandler(data, error);
                        }];
        }
    });
}

void MTROTAProviderDelegateBridge::HandleApplyUpdateRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::ApplyUpdateRequest::DecodableType & commandData)
{
    assertChipStackLockedByCurrentThread();

    NodeId nodeId;
    MTRDeviceController * controller;
    if (!GetPeerNodeInfo(commandObj, commandPath, &nodeId, &controller)) {
        return;
    }

    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    auto completionHandler = ^(
        MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, "ApplyUpdateRequest", data, error);
                VerifyOrReturn(handler != nullptr);

                ChipLogDetail(Controller, "ApplyUpdateRequest: application responded with: %s", [[data description] UTF8String]);

                Commands::ApplyUpdateResponse::Type response;
                ConvertFromApplyUpdateRequestResponseParms(data, response);
                handler->AddResponse(cachedCommandPath, response);
                handle.Release();
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    auto * commandParams = [[MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
    ConvertToApplyUpdateRequestParams(commandData, commandParams);

    auto strongDelegate = controller.otaProviderDelegate;
    dispatch_async(controller.otaProviderDelegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleApplyUpdateRequestForNodeID:controller:params:completion:)]) {
            [strongDelegate handleApplyUpdateRequestForNodeID:@(nodeId)
                                                   controller:controller
                                                       params:commandParams
                                                   completion:completionHandler];
        } else {
            // Cast is safe because subclass does not add any selectors.
            [strongDelegate
                handleApplyUpdateRequestForNodeID:@(nodeId)
                                       controller:controller
                                           params:static_cast<MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams *>(
                                                      commandParams)
                                completionHandler:^(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                                    NSError * _Nullable error) {
                                    completionHandler(data, error);
                                }];
        }
    });
}

void MTROTAProviderDelegateBridge::HandleNotifyUpdateApplied(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::NotifyUpdateApplied::DecodableType & commandData)
{
    assertChipStackLockedByCurrentThread();

    NodeId nodeId;
    MTRDeviceController * controller;
    if (!GetPeerNodeInfo(commandObj, commandPath, &nodeId, &controller)) {
        return;
    }

    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    auto completionHandler = ^(NSError * _Nullable error) {
        [controller
            asyncDispatchToMatterQueue:^() {
                assertChipStackLockedByCurrentThread();

                CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, "NotifyUpdateApplied", error);
                VerifyOrReturn(handler != nullptr);

                handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Success);
                handle.Release();
            }
                          errorHandler:^(NSError *) {
                              // Not much we can do here
                          }];
    };

    auto * commandParams = [[MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
    ConvertToNotifyUpdateAppliedParams(commandData, commandParams);

    auto strongDelegate = controller.otaProviderDelegate;
    dispatch_async(controller.otaProviderDelegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleNotifyUpdateAppliedForNodeID:controller:params:completion:)]) {
            [strongDelegate handleNotifyUpdateAppliedForNodeID:@(nodeId)
                                                    controller:controller
                                                        params:commandParams
                                                    completion:completionHandler];
        } else {
            // Cast is safe because subclass does not add any selectors.
            [strongDelegate
                handleNotifyUpdateAppliedForNodeID:@(nodeId)
                                        controller:controller
                                            params:static_cast<MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams *>(
                                                       commandParams)
                                 completionHandler:completionHandler];
        }
    });
}

CHIP_ERROR MTROTAProviderDelegateBridge::ConvertToQueryImageParams(
    const Commands::QueryImage::DecodableType & commandData, MTROTASoftwareUpdateProviderClusterQueryImageParams * commandParams)
{
    commandParams.vendorID = [NSNumber numberWithUnsignedShort:commandData.vendorID];
    commandParams.productID = [NSNumber numberWithUnsignedShort:commandData.productID];
    commandParams.softwareVersion = [NSNumber numberWithUnsignedLong:commandData.softwareVersion];
    auto iterator = commandData.protocolsSupported.begin();
    NSMutableArray * protocolsSupported = [[NSMutableArray alloc] init];
    while (iterator.Next()) {
        DownloadProtocolEnum protocol = iterator.GetValue();
        [protocolsSupported addObject:[NSNumber numberWithInt:to_underlying(protocol)]];
    }
    ReturnErrorOnFailure(iterator.GetStatus());
    commandParams.protocolsSupported = protocolsSupported;

    if (commandData.hardwareVersion.HasValue()) {
        commandParams.hardwareVersion = [NSNumber numberWithUnsignedShort:commandData.hardwareVersion.Value()];
    }

    if (commandData.location.HasValue()) {
        commandParams.location = AsString(commandData.location.Value());
    }

    if (commandData.requestorCanConsent.HasValue()) {
        commandParams.requestorCanConsent = [NSNumber numberWithBool:commandData.requestorCanConsent.Value()];
    }

    if (commandData.metadataForProvider.HasValue()) {
        commandParams.metadataForProvider = AsData(commandData.metadataForProvider.Value());
    }
    return CHIP_NO_ERROR;
}

void MTROTAProviderDelegateBridge::ConvertFromQueryImageResponseParams(
    const MTROTASoftwareUpdateProviderClusterQueryImageResponseParams * responseParams,
    Commands::QueryImageResponse::Type & response)
{
    response.status = static_cast<StatusEnum>([responseParams.status intValue]);

    if (responseParams.delayedActionTime) {
        response.delayedActionTime.SetValue([responseParams.delayedActionTime unsignedIntValue]);
    }

    if (responseParams.imageURI) {
        response.imageURI.SetValue(AsCharSpan(responseParams.imageURI));
    }

    if (responseParams.softwareVersion) {
        response.softwareVersion.SetValue([responseParams.softwareVersion unsignedIntValue]);
    }

    if (responseParams.softwareVersionString) {
        response.softwareVersionString.SetValue(AsCharSpan(responseParams.softwareVersionString));
    }

    if (responseParams.updateToken) {
        response.updateToken.SetValue(AsByteSpan(responseParams.updateToken));
    }

    if (responseParams.userConsentNeeded) {
        response.userConsentNeeded.SetValue([responseParams.userConsentNeeded boolValue]);
    }

    if (responseParams.metadataForRequestor) {
        response.metadataForRequestor.SetValue(AsByteSpan(responseParams.metadataForRequestor));
    }
}

void MTROTAProviderDelegateBridge::ConvertToApplyUpdateRequestParams(
    const Commands::ApplyUpdateRequest::DecodableType & commandData,
    MTROTASoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams)
{
    commandParams.updateToken = AsData(commandData.updateToken);
    commandParams.newVersion = [NSNumber numberWithUnsignedLong:commandData.newVersion];
}

void MTROTAProviderDelegateBridge::ConvertFromApplyUpdateRequestResponseParms(
    const MTROTASoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
    Commands::ApplyUpdateResponse::Type & response)
{
    response.action = static_cast<ApplyUpdateActionEnum>([responseParams.action intValue]);
    response.delayedActionTime = [responseParams.delayedActionTime unsignedIntValue];
}

void MTROTAProviderDelegateBridge::ConvertToNotifyUpdateAppliedParams(
    const Commands::NotifyUpdateApplied::DecodableType & commandData,
    MTROTASoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams)
{
    commandParams.updateToken = AsData(commandData.updateToken);
    commandParams.softwareVersion = [NSNumber numberWithUnsignedLong:commandData.softwareVersion];
}
