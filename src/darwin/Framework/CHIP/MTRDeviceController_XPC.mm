/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <os/lock.h>

#import "MTRDeviceController_XPC.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDevice_XPC.h"
#import "MTRLogging_Internal.h"
#import "MTRXPCClientProtocol.h"
#import "MTRXPCServerProtocol.h"

#define MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_GETTER(NAME, TYPE, DEFAULT_VALUE, GETTER_NAME)                 \
    MTR_SIMPLE_REMOTE_XPC_GETTER(self.xpcConnection, NAME, TYPE, DEFAULT_VALUE, GETTER_NAME, deviceController \
                                 : self.uniqueIdentifier)

#define MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_COMMAND(METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS)                 \
    MTR_SIMPLE_REMOTE_XPC_COMMAND(self.xpcConnection, METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS, deviceController \
                                  : self.uniqueIdentifier)

@interface MTRDeviceController_XPC ()

@property (nonatomic, retain, readwrite) NSUUID * uniqueIdentifier;

@end

// #define MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

@implementation MTRDeviceController_XPC

@synthesize uniqueIdentifier = _uniqueIdentifier;
@synthesize nodeIDToDeviceMap = _nodeIDToDeviceMap;
@synthesize deviceMapLock = _deviceMapLock;

- (id)initWithUniqueIdentifier:(NSUUID *)UUID xpConnectionBlock:(NSXPCConnection * (^)(void) )connectionBlock
{
    if (self = [super initForSubclasses]) {
        MTR_LOG("Setting up XPC Controller for UUID: %@ with connection block: %p", UUID, connectionBlock);

        if (UUID == nil) {
            MTR_LOG_ERROR("MTRDeviceController_XPC initWithUniqueIdentifier failed, nil UUID");
            return nil;
        }
        if (connectionBlock == nil) {
            MTR_LOG_ERROR("MTRDeviceController_XPC initWithUniqueIdentifier failed, nil connectionBlock");
            return nil;
        }

        self.xpcConnection = connectionBlock();
        self.uniqueIdentifier = UUID;

        MTR_LOG("Set up XPC Connection: %@", self.xpcConnection);
        if (self.xpcConnection) {
            self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];

            self.xpcConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
            self.xpcConnection.exportedObject = self;

            MTR_LOG("Resuming new XPC connection");
            [self.xpcConnection resume];
        } else {
            MTR_LOG_ERROR("Failed to set up XPC Connection");
            return nil;
        }
    }

    return self;
}

#ifdef MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR
- (id)initWithUniqueIdentifier:(NSUUID *)UUID machServiceName:(NSString *)machServiceName options:(NSXPCConnectionOptions)options
{
    if (self = [super initForSubclasses]) {
        MTR_LOG("Setting up XPC Controller for UUID: %@  with machServiceName: %s options: %d", UUID, machServiceName, options);
        self.xpcConnection = [[NSXPCConnection alloc] initWithMachServiceName:machServiceName options:options];
        self.uniqueIdentifier = UUID;

        MTR_LOG("Set up XPC Connection: %@", self.xpcConnection);
        if (self.xpcConnection) {
            self.xpcConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];

            self.xpcConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
            self.xpcConnection.exportedObject = self;

            MTR_LOG("%s: resuming new XPC connection");
            [self.xpcConnection resume];
        } else {
            MTR_LOG_ERROR("Failed to set up XPC Connection");
            return nil;
        }
    }

    return self;
}
#endif // MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

- (nullable instancetype)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters
                                      error:(NSError * __autoreleasing *)error
{
    MTR_LOG_ERROR("%s: unimplemented method called", __PRETTY_FUNCTION__);
    return nil;
}

// If prefetchedClusterData is not provided, load attributes individually from controller data store
- (MTRDevice *)_setupDeviceForNodeID:(NSNumber *)nodeID prefetchedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)prefetchedClusterData
{
    MTR_LOG("%s", __PRETTY_FUNCTION__);
    os_unfair_lock_assert_owner(&_deviceMapLock);

    MTRDevice * deviceToReturn = [[MTRDevice_XPC alloc] initWithNodeID:nodeID controller:self];
    // If we're not running, don't add the device to our map.  That would
    // create a cycle that nothing would break.  Just return the device,
    // which will be in exactly the state it would be in if it were created
    // while we were running and then we got shut down.
    if ([self isRunning]) {
        [_nodeIDToDeviceMap setObject:deviceToReturn forKey:nodeID];
    }
    MTR_LOG("%s: returning XPC device for node id %@", __PRETTY_FUNCTION__, nodeID);
    return deviceToReturn;
}

