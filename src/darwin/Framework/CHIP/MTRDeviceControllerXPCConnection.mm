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

#import "MTRDeviceControllerXPCConnection.h"
#import "MTRDeviceControllerOverXPC.h"
#import "MTRLogging_Internal.h"

#import <Foundation/Foundation.h>

@interface MTRDeviceControllerXPCProxyHandle ()
@property (weak, nonatomic, readonly) NSXPCConnection * xpcConnection;

- (instancetype)initWithXPCConnection:(NSXPCConnection *)xpcConnection;

@end

@implementation MTRDeviceControllerXPCProxyHandle

- (instancetype)initWithXPCConnection:(NSXPCConnection *)xpcConnection
{
    if ([super init]) {
        _xpcConnection = xpcConnection;
    }
    return self;
}

- (id<MTRDeviceControllerServerProtocol>)proxy
{
    return [_xpcConnection remoteObjectProxy];
}

- (void)dealloc
{
    [_xpcConnection invalidate];
}

@end

@interface MTRDeviceControllerXPCConnection ()

@property (strong, nonatomic, readonly) NSXPCInterface * remoteDeviceServerProtocol;
@property (strong, nonatomic, readonly) NSXPCInterface * remoteDeviceClientProtocol;
@property (strong, nonatomic, readonly) NSXPCConnection * (^connectBlock)(void);
@property (weak, nonatomic, readwrite) MTRDeviceControllerXPCProxyHandle * proxyHandle;
@property (strong, nonatomic, readwrite) MTRDeviceControllerXPCProxyHandle * proxyRetainerForReports;
@property (strong, atomic, readonly) dispatch_queue_t workQueue;

@property (strong, nonatomic, readonly) NSMutableDictionary<id, NSMutableDictionary *> * reportRegistry;

@end

@implementation MTRDeviceControllerXPCConnection

- (instancetype)initWithWorkQueue:(dispatch_queue_t)workQueue connectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    if ([super init]) {
        _remoteDeviceServerProtocol = [MTRDeviceController xpcInterfaceForServerProtocol];
        _remoteDeviceClientProtocol = [MTRDeviceController xpcInterfaceForClientProtocol];
        _connectBlock = connectBlock;
        _workQueue = workQueue;
        _reportRegistry = [[NSMutableDictionary alloc] init];
    }
    return self;
}

// This class method is for unit testing
+ (MTRDeviceControllerXPCConnection *)connectionWithWorkQueue:(dispatch_queue_t)workQueue
                                                 connectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    return [[MTRDeviceControllerXPCConnection alloc] initWithWorkQueue:workQueue connectBlock:connectBlock];
}

- (void)getProxyHandleWithCompletion:(MTRGetProxyHandleHandler)completion
{
    dispatch_async(_workQueue, ^{
        MTRDeviceControllerXPCProxyHandle * container = self.proxyHandle;
        if (!container) {
            NSXPCConnection * xpcConnection = self.connectBlock();
            if (!xpcConnection) {
                MTR_LOG_ERROR("Cannot connect to XPC server for remote controller");
                completion(self.workQueue, nil);
                return;
            }
            xpcConnection.remoteObjectInterface = self.remoteDeviceServerProtocol;
            xpcConnection.exportedInterface = self.remoteDeviceClientProtocol;
            xpcConnection.exportedObject = self;
            [xpcConnection resume];
            container = [[MTRDeviceControllerXPCProxyHandle alloc] initWithXPCConnection:xpcConnection];
            self.proxyHandle = container;
            __weak typeof(self) weakSelf = self;
            xpcConnection.invalidationHandler = ^{
                typeof(self) strongSelf = weakSelf;
                if (strongSelf) {
                    dispatch_async(strongSelf.workQueue, ^{
                        strongSelf.proxyHandle = nil;
                        strongSelf.proxyRetainerForReports = nil;
                        [strongSelf.reportRegistry removeAllObjects];
                        MTR_LOG_DEBUG("CHIP XPC connection disconnected");
                    });
                }
            };
            MTR_LOG_DEBUG("CHIP XPC connection established");
        }
        completion(self.workQueue, container);
    });
}

- (void)registerReportHandlerWithController:(id<NSCopying>)controller nodeID:(NSNumber *)nodeID handler:(MTRXPCReportHandler)handler
{
    dispatch_async(_workQueue, ^{
        BOOL shouldRetainProxyForReport = ([self.reportRegistry count] == 0);
        NSMutableDictionary * controllerDictionary = self.reportRegistry[controller];
        if (!controllerDictionary) {
            controllerDictionary = [[NSMutableDictionary alloc] init];
            [self.reportRegistry setObject:controllerDictionary forKey:controller];
        }
        NSMutableArray * nodeArray = controllerDictionary[nodeID];
        if (!nodeArray) {
            nodeArray = [[NSMutableArray alloc] init];
            [controllerDictionary setObject:nodeArray forKey:nodeID];
        }
        [nodeArray addObject:handler];
        if (shouldRetainProxyForReport) {
            self.proxyRetainerForReports = self.proxyHandle;
        }
    });
}

- (void)deregisterReportHandlersWithController:(id<NSCopying>)controller
                                        nodeID:(NSNumber *)nodeID
                                    completion:(dispatch_block_t)completion
{
    dispatch_async(_workQueue, ^{
        __auto_type clearRegistry = ^{
            NSMutableDictionary * controllerDictionary = self.reportRegistry[controller];
            if (!controllerDictionary) {
                completion();
                return;
            }
            NSMutableArray * nodeArray = controllerDictionary[nodeID];
            if (!nodeArray) {
                completion();
                return;
            }
            [controllerDictionary removeObjectForKey:nodeID];
            if ([controllerDictionary count] == 0) {
                // Dereference proxy retainer for reports so that XPC connection may be invalidated if no longer used.
                self.proxyRetainerForReports = nil;
            }
            completion();
        };
        [self
            getProxyHandleWithCompletion:^(dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
                if (handle) {
                    MTR_LOG_DEBUG("CHIP XPC connection requests to stop reports");
                    [handle.proxy stopReportsWithController:controller
                                                     nodeId:nodeID.unsignedLongLongValue
                                                 completion:^{
                                                     // Make sure handle stays alive until we get to this
                                                     // completion (in particular while we are working
                                                     // with handle.proxy).
                                                     __auto_type handleRetainer = handle;
                                                     (void) handleRetainer;
                                                     dispatch_async(queue, clearRegistry);
                                                 }];
                } else {
                    MTR_LOG_ERROR("CHIP XPC connection failed to stop reporting");
                    clearRegistry();
                }
            }];
    });
}

- (void)handleReportWithController:(id)controller
                            nodeId:(uint64_t)nodeId
                            values:(id _Nullable)values
                             error:(NSError * _Nullable)error
{
    dispatch_async(_workQueue, ^{
        NSMutableDictionary * controllerDictionary = self.reportRegistry[controller];
        if (!controllerDictionary) {
            return;
        }
        NSMutableArray * nodeArray = controllerDictionary[@(nodeId)];
        if (!nodeArray) {
            return;
        }
        for (void (^handler)(id _Nullable values, NSError * _Nullable error) in nodeArray) {
            handler(values, error);
        }
    });
}

- (void)callSubscriptionEstablishedHandler:(dispatch_block_t)handler
{
    // Call the handler from our _workQueue, so that we guarantee the same
    // number of queue hops as for handleReportWithController.
    dispatch_async(_workQueue, handler);
}

@end
