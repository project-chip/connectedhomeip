/**
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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
#import "MTRError_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app/clusters/ota-provider/ota-provider.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/Global.h>
#include <lib/support/TypeTraits.h>
#include <platform/PlatformManager.h>
#include <protocols/interaction_model/Constants.h>

#include <messaging/ExchangeMgr.h>
#include <platform/LockTracker.h>
#include <protocols/bdx/AsyncTransferFacilitator.h>
#include <protocols/bdx/BdxUri.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace chip::bdx;
using Protocols::InteractionModel::Status;

namespace {

constexpr uint32_t kMaxBDXURILen = 256;

// Time in seconds after which the requestor should retry calling query image if
// busy status is receieved.  The spec minimum is 2 minutes, but in practice OTA
// generally takes a lot longer than that and devices only retry a few times
// before giving up.  Default to 10 minutes for now, until we have a better
// system of computing an expected completion time for the currently-running
// OTA.
constexpr uint32_t kDelayedActionTimeSeconds = 600;

NSInteger const kOtaProviderEndpoint = 0;
} // anonymous namespace

MTROTAProviderDelegateBridge::MTROTAProviderDelegateBridge()
{
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this);
}

MTROTAProviderDelegateBridge::~MTROTAProviderDelegateBridge()
{
    Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, nullptr);
}

CHIP_ERROR MTROTAProviderDelegateBridge::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeManager)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err = mOtaUnsolicitedBDXMsgHandler.Init(systemLayer, exchangeManager);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(Controller, "Failed to initialize the unsolicited BDX Message handler with err %s", err.AsString());
    }
    return err;
}

void MTROTAProviderDelegateBridge::Shutdown()
{
    assertChipStackLockedByCurrentThread();

    mOtaUnsolicitedBDXMsgHandler.Shutdown();
}

void MTROTAProviderDelegateBridge::ControllerShuttingDown(MTRDeviceController_Concrete * controller)
{
    assertChipStackLockedByCurrentThread();

    mOtaUnsolicitedBDXMsgHandler.ControllerShuttingDown(controller);
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

                // If the MTROTAUnsolicitedBDXMessageHandler already has an ongoing transfer, send busy error.
                if (MTROTAUnsolicitedBDXMessageHandler::GetInstance()->IsInAnOngoingTransfer()) {
                    ChipLogError(
                        Controller, "Responding with Busy due to being in the middle of handling another BDX transfer");
                    Commands::QueryImageResponse::Type response;
                    response.status = static_cast<StatusEnum>(MTROTASoftwareUpdateProviderStatusBusy);
                    response.delayedActionTime.SetValue(delegateResponse.delayedActionTime.ValueOr(kDelayedActionTimeSeconds));
                    handler->AddResponse(cachedCommandPath, response);
                    handle.Release();
                    return;
                }

                char uriBuffer[kMaxBDXURILen];
                MutableCharSpan uri(uriBuffer);
                CHIP_ERROR err = bdx::MakeURI(ourNodeId.GetNodeId(), AsCharSpan(data.imageURI), uri);
                if (CHIP_NO_ERROR != err) {
                    LogErrorOnFailure(err);
                    handler->AddStatus(cachedCommandPath, StatusIB(err).mStatus);
                    handle.Release();
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
