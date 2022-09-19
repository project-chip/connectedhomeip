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

#import "MTRCluster.h"
#import "MTRClusterStateCacheContainer+XPC.h"
#import "MTRDeviceController+XPC.h"
#import "MTRDeviceControllerOverXPC_Internal.h"
#import "MTRDeviceControllerXPCConnection.h"
#import "MTRError.h"
#import "MTRLogging.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceOverXPC ()

@property (nonatomic, strong, readonly, nullable) id<NSCopying> controllerID;
@property (nonatomic, strong, readonly) MTRDeviceControllerOverXPC * controller;
@property (nonatomic, readonly) NSNumber * nodeID;
@property (nonatomic, strong, readonly) MTRDeviceControllerXPCConnection * xpcConnection;

@end

@implementation MTRDeviceOverXPC

- (instancetype)initWithControllerOverXPC:(MTRDeviceControllerOverXPC *)controllerOverXPC
                                 deviceID:(NSNumber *)deviceID
                            xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
{
    _controllerID = controllerOverXPC.controllerID;
    _controller = controllerOverXPC;
    _nodeID = deviceID;
    _xpcConnection = xpcConnection;
    return self;
}

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                        params:(MTRSubscribeParams *)params
    clusterStateCacheContainer:(MTRClusterStateCacheContainer * _Nullable)clusterStateCacheContainer
        attributeReportHandler:(void (^_Nullable)(NSArray * value))attributeReportHandler
            eventReportHandler:(void (^_Nullable)(NSArray * value))eventReportHandler
                  errorHandler:(void (^)(NSError * error))errorHandler
       subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler
       resubscriptionScheduled:(MTRDeviceResubscriptionScheduledHandler _Nullable)resubscriptionScheduledHandler
{
    MTR_LOG_DEBUG("Subscribing all attributes... Note that attributeReportHandler, eventReportHandler, and resubscriptionScheduled "
                  "are not supported.");
    __auto_type workBlock = ^{
        if (clusterStateCacheContainer) {
            [clusterStateCacheContainer setXPCConnection:self->_xpcConnection controllerID:self.controllerID deviceID:self.nodeID];
        }
        [self->_xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy subscribeWithController:self.controllerID
                                               nodeID:self.nodeID
                                               params:[MTRDeviceController encodeXPCSubscribeParams:params]
                                          shouldCache:(clusterStateCacheContainer != nil)
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
    };

    if (self.controllerID != nil) {
        workBlock();
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID, NSError * _Nullable error) {
                                             if (error != nil) {
                                                 // We're already running on the right queue.
                                                 errorHandler(error);
                                                 return;
                                             }

                                             self->_controllerID = controllerID;
                                             workBlock();
                                         }];
    }
}

- (void)readAttributePathWithEndpointID:(NSNumber * _Nullable)endpointID
                              clusterID:(NSNumber * _Nullable)clusterID
                            attributeID:(NSNumber * _Nullable)attributeID
                                 params:(MTRReadParams * _Nullable)params
                                  queue:(dispatch_queue_t)queue
                             completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Reading attribute ...");
    __auto_type workBlock = ^{
        [self->_xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy readAttributeWithController:self.controllerID
                                                   nodeID:self.nodeID
                                               endpointID:endpointID
                                                clusterID:clusterID
                                              attributeID:attributeID
                                                   params:[MTRDeviceController encodeXPCReadParams:params]
                                               completion:^(id _Nullable values, NSError * _Nullable error) {
                                                   dispatch_async(queue, ^{
                                                       MTR_LOG_DEBUG("Attribute read");
                                                       completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                       // The following captures the proxy handle in the closure so that the
                                                       // handle won't be released prior to block call.
                                                       __auto_type handleRetainer = handle;
                                                       (void) handleRetainer;
                                                   });
                                               }];
            } else {
                dispatch_async(queue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to read attribute");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
    };

    if (self.controllerID != nil) {
        workBlock();
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID, NSError * _Nullable error) {
                                             if (error != nil) {
                                                 // We're already running on the right queue.
                                                 completion(nil, error);
                                                 return;
                                             }

                                             self->_controllerID = controllerID;
                                             workBlock();
                                         }];
    }
}

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Writing attribute ...");
    __auto_type workBlock = ^{
        [self->_xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy writeAttributeWithController:self.controllerID
                                                    nodeID:self.nodeID
                                                endpointID:endpointID
                                                 clusterID:clusterID
                                               attributeID:attributeID
                                                     value:value
                                         timedWriteTimeout:timeoutMs
                                                completion:^(id _Nullable values, NSError * _Nullable error) {
                                                    dispatch_async(queue, ^{
                                                        MTR_LOG_DEBUG("Attribute written");
                                                        completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                        // The following captures the proxy handle in the closure so that the
                                                        // handle won't be released prior to block call.
                                                        __auto_type handleRetainer = handle;
                                                        (void) handleRetainer;
                                                    });
                                                }];
            } else {
                dispatch_async(queue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to write attribute");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
    };

    if (self.controllerID != nil) {
        workBlock();
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID, NSError * _Nullable error) {
                                             if (error != nil) {
                                                 // We're already running on the right queue.
                                                 completion(nil, error);
                                                 return;
                                             }

                                             self->_controllerID = controllerID;
                                             workBlock();
                                         }];
    }
}

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Invoking command ...");
    __auto_type workBlock = ^{
        [self->_xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy invokeCommandWithController:self.controllerID
                                                   nodeID:self.nodeID
                                               endpointID:endpointID
                                                clusterID:clusterID
                                                commandID:commandID
                                                   fields:commandFields
                                       timedInvokeTimeout:timeoutMs
                                               completion:^(id _Nullable values, NSError * _Nullable error) {
                                                   dispatch_async(queue, ^{
                                                       MTR_LOG_DEBUG("Command invoked");
                                                       completion([MTRDeviceController decodeXPCResponseValues:values], error);
                                                       // The following captures the proxy handle in the closure so that the
                                                       // handle won't be released prior to block call.
                                                       __auto_type handleRetainer = handle;
                                                       (void) handleRetainer;
                                                   });
                                               }];
            } else {
                dispatch_async(queue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to invoke command");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
    };

    if (self.controllerID != nil) {
        workBlock();
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID, NSError * _Nullable error) {
                                             if (error != nil) {
                                                 // We're already running on the right queue.
                                                 completion(nil, error);
                                                 return;
                                             }

                                             self->_controllerID = controllerID;
                                             workBlock();
                                         }];
    }
}

