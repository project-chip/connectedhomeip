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

NS_ASSUME_NONNULL_BEGIN

/**
 * handle for XPC remote object proxy for remote device controller
 *
 * Releasing the handle may cause invalidating the XPC connection. Hence, in order to retain the connection, the handle must not be
 * released.
 */
@interface CHIPDeviceControllerXPCProxyHandle : NSObject

@property (nonatomic, readonly, getter=proxy) id<CHIPDeviceControllerServerProtocol> proxy;

@end

/**
 * class to manage XPC connection for remote device controller
 *
 * This class is in charge of making a new XPC connection and disconnecting as needed by the clients and by the report handlers.
 */
@interface CHIPDeviceControllerXPCConnection<CHIPDeviceControllerClientProtocol> : NSObject

/**
 * This method is just for test purpsoe.
 */
+ (instancetype)connectionWithWorkQueue:(dispatch_queue_t)workQueue connectBlock:(NSXPCConnection * (^)(void) )connectBlock;

- (void)getProxyHandleWithCompletion:(void (^)(dispatch_queue_t queue,
                                         CHIPDeviceControllerXPCProxyHandle * _Nullable container))completion;
- (void)registerReportHandlerWithController:(id<NSCopying>)controller
                                     nodeId:(NSUInteger)nodeId
                                    handler:(void (^)(id _Nullable value, NSError * _Nullable error))handler;
- (void)deregisterReportHandlersWithController:(id<NSCopying>)controller
                                        nodeId:(NSUInteger)nodeId
                                    completion:(void (^)(void))completion;

@end

NS_ASSUME_NONNULL_END
