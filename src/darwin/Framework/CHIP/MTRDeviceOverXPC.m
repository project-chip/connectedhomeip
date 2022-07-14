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

#import "MTRDeviceOverXPC.h"

#import "MTRAttributeCacheContainer+XPC.h"
#import "MTRCluster.h"
#import "MTRDeviceController+XPC.h"
#import "MTRDeviceControllerXPCConnection.h"
#import "MTRError.h"
#import "MTRLogging.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceOverXPC ()

@property (nonatomic, strong, readonly) id<NSCopying> controller;
@property (nonatomic, readonly) uint64_t nodeId;
@property (nonatomic, strong, readonly) MTRDeviceControllerXPCConnection * xpcConnection;

@end

@implementation MTRDeviceOverXPC

- (instancetype)initWithController:(id<NSCopying>)controller
                          deviceId:(uint64_t)deviceId
                     xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
{
    _controller = controller;
    _nodeId = deviceId;
    _xpcConnection = xpcConnection;
    return self;
}

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
                     params:(nullable MTRSubscribeParams *)params
             cacheContainer:(MTRAttributeCacheContainer * _Nullable)attributeCacheContainer
     attributeReportHandler:(nullable void (^)(NSArray * value))attributeReportHandler
         eventReportHandler:(nullable void (^)(NSArray * value))eventReportHandler
               errorHandler:(void (^)(NSError * error))errorHandler
    subscriptionEstablished:(nullable void (^)(void))subscriptionEstablishedHandler;
{
    MTR_LOG_DEBUG("Subscribing all attributes... Note that reportHandler is not supported.");
    if (attributeCacheContainer) {
        [attributeCacheContainer setXPCConnection:_xpcConnection controllerId:self.controller deviceId:self.nodeId];
    }
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy subscribeWithController:self.controller
                                               nodeId:self.nodeId
                                          minInterval:@(minInterval)
                                          maxInterval:@(maxInterval)
                                               params:[MTRDeviceController encodeXPCSubscribeParams:params]
                                          shouldCache:(attributeCacheContainer != nil)
                                           completion:^(NSError * _Nullable error) {
                                               dispatch_async(queue, ^{
                                                   if (error) {
                                                       errorHandler(error);
                                                   } else {
                                                       subscriptionEstablishedHandler();
                                                   }
                                               });
                                               __auto_type handleRetainer = handle;
                                               (void) handleRetainer;
                                           }];
            } else {
                MTR_LOG_ERROR("Failed to obtain XPC connection to write attribute");
                dispatch_async(queue, ^{
                    errorHandler([NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(MTRReadParams * _Nullable)params
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Reading attribute ...");
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy readAttributeWithController:self.controller
                                                   nodeId:self.nodeId
                                               endpointId:endpointId
                                                clusterId:clusterId
                                              attributeId:attributeId
                                                   params:[MTRDeviceController encodeXPCReadParams:params]
                                               completion:^(id _Nullable values, NSError * _Nullable error) {
                                                   dispatch_async(clientQueue, ^{
                                                       MTR_LOG_DEBUG("Attribute read");
                                                       completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                       // The following captures the proxy handle in the closure so that the
                                                       // handle won't be released prior to block call.
                                                       __auto_type handleRetainer = handle;
                                                       (void) handleRetainer;
                                                   });
                                               }];
            } else {
                dispatch_async(clientQueue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to read attribute");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)writeAttributeWithEndpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Writing attribute ...");
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy writeAttributeWithController:self.controller
                                                    nodeId:self.nodeId
                                                endpointId:endpointId
                                                 clusterId:clusterId
                                               attributeId:attributeId
                                                     value:value
                                         timedWriteTimeout:timeoutMs
                                                completion:^(id _Nullable values, NSError * _Nullable error) {
                                                    dispatch_async(clientQueue, ^{
                                                        MTR_LOG_DEBUG("Attribute written");
                                                        completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                        // The following captures the proxy handle in the closure so that the
                                                        // handle won't be released prior to block call.
                                                        __auto_type handleRetainer = handle;
                                                        (void) handleRetainer;
                                                    });
                                                }];
            } else {
                dispatch_async(clientQueue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to write attribute");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)invokeCommandWithEndpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Invoking command ...");
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy invokeCommandWithController:self.controller
                                                   nodeId:self.nodeId
                                               endpointId:endpointId
                                                clusterId:clusterId
                                                commandId:commandId
                                                   fields:commandFields
                                       timedInvokeTimeout:timeoutMs
                                               completion:^(id _Nullable values, NSError * _Nullable error) {
                                                   dispatch_async(clientQueue, ^{
                                                       MTR_LOG_DEBUG("Command invoked");
                                                       completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                       // The following captures the proxy handle in the closure so that the
                                                       // handle won't be released prior to block call.
                                                       __auto_type handleRetainer = handle;
                                                       (void) handleRetainer;
                                                   });
                                               }];
            } else {
                dispatch_async(clientQueue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to invoke command");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)subscribeAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(MTRSubscribeParams * _Nullable)params
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(MTRDeviceResponseHandler)reportHandler
                 subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler
{
    MTR_LOG_DEBUG("Subscribing attribute ...");
    [_xpcConnection getProxyHandleWithCompletion:^(
        dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
        if (handle) {
            MTR_LOG_DEBUG("Setup report handler");
            [self.xpcConnection
                registerReportHandlerWithController:self.controller
                                             nodeId:self.nodeId
                                            handler:^(id _Nullable values, NSError * _Nullable error) {
                                                if (values && ![values isKindOfClass:[NSArray class]]) {
                                                    MTR_LOG_ERROR("Unsupported report format");
                                                    return;
                                                }
                                                if (!values) {
                                                    MTR_LOG_DEBUG("Error report received");
                                                    dispatch_async(clientQueue, ^{
                                                        reportHandler(values, error);
                                                    });
                                                    return;
                                                }
                                                __auto_type decodedValues = [MTRDeviceController decodeXPCResponseValues:values];
                                                NSMutableArray<NSDictionary<NSString *, id> *> * filteredValues =
                                                    [NSMutableArray arrayWithCapacity:[decodedValues count]];
                                                for (NSDictionary<NSString *, id> * decodedValue in decodedValues) {
                                                    MTRAttributePath * attributePath = decodedValue[MTRAttributePathKey];
                                                    if ((endpointId == nil || [attributePath.endpoint isEqualToNumber:endpointId])
                                                        && (clusterId == nil || [attributePath.cluster isEqualToNumber:clusterId])
                                                        && (attributeId == nil ||
                                                            [attributePath.attribute isEqualToNumber:attributeId])) {
                                                        [filteredValues addObject:decodedValue];
                                                    }
                                                }
                                                if ([filteredValues count] > 0) {
                                                    MTR_LOG_DEBUG("Report received");
                                                    dispatch_async(clientQueue, ^{
                                                        reportHandler(filteredValues, error);
                                                    });
                                                }
                                            }];
            [handle.proxy subscribeAttributeWithController:self.controller
                                                    nodeId:self.nodeId
                                                endpointId:endpointId
                                                 clusterId:clusterId
                                               attributeId:attributeId
                                               minInterval:minInterval
                                               maxInterval:maxInterval
                                                    params:[MTRDeviceController encodeXPCSubscribeParams:params]
                                        establishedHandler:^{
                                            dispatch_async(clientQueue, ^{
                                                MTR_LOG_DEBUG("Subscription established");
                                                subscriptionEstablishedHandler();
                                                // The following captures the proxy handle in the closure so that the handle
                                                // won't be released prior to block call.
                                                __auto_type handleRetainer = handle;
                                                (void) handleRetainer;
                                            });
                                        }];
        } else {
            dispatch_async(clientQueue, ^{
                MTR_LOG_ERROR("Failed to obtain XPC connection to subscribe to attribute");
                subscriptionEstablishedHandler();
                reportHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            });
        }
    }];
}

- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)clientQueue completion:(void (^)(void))completion
{
    MTR_LOG_DEBUG("Deregistering report handlers");
    [_xpcConnection deregisterReportHandlersWithController:self.controller
                                                    nodeId:self.nodeId
                                                completion:^{
                                                    dispatch_async(clientQueue, completion);
                                                }];
}

@end

NS_ASSUME_NONNULL_END