#pragma mark - XPC Action Overrides

MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_GETTER(isRunning, BOOL, NO, getIsRunningWithReply)
MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_GETTER(controllerNodeID, NSNumber *, nil, controllerNodeIDWithReply)

// Not Supported via XPC
// - (oneway void)deviceController:(NSUUID *)controller setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload newNodeID:(NSNumber *)newNodeID withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult *)discoveredDevice payload:(MTRSetupPayload *)payload newNodeID:(NSNumber *)newNodeID withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller commissionNodeWithID:(NSNumber *)nodeID commissioningParams:(MTRCommissioningParameters *)commissioningParams withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller continueCommissioningDevice:(void *)opaqueDeviceHandle ignoreAttestationFailure:(BOOL)ignoreAttestationFailure withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller cancelCommissioningForNodeID:(NSNumber *)nodeID withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (nullable MTRBaseDevice *)deviceController:(NSUUID *)controller deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error;
// - (oneway void)deviceController:(NSUUID *)controller startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate withReply:(void(^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller stopBrowseForCommissionablesWithReply:(void(^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller attestationChallengeForDeviceID:(NSNumber *)deviceID withReply:(void(^)(NSData * _Nullable))reply;

//- (oneway void)deviceController:(NSUUID *)controller addServerEndpoint:(MTRServerEndpoint *)endpoint withReply:(void(^)(BOOL success))reply;
//- (oneway void)deviceController:(NSUUID *)controller removeServerEndpoint:(MTRServerEndpoint *)endpoint;

MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_COMMAND(shutdown, shutdownDeviceController
                                               : self.uniqueIdentifier)

#pragma mark - MTRDeviceProtocol Client

// All pass through, we could do some fancy redirection here based on protocol, but that's that for another day
- (oneway void)device:(NSNumber *)nodeID stateChanged:(MTRDeviceState)state
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received device: %@ stateChanged: %lu   found device: %@", nodeID, (unsigned long) state, device);
    [device device:nodeID stateChanged:state];
}
- (oneway void)device:(NSNumber *)nodeID receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received device: %@ receivedAttributeReport: %@     found device: %@", nodeID, attributeReport, device);

    [device device:nodeID receivedAttributeReport:attributeReport];
}
- (oneway void)device:(NSNumber *)nodeID receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received device: %@ receivedEventReport: %@     found device: %@", nodeID, eventReport, device);

    [device device:nodeID receivedEventReport:eventReport];
}
- (oneway void)deviceBecameActive:(NSNumber *)nodeID
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received deviceBecameActive: %@ found device: %@", nodeID, device);

    [device deviceBecameActive:nodeID];
}
- (oneway void)deviceCachePrimed:(NSNumber *)nodeID
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received deviceCachePrimed: %@ found device: %@", nodeID, device);

    [device deviceCachePrimed:nodeID];
}
- (oneway void)deviceConfigurationChanged:(NSNumber *)nodeID
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received deviceConfigurationChanged: %@ found device: %@", nodeID, device);

    [device deviceConfigurationChanged:nodeID];
}

#pragma mark - MTRDeviceController Protocol Client

// Not Supported via XPC
//- (oneway void)controller:(NSUUID *)controller statusUpdate:(MTRCommissioningStatus)status {
//  }
//- (oneway void)controller:(NSUUID *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error {
//
//}
//- (oneway void)controller:(NSUUID *)controller commissioningComplete:(NSError * _Nullable)error nodeID:(NSNumber * _Nullable)nodeID metrics:(MTRMetrics * _Nullable)metrics {
//
//}
//- (oneway void)controller:(NSUUID *)controller readCommissioningInfo:(MTRProductIdentity *)info {
//
//}

@end
