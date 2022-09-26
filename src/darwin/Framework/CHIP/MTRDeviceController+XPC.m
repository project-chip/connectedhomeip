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
static NSString * const kMinIntervalKey = @"minInterval";
static NSString * const kMaxIntervalKey = @"maxInterval";

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
    outParams.fabricFiltered = [inParams[kFabricFilteredKey] boolValue];
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
    result[kFabricFilteredKey] = @(params.fabricFiltered);
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

+ (NSDictionary<NSString *, id> *)encodeXPCSubscribeParams:(MTRSubscribeParams *)params
{
    NSMutableDictionary<NSString *, id> * result =
        [NSMutableDictionary dictionaryWithDictionary:[MTRDeviceController encodeXPCReadParams:params]];
    result[kKeepPreviousSubscriptionsKey] = @(params.keepPreviousSubscriptions);
    result[kAutoResubscribeKey] = @(params.autoResubscribe);
    result[kMinIntervalKey] = params.minInterval;
    result[kMaxIntervalKey] = params.maxInterval;
    return result;
}

+ (MTRSubscribeParams *)decodeXPCSubscribeParams:(NSDictionary<NSString *, id> *)params
{
    MTRSubscribeParams * result = [[MTRSubscribeParams alloc] initWithMinInterval:params[kMinIntervalKey]
                                                                      maxInterval:params[kMaxIntervalKey]];
    decodeReadParams(params, result);
    result.keepPreviousSubscriptions = [params[kKeepPreviousSubscriptionsKey] boolValue];
    result.autoResubscribe = [params[kAutoResubscribeKey] boolValue];
    return result;
}

@end

NS_ASSUME_NONNULL_END