- (void)subscribeAttributePathWithEndpointID:(NSNumber * _Nullable)endpointID
                                   clusterID:(NSNumber * _Nullable)clusterID
                                 attributeID:(NSNumber * _Nullable)attributeID
                                      params:(MTRSubscribeParams *)params
                                       queue:(dispatch_queue_t)queue
                               reportHandler:(MTRDeviceResponseHandler)reportHandler
                     subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler
{
    MTR_LOG_DEBUG("Subscribing attribute ...");
    __auto_type workBlock = ^{
        [self->_xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                MTR_LOG_DEBUG("Setup report handler");
                [self.xpcConnection
                    registerReportHandlerWithController:self.controllerID
                                                 nodeID:self.nodeID
                                                handler:^(id _Nullable values, NSError * _Nullable error) {
                                                    if (values && ![values isKindOfClass:[NSArray class]]) {
                                                        MTR_LOG_ERROR("Unsupported report format");
                                                        return;
                                                    }
                                                    if (!values) {
                                                        MTR_LOG_DEBUG("Error report received");
                                                        dispatch_async(queue, ^{
                                                            reportHandler(values, error);
                                                        });
                                                        return;
                                                    }
                                                    __auto_type decodedValues =
                                                        [MTRDeviceController decodeXPCResponseValues:values];
                                                    NSMutableArray<NSDictionary<NSString *, id> *> * filteredValues =
                                                        [NSMutableArray arrayWithCapacity:[decodedValues count]];
                                                    for (NSDictionary<NSString *, id> * decodedValue in decodedValues) {
                                                        MTRAttributePath * attributePath = decodedValue[MTRAttributePathKey];
                                                        if ((endpointID == nil ||
                                                                [attributePath.endpoint isEqualToNumber:endpointID])
                                                            && (clusterID == nil ||
                                                                [attributePath.cluster isEqualToNumber:clusterID])
                                                            && (attributeID == nil ||
                                                                [attributePath.attribute isEqualToNumber:attributeID])) {
                                                            [filteredValues addObject:decodedValue];
                                                        }
                                                    }
                                                    if ([filteredValues count] > 0) {
                                                        MTR_LOG_DEBUG("Report received");
                                                        dispatch_async(queue, ^{
                                                            reportHandler(filteredValues, error);
                                                        });
                                                    }
                                                }];

                [handle.proxy subscribeAttributeWithController:self.controllerID
                                                        nodeID:self.nodeID
                                                    endpointID:endpointID
                                                     clusterID:clusterID
                                                   attributeID:attributeID
                                                        params:[MTRDeviceController encodeXPCSubscribeParams:params]
                                            establishedHandler:^{
                                                dispatch_async(queue, ^{
                                                    MTR_LOG_DEBUG("Subscription established");
                                                    subscriptionEstablishedHandler();
                                                    // The following captures the proxy handle in the closure so that the handle
                                                    // won't be released prior to block call.
                                                    __auto_type handleRetainer = handle;
                                                    (void) handleRetainer;
                                                });
                                            }];
            } else {
                dispatch_async(queue, ^{
                    MTR_LOG_ERROR("Failed to obtain XPC connection to subscribe to attribute");
                    subscriptionEstablishedHandler();
                    reportHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
    };

    if (self.controllerID != nil) {
        workBlock();
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID, NSError * _Nullable error) {
                                             if (error != nil) {
                                                 // We're already running on the right queue.
                                                 reportHandler(nil, error);
                                                 return;
                                             }

                                             self->_controllerID = controllerID;
                                             workBlock();
                                         }];
    }
}

- (void)deregisterReportHandlersWithQueue:(dispatch_queue_t)queue completion:(void (^)(void))completion
{
    MTR_LOG_DEBUG("Deregistering report handlers");
    __auto_type workBlock = ^{
        [self->_xpcConnection deregisterReportHandlersWithController:self.controllerID
                                                              nodeID:self.nodeID
                                                          completion:^{
                                                              dispatch_async(queue, completion);
                                                          }];
    };

    if (self.controllerID != nil) {
        workBlock();
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID, NSError * _Nullable error) {
                                             if (error != nil) {
                                                 // We're already running on the right queue.
                                                 completion();
                                                 return;
                                             }

                                             self->_controllerID = controllerID;
                                             workBlock();
                                         }];
    }
}

- (void)openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode
                                   discriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    MTR_LOG_ERROR("MTRDevice doesn't support openCommissioningWindowWithSetupPasscode over XPC");
    dispatch_async(queue, ^{
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]);
    });
}

@end

NS_ASSUME_NONNULL_END
