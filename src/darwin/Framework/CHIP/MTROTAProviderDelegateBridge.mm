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

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this);
}

void MTROTAProviderDelegateBridge::HandleQueryImage(chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData)
{
    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterQueryImageParams alloc] init];
        CHIP_ERROR err = ConvertToQueryImageParams(commandData, commandParams);
        if (err != CHIP_NO_ERROR) {
            commandObj->AddStatus(commandPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        // Make sure to hold on to the command handler and command path to be used in the completion block
        __block chip::app::CommandHandler::Handle handle(commandObj);
        __block chip::app::ConcreteCommandPath cachedCommandPath(
            commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

        dispatch_async(mQueue, ^{
            [strongDelegate handleQueryImage:commandParams
                           completionHandler:^(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                               NSError * _Nullable error) {
                               dispatch_async(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
                                   chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type response;
                                   ConvertFromQueryImageResponseParms(data, response);

                                   chip::app::CommandHandler * handler = handle.Get();
                                   if (handler) {
                                       handler->AddResponse(cachedCommandPath, response);
                                       handle.Release();
                                   }
                               });
                           }];
        });
    }
}

void MTROTAProviderDelegateBridge::HandleApplyUpdateRequest(chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData)
{
    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block chip::app::CommandHandler::Handle handle(commandObj);
    __block chip::app::ConcreteCommandPath cachedCommandPath(
        commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
        ConvertToApplyUpdateRequestParams(commandData, commandParams);

        dispatch_async(mQueue, ^{
            [strongDelegate
                handleApplyUpdateRequest:commandParams
                       completionHandler:^(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                           NSError * _Nullable error) {
                           dispatch_async(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
                               chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type response;
                               ConvertFromApplyUpdateRequestResponseParms(data, response);

                               chip::app::CommandHandler * handler = handle.Get();
                               if (handler) {
                                   handler->AddResponse(cachedCommandPath, response);
                                   handle.Release();
                               }
                           });
                       }];
        });
    }
}

void MTROTAProviderDelegateBridge::HandleNotifyUpdateApplied(chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData)
{
    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block chip::app::CommandHandler::Handle handle(commandObj);
    __block chip::app::ConcreteCommandPath cachedCommandPath(
        commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
        ConvertToNotifyUpdateAppliedParams(commandData, commandParams);

        dispatch_async(mQueue, ^{
            [strongDelegate
                handleNotifyUpdateApplied:commandParams
                        completionHandler:^(NSError * _Nullable error) {
                            dispatch_async(chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
                                chip::app::CommandHandler * handler = handle.Get();
                                if (handler) {
                                    handler->AddStatus(cachedCommandPath, chip::Protocols::InteractionModel::Status::Success);
                                    handle.Release();
                                }
                            });
                        }];
        });
    }
}

CHIP_ERROR MTROTAProviderDelegateBridge::ConvertToQueryImageParams(
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterQueryImageParams * commandParams)
{
    commandParams.vendorId = [NSNumber numberWithUnsignedShort:commandData.vendorId];
    commandParams.productId = [NSNumber numberWithUnsignedShort:commandData.productId];
    commandParams.softwareVersion = [NSNumber numberWithUnsignedLong:commandData.softwareVersion];
    auto iterator = commandData.protocolsSupported.begin();
    NSMutableArray * protocolsSupported = [[NSMutableArray alloc] init];
    while (iterator.Next()) {
        chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol protocol = iterator.GetValue();
        [protocolsSupported addObject:[NSNumber numberWithInt:chip::to_underlying(protocol)]];
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
    chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type & response)
{
    response.status = static_cast<chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus>([responseParams.status intValue]);

    if (responseParams.delayedActionTime) {
        response.delayedActionTime.SetValue([responseParams.delayedActionTime unsignedIntValue]);
    }

    if (responseParams.imageURI) {
        response.imageURI.SetValue(chip::CharSpan([responseParams.imageURI UTF8String], responseParams.imageURI.length));
    }

    if (responseParams.softwareVersion) {
        response.softwareVersion.SetValue([responseParams.softwareVersion unsignedIntValue]);
    }

    if (responseParams.softwareVersionString) {
        response.softwareVersionString.SetValue(
            chip::CharSpan([responseParams.softwareVersionString UTF8String], responseParams.softwareVersionString.length));
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
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams)
{
    commandParams.updateToken = AsData(commandData.updateToken);
    commandParams.newVersion = [NSNumber numberWithUnsignedLong:commandData.newVersion];
}

void MTROTAProviderDelegateBridge::ConvertFromApplyUpdateRequestResponseParms(
    const MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
    chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type & response)
{
    response.action
        = static_cast<chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction>([responseParams.action intValue]);
    response.delayedActionTime = [responseParams.delayedActionTime unsignedIntValue];
}

void MTROTAProviderDelegateBridge::ConvertToNotifyUpdateAppliedParams(
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams)
{
    commandParams.updateToken = AsData(commandData.updateToken);
    commandParams.softwareVersion = [NSNumber numberWithUnsignedLong:commandData.softwareVersion];
}
