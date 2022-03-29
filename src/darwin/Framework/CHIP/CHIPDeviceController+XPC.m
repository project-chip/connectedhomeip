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

#import "CHIPDeviceController+XPC.h"

#import "CHIPCluster.h"
#import "CHIPDevice.h"
#import "CHIPDeviceControllerOverXPC.h"

NS_ASSUME_NONNULL_BEGIN

static NSString * const kFabricFilteredKey = @"fabricFiltered";
static NSString * const kKeepPreviousSubscriptionsKey = @"keepPreviousSubscriptions";
static NSString * const kAutoResubscribeKey = @"autoResubscribe";

static NSArray * _Nullable encodeAttributePath(CHIPAttributePath * _Nullable path)
{
    if (!path) {
        return nil;
    }
    return @[ path.endpoint, path.cluster, path.attribute ];
}

static NSArray * _Nullable encodeCommandPath(CHIPCommandPath * _Nullable path)
{
    if (!path) {
        return nil;
    }
    return @[ path.endpoint, path.cluster, path.command ];
}

static CHIPAttributePath * _Nullable decodeAttributePath(NSArray * _Nullable pathArray)
{
    if (pathArray == nil || [pathArray count] != 3) {
        return nil;
    }
    return [CHIPAttributePath attributePathWithEndpointId:pathArray[0] clusterId:pathArray[1] attributeId:pathArray[2]];
}

static CHIPCommandPath * _Nullable decodeCommandPath(NSArray * _Nullable pathArray)
{
    if (pathArray == nil || [pathArray count] != 3) {
        return nil;
    }
    return [CHIPCommandPath commandPathWithEndpointId:pathArray[0] clusterId:pathArray[1] commandId:pathArray[2]];
}

static void decodeReadParams(NSDictionary<NSString *, id> * inParams, CHIPReadParams * outParams)
{
    outParams.fabricFiltered = inParams[kFabricFilteredKey];
}

@implementation CHIPDeviceController (XPC)

+ (CHIPDeviceController *)sharedControllerWithId:(id<NSCopying> _Nullable)controllerId
                                 xpcConnectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    return [CHIPDeviceControllerOverXPC sharedControllerWithId:controllerId xpcConnectBlock:connectBlock];
}

+ (NSArray<NSDictionary<NSString *, id> *> * _Nullable)encodeXPCResponseValues:
    (NSArray<NSDictionary<NSString *, id> *> * _Nullable)values
{
    if (!values) {
        return values;
    }
    NSMutableArray * result = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * value in values) {
        if (!value || (value[kCHIPAttributePathKey] == nil && value[kCHIPCommandPathKey] == nil)) {
            [result addObject:value];
            continue;
        }
        NSMutableDictionary<NSString *, id> * resultValue = [NSMutableDictionary dictionaryWithCapacity:[value count]];
        [resultValue addEntriesFromDictionary:value];
        if (value[kCHIPAttributePathKey]) {
            resultValue[kCHIPAttributePathKey] = encodeAttributePath(value[kCHIPAttributePathKey]);
        }
        if (value[kCHIPCommandPathKey]) {
            resultValue[kCHIPCommandPathKey] = encodeCommandPath(value[kCHIPCommandPathKey]);
        }
        [result addObject:resultValue];
    }
    return result;
}

+ (NSArray<NSDictionary<NSString *, id> *> * _Nullable)decodeXPCResponseValues:
    (NSArray<NSDictionary<NSString *, id> *> * _Nullable)values
{
    if (!values) {
        return values;
    }
    NSMutableArray * result = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * value in values) {
        if (!value || (value[kCHIPAttributePathKey] == nil && value[kCHIPCommandPathKey] == nil)) {
            [result addObject:value];
        }
        NSMutableDictionary<NSString *, id> * resultValue = [NSMutableDictionary dictionaryWithCapacity:[value count]];
        [resultValue addEntriesFromDictionary:value];
        if (value[kCHIPAttributePathKey]) {
            resultValue[kCHIPAttributePathKey] = decodeAttributePath(value[kCHIPAttributePathKey]);
        }
        if (value[kCHIPCommandPathKey]) {
            resultValue[kCHIPCommandPathKey] = decodeCommandPath(value[kCHIPCommandPathKey]);
        }
        [result addObject:resultValue];
    }
    return result;
}

+ (NSDictionary<NSString *, id> * _Nullable)encodeXPCReadParams:(CHIPReadParams *)params
{
    if (!params) {
        return nil;
    }
    NSMutableDictionary<NSString *, id> * result = [NSMutableDictionary dictionary];
    if (params.fabricFiltered) {
        result[kFabricFilteredKey] = params.fabricFiltered;
    }
    return result;
}

+ (CHIPReadParams * _Nullable)decodeXPCReadParams:(NSDictionary<NSString *, id> * _Nullable)params
{
    if (!params) {
        return nil;
    }
    CHIPReadParams * result = [[CHIPReadParams alloc] init];
    decodeReadParams(params, result);
    return result;
}

+ (NSDictionary<NSString *, id> * _Nullable)encodeXPCSubscribeParams:(CHIPSubscribeParams *)params
{
    if (!params) {
        return nil;
    }
    NSMutableDictionary<NSString *, id> * result =
        [NSMutableDictionary dictionaryWithDictionary:[CHIPDeviceController encodeXPCReadParams:params]];
    if (params.keepPreviousSubscriptions) {
        result[kKeepPreviousSubscriptionsKey] = params.keepPreviousSubscriptions;
    }
    if (params.autoResubscribe) {
        result[kAutoResubscribeKey] = params.autoResubscribe;
    }
    return result;
}

+ (CHIPSubscribeParams * _Nullable)decodeXPCSubscribeParams:(NSDictionary<NSString *, id> * _Nullable)params
{
    if (!params) {
        return nil;
    }
    CHIPSubscribeParams * result = [[CHIPSubscribeParams alloc] init];
    decodeReadParams(params, result);
    result.keepPreviousSubscriptions = params[kKeepPreviousSubscriptionsKey];
    result.autoResubscribe = params[kAutoResubscribeKey];
    return result;
}

@end

NS_ASSUME_NONNULL_END
