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

#import "CHIPDeviceControllerXPCConnection.h"
#import "CHIPDeviceControllerOverXPC.h"
#import "CHIPLogging.h"

#import <Foundation/Foundation.h>

@interface CHIPDeviceControllerXPCProxyHandle ()
@property (weak, nonatomic, readonly) NSXPCConnection * xpcConnection;

- (instancetype)initWithXPCConnection:(NSXPCConnection *)xpcConnection;

@end

@implementation CHIPDeviceControllerXPCProxyHandle

- (instancetype)initWithXPCConnection:(NSXPCConnection *)xpcConnection
{
    if ([super init]) {
        _xpcConnection = xpcConnection;
    }
    return self;
}

- (id<CHIPDeviceControllerServerProtocol>)proxy
{
    return [_xpcConnection remoteObjectProxy];
}

- (void)dealloc
{
    [_xpcConnection invalidate];
}

@end

@interface CHIPDeviceControllerXPCConnection ()

@property (strong, nonatomic, readonly) NSXPCInterface * remoteDeviceServerProtocol;
@property (strong, nonatomic, readonly) NSXPCInterface * remoteDeviceClientProtocol;
@property (strong, nonatomic, readonly) NSXPCConnection * (^connectBlock)(void);
@property (weak, nonatomic, readwrite) CHIPDeviceControllerXPCProxyHandle * proxyHandle;
@property (strong, nonatomic, readwrite) CHIPDeviceControllerXPCProxyHandle * proxyRetainerForReports;
@property (strong, atomic, readonly) dispatch_queue_t workQueue;

@property (strong, nonatomic, readonly) NSMutableDictionary<id, NSMutableDictionary *> * reportRegistry;

@end

@implementation CHIPDeviceControllerXPCConnection

- (instancetype)initWithWorkQueue:(dispatch_queue_t)workQueue connectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    if ([super init]) {
        _remoteDeviceServerProtocol = [NSXPCInterface interfaceWithProtocol:@protocol(CHIPDeviceControllerServerProtocol)];
        _remoteDeviceClientProtocol = [NSXPCInterface interfaceWithProtocol:@protocol(CHIPDeviceControllerClientProtocol)];
        _connectBlock = connectBlock;
        _workQueue = workQueue;
        _reportRegistry = [[NSMutableDictionary alloc] init];
    }
    return self;
}

// This class method is for unit testing
+ (instancetype)connectionWithWorkQueue:(dispatch_queue_t)workQueue connectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    return [[CHIPDeviceControllerXPCConnection alloc] initWithWorkQueue:workQueue connectBlock:connectBlock];
}

- (void)getProxyHandleWithCompletion:(void (^)(dispatch_queue_t queue,
                                         CHIPDeviceControllerXPCProxyHandle * _Nullable container))completion
{
    dispatch_async(_workQueue, ^{
        CHIPDeviceControllerXPCProxyHandle * container = self.proxyHandle;
        if (!container) {
            NSXPCConnection * xpcConnection = self.connectBlock();
            if (!xpcConnection) {
                CHIP_LOG_ERROR("Cannot connect to XPC server for remote controller");
                completion(self.workQueue, nil);
            }
            xpcConnection.remoteObjectInterface = self.remoteDeviceServerProtocol;
            xpcConnection.exportedInterface = self.remoteDeviceClientProtocol;
            xpcConnection.exportedObject = self;
            [xpcConnection resume];
            container = [[CHIPDeviceControllerXPCProxyHandle alloc] initWithXPCConnection:xpcConnection];
            self.proxyHandle = container;
            __weak typeof(self) weakSelf = self;
            xpcConnection.invalidationHandler = ^{
                typeof(self) strongSelf = weakSelf;
                if (strongSelf) {
                    dispatch_async(strongSelf.workQueue, ^{
                        strongSelf.proxyHandle = nil;
                        strongSelf.proxyRetainerForReports = nil;
                        [strongSelf.reportRegistry removeAllObjects];
                        CHIP_LOG_DEBUG("CHIP XPC connection disconnected");
                    });
                }
            };
            CHIP_LOG_DEBUG("CHIP XPC connection established");
        }
        completion(self.workQueue, container);
    });
}

- (void)registerReportHandlerWithController:(id<NSCopying>)controller
                                     nodeId:(NSUInteger)nodeId
                                    handler:(void (^)(id _Nullable value, NSError * _Nullable error))handler
{
    dispatch_async(_workQueue, ^{
        BOOL shouldRetainProxyForReport = ([self.reportRegistry count] == 0);
        NSMutableDictionary * controllerDictionary = self.reportRegistry[controller];
        if (!controllerDictionary) {
            controllerDictionary = [[NSMutableDictionary alloc] init];
            [self.reportRegistry setObject:controllerDictionary forKey:controller];
        }
        NSNumber * nodeIdKey = [NSNumber numberWithUnsignedInteger:nodeId];
        NSMutableArray * nodeArray = controllerDictionary[nodeIdKey];
        if (!nodeArray) {
            nodeArray = [[NSMutableArray alloc] init];
            [controllerDictionary setObject:nodeArray forKey:nodeIdKey];
        }
        [nodeArray addObject:handler];
        if (shouldRetainProxyForReport) {
            self.proxyRetainerForReports = self.proxyHandle;
        }
    });
}

- (void)deregisterReportHandlersWithController:(id<NSCopying>)controller
                                        nodeId:(NSUInteger)nodeId
                                    completion:(void (^)(void))completion
{
    dispatch_async(_workQueue, ^{
        NSMutableDictionary * controllerDictionary = self.reportRegistry[controller];
        if (!controllerDictionary) {
            completion();
            return;
        }
        NSNumber * nodeIdKey = [NSNumber numberWithUnsignedInteger:nodeId];
        NSMutableArray * nodeArray = controllerDictionary[nodeIdKey];
        if (!nodeArray) {
            completion();
            return;
        }
        [controllerDictionary removeObjectForKey:nodeIdKey];
        if ([controllerDictionary count] == 0) {
            // Dereference proxy retainer for reports so that XPC connection may be invalidated if no longer used.
            self.proxyRetainerForReports = nil;
        }
        completion();
    });
}

- (void)handleReportWithController:(id)controller
                            nodeId:(NSUInteger)nodeId
                             value:(id _Nullable)value
                             error:(NSError * _Nullable)error
{
    dispatch_async(_workQueue, ^{
        NSMutableDictionary * controllerDictionary = self.reportRegistry[controller];
        if (!controllerDictionary) {
            return;
        }
        NSNumber * nodeIdKey = [NSNumber numberWithUnsignedInteger:nodeId];
        NSMutableArray * nodeArray = controllerDictionary[nodeIdKey];
        if (!nodeArray) {
            return;
        }
        for (void (^handler)(id _Nullable value, NSError * _Nullable error) in nodeArray) {
            handler(value, error);
        }
    });
}

@end
