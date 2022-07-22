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
#import "NSDataSpanConversion.h"

#include <app/clusters/ota-provider/ota-provider.h>
#include <lib/support/TypeTraits.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/Constants.h>

// BDX
#include <MTRError_Internal.h>
#include <protocols/bdx/TransferFacilitator.h>

#include <app/InteractionModelEngine.h> // For InteractionModelEngine::GetInstance()->GetExchangeManager();
#include <platform/CHIPDeviceLayer.h> // For &DeviceLayer::SystemLayer()
// BDX

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

// TODO Expose a method onto the delegate to make that configurable.
constexpr uint32_t kMaxBdxBlockSize = 1024;
constexpr System::Clock::Timeout kBdxTimeout = System::Clock::Seconds16(5 * 60); // OTA Spec mandates >= 5 minutes
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);
constexpr bdx::TransferRole kBdxRole = bdx::TransferRole::kSender;

class BdxOTASender : public bdx::Responder {
public:
    BdxOTASender() {}

    CHIP_ERROR Start(FabricIndex fabricIndex, NodeId nodeId)
    {
        if (mInitialized) {
            VerifyOrReturnError(mFabricIndex.HasValue() && mNodeId.HasValue(), CHIP_ERROR_INCORRECT_STATE);

            // Prevent a new node connection since another is active
            VerifyOrReturnError(mFabricIndex.Value() == fabricIndex && mNodeId.Value() == nodeId, CHIP_ERROR_BUSY);

            // Reset stale connection from the Same Node if exists
            Reset();
        }
        mInitialized = true;

        mFabricIndex.SetValue(fabricIndex);
        mNodeId.SetValue(nodeId);

        BitFlags<bdx::TransferControlFlags> flags(bdx::TransferControlFlags::kReceiverDrive);
        // TODO Have a better mechanism to remove the need from getting an instance of the system layer here.
        return PrepareForTransfer(&DeviceLayer::SystemLayer(), kBdxRole, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollIntervalMs);
    }

    void SetDelegate(id<MTROTAProviderDelegate> delegate, dispatch_queue_t queue)
    {
        // TODO Have a better mechanism to retrieve the exchange manager instance
        // In order to register ourself as a protocol handler for BDX, it needs to be a reference
        // to the exchange manager instance. That's not ideal but the reference is retrieved
        // from the interaction model engine instance.
        auto exchangeMgr = InteractionModelEngine::GetInstance()->GetExchangeManager();
        if (delegate && queue) {
            mDelegate = delegate;
            mDelegateQueue = queue;
            mWorkQueue = DeviceLayer::PlatformMgrImpl().GetWorkQueue();
            exchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
        } else {
            Reset();
            exchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
        }
    }

private:
    CHIP_ERROR OnMessageToSend(bdx::TransferSession::OutputEvent & event)
    {
        VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDelegate != nil, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mDelegateQueue != nil, CHIP_ERROR_INCORRECT_STATE);

        Messaging::SendFlags sendFlags;

        // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
        // the end of the transfer.
        if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport)) {
            sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
        }

        auto & msgTypeData = event.msgTypeData;
        return mExchangeCtx->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);
    }

    CHIP_ERROR OnTransferSessionBegin(bdx::TransferSession::OutputEvent & event)
    {
        uint16_t fdl = 0;
        auto fd = mTransfer.GetFileDesignator(fdl);
        VerifyOrReturnError(fdl <= bdx::kMaxFileDesignatorLen, CHIP_ERROR_INVALID_ARGUMENT);

        auto fileDesignator = [[NSString alloc] initWithBytes:fd length:fdl encoding:NSUTF8StringEncoding];
        auto offset = @(mTransfer.GetStartOffset());
        auto completionHandler = ^(NSError * error) {
            dispatch_async(mWorkQueue, ^{
                if (error != nil) {
                    LogErrorOnFailure([MTRError errorToCHIPErrorCode:error]);
                    LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
                    return;
                }

                // bdx::TransferSession will automatically reject a transfer if there are no
                // common supported control modes. It will also default to the smaller
                // block size.
                bdx::TransferSession::TransferAcceptData acceptData;
                acceptData.ControlMode = bdx::TransferControlFlags::kReceiverDrive;
                acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
                acceptData.StartOffset = mTransfer.GetStartOffset();
                acceptData.Length = mTransfer.GetTransferLength();

                LogErrorOnFailure(mTransfer.AcceptTransfer(acceptData));
            });
        };

        dispatch_async(mDelegateQueue, ^{
            [mDelegate handleBDXTransferSessionBegin:fileDesignator offset:offset completionHandler:completionHandler];
        });

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnTransferSessionEnd(bdx::TransferSession::OutputEvent & event)
    {
        CHIP_ERROR error = CHIP_ERROR_INTERNAL;
        if (event.EventType == bdx::TransferSession::OutputEventType::kAckEOFReceived) {
            error = CHIP_NO_ERROR;
        } else if (event.EventType == bdx::TransferSession::OutputEventType::kTransferTimeout) {
            error = CHIP_ERROR_TIMEOUT;
        }

        auto delegate = mDelegate; // mDelegate will be set to nil by Reset, so get a strong ref to it.
        dispatch_async(mDelegateQueue, ^{
            [delegate handleBDXTransferSessionEnd:[MTRError errorForCHIPErrorCode:error]];
        });

        Reset();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnBlockQuery(bdx::TransferSession::OutputEvent & event)
    {
        auto blockSize = @(mTransfer.GetTransferBlockSize());
        auto blockIndex = @(mTransfer.GetNextBlockNum());

        auto bytesToSkip = @(0);
        if (event.EventType == bdx::TransferSession::OutputEventType::kQueryWithSkipReceived) {
            bytesToSkip = @(event.bytesToSkip.BytesToSkip);
        }

        auto completionHandler = ^(NSData * _Nullable data, BOOL isEOF) {
            dispatch_async(mWorkQueue, ^{
                if (data == nil) {
                    LogErrorOnFailure(mTransfer.AbortTransfer(bdx::StatusCode::kUnknown));
                    return;
                }

                bdx::TransferSession::BlockData blockData;
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

        dispatch_async(mDelegateQueue, ^{
            [mDelegate handleBDXQuery:blockSize blockIndex:blockIndex bytesToSkip:bytesToSkip completionHandler:completionHandler];
        });

        return CHIP_NO_ERROR;
    }

    void HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event) override
    {
        VerifyOrReturn(mDelegate != nil);
        VerifyOrReturn(mDelegateQueue != nil);

        CHIP_ERROR err = CHIP_NO_ERROR;
        switch (event.EventType) {
        case bdx::TransferSession::OutputEventType::kInitReceived:
            err = OnTransferSessionBegin(event);
            break;
        case bdx::TransferSession::OutputEventType::kStatusReceived:
            ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
            [[fallthrough]];
        case bdx::TransferSession::OutputEventType::kAckEOFReceived:
        case bdx::TransferSession::OutputEventType::kInternalError:
        case bdx::TransferSession::OutputEventType::kTransferTimeout:
            err = OnTransferSessionEnd(event);
            break;
        case bdx::TransferSession::OutputEventType::kQueryWithSkipReceived:
        case bdx::TransferSession::OutputEventType::kQueryReceived:
            err = OnBlockQuery(event);
            break;
        case bdx::TransferSession::OutputEventType::kMsgToSend:
            err = OnMessageToSend(event);
            break;
        case bdx::TransferSession::OutputEventType::kNone:
        case bdx::TransferSession::OutputEventType::kAckReceived:
            // Nothing to do.
            break;
        case bdx::TransferSession::OutputEventType::kAcceptReceived:
        case bdx::TransferSession::OutputEventType::kBlockReceived:
        default:
            // Should never happens.
            chipDie();
            break;
        }
        LogErrorOnFailure(err);
    }

    void Reset()
    {
        mFabricIndex.ClearValue();
        mNodeId.ClearValue();
        mTransfer.Reset();
        if (mExchangeCtx != nullptr) {
            mExchangeCtx->Close();
            mExchangeCtx = nullptr;
        }

        mDelegate = nil;
        mDelegateQueue = nil;
        mWorkQueue = nil;

        mInitialized = false;
    }

    bool mInitialized = false;
    Optional<FabricIndex> mFabricIndex;
    Optional<NodeId> mNodeId;
    id<MTROTAProviderDelegate> mDelegate = nil;
    dispatch_queue_t mDelegateQueue = nil;
    dispatch_queue_t mWorkQueue = nil;
};

BdxOTASender gOtaSender;

static NSInteger const kOtaProviderEndpoint = 0;

MTROTAProviderDelegateBridge::MTROTAProviderDelegateBridge(void)
    : mDelegate(nil)
{
}

MTROTAProviderDelegateBridge::~MTROTAProviderDelegateBridge(void) {}

void MTROTAProviderDelegateBridge::setDelegate(id<MTROTAProviderDelegate> delegate, dispatch_queue_t queue)
{
    mDelegate = delegate ?: nil;
    mQueue = queue ?: nil;

    gOtaSender.SetDelegate(delegate, queue);
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this);
}

void MTROTAProviderDelegateBridge::HandleQueryImage(
    CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const Commands::QueryImage::DecodableType & commandData)
{
    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterQueryImageParams alloc] init];
        CHIP_ERROR err = ConvertToQueryImageParams(commandData, commandParams);
        if (err != CHIP_NO_ERROR) {
            commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        // Make sure to hold on to the command handler and command path to be used in the completion block
        __block CommandHandler::Handle handle(commandObj);
        __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

        dispatch_async(mQueue, ^{
            [strongDelegate handleQueryImage:commandParams
                           completionHandler:^(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                               NSError * _Nullable error) {
                               dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
                                   Commands::QueryImageResponse::Type response;
                                   ConvertFromQueryImageResponseParms(data, response);

                                   CommandHandler * handler = handle.Get();
                                   if (handler) {
                                       auto hasUpdate =
                                           [data.status isEqual:@(MTROtaSoftwareUpdateProviderOTAQueryStatusUpdateAvailable)];
                                       auto isBDXProtocolSupported = [commandParams.protocolsSupported
                                           containsObject:@(MTROtaSoftwareUpdateProviderOTADownloadProtocolBDXSynchronous)];

                                       if (hasUpdate && isBDXProtocolSupported) {
                                           auto fabricIndex = handler->GetSubjectDescriptor().fabricIndex;
                                           auto nodeId = handler->GetSubjectDescriptor().subject;
                                           CHIP_ERROR err = gOtaSender.Start(fabricIndex, nodeId);
                                           if (CHIP_NO_ERROR != err) {
                                               LogErrorOnFailure(err);
                                               handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Failure);
                                               handle.Release();
                                               return;
                                           }
                                       }

                                       handler->AddResponse(cachedCommandPath, response);
                                       handle.Release();
                                   }
                               });
                           }];
        });
    }
}

