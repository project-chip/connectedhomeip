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

#import "MCCommandObjects.h"

#import "MCCastingApp.h"
#import "MCCommand_Internal.h"
#import "MCErrorUtils.h"

#include "core/Command.h"
#include <app-common/zap-generated/cluster-objects.h>

#import <Foundation/Foundation.h>

@implementation MCContentLauncherClusterLaunchURLRequest

- (instancetype)init
{
    if (self = [super init]) {
        _contentURL = @"";
        _displayString = nil;
        _brandingInformation = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MCContentLauncherClusterLaunchURLRequest alloc] init];
    other.contentURL = self.contentURL;
    other.displayString = self.displayString;
    other.brandingInformation = self.brandingInformation;
    return other;
}

- (NSString *)description
{
    NSString * descriptionString = [NSString stringWithFormat:@"<%@: contentURL:%@; displayString:%@; brandingInformation:%@; >", NSStringFromClass([self class]), _contentURL, _displayString, _brandingInformation];
    return descriptionString;
}
@end

@implementation MCContentLauncherClusterLauncherResponse

- (instancetype)init
{
    if (self = [super init]) {
        _status = @(0);
        _data = nil;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone;
{
    auto other = [[MCContentLauncherClusterLauncherResponse alloc] init];

    other.status = self.status;
    other.data = self.data;
    return other;
}

- (instancetype)initWithDecodableStruct:(const chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::DecodableType &)decodableStruct
{
    if (self = [super init]) {
        _status = [NSNumber numberWithUnsignedChar:chip::to_underlying(decodableStruct.status)];
        _data = [[NSString alloc] initWithBytes:decodableStruct.data.Value().data() length:decodableStruct.data.Value().size() encoding:NSUTF8StringEncoding];
    }
    return self;
}

@end

@implementation MCContentLauncherClusterLaunchURLCommand

- (void)invoke:(id)request
                 context:(void * _Nullable)context
              completion:(void (^_Nonnull __strong)(void *, NSError *, id))completion
    timedInvokeTimeoutMs:(NSNumber * _Nullable)timedInvokeTimeoutMs
{
    MCCommandTemplate<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type> * mcCommand = new MCCommandTemplate<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type>(
        self.cppCommand,
        [self](id objCRequest) {
            return [self getCppRequestFromObjC:objCRequest];
        },
        [self](std::any cppResponse) {
            return [self getObjCResponseFromCpp:cppResponse];
        });
    mcCommand->invoke(
        request, context, [mcCommand, completion](void * context, NSError * err, id response) {
            completion(context, err, response);
            delete mcCommand;
        }, timedInvokeTimeoutMs);
}

- (std::any)getCppRequestFromObjC:(MCContentLauncherClusterLaunchURLRequest *)objcRequest
{
    VerifyOrReturnValue(objcRequest != nil, nullptr);

    std::shared_ptr<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type> cppRequest = std::make_shared<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type>();
    cppRequest->contentURL = chip::CharSpan([objcRequest.contentURL UTF8String], [objcRequest.contentURL lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
    if (objcRequest.displayString != nil) {
        cppRequest->displayString = chip::Optional<chip::CharSpan>(chip::CharSpan([objcRequest.displayString UTF8String], [objcRequest.displayString lengthOfBytesUsingEncoding:NSUTF8StringEncoding]));
    }
    cppRequest->brandingInformation = chip::MakeOptional(chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type()); // TODO: map brandingInformation
    return std::any(cppRequest);
}

- (id)getObjCResponseFromCpp:(std::any)cppResponse
{
    MCContentLauncherClusterLauncherResponse * objCResponse = nil;
    if (cppResponse.type() == typeid(std::shared_ptr<const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type::ResponseType>)) {
        std::shared_ptr<const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type::ResponseType> responseSharedPtr = std::any_cast<std::shared_ptr<const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type::ResponseType>>(cppResponse);
        objCResponse = responseSharedPtr != nil ? [[MCContentLauncherClusterLauncherResponse alloc] initWithDecodableStruct:*responseSharedPtr] : nil;
    }
    return objCResponse;
}
@end
