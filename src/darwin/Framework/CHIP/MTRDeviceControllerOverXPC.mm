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
#import "MTRLogging_Internal.h"

#import <Foundation/Foundation.h>

static dispatch_once_t workQueueInitOnceToken;
static dispatch_queue_t globalWorkQueue;

static void SetupXPCQueue(void)
{
    dispatch_once(&workQueueInitOnceToken, ^{
        globalWorkQueue
            = dispatch_queue_create("org.csa-iot.matter.framework.xpc.workqueue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    });
}

@implementation MTRDeviceControllerOverXPC

+ (MTRDeviceControllerOverXPC *)sharedControllerWithID:(id<NSCopying> _Nullable)controllerID
                                       xpcConnectBlock:(MTRXPCConnectBlock)xpcConnectBlock
{
    SetupXPCQueue();
    return [[MTRDeviceControllerOverXPC alloc] initWithControllerID:controllerID
                                                          workQueue:globalWorkQueue
                                                       connectBlock:xpcConnectBlock];
}

- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDeviceController doesn't support setupCommissioningSessionWithPayload over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support pairDevice over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
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
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support pairDevice over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (BOOL)commissionDevice:(uint64_t)deviceID
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support commissionDevice over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support stopDevicePairing over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (nullable MTRBaseDevice *)getDeviceBeingCommissioned:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDevice doesn't support getDeviceBeingCommissioned over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return nil;
}

- (BOOL)commissionNodeWithID:(NSNumber *)nodeID
         commissioningParams:(MTRCommissioningParameters *)commissioningParams
                       error:(NSError * __autoreleasing *)error;
{
    MTR_LOG_ERROR("MTRDeviceController doesn't support commissionNodeWithID over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (BOOL)cancelCommissioningForNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDeviceController doesn't support cancelCommissioningForNodeID over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return NO;
}

- (nullable MTRBaseDevice *)deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("MTRDeviceController doesn't support deviceBeingCommissionedWithNodeID over XPC");
    if (error != nil) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeInvalidState userInfo:nil];
    }
    return nil;
}

- (BOOL)getBaseDevice:(uint64_t)deviceID
                queue:(dispatch_queue_t)queue
    completionHandler:(MTRDeviceConnectionCallback)completionHandler
{
    // Consumers expect their getAnyDeviceControllerWithCompletion to be called
    // under here if we don't have a controller id aleady, so make sure we do
    // that.
    [self fetchControllerIdWithQueue:queue
                          completion:^(id _Nullable controllerID, MTRDeviceControllerXPCProxyHandle * _Nullable handle,
                              NSError * _Nullable error) {
                              if (error != nil) {
                                  completionHandler(nil, error);
                                  return;
                              }

                              __auto_type * device = [self baseDeviceForNodeID:@(deviceID)];
                              completionHandler(device, nil);
                          }];
    return YES;
}

- (void)fetchControllerIdWithQueue:(dispatch_queue_t)queue completion:(MTRFetchControllerIDCompletion)completion
{
    // Capture the proxy handle so that it won't be released prior to block call.
    __block MTRDeviceControllerXPCProxyHandle * handleRetainer = nil;

    dispatch_async(_workQueue, ^{
        dispatch_group_t group = dispatch_group_create();
        if (!self.controllerXPCID) {
            dispatch_group_enter(group);
            [self.xpcConnection getProxyHandleWithCompletion:^(
                dispatch_queue_t _Nonnull proxyQueue, MTRDeviceControllerXPCProxyHandle * _Nullable handle) {
                if (handle) {
                    [handle.proxy getAnyDeviceControllerWithCompletion:^(id _Nullable controller, NSError * _Nullable error) {
                        if (error) {
                            MTR_LOG_ERROR("Failed to fetch any shared remote controller");
                        } else {
                            self.controllerXPCID = controller;
                            handleRetainer = handle;
                        }
                        dispatch_group_leave(group);
                    }];
                } else {
                    MTR_LOG_ERROR("XPC disconnected while retrieving any shared remote controller");
                    dispatch_group_leave(group);
                }
            }];
        }
        dispatch_group_notify(group, queue, ^{
            if (self.controllerXPCID) {
                completion(self.controllerXPCID, handleRetainer, nil);
            } else {
                completion(nil, nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
            }
        });
    });
}

- (MTRBaseDevice *)baseDeviceForNodeID:(NSNumber *)nodeID
{
    return [[MTRDeviceOverXPC alloc] initWithControllerOverXPC:self deviceID:nodeID xpcConnection:self.xpcConnection];
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

- (instancetype)initWithControllerID:(id)controllerID
                           workQueue:(dispatch_queue_t)queue
                       xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
{
    _controllerXPCID = controllerID;
    _workQueue = queue;
    _xpcConnection = xpcConnection;
    return self;
}

// This is interface for unit testing
- (instancetype)initWithControllerID:(id)controllerID
                           workQueue:(dispatch_queue_t)queue
                        connectBlock:(MTRXPCConnectBlock)connectBlock
{
    return [self initWithControllerID:controllerID
                            workQueue:queue
                        xpcConnection:[MTRDeviceControllerXPCConnection connectionWithWorkQueue:queue connectBlock:connectBlock]];
}

@end
