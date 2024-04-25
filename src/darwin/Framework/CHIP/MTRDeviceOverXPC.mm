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
#import "MTRLogging_Internal.h"

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRFetchProxyHandleCompletion)(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error);

@interface MTRDeviceOverXPC ()

@property (nonatomic, strong, readonly, nullable) id<NSCopying> controllerID;
@property (nonatomic, strong, readonly) MTRDeviceControllerOverXPC * controller;
@property (nonatomic, readonly) NSNumber * nodeID;
@property (nonatomic, strong, readonly) MTRDeviceControllerXPCConnection * xpcConnection;

- (void)fetchProxyHandleWithQueue:(dispatch_queue_t)queue completion:(MTRFetchProxyHandleCompletion)completion;

@end

@implementation MTRDeviceOverXPC

- (instancetype)initWithControllerOverXPC:(MTRDeviceControllerOverXPC *)controllerOverXPC
                                 deviceID:(NSNumber *)deviceID
                            xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
{
    _controllerID = controllerOverXPC.controllerXPCID;
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

    __auto_type workBlock = ^(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
        if (error != nil) {
            errorHandler(error);
            return;
        }

        if (clusterStateCacheContainer) {
            [clusterStateCacheContainer setXPCConnection:self->_xpcConnection controllerID:self.controllerID deviceID:self.nodeID];
        }

        [handle.proxy subscribeWithController:self.controllerID
                                       nodeId:self.nodeID.unsignedLongLongValue
                                  minInterval:params.minInterval
                                  maxInterval:params.maxInterval
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
    };

    [self fetchProxyHandleWithQueue:queue completion:workBlock];
}

- (void)readAttributesWithEndpointID:(NSNumber * _Nullable)endpointID
                           clusterID:(NSNumber * _Nullable)clusterID
                         attributeID:(NSNumber * _Nullable)attributeID
                              params:(MTRReadParams * _Nullable)params
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    MTR_LOG_DEBUG("Reading attribute ...");

    __auto_type workBlock = ^(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
        if (error != nil) {
            completion(nil, error);
            return;
        }

        [handle.proxy readAttributeWithController:self.controllerID
                                           nodeId:self.nodeID.unsignedLongLongValue
                                       endpointId:endpointID
                                        clusterId:clusterID
                                      attributeId:attributeID
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
    };

    [self fetchProxyHandleWithQueue:queue completion:workBlock];
}

- (void)readAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                    params:(MTRReadParams * _Nullable)params
                     queue:(dispatch_queue_t)queue
                completion:(MTRDeviceResponseHandler)completion
{
    [self readAttributePaths:attributePaths eventPaths:eventPaths params:params includeDataVersion:NO queue:queue completion:completion];
}

- (void)readAttributePaths:(NSArray<MTRAttributeRequestPath *> * _Nullable)attributePaths
                eventPaths:(NSArray<MTREventRequestPath *> * _Nullable)eventPaths
                    params:(MTRReadParams * _Nullable)params
        includeDataVersion:(BOOL)includeDataVersion
                     queue:(dispatch_queue_t)queue
                completion:(MTRDeviceResponseHandler)completion
{
    if (attributePaths == nil || eventPaths != nil) {
        MTR_LOG_ERROR("MTRBaseDevice doesn't support reading event paths over XPC");
        dispatch_async(queue, ^{
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]);
        });
        return;
    }

    // TODO: Have a better XPC setup for the multiple-paths case, instead of
    // just converting it into a bunch of separate reads.
    auto expectedResponses = attributePaths.count;
    __block decltype(expectedResponses) responses = 0;
    NSMutableArray<NSDictionary<NSString *, id> *> * seenValues = [[NSMutableArray alloc] init];
    __block BOOL dispatched = NO;

    for (MTRAttributeRequestPath * path in attributePaths) {
        __auto_type singleAttributeResponseHandler = ^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
            if (dispatched) {
                // We hit an error earlier or something.
                return;
            }

            if (error != nil) {
                dispatched = YES;
                completion(nil, error);
                return;
            }

            [seenValues addObjectsFromArray:values];
            ++responses;
            if (responses == expectedResponses) {
                dispatched = YES;
                completion([seenValues copy], nil);
            };
        };

        [self readAttributesWithEndpointID:path.endpoint
                                 clusterID:path.cluster
                               attributeID:path.attribute
                                    params:params
                                     queue:queue
                                completion:singleAttributeResponseHandler];
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

    __auto_type workBlock = ^(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
        if (error != nil) {
            completion(nil, error);
            return;
        }

        [handle.proxy writeAttributeWithController:self.controllerID
                                            nodeId:self.nodeID.unsignedLongLongValue
                                        endpointId:endpointID
                                         clusterId:clusterID
                                       attributeId:attributeID
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
    };

    [self fetchProxyHandleWithQueue:queue completion:workBlock];
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

    __auto_type workBlock = ^(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
        if (error != nil) {
            completion(nil, error);
            return;
        }

        [handle.proxy invokeCommandWithController:self.controllerID
                                           nodeId:self.nodeID.unsignedLongLongValue
                                       endpointId:endpointID
                                        clusterId:clusterID
                                        commandId:commandID
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
    };

    [self fetchProxyHandleWithQueue:queue completion:workBlock];
}

- (void)_invokeCommandWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                           commandID:(NSNumber *)commandID
                       commandFields:(id)commandFields
                  timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
         serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                               queue:(dispatch_queue_t)queue
                          completion:(MTRDeviceResponseHandler)completion
{
    if (serverSideProcessingTimeout != nil) {
        MTR_LOG_ERROR("MTRBaseDevice doesn't support invokes with a server-side processing timeout over XPC");
        dispatch_async(queue, ^{
            completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]);
        });
        return;
    }

    [self invokeCommandWithEndpointID:endpointID
                            clusterID:clusterID
                            commandID:commandID
                        commandFields:commandFields
                   timedInvokeTimeout:timeoutMs
                                queue:queue
                           completion:completion];
}

