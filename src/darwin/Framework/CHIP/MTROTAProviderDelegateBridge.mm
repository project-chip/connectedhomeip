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
#import "MTRControllerFactory_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app/clusters/ota-provider/ota-provider.h>
#include <lib/support/TypeTraits.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/Constants.h>

#include <MTRError_Internal.h>
#include <messaging/ExchangeMgr.h>
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
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);
constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kSender;

class BdxOTASender : public bdx::Responder {
public:
    BdxOTASender() {};

    CHIP_ERROR PrepareForTransfer(FabricIndex fabricIndex, NodeId nodeId)
    {
        VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

        ReturnErrorOnFailure(ConfigureState(fabricIndex, nodeId));

        BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);
        return Responder::PrepareForTransfer(mSystemLayer, kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollIntervalMs);
    }

    CHIP_ERROR Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr)
    {
        VerifyOrReturnError(mSystemLayer == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mExchangeMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

        exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);

        mSystemLayer = systemLayer;
        mExchangeMgr = exchangeMgr;
        mWorkQueue = DeviceLayer::PlatformMgrImpl().GetWorkQueue();

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Shutdown()
    {
        VerifyOrReturnError(mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);

        mExchangeMgr = nullptr;
        mSystemLayer = nullptr;
        mWorkQueue = nil;

        ResetState();

        return CHIP_NO_ERROR;
    }

    void SetDelegate(id<MTROTAProviderDelegate> delegate)
    {
        if (delegate) {
            mDelegate = delegate;
        } else {
            ResetState();
        }
    }

private:
    CHIP_ERROR OnMessageToSend(TransferSession::OutputEvent & event)
    {
        VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);

        Messaging::SendFlags sendFlags;

        // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
        // the end of the transfer.
        if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
            sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
        }

        auto & msgTypeData = event.msgTypeData;
        return mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    }

    CHIP_ERROR OnTransferSessionBegin(TransferSession::OutputEvent & event)
    {
        VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        uint16_t fdl = 0;
        auto fd = mTransfer.GetFileDesignator(fdl);
        VerifyOrReturnError(fdl <= bdx::kMaxFileDesignatorLen, CHIP_ERROR_INVALID_ARGUMENT);

        auto fileDesignator = [[NSString alloc] initWithBytes:fd length:fdl encoding:NSUTF8StringEncoding];
        auto offset = @(mTransfer.GetStartOffset());
        auto completionHandler = ^(NSError * _Nullable error) {
            dispatch_async(mWorkQueue, ^{
                if (error != nil) {
                    LogErrorOnFailure([MTRError errorToCHIPErrorCode:error]);
                    LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
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
            });
        };

        auto * controller = [[MTRControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
        VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);
        auto nodeId = @(mNodeId.Value());

        auto strongDelegate = mDelegate;
        dispatch_async(mWorkQueue, ^{
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

    CHIP_ERROR OnTransferSessionEnd(TransferSession::OutputEvent & event)
    {
        VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);

        CHIP_ERROR error = CHIP_NO_ERROR;
        if (event.EventType == TransferSession::OutputEventType::kTransferTimeout) {
            error = CHIP_ERROR_TIMEOUT;
        } else if (event.EventType != TransferSession::OutputEventType::kAckEOFReceived) {
            error = CHIP_ERROR_INTERNAL;
        }

        auto * controller = [[MTRControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
        VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);
        auto nodeId = @(mNodeId.Value());

        auto strongDelegate = mDelegate;
        dispatch_async(mWorkQueue, ^{
            [strongDelegate handleBDXTransferSessionEndForNodeID:nodeId
                                                      controller:controller
                                                           error:[MTRError errorForCHIPErrorCode:error]];
        });

        ResetState();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnBlockQuery(TransferSession::OutputEvent & event)
    {
        VerifyOrReturnError(mFabricIndex.HasValue(), CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);

        auto blockSize = @(mTransfer.GetTransferBlockSize());
        auto blockIndex = @(mTransfer.GetNextBlockNum());

        auto bytesToSkip = @(0);
        if (event.EventType == TransferSession::OutputEventType::kQueryWithSkipReceived) {
            bytesToSkip = @(event.bytesToSkip.BytesToSkip);
        }

        auto completionHandler = ^(NSData * _Nullable data, BOOL isEOF) {
            dispatch_async(mWorkQueue, ^{
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
            });
        };

        // TODO Handle MaxLength

        auto * controller = [[MTRControllerFactory sharedInstance] runningControllerForFabricIndex:mFabricIndex.Value()];
        VerifyOrReturnError(controller != nil, CHIP_ERROR_INCORRECT_STATE);
        auto nodeId = @(mNodeId.Value());

        auto strongDelegate = mDelegate;
        dispatch_async(mWorkQueue, ^{
            if ([strongDelegate respondsToSelector:@selector
                                (handleBDXQueryForNodeID:controller:blockSize:blockIndex:bytesToSkip:completion:)]) {
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
        if (mInitialized) {
            // Prevent a new node connection since another is active.
            VerifyOrReturnError(mFabricIndex.Value() == fabricIndex && mNodeId.Value() == nodeId, CHIP_ERROR_BUSY);

            // Reset stale connection from the same Node if exists.
            ResetState();
        }

        mFabricIndex.SetValue(fabricIndex);
        mNodeId.SetValue(nodeId);

        mInitialized = true;

        return CHIP_NO_ERROR;
    }

    void ResetState()
    {
        if (!mInitialized) {
            return;
        }

        Responder::ResetTransfer();
        mFabricIndex.ClearValue();
        mNodeId.ClearValue();

        if (mExchangeCtx != nullptr) {
            mExchangeCtx->Close();
            mExchangeCtx = nullptr;
        }

        mInitialized = false;
    }

    bool mInitialized = false;
    Optional<FabricIndex> mFabricIndex;
    Optional<NodeId> mNodeId;
    id<MTROTAProviderDelegate> mDelegate = nil;
    dispatch_queue_t mWorkQueue = nil;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
};

BdxOTASender gOtaSender;

static NSInteger const kOtaProviderEndpoint = 0;

MTROTAProviderDelegateBridge::MTROTAProviderDelegateBridge(id<MTROTAProviderDelegate> delegate)
    : mDelegate(delegate)
    , mWorkQueue(DeviceLayer::PlatformMgrImpl().GetWorkQueue())
{
    gOtaSender.SetDelegate(delegate);
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this);
}

MTROTAProviderDelegateBridge::~MTROTAProviderDelegateBridge()
{
    gOtaSender.SetDelegate(nil);
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, nullptr);
}

CHIP_ERROR MTROTAProviderDelegateBridge::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeManager)
{
    return gOtaSender.Init(systemLayer, exchangeManager);
}

void MTROTAProviderDelegateBridge::Shutdown() { gOtaSender.Shutdown(); }

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
        [[MTRControllerFactory sharedInstance] runningControllerForFabricIndex:commandHandler->GetAccessingFabricIndex()];
    if (controller == nil) {
        commandHandler->AddStatus(commandPath, Status::Failure);
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
        ChipLogError(Controller, "%s: application returned error: '%s', sending error: '%s'", prefix,
            [desc cStringUsingEncoding:NSUTF8StringEncoding], chip::ErrorStr(err));

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
    NodeId nodeId;
    MTRDeviceController * controller;
    if (!GetPeerNodeInfo(commandObj, commandPath, &nodeId, &controller)) {
        return;
    }

    auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterQueryImageParams alloc] init];
    CHIP_ERROR err = ConvertToQueryImageParams(commandData, commandParams);
    if (err != CHIP_NO_ERROR) {
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    auto completionHandler = ^(
        MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data, NSError * _Nullable error) {
        dispatch_async(mWorkQueue, ^{
            CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, "QueryImage", data, error);
            VerifyOrReturn(handler != nullptr);

            ChipLogDetail(Controller, "QueryImage: application responded with: %s",
                [[data description] cStringUsingEncoding:NSUTF8StringEncoding]);

            Commands::QueryImageResponse::Type response;
            ConvertFromQueryImageResponseParms(data, response);

            auto hasUpdate = [data.status isEqual:@(MTROtaSoftwareUpdateProviderOTAQueryStatusUpdateAvailable)];
            auto isBDXProtocolSupported =
                [commandParams.protocolsSupported containsObject:@(MTROtaSoftwareUpdateProviderOTADownloadProtocolBDXSynchronous)];

            if (hasUpdate && isBDXProtocolSupported) {
                auto fabricIndex = handler->GetSubjectDescriptor().fabricIndex;
                auto nodeId = handler->GetSubjectDescriptor().subject;
                CHIP_ERROR err = gOtaSender.PrepareForTransfer(fabricIndex, nodeId);
                if (CHIP_NO_ERROR != err) {
                    LogErrorOnFailure(err);
                    handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Failure);
                    handle.Release();
                    return;
                }

                auto targetNodeId = handler->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetLocalScopedNodeId();

                char uriBuffer[kMaxBDXURILen];
                MutableCharSpan uri(uriBuffer);
                err = bdx::MakeURI(targetNodeId.GetNodeId(), AsCharSpan(data.imageURI), uri);
                if (CHIP_NO_ERROR != err) {
                    LogErrorOnFailure(err);
                    handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Failure);
                    handle.Release();
                    return;
                }

                response.imageURI.SetValue(uri);
                handler->AddResponse(cachedCommandPath, response);
                handle.Release();
                return;
            }

            handler->AddResponse(cachedCommandPath, response);
            handle.Release();
        });
    };

    auto strongDelegate = mDelegate;
    dispatch_async(mWorkQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleQueryImageForNodeID:controller:params:completion:)]) {
            [strongDelegate handleQueryImageForNodeID:@(nodeId)
                                           controller:controller
                                               params:commandParams
                                           completion:completionHandler];
        } else {
            [strongDelegate handleQueryImageForNodeID:@(nodeId)
                                           controller:controller
                                               params:commandParams
                                    completionHandler:completionHandler];
        }
    });
}

