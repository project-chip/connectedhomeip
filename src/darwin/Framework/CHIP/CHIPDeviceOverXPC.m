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

#import "CHIPDeviceOverXPC.h"

#import "CHIPDeviceController+XPC.h"
#import "CHIPDeviceControllerXPCConnection.h"
#import "CHIPError.h"
#import "CHIPLogging.h"

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDeviceOverXPC ()

@property (nonatomic, strong, readonly) id<NSCopying> controller;
@property (nonatomic, readonly) uint64_t nodeId;
@property (nonatomic, strong, readonly) CHIPDeviceControllerXPCConnection * xpcConnection;

@end

@implementation CHIPDeviceOverXPC

- (instancetype)initWithController:(id<NSCopying>)controller
                          deviceId:(uint64_t)deviceId
                     xpcConnection:(CHIPDeviceControllerXPCConnection *)xpcConnection
{
    _controller = controller;
    _nodeId = deviceId;
    _xpcConnection = xpcConnection;
    return self;
}

- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(uint16_t)minInterval
                maxInterval:(uint16_t)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler
{
    dispatch_async(queue, ^{
        CHIP_LOG_ERROR("All attribute subscription is not supported by remote device");
        subscriptionEstablishedHandler();
        reportHandler(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
    });
}

- (void)readAttributeWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion
{
    CHIP_LOG_DEBUG("Reading attribute ...");
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy readAttributeWithController:self.controller
                                                   nodeId:self.nodeId
                                               endpointId:endpointId
                                                clusterId:clusterId
                                              attributeId:attributeId
                                               completion:^(id _Nullable values, NSError * _Nullable error) {
                                                   dispatch_async(clientQueue, ^{
                                                       CHIP_LOG_DEBUG("Attribute read");
                                                       completion([CHIPDeviceController decodeXPCResponseValues:values], error);
                                                       // The following captures the proxy handle in the closure so that the
                                                       // handle won't be released prior to block call.
                                                       __auto_type handleRetainer = handle;
                                                       (void) handleRetainer;
                                                   });
                                               }];
            } else {
                dispatch_async(clientQueue, ^{
                    CHIP_LOG_ERROR("Failed to obtain XPC connection to read attribute");
                    completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)writeAttributeWithEndpointId:(NSUInteger)endpointId
                           clusterId:(NSUInteger)clusterId
                         attributeId:(NSUInteger)attributeId
                               value:(id)value
                         clientQueue:(dispatch_queue_t)clientQueue
                          completion:(CHIPDeviceResponseHandler)completion
{
    CHIP_LOG_DEBUG("Writing attribute ...");
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy writeAttributeWithController:self.controller
                                                    nodeId:self.nodeId
                                                endpointId:endpointId
                                                 clusterId:clusterId
                                               attributeId:attributeId
                                                     value:value
                                                completion:^(id _Nullable values, NSError * _Nullable error) {
                                                    dispatch_async(clientQueue, ^{
                                                        CHIP_LOG_DEBUG("Attribute written");
                                                        completion([CHIPDeviceController decodeXPCResponseValues:values], error);
                                                        // The following captures the proxy handle in the closure so that the
                                                        // handle won't be released prior to block call.
                                                        __auto_type handleRetainer = handle;
                                                        (void) handleRetainer;
                                                    });
                                                }];
            } else {
                dispatch_async(clientQueue, ^{
                    CHIP_LOG_ERROR("Failed to obtain XPC connection to write attribute");
                    completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)invokeCommandWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                          commandId:(NSUInteger)commandId
                      commandFields:(id)commandFields
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(CHIPDeviceResponseHandler)completion
{
    CHIP_LOG_DEBUG("Invoking command ...");
    [_xpcConnection
        getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
            if (handle) {
                [handle.proxy invokeCommandWithController:self.controller
                                                   nodeId:self.nodeId
                                               endpointId:endpointId
                                                clusterId:clusterId
                                                commandId:commandId
                                                   fields:commandFields
                                               completion:^(id _Nullable values, NSError * _Nullable error) {
                                                   dispatch_async(clientQueue, ^{
                                                       CHIP_LOG_DEBUG("Command invoked");
                                                       completion([CHIPDeviceController decodeXPCResponseValues:values], error);
                                                       // The following captures the proxy handle in the closure so that the
                                                       // handle won't be released prior to block call.
                                                       __auto_type handleRetainer = handle;
                                                       (void) handleRetainer;
                                                   });
                                               }];
            } else {
                dispatch_async(clientQueue, ^{
                    CHIP_LOG_ERROR("Failed to obtain XPC connection to invoke command");
                    completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
                });
            }
        }];
}

- (void)subscribeAttributeWithEndpointId:(NSUInteger)endpointId
                               clusterId:(NSUInteger)clusterId
                             attributeId:(NSUInteger)attributeId
                             minInterval:(NSUInteger)minInterval
                             maxInterval:(NSUInteger)maxInterval
                             clientQueue:(dispatch_queue_t)clientQueue
                           reportHandler:(CHIPDeviceResponseHandler)reportHandler
                 subscriptionEstablished:(void (^_Nullable)(void))subscriptionEstablishedHandler
{
    CHIP_LOG_DEBUG("Subscribing attribute ...");
    [_xpcConnection getProxyHandleWithCompletion:^(
        dispatch_queue_t _Nonnull queue, CHIPDeviceControllerXPCProxyHandle * _Nullable handle) {
        if (handle) {
            CHIP_LOG_DEBUG("Setup report handler");
            [self.xpcConnection
                registerReportHandlerWithController:self.controller
                                             nodeId:self.nodeId
                                            handler:^(id _Nullable values, NSError * _Nullable error) {
                                                if (values && ![values isKindOfClass:[NSArray class]]) {
                                                    CHIP_LOG_ERROR("Unsupported report format");
                                                    return;
                                                }
                                                if (!values) {
                                                    CHIP_LOG_DEBUG("Error report received");
                                                    dispatch_async(clientQueue, ^{
                                                        reportHandler(values, error);
                                                    });
                                                    return;
                                                }
                                                __auto_type decodedValues = [CHIPDeviceController decodeXPCResponseValues:values];
                                                NSMutableArray<NSDictionary<NSString *, id> *> * filteredValues =
                                                    [NSMutableArray arrayWithCapacity:[decodedValues count]];
                                                for (NSDictionary<NSString *, id> * decodedValue in decodedValues) {
                                                    CHIPAttributePath * attributePath = decodedValue[kCHIPAttributePathKey];
                                                    if (([attributePath.endpoint unsignedIntegerValue] == endpointId
                                                            || endpointId == 0xffff)
                                                        && ([attributePath.cluster unsignedIntegerValue] == clusterId
                                                            || clusterId == 0xffffffff)
                                                        && ([attributePath.attribute unsignedIntegerValue] == attributeId
                                                            || attributeId == 0xffffffff)) {
                                                        [filteredValues addObject:decodedValue];
                                                    }
                                                }
                                                if ([filteredValues count] > 0) {
                                                    CHIP_LOG_DEBUG("Report received");
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
                                        establishedHandler:^{
                                            dispatch_async(clientQueue, ^{
                                                CHIP_LOG_DEBUG("Subscription established");
                                                subscriptionEstablishedHandler();
                                                // The following captures the proxy handle in the closure so that the handle
                                                // won't be released prior to block call.
                                                __auto_type handleRetainer = handle;
                                                (void) handleRetainer;
                                            });
                                        }];
        } else {
            dispatch_async(clientQueue, ^{
                CHIP_LOG_ERROR("Failed to obtain XPC connection to subscribe to attribute");
                subscriptionEstablishedHandler();
                reportHandler(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
            });
        }
    }];
}

- (void)deregisterReportHandlersWithClientQueue:(dispatch_queue_t)clientQueue completion:(void (^)(void))completion
{
    CHIP_LOG_DEBUG("Deregistering report handlers");
    [_xpcConnection deregisterReportHandlersWithController:self.controller
                                                    nodeId:self.nodeId
                                                completion:^{
                                                    dispatch_async(clientQueue, completion);
                                                }];
}

@end

NS_ASSUME_NONNULL_END