- (void)subscribeToAttributesWithEndpointID:(NSNumber * _Nullable)endpointID
                                  clusterID:(NSNumber * _Nullable)clusterID
                                attributeID:(NSNumber * _Nullable)attributeID
                                     params:(MTRSubscribeParams * _Nullable)params
                                      queue:(dispatch_queue_t)queue
                              reportHandler:(MTRDeviceResponseHandler)reportHandler
                    subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler
{
    MTR_LOG_DEBUG("Subscribing attribute ...");

    __auto_type workBlock = ^(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
        MTR_LOG_DEBUG("Setup report handler");

        if (error != nil) {
            subscriptionEstablishedHandler();
            reportHandler(nil, error);
            return;
        }

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
                                            __auto_type decodedValues = [MTRDeviceController decodeXPCResponseValues:values];
                                            NSMutableArray<NSDictionary<NSString *, id> *> * filteredValues =
                                                [NSMutableArray arrayWithCapacity:[decodedValues count]];
                                            for (NSDictionary<NSString *, id> * decodedValue in decodedValues) {
                                                MTRAttributePath * attributePath = decodedValue[MTRAttributePathKey];
                                                if ((endpointID == nil || [attributePath.endpoint isEqualToNumber:endpointID])
                                                    && (clusterID == nil || [attributePath.cluster isEqualToNumber:clusterID])
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
                                                nodeId:self.nodeID.unsignedLongLongValue
                                            endpointId:endpointID
                                             clusterId:clusterID
                                           attributeId:attributeID
                                           minInterval:params.minInterval
                                           maxInterval:params.maxInterval
                                                params:[MTRDeviceController encodeXPCSubscribeParams:params]
                                    establishedHandler:^{
                                        [self.xpcConnection callSubscriptionEstablishedHandler:^{
                                            dispatch_async(queue, ^{
                                                MTR_LOG_DEBUG("Subscription established");
                                                subscriptionEstablishedHandler();
                                                // The following captures the proxy handle in the closure so that the handle
                                                // won't be released prior to block call.
                                                __auto_type handleRetainer = handle;
                                                (void) handleRetainer;
                                            });
                                        }];
                                    }];
    };

    [self fetchProxyHandleWithQueue:queue completion:workBlock];
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
                                         completion:^(id _Nullable controllerID,
                                             MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
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
    MTR_LOG_ERROR("MTRBaseDevice doesn't support openCommissioningWindowWithSetupPasscode over XPC");
    dispatch_async(queue, ^{
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]);
    });
}

- (void)openCommissioningWindowWithDiscriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    MTR_LOG_ERROR("MTRBaseDevice doesn't support openCommissioningWindowWithDiscriminator over XPC");
    dispatch_async(queue, ^{
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil]);
    });
}

- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    MTR_LOG_DEBUG("Downloading log ...");

    __auto_type workBlock = ^(MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
        if (error != nil) {
            completion(nil, error);
            return;
        }

        [handle.proxy downloadLogWithController:self.controllerID
                                         nodeId:self.nodeID
                                           type:type
                                        timeout:timeout
                                     completion:^(NSString * _Nullable url, NSError * _Nullable error) {
                                         dispatch_async(queue, ^{
                                             MTR_LOG_DEBUG("Download log");
                                             completion([NSURL URLWithString:url], error);
                                             // The following captures the proxy handle in the closure so that the
                                             // handle won't be released prior to block call.
                                             __auto_type handleRetainer = handle;
                                             (void) handleRetainer;
                                         });
                                     }];
    };

    [self fetchProxyHandleWithQueue:queue completion:workBlock];
}

- (void)fetchProxyHandleWithQueue:(dispatch_queue_t)queue completion:(MTRFetchProxyHandleCompletion)completion
{
    if (self.controllerID != nil) {
        [self->_xpcConnection getProxyHandleWithCompletion:^(
            dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
            dispatch_async(queue, ^{
                if (handle == nil) {
                    MTR_LOG_ERROR("Failed to obtain XPC connection");
                    completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
                } else {
                    completion(handle, nil);
                }
            });
        }];
    } else {
        [self.controller fetchControllerIdWithQueue:queue
                                         completion:^(id _Nullable controllerID,
                                             MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error) {
                                             // We're already running on the right queue.
                                             if (error != nil) {
                                                 completion(nil, error);
                                             } else {
                                                 self->_controllerID = controllerID;
                                                 completion(handle, nil);
                                             }
                                         }];
    }
}

@end

NS_ASSUME_NONNULL_END