void MTROTAProviderDelegateBridge::HandleApplyUpdateRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::ApplyUpdateRequest::DecodableType & commandData)
{
    NodeId nodeId;
    MTRDeviceController * controller;
    if (!GetPeerNodeInfo(commandObj, commandPath, &nodeId, &controller)) {
        return;
    }

    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    auto completionHandler
        = ^(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data, NSError * _Nullable error) {
              dispatch_async(mWorkQueue, ^{
                  CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, "ApplyUpdateRequest", data, error);
                  VerifyOrReturn(handler != nullptr);

                  ChipLogDetail(Controller, "ApplyUpdateRequest: application responded with: %s",
                      [[data description] cStringUsingEncoding:NSUTF8StringEncoding]);

                  Commands::ApplyUpdateResponse::Type response;
                  ConvertFromApplyUpdateRequestResponseParms(data, response);
                  handler->AddResponse(cachedCommandPath, response);
                  handle.Release();
              });
          };

    auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
    ConvertToApplyUpdateRequestParams(commandData, commandParams);

    auto strongDelegate = mDelegate;
    dispatch_async(mWorkQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleApplyUpdateRequestForNodeID:controller:params:completion:)]) {
            [strongDelegate handleApplyUpdateRequestForNodeID:@(nodeId)
                                                   controller:controller
                                                       params:commandParams
                                                   completion:completionHandler];
        } else {
            [strongDelegate handleApplyUpdateRequestForNodeID:@(nodeId)
                                                   controller:controller
                                                       params:commandParams
                                            completionHandler:completionHandler];
        }
    });
}