void MTROTAProviderDelegateBridge::HandleApplyUpdateRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::ApplyUpdateRequest::DecodableType & commandData)
{
    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
        ConvertToApplyUpdateRequestParams(commandData, commandParams);

        dispatch_async(mQueue, ^{
            [strongDelegate
                handleApplyUpdateRequest:commandParams
                       completionHandler:^(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                           NSError * _Nullable error) {
                           dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
                               Commands::ApplyUpdateResponse::Type response;
                               ConvertFromApplyUpdateRequestResponseParms(data, response);

                               CommandHandler * handler = handle.Get();
                               if (handler) {
                                   handler->AddResponse(cachedCommandPath, response);
                                   handle.Release();
                               }
                           });
                       }];
        });
    }
}

void MTROTAProviderDelegateBridge::HandleNotifyUpdateApplied(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
    const Commands::NotifyUpdateApplied::DecodableType & commandData)
{
    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block CommandHandler::Handle handle(commandObj);
    __block ConcreteCommandPath cachedCommandPath(commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
        ConvertToNotifyUpdateAppliedParams(commandData, commandParams);

        dispatch_async(mQueue, ^{
            [strongDelegate handleNotifyUpdateApplied:commandParams
                                    completionHandler:^(NSError * _Nullable error) {
                                        dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
                                            CommandHandler * handler = handle.Get();
                                            if (handler) {
                                                handler->AddStatus(cachedCommandPath, Protocols::InteractionModel::Status::Success);
                                                handle.Release();
                                            }
                                        });
                                    }];
        });
    }
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
        commandParams.location = [[NSString alloc] initWithBytes:commandData.location.Value().data()
                                                          length:commandData.location.Value().size()
                                                        encoding:NSUTF8StringEncoding];
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
        response.imageURI.SetValue(CharSpan([responseParams.imageURI UTF8String], responseParams.imageURI.length));
    }

    if (responseParams.softwareVersion) {
        response.softwareVersion.SetValue([responseParams.softwareVersion unsignedIntValue]);
    }

    if (responseParams.softwareVersionString) {
        response.softwareVersionString.SetValue(
            CharSpan([responseParams.softwareVersionString UTF8String], responseParams.softwareVersionString.length));
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
