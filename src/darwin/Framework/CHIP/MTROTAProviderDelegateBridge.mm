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

#include <app/clusters/ota-provider/ota-provider.h>

static NSInteger const kOtaProviderEndpoint = 0;

MTROTAProviderDelegateBridge::MTROTAProviderDelegateBridge(void)
    : mDelegate(nil)
{
}

MTROTAProviderDelegateBridge::~MTROTAProviderDelegateBridge(void) {}

void MTROTAProviderDelegateBridge::setDelegate(id<MTROTAProviderDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mDelegate = nil;
        mQueue = nil;
    }

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, this);
}

void MTROTAProviderDelegateBridge::HandleQueryImage(chip::app::CommandHandler * commandObj,
    const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData)
{
    // Make sure to hold on to the command handler and command path to be used in the completion block
    __block chip::app::CommandHandler::Handle handle(commandObj);
    __block chip::app::ConcreteCommandPath cachedCommandPath(
        commandPath.mEndpointId, commandPath.mClusterId, commandPath.mCommandId);

    id<MTROTAProviderDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(handleQueryImage:completionHandler:)]) {
        if (strongDelegate && mQueue) {
            auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterQueryImageParams alloc] init];
            ConvertToQueryImageParams(commandData, commandParams);

            dispatch_async(mQueue, ^{
                [strongDelegate handleQueryImage:commandParams
                               completionHandler:^(MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * _Nullable data,
                                   NSError * _Nullable error) {
                                   chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type response;
                                   ConvertFromQueryImageResponseParms(data, response);

                                   chip::app::CommandHandler * handler = handle.Get();
                                   if (handler) {
                                       handler->AddResponse(cachedCommandPath, response);
                                       handle.Release();
                                   }
                               }];
            });
        }
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
    if ([strongDelegate respondsToSelector:@selector(handleApplyUpdateRequest:completionHandler:)]) {
        if (strongDelegate && mQueue) {
            auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams alloc] init];
            ConvertToApplyUpdateRequestParams(commandData, commandParams);

            dispatch_async(mQueue, ^{
                [strongDelegate
                    handleApplyUpdateRequest:commandParams
                           completionHandler:^(MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * _Nullable data,
                               NSError * _Nullable error) {
                               chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type response;
                               ConvertFromApplyUpdateRequestResponseParms(data, response);

                               chip::app::CommandHandler * handler = handle.Get();
                               if (handler) {
                                   handler->AddResponse(cachedCommandPath, response);
                                   handle.Release();
                               }
                           }];
            });
        }
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
    if ([strongDelegate respondsToSelector:@selector(handleNotifyUpdateApplied:completionHandler:)]) {
        if (strongDelegate && mQueue) {
            auto * commandParams = [[MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams alloc] init];
            ConvertToNotifyUpdateAppliedParams(commandData, commandParams);

            dispatch_async(mQueue, ^{
                [strongDelegate
                    handleNotifyUpdateApplied:commandParams
                            completionHandler:^(NSError * _Nullable error) {
                                chip::app::CommandHandler * handler = handle.Get();
                                if (handler) {
                                    handler->AddStatus(cachedCommandPath, chip::Protocols::InteractionModel::Status::Success);
                                    handle.Release();
                                }
                            }];
            });
        }
    }
}

void MTROTAProviderDelegateBridge::ConvertToQueryImageParams(
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterQueryImageParams * commandParams)
{
    commandParams.vendorId = [NSNumber numberWithInt:commandData.vendorId];
    commandParams.productId = [NSNumber numberWithInt:commandData.productId];
    commandParams.softwareVersion = [NSNumber numberWithInt:commandData.softwareVersion];
    auto iterator = commandData.protocolsSupported.begin();
    NSMutableArray * protocolsSupported = [[NSMutableArray alloc] init];
    while (iterator.Next()) {
        chip::app::Clusters::OtaSoftwareUpdateProvider::OTADownloadProtocol protocol = iterator.GetValue();
        [protocolsSupported addObject:[NSNumber numberWithInt:static_cast<int>(protocol)]];
    }
    commandParams.protocolsSupported = [protocolsSupported copy];

    if (commandData.hardwareVersion.HasValue()) {
        commandParams.hardwareVersion = [NSNumber numberWithInt:commandData.hardwareVersion.Value()];
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
        commandParams.metadataForProvider = [NSData dataWithBytes:commandData.metadataForProvider.Value().data()
                                                           length:commandData.metadataForProvider.Value().size()];
    }
}

void MTROTAProviderDelegateBridge::ConvertFromQueryImageResponseParms(
    const MTROtaSoftwareUpdateProviderClusterQueryImageResponseParams * responseParams,
    chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Type & response)
{
    response.status = static_cast<chip::app::Clusters::OtaSoftwareUpdateProvider::OTAQueryStatus>([responseParams.status intValue]);

    if (responseParams.delayedActionTime) {
        response.delayedActionTime.SetValue([responseParams.delayedActionTime intValue]);
    }

    if (responseParams.imageURI) {
        response.imageURI.SetValue(chip::CharSpan([responseParams.imageURI UTF8String], responseParams.imageURI.length));
    }

    if (responseParams.softwareVersion) {
        response.softwareVersion.SetValue([responseParams.softwareVersion intValue]);
    }

    if (responseParams.softwareVersionString) {
        response.softwareVersionString.SetValue(
            chip::CharSpan([responseParams.softwareVersionString UTF8String], responseParams.softwareVersionString.length));
    }

    if (responseParams.updateToken) {
        UInt8 * updateTokenBytes = (UInt8 *) responseParams.updateToken.bytes;
        response.updateToken.SetValue(chip::ByteSpan(updateTokenBytes, responseParams.updateToken.length));
    }

    if (responseParams.userConsentNeeded) {
        response.userConsentNeeded.SetValue([responseParams.userConsentNeeded boolValue]);
    }

    if (responseParams.metadataForRequestor) {
        UInt8 * metadataForRequestorBytes = (UInt8 *) responseParams.metadataForRequestor.bytes;
        response.metadataForRequestor.SetValue(
            chip::ByteSpan(metadataForRequestorBytes, responseParams.metadataForRequestor.length));
    }
}

void MTROTAProviderDelegateBridge::ConvertToApplyUpdateRequestParams(
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterApplyUpdateRequestParams * commandParams)
{
    commandParams.updateToken = [NSData dataWithBytes:commandData.updateToken.data() length:commandData.updateToken.size()];
    commandParams.newVersion = [NSNumber numberWithInt:commandData.newVersion];
}

void MTROTAProviderDelegateBridge::ConvertFromApplyUpdateRequestResponseParms(
    const MTROtaSoftwareUpdateProviderClusterApplyUpdateResponseParams * responseParams,
    chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Type & response)
{
    response.action
        = static_cast<chip::app::Clusters::OtaSoftwareUpdateProvider::OTAApplyUpdateAction>([responseParams.action intValue]);
    response.delayedActionTime = [responseParams.delayedActionTime intValue];
}

void MTROTAProviderDelegateBridge::ConvertToNotifyUpdateAppliedParams(
    const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::DecodableType & commandData,
    MTROtaSoftwareUpdateProviderClusterNotifyUpdateAppliedParams * commandParams)
{
    commandParams.updateToken = [NSData dataWithBytes:commandData.updateToken.data() length:commandData.updateToken.size()];
    commandParams.softwareVersion = [NSNumber numberWithInt:commandData.softwareVersion];
}
