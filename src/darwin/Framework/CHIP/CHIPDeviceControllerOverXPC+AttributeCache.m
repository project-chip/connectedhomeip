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

#import "CHIPDeviceControllerOverXPC+AttributeCache.h"
#import "CHIPDeviceControllerOverXPC_Internal.h"
#import "CHIPError.h"
#import "CHIPLogging.h"

NS_ASSUME_NONNULL_BEGIN

@implementation CHIPDeviceControllerOverXPC (AttributeCache)

- (void)subscribeAttributeCacheWithNodeId:(uint64_t)nodeId
                                   params:(CHIPSubscribeParams * _Nullable)params
                               completion:(void (^)(NSError * _Nullable error))completion
{
    dispatch_async(self.workQueue, ^{
        dispatch_group_t group = dispatch_group_create();
        if (!self.controllerId) {
            dispatch_group_enter(group);
            [self.xpcConnection getProxyHandleWithCompletion:^(
                dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
                if (handle) {
                    [handle.proxy getAnyDeviceControllerWithCompletion:^(id _Nullable controller, NSError * _Nullable error) {
                        if (error) {
                            CHIP_LOG_ERROR("Failed to fetch any shared remote controller");
                        } else {
                            self.controllerId = controller;
                        }
                        dispatch_group_leave(group);
                        __auto_type handleRetainer = handle;
                        (void) handleRetainer;
                    }];
                } else {
                    CHIP_LOG_ERROR("XPC disconnected while retrieving any shared remote controller");
                    dispatch_group_leave(group);
                }
            }];
        }
        dispatch_group_notify(group, self.workQueue, ^{
            if (self.controllerId) {
                [self.xpcConnection getProxyHandleWithCompletion:^(
                    dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
                    if (handle) {
                        [handle.proxy subscribeAttributeCacheWithController:self.controllerId
                                                                     nodeId:nodeId
                                                                     params:[CHIPDeviceController encodeXPCSubscribeParams:params]
                                                                 completion:^(NSError * _Nullable error) {
                                                                     if (error) {
                                                                         CHIP_LOG_ERROR("Attribute cache subscription for "
                                                                                        "controller %@ and node %llu failed: %@",
                                                                             self.controllerId, nodeId, error);
                                                                         completion(error);
                                                                         return;
                                                                     }
                                                                     completion(nil);
                                                                     __auto_type handleRetainer = handle;
                                                                     (void) handleRetainer;
                                                                 }];
                    } else {
                        CHIP_LOG_ERROR(
                            "Attribute cache subscription for controller %@ and node %llu failed due to XPC connection failure",
                            self.controllerId, nodeId);
                        completion([NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
                    }
                }];
            } else {
                CHIP_LOG_ERROR("Attribute cache subscription for node %llu failed due to lack of controller ID", nodeId);
                completion([NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
            }
        });
    });
}

- (void)readAttributeCacheWithNodeId:(uint64_t)nodeId
                          endpointId:(NSNumber * _Nullable)endpointId
                           clusterId:(NSNumber * _Nullable)clusterId
                         attributeId:(NSNumber * _Nullable)attributeId
                          completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    dispatch_async(self.workQueue, ^{
        if (!self.controllerId) {
            CHIP_LOG_ERROR("Attribute cache wasn't subscribed yet.");
            completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
            return;
        }
        [self.xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy readAttributeCacheWithController:self.controllerId
                                                        nodeId:nodeId
                                                    endpointId:endpointId
                                                     clusterId:clusterId
                                                   attributeId:attributeId
                                                    completion:^(id _Nullable values, NSError * _Nullable error) {
                                                        completion([CHIPDeviceController decodeXPCResponseValues:values], error);
                                                        __auto_type handleRetainer = handle;
                                                        (void) handleRetainer;
                                                    }];
            } else {
                CHIP_LOG_ERROR("Attribute cache read failed due to XPC connection failure");
                completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
            }
        }];
    });
}

@end

NS_ASSUME_NONNULL_END
