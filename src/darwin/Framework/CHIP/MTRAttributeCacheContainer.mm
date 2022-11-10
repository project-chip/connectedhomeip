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

#import <Foundation/Foundation.h>

#import "MTRAttributeCacheContainer_Internal.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCluster.h"
#import "MTRDeviceControllerXPCConnection.h"
#import "MTRError.h"
#import "MTRError_Internal.h"
#import "MTRLogging.h"

#include <app/InteractionModelEngine.h>
#include <lib/support/ErrorStr.h>
#include <platform/PlatformManager.h>

using namespace chip;

@implementation MTRAttributeCacheContainer

- (instancetype)init
{
    if ([super init]) {
        _cppAttributeCache = nullptr;
        _shouldUseXPC = NO;
    }
    return self;
}

- (void)setXPCConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
            controllerID:(id<NSCopying>)controllerID
                deviceID:(NSNumber *)deviceID
{
    self.xpcConnection = xpcConnection;
    self.xpcControllerID = controllerID;
    self.deviceID = deviceID;
    self.shouldUseXPC = YES;
}

static CHIP_ERROR AppendAttibuteValueToArray(
    const chip::app::ConcreteAttributePath & path, chip::app::ClusterStateCache * cache, NSMutableArray * array)
{
    chip::TLV::TLVReader reader;
    CHIP_ERROR err = cache->Get(path, reader);
    if (err == CHIP_NO_ERROR) {
        id obj = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
        if (obj) {
            [array addObject:@ { MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:path], MTRDataKey : obj }];
            return CHIP_NO_ERROR;
        }
        MTR_LOG_ERROR("Error: Cached value could not be converted to generic NSObject");
        [array addObject:@ {
            MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:path],
            MTRErrorKey : [MTRError errorForCHIPErrorCode:CHIP_ERROR_DECODE_FAILED]
        }];
        return CHIP_ERROR_DECODE_FAILED;
    }
    MTR_LOG_ERROR("Error: Failed to read from attribute cache: %s", err.AsString());
    [array addObject:@ {
        MTRAttributePathKey : [[MTRAttributePath alloc] initWithPath:path],
        MTRErrorKey : [MTRError errorForCHIPErrorCode:err]
    }];
    return err;
}

- (void)readAttributeWithEndpointID:(NSNumber * _Nullable)endpointID
                          clusterID:(NSNumber * _Nullable)clusterID
                        attributeID:(NSNumber * _Nullable)attributeID
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    __auto_type completionHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
        dispatch_async(queue, ^{
            completion(values, error);
        });
    };

    if (self.shouldUseXPC) {
        MTRDeviceControllerXPCConnection * xpcConnection = self.xpcConnection;
        if (!xpcConnection) {
            MTR_LOG_ERROR("Attribute cache read failed: MTRDeviceController was already disposed");
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            return;
        }
        __auto_type controllerId = self.xpcControllerID;
        NSNumber * nodeId = self.deviceID;
        [xpcConnection
            getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
                if (handle) {
                    [handle.proxy readAttributeCacheWithController:controllerId
                                                            nodeId:nodeId.unsignedLongLongValue
                                                        endpointId:endpointID
                                                         clusterId:clusterID
                                                       attributeId:attributeID
                                                        completion:^(id _Nullable values, NSError * _Nullable error) {
                                                            completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                            __auto_type handleRetainer = handle;
                                                            (void) handleRetainer;
                                                        }];
                } else {
                    MTR_LOG_ERROR("Attribute cache read failed due to XPC connection failure");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                }
            }];
        return;
    }

    dispatch_async(DeviceLayer::PlatformMgrImpl().GetWorkQueue(), ^{
        if (endpointID == nil && clusterID == nil) {
            MTR_LOG_ERROR("Error: currently read from attribute cache does not support wildcards for both endpoint and cluster");
            completionHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidArgument userInfo:nil]);
            return;
        }

        if (!self.cppAttributeCache) {
            MTR_LOG_ERROR("Error: No attribute cache available to read from");
            completionHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            return;
        }

        NSMutableArray * result = [[NSMutableArray alloc] init];
        CHIP_ERROR err = CHIP_NO_ERROR;
        if (endpointID == nil) {
            err = self.cppAttributeCache->ForEachAttribute(
                static_cast<chip::ClusterId>([clusterID unsignedLongValue]), [&](const app::ConcreteAttributePath & path) {
                    if (attributeID == nil
                        || static_cast<chip::AttributeId>([attributeID unsignedLongValue]) == path.mAttributeId) {
                        (void) AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
                    }
                    return CHIP_NO_ERROR;
                });
        } else if (clusterID == nil) {
            err = self.cppAttributeCache->ForEachCluster(
                static_cast<chip::EndpointId>([endpointID unsignedShortValue]), [&](chip::ClusterId enumeratedClusterId) {
                    (void) self.cppAttributeCache->ForEachAttribute(static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
                        enumeratedClusterId, [&](const app::ConcreteAttributePath & path) {
                            if (attributeID == nil
                                || static_cast<chip::AttributeId>([attributeID unsignedLongValue]) == path.mAttributeId) {
                                (void) AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
                            }
                            return CHIP_NO_ERROR;
                        });
                    return CHIP_NO_ERROR;
                });
        } else if (attributeID == nil) {
            err = self.cppAttributeCache->ForEachAttribute(static_cast<chip::EndpointId>([endpointID unsignedShortValue]),
                static_cast<chip::ClusterId>([clusterID unsignedLongValue]), [&](const app::ConcreteAttributePath & path) {
                    (void) AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
                    return CHIP_NO_ERROR;
                });
        } else {
            app::ConcreteAttributePath path;
            path.mEndpointId = static_cast<chip::EndpointId>([endpointID unsignedShortValue]);
            path.mClusterId = static_cast<chip::ClusterId>([clusterID unsignedLongValue]);
            path.mAttributeId = static_cast<chip::AttributeId>([attributeID unsignedLongValue]);
            err = AppendAttibuteValueToArray(path, self.cppAttributeCache, result);
        }
        if (err == CHIP_NO_ERROR) {
            completionHandler(result, nil);
        } else {
            completionHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:err.AsInteger() userInfo:nil]);
        }
    });
}

@end

@implementation MTRAttributeCacheContainer (Deprecated)

- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    [self readAttributeWithEndpointID:endpointId
                            clusterID:clusterId
                          attributeID:attributeId
                                queue:clientQueue
                           completion:completion];
}

@end
