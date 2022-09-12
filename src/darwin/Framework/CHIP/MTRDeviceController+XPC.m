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

#import "MTRDeviceController+XPC.h"

#import "MTRBaseDevice.h"
#import "MTRCluster.h"
#import "MTRDeviceControllerOverXPC.h"

NS_ASSUME_NONNULL_BEGIN

static NSString * const kFabricFilteredKey = @"fabricFiltered";
static NSString * const kKeepPreviousSubscriptionsKey = @"keepPreviousSubscriptions";
static NSString * const kAutoResubscribeKey = @"autoResubscribe";

static NSArray * _Nullable encodeAttributePath(MTRAttributePath * _Nullable path)
{
    if (!path) {
        return nil;
    }
    return @[ path.endpoint, path.cluster, path.attribute ];
}

static NSArray * _Nullable encodeCommandPath(MTRCommandPath * _Nullable path)
{
    if (!path) {
        return nil;
    }
    return @[ path.endpoint, path.cluster, path.command ];
}

static MTRAttributePath * _Nullable decodeAttributePath(NSArray * _Nullable pathArray)
{
    if (pathArray == nil || [pathArray count] != 3) {
        return nil;
    }
    return [MTRAttributePath attributePathWithEndpointID:pathArray[0] clusterID:pathArray[1] attributeID:pathArray[2]];
}

static MTRCommandPath * _Nullable decodeCommandPath(NSArray * _Nullable pathArray)
{
    if (pathArray == nil || [pathArray count] != 3) {
        return nil;
    }
    return [MTRCommandPath commandPathWithEndpointID:pathArray[0] clusterID:pathArray[1] commandID:pathArray[2]];
}

static void decodeReadParams(NSDictionary<NSString *, id> * inParams, MTRReadParams * outParams)
{
    outParams.fabricFiltered = inParams[kFabricFilteredKey];
}

@implementation MTRDeviceController (XPC)

+ (MTRDeviceController *)sharedControllerWithID:(id<NSCopying> _Nullable)controllerID
                                xpcConnectBlock:(MTRXPCConnectBlock)xpcConnectBlock
{
    return [MTRDeviceControllerOverXPC sharedControllerWithID:controllerID xpcConnectBlock:xpcConnectBlock];
}

+ (NSArray<NSDictionary<NSString *, id> *> * _Nullable)encodeXPCResponseValues:
    (NSArray<NSDictionary<NSString *, id> *> * _Nullable)values
{
    if (!values) {
        return values;
    }
    NSMutableArray * result = [NSMutableArray array];
    for (NSDictionary<NSString *, id> * value in values) {
        if (!value || (value[MTRAttributePathKey] == nil && value[MTRCommandPathKey] == nil)) {
            [result addObject:value];
            continue;
        }
        NSMutableDictionary<NSString *, id> * resultValue = [NSMutableDictionary dictionaryWithCapacity:[value count]];
        [resultValue addEntriesFromDictionary:value];
        if (value[MTRAttributePathKey]) {
            resultValue[MTRAttributePathKey] = encodeAttributePath(value[MTRAttributePathKey]);
        }
        if (value[MTRCommandPathKey]) {
            resultValue[MTRCommandPathKey] = encodeCommandPath(value[MTRCommandPathKey]);
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
        if (!value || (value[MTRAttributePathKey] == nil && value[MTRCommandPathKey] == nil)) {
            [result addObject:value];
        }
        NSMutableDictionary<NSString *, id> * resultValue = [NSMutableDictionary dictionaryWithCapacity:[value count]];
        [resultValue addEntriesFromDictionary:value];
        if (value[MTRAttributePathKey]) {
            resultValue[MTRAttributePathKey] = decodeAttributePath(value[MTRAttributePathKey]);
        }
        if (value[MTRCommandPathKey]) {
            resultValue[MTRCommandPathKey] = decodeCommandPath(value[MTRCommandPathKey]);
        }
        [result addObject:resultValue];
    }
    return result;
}

+ (NSDictionary<NSString *, id> * _Nullable)encodeXPCReadParams:(MTRReadParams *)params
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

+ (MTRReadParams * _Nullable)decodeXPCReadParams:(NSDictionary<NSString *, id> * _Nullable)params
{
    if (!params) {
        return nil;
    }
    MTRReadParams * result = [[MTRReadParams alloc] init];
    decodeReadParams(params, result);
    return result;
}

+ (NSDictionary<NSString *, id> * _Nullable)encodeXPCSubscribeParams:(MTRSubscribeParams *)params
{
    if (!params) {
        return nil;
    }
    NSMutableDictionary<NSString *, id> * result =
        [NSMutableDictionary dictionaryWithDictionary:[MTRDeviceController encodeXPCReadParams:params]];
    if (params.keepPreviousSubscriptions) {
        result[kKeepPreviousSubscriptionsKey] = params.keepPreviousSubscriptions;
    }
    if (params.autoResubscribe) {
        result[kAutoResubscribeKey] = params.autoResubscribe;
    }
    return result;
}

+ (MTRSubscribeParams * _Nullable)decodeXPCSubscribeParams:(NSDictionary<NSString *, id> * _Nullable)params
{
    if (!params) {
        return nil;
    }
    MTRSubscribeParams * result = [[MTRSubscribeParams alloc] init];
    decodeReadParams(params, result);
    result.keepPreviousSubscriptions = params[kKeepPreviousSubscriptionsKey];
    result.autoResubscribe = params[kAutoResubscribeKey];
    return result;
}

@end

@implementation MTRDeviceController (Deprecated_XPC)

+ (MTRDeviceController *)sharedControllerWithId:(id<NSCopying> _Nullable)controllerID
                                xpcConnectBlock:(MTRXPCConnectBlock)xpcConnectBlock
{
    return [MTRDeviceController sharedControllerWithID:controllerID xpcConnectBlock:xpcConnectBlock];
}

@end

NS_ASSUME_NONNULL_END
