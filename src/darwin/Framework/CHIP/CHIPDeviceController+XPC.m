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

#import "CHIPDevice.h"
#import "CHIPDeviceControllerOverXPC.h"

NS_ASSUME_NONNULL_BEGIN

static NSArray * _Nullable serializeAttributePath(CHIPAttributePath * _Nullable path)
{
    if (!path) {
        return nil;
    }
    return @[ path.endpoint, path.cluster, path.attribute ];
}

static NSArray * _Nullable serializeCommandPath(CHIPCommandPath * _Nullable path)
{
    if (!path) {
        return nil;
    }
    return @[ path.endpoint, path.cluster, path.command ];
}

static CHIPAttributePath * _Nullable deserializeAttributePath(NSArray * _Nullable pathArray)
{
    if (pathArray == nil || [pathArray count] != 3) {
        return nil;
    }
    return [CHIPAttributePath attributePathWithEndpointId:pathArray[0] clusterId:pathArray[1] attributeId:pathArray[2]];
}

static CHIPCommandPath * _Nullable deserializeCommandPath(NSArray * _Nullable pathArray)
{
    if (pathArray == nil || [pathArray count] != 3) {
        return nil;
    }
    return [CHIPCommandPath commandPathWithEndpointId:pathArray[0] clusterId:pathArray[1] commandId:pathArray[2]];
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
            resultValue[kCHIPAttributePathKey] = serializeAttributePath(value[kCHIPAttributePathKey]);
        }
        if (value[kCHIPCommandPathKey]) {
            resultValue[kCHIPCommandPathKey] = serializeCommandPath(value[kCHIPCommandPathKey]);
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
            resultValue[kCHIPAttributePathKey] = deserializeAttributePath(value[kCHIPAttributePathKey]);
        }
        if (value[kCHIPCommandPathKey]) {
            resultValue[kCHIPCommandPathKey] = deserializeCommandPath(value[kCHIPCommandPathKey]);
        }
        [result addObject:resultValue];
    }
    return result;
}

@end

NS_ASSUME_NONNULL_END