void MTROTAProviderDelegateBridge::HandleNotifyUpdateApplied(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::NotifyUpdateApplied::DecodableType & commandData)
{
    NodeId nodeId;
    MTRDeviceController * controller;
    if (!GetPeerNodeInfo(commandObj, commandPath, &nodeId, &controller)) {
        return;
    }

    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    auto completionHandler = ^(NSError * _Nullable error) {
        dispatch_async(mWorkQueue, ^{
            CommandHandler * handler = EnsureValidState(handle, cachedCommandPath, "NotifyUpdateApplied", error);
            VerifyOrReturn(handler != nullptr);

            handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Success);
            handle.Release();
        });
    };

    auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
    ConvertToNotifyUpdateAppliedParams(commandData, commandParams);

    auto strongDelegate = mDelegate;
    dispatch_async(mWorkQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(handleNotifyUpdateAppliedForNodeID:controller:params:completion:)]) {
            [strongDelegate handleNotifyUpdateAppliedForNodeID:@(nodeId)
                                                    controller:controller
                                                        params:commandParams
                                                    completion:completionHandler];
        } else {
            [strongDelegate handleNotifyUpdateAppliedForNodeID:@(nodeId)
                                                    controller:controller
                                                        params:commandParams
                                             completionHandler:completionHandler];
        }
    });
}

CHIP_ERROR MTROTAProviderDelegateBridge::ConvertToQueryImageParams(
    const Commands::QueryImage::DecodableType & commandData, MTROtaSoftwareUpdateProviderClusterQueryImageParams * commandParams)
{
    commandParams.vendorId = [NSNumber numberWithUnsignedShort:commandData.vendorId];
    commandParams.productId = [NSNumber numberWithUnsignedShort:commandData.productId];
    commandParams.softwareVersion = [NSNumber numberWithUnsignedLong:commandData.softwareVersion];
    auto iterator = commandData.protocolsSupported.begin();
    NSMutableArray * protocolsSupported = [[NSMutableArray alloc] init];
    while (iterator.Next()) {
        OTADownloadProtocol protocol = iterator.GetValue();
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

void MTROTAProviderDelegateBridge::ConvertFromQueryImageResponseParms(
    const MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * responseParams,
    Commands::QueryImageResponse::Type & response)
{
    response.status = static_cast<OTAQueryStatus>([responseParams.status intValue]);

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
    MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams)
{
    commandParams.updateToken = AsData(commandData.updateToken);
    commandParams.newVersion = [NSNumber numberWithUnsignedLong:commandData.newVersion];
}

void MTROTAProviderDelegateBridge::ConvertFromApplyUpdateRequestResponseParms(
    const MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
    Commands::ApplyUpdateResponse::Type & response)
{
    response.action = static_cast<OTAApplyUpdateAction>([responseParams.action intValue]);
    response.delayedActionTime = [responseParams.delayedActionTime unsignedIntValue];
}

void MTROTAProviderDelegateBridge::ConvertToNotifyUpdateAppliedParams(
    const Commands::NotifyUpdateApplied::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams)
{
    commandParams.updateToken = AsData(commandData.updateToken);
    commandParams.softwareVersion = [NSNumber numberWithUnsignedLong:commandData.softwareVersion];
}
