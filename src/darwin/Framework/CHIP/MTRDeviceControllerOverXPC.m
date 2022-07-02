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

#import "MTRDeviceControllerOverXPC_Internal.h"

#import "MTRDeviceController+XPC.h"
#import "MTRDeviceControllerXPCConnection.h"
#import "MTRDeviceOverXPC.h"
#import "MTRError.h"
#import "MTRLogging.h"

#import <Foundation/Foundation.h>

static dispatch_once_t workQueueInitOnceToken;
static dispatch_queue_t globalWorkQueue;

static void SetupXPCQueue(void)
{
    dispatch_once(&workQueueInitOnceToken, ^{
        globalWorkQueue = dispatch_queue_create("com.apple.matter.framework.xpc.workqueue", DISPATCH_QUEUE_SERIAL);
    });
}

@implementation MTRDeviceControllerOverXPC

+ (MTRDeviceControllerOverXPC *)sharedControllerWithId:(id<NSCopying> _Nullable)controllerId
                                       xpcConnectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    SetupXPCQueue();
    return [[MTRDeviceControllerOverXPC alloc] initWithControllerId:controllerId
                                                          workQueue:globalWorkQueue
                                                       connectBlock:connectBlock];
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support pairDevice over XPC");
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support pairDevice over XPC");
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support pairDevice over XPC");
    return NO;
}

- (BOOL)commissionDevice:(uint64_t)deviceId
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support pairDevice over XPC");
    return NO;
}

- (void)setListenPort:(uint16_t)port
{
    MTR_LOG_ERROR("MTRDevice doesn't support setListenPort over XPC");
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support stopDevicePairing over XPC");
    return NO;
}

- (nullable MTRDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support getDeviceBeingCommissioned over XPC");
    return nil;
}

- (BOOL)deviceBeingCommissionedOverBLE:(uint64_t)deviceId
{
    MTR_LOG_ERROR("MTRDevice doesn't support deviceBeingCommissionedOverBLE over XPC");
    return NO;
}

- (BOOL)getDevice:(uint64_t)deviceID queue:(dispatch_queue_t)queue completionHandler:(MTRDeviceConnectionCallback)completionHandler
{
    dispatch_async(_workQueue, ^{
        dispatch_group_t group = dispatch_group_create();
        if (!self.controllerId) {
            dispatch_group_enter(group);
            [self.xpcConnection getProxyHandleWithCompletion:^(
                dispatch_queue_t _Nonnull queue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
                if (handle) {
                    [handle.proxy getAnyDeviceControllerWithCompletion:^(id _Nullable controller, NSError * _Nullable error) {
                        if (error) {
                            MTR_LOG_ERROR("Failed to fetch any shared remote controller");
                        } else {
                            self.controllerId = controller;
                        }
                        dispatch_group_leave(group);
                        __auto_type handleRetainer = handle;
                        (void) handleRetainer;
                    }];
                } else {
                    MTR_LOG_ERROR("XPC disconnected while retrieving any shared remote controller");
                    dispatch_group_leave(group);
                }
            }];
        }
        dispatch_group_notify(group, queue, ^{
            if (self.controllerId) {
                MTRDeviceOverXPC * device = [[MTRDeviceOverXPC alloc] initWithController:self.controllerId
                                                                                deviceId:deviceID
                                                                           xpcConnection:self.xpcConnection];
                completionHandler(device, nil);
            } else {
                completionHandler(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            }
        });
    });
    return YES;
}

- (BOOL)openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support openPairingWindow over XPC");
    return NO;
}

- (nullable NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                                       duration:(NSUInteger)duration
                                  discriminator:(NSUInteger)discriminator
                                       setupPIN:(NSUInteger)setupPIN
                                          error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support openPairingWindow over XPC");
    return nil;
}

- (instancetype)initWithControllerId:(id)controllerId
                           workQueue:(dispatch_queue_t)queue
                       xpcConnection:(MTRDeviceControllerXPCConnection *)connection
{
    _controllerId = controllerId;
    _workQueue = queue;
    _xpcConnection = connection;
    return self;
}

// This is interface for unit testing
- (instancetype)initWithControllerId:(id)controllerId
                           workQueue:(dispatch_queue_t)queue
                        connectBlock:(NSXPCConnection * (^)(void) )connectBlock
{
    return [self initWithControllerId:controllerId
                            workQueue:queue
                        xpcConnection:[MTRDeviceControllerXPCConnection connectionWithWorkQueue:queue connectBlock:connectBlock]];
}

@end
