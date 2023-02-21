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

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRXPCReportHandler)(id _Nullable values, NSError * _Nullable error);

/**
 * handle for XPC remote object proxy for remote device controller
 *
 * Releasing the handle may cause invalidating the XPC connection. Hence, in order to retain the connection, the handle must not be
 * released.
 */
@interface MTRDeviceControllerXPCProxyHandle : NSObject

@property (nonatomic, readonly, getter=proxy) id<MTRDeviceControllerServerProtocol> proxy;

@end

typedef void (^MTRGetProxyHandleHandler)(dispatch_queue_t queue, MTRDeviceControllerXPCProxyHandle * _Nullable container);

/**
 * class to manage XPC connection for remote device controller
 *
 * This class is in charge of making a new XPC connection and disconnecting as needed by the clients and by the report handlers.
 */
@interface MTRDeviceControllerXPCConnection<MTRDeviceControllerClientProtocol> : NSObject

/**
 * This method is just for test purpsoe.
 */
+ (MTRDeviceControllerXPCConnection *)connectionWithWorkQueue:(dispatch_queue_t)workQueue
                                                 connectBlock:(MTRXPCConnectBlock)connectBlock;

- (void)getProxyHandleWithCompletion:(MTRGetProxyHandleHandler)completion;
- (void)registerReportHandlerWithController:(id<NSCopying>)controller
                                     nodeID:(NSNumber *)nodeID
                                    handler:(MTRXPCReportHandler)handler;
- (void)deregisterReportHandlersWithController:(id<NSCopying>)controller
                                        nodeID:(NSNumber *)nodeID
                                    completion:(dispatch_block_t)completion;

@end

NS_ASSUME_NONNULL_END
