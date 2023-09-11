/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRDefines.h>

#import "MTRDeviceController+XPC.h"

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRXPCReportHandler)(id _Nullable values, NSError * _Nullable error);

/**
 * handle for XPC remote object proxy for remote device controller
 *
 * Releasing the handle may cause invalidating the XPC connection. Hence, in order to retain the connection, the handle must not be
 * released.
 */
MTR_HIDDEN
@interface MTRDeviceControllerXPCProxyHandle : NSObject

@property (nonatomic, readonly, getter=proxy) id<MTRDeviceControllerServerProtocol> proxy;

@end

typedef void (^MTRGetProxyHandleHandler)(dispatch_queue_t queue, MTRDeviceControllerXPCProxyHandle * _Nullable container);

/**
 * class to manage XPC connection for remote device controller
 *
 * This class is in charge of making a new XPC connection and disconnecting as needed by the clients and by the report handlers.
 */
MTR_HIDDEN
@interface MTRDeviceControllerXPCConnection<MTRDeviceControllerClientProtocol> : NSObject

/**
 * This method is just for test purpose.
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
- (void)callSubscriptionEstablishedHandler:(dispatch_block_t)handler;

@end

NS_ASSUME_NONNULL_END
