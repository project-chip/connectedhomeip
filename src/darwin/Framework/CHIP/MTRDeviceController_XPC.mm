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

#import "MTRDeviceController_XPC.h"

#import "MTRDefines_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRDevice_Internal.h"
#import "MTRDevice_XPC.h"
#import "MTRDevice_XPC_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRUnfairLock.h"
#import "MTRXPCClientProtocol.h"
#import "MTRXPCServerProtocol.h"

#define MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_GETTER(NAME, TYPE, DEFAULT_VALUE, GETTER_NAME)                 \
    MTR_SIMPLE_REMOTE_XPC_GETTER(self.xpcConnection, NAME, TYPE, DEFAULT_VALUE, GETTER_NAME, deviceController \
                                 : self.uniqueIdentifier)

#define MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_COMMAND(METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS)                 \
    MTR_SIMPLE_REMOTE_XPC_COMMAND(self.xpcConnection, METHOD_SIGNATURE, ADDITIONAL_ARGUMENTS, deviceController \
                                  : self.uniqueIdentifier)

@interface MTRDeviceController_XPC ()

@property (nonnull, atomic, readwrite, retain) MTRXPCDeviceControllerParameters * xpcParameters;
@property (atomic, readwrite, assign) NSTimeInterval xpcRetryTimeInterval;
@property (atomic, readwrite, assign) BOOL xpcConnectedOrConnecting;
@property (nonatomic, readonly, retain) dispatch_queue_t workQueue;

@end

NSString * const MTRDeviceControllerRegistrationControllerContextKey = @"MTRDeviceControllerRegistrationControllerContext";
NSString * const MTRDeviceControllerRegistrationNodeIDsKey = @"MTRDeviceControllerRegistrationNodeIDs";
NSString * const MTRDeviceControllerRegistrationNodeIDKey = @"MTRDeviceControllerRegistrationNodeID";
NSString * const MTRDeviceControllerRegistrationControllerNodeIDKey = @"MTRDeviceControllerRegistrationControllerNodeID";
NSString * const MTRDeviceControllerRegistrationControllerIsRunningKey = @"MTRDeviceControllerRegistrationControllerIsRunning";
NSString * const MTRDeviceControllerRegistrationDeviceInternalStateKey = @"MTRDeviceControllerRegistrationDeviceInternalState";
NSString * const MTRDeviceControllerRegistrationControllerCompressedFabricIDKey = @"MTRDeviceControllerRegistrationControllerCompressedFabricID";

// #define MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

@implementation MTRDeviceController_XPC {
    // Protects access to the data set in controllerConfigurationUpdated:
    os_unfair_lock _configurationLock;
}

#pragma mark - Node ID Management

MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_COMMAND(updateControllerConfiguration
                                               : (NSDictionary *) controllerState, updateControllerConfiguration
                                               : (NSDictionary *) controllerState)

MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_COMMAND(deleteNodeID
                                               : (NSNumber *) nodeID, deleteNodeID
                                               : (NSNumber *) nodeID)

MTR_DEVICECONTROLLER_SIMPLE_REMOTE_XPC_GETTER(nodesWithStoredData,
    NSArray<NSNumber *> *,
    @[], // Default return value
    getNodesWithStoredDataWithReply)

- (void)_updateRegistrationInfo
{
    std::lock_guard lock(*self.deviceMapLock);

    NSMutableDictionary * registrationInfo = [NSMutableDictionary dictionary];

    NSMutableDictionary * controllerContext = [NSMutableDictionary dictionary];
    NSMutableArray * nodeIDs = [NSMutableArray array];

    for (NSNumber * nodeID in [self.nodeIDToDeviceMap keyEnumerator]) {
        MTRDevice * device = [self.nodeIDToDeviceMap objectForKey:nodeID];
        if ([device delegateExists]) {
            NSMutableDictionary * nodeDictionary = [NSMutableDictionary dictionary];
            MTR_REQUIRED_ATTRIBUTE(MTRDeviceControllerRegistrationNodeIDKey, nodeID, nodeDictionary)

            [nodeIDs addObject:nodeDictionary];
        }
    }
    MTR_REQUIRED_ATTRIBUTE(MTRDeviceControllerRegistrationNodeIDsKey, nodeIDs, registrationInfo)
    MTR_REQUIRED_ATTRIBUTE(MTRDeviceControllerRegistrationControllerContextKey, controllerContext, registrationInfo)

    [self updateControllerConfiguration:registrationInfo];
}

- (void)_registerNodeID:(NSNumber *)nodeID
{
    [self _updateRegistrationInfo];
}

- (void)_unregisterNodeID:(NSNumber *)nodeID
{
    [self _updateRegistrationInfo];
}

- (void)removeDevice:(MTRDevice *)device
{
    [super removeDevice:device];
    [self _updateRegistrationInfo];
}

- (void)forgetDeviceWithNodeID:(NSNumber *)nodeID
{
    MTR_LOG("%@: Forgetting device with node ID: %@", self, nodeID);
    [self deleteNodeID:nodeID];
    [super forgetDeviceWithNodeID:nodeID];
}

#pragma mark - XPC

@synthesize controllerNodeID = _controllerNodeID;
- (nullable NSNumber *)controllerNodeID
{
    std::lock_guard lock(_configurationLock);
    return _controllerNodeID;
}

@synthesize compressedFabricID = _compressedFabricID;
- (nullable NSNumber *)compressedFabricID
{
    std::lock_guard lock(_configurationLock);
    return _compressedFabricID;
}

+ (NSMutableSet *)_allowedClasses
{
    static NSArray * sBaseAllowedClasses = @[
        [NSString class],
        [NSNumber class],
        [NSData class],
        [NSArray class],
        [NSDictionary class],
        [NSError class],
        [NSDate class],
    ];

    return [NSMutableSet setWithArray:sBaseAllowedClasses];
}

- (NSXPCInterface *)_interfaceForServerProtocol
{
    NSXPCInterface * interface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];

    NSMutableSet * allowedClasses = [MTRDeviceController_XPC _allowedClasses];
    [allowedClasses addObjectsFromArray:@[
        [MTRCommandPath class],
    ]];

    [interface setClasses:allowedClasses
              forSelector:@selector(deviceController:nodeID:invokeCommandWithEndpointID:clusterID:commandID:commandFields:expectedValues:expectedValueInterval:timedInvokeTimeout:serverSideProcessingTimeout:completion:)
            argumentIndex:0
                  ofReply:YES];

    // invokeCommands has the same reply types as invokeCommandWithEndpointID.
    [interface setClasses:allowedClasses
              forSelector:@selector(deviceController:nodeID:invokeCommands:completion:)
            argumentIndex:0
                  ofReply:YES];

    // invokeCommands gets handed MTRCommandWithRequiredResponse (which includes
    // MTRCommandPath, which is already in allowedClasses).
    [allowedClasses addObjectsFromArray:@[
        [MTRCommandWithRequiredResponse class],
    ]];
    [interface setClasses:allowedClasses
              forSelector:@selector(deviceController:nodeID:invokeCommands:completion:)
            argumentIndex:2
                  ofReply:NO];

    // readAttributePaths: gets handed an array of MTRAttributeRequestPath.
    allowedClasses = [MTRDeviceController_XPC _allowedClasses];
    [allowedClasses addObjectsFromArray:@[
        [MTRAttributeRequestPath class],
    ]];
    [interface setClasses:allowedClasses
              forSelector:@selector(deviceController:nodeID:readAttributePaths:withReply:)
            argumentIndex:2
                  ofReply:NO];

    // readAttributePaths: returns response-value dictionaries that have
    // attribute paths and values.
    allowedClasses = [MTRDeviceController_XPC _allowedClasses];
    [allowedClasses addObjectsFromArray:@[
        [MTRAttributePath class],
    ]];
    [interface setClasses:allowedClasses
              forSelector:@selector(deviceController:nodeID:readAttributePaths:withReply:)
            argumentIndex:0
                  ofReply:YES];

    return interface;
}

- (NSXPCInterface *)_interfaceForClientProtocol
{
    NSXPCInterface * interface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];
    NSMutableSet * allowedClasses = [MTRDeviceController_XPC _allowedClasses];
    [allowedClasses addObjectsFromArray:@[
        [MTRAttributePath class],
    ]];

    [interface setClasses:allowedClasses
              forSelector:@selector(device:receivedAttributeReport:)
            argumentIndex:1
                  ofReply:NO];

    allowedClasses = [MTRDeviceController_XPC _allowedClasses];
    [allowedClasses addObjectsFromArray:@[
        [MTREventPath class],
    ]];

    [interface setClasses:allowedClasses
              forSelector:@selector(device:receivedEventReport:)
            argumentIndex:1
                  ofReply:NO];

    allowedClasses = [MTRDeviceController_XPC _allowedClasses];

    [interface setClasses:allowedClasses
              forSelector:@selector(controller:controllerConfigurationUpdated:)
            argumentIndex:1
                  ofReply:NO];

    return interface;
}

- (id<NSCopying>)controllerXPCID
{
    return [self.uniqueIdentifier UUIDString];
}

- (void)_startXPCConnectionRetry
{
    if (!self.xpcConnectedOrConnecting) {
        MTR_LOG("%@: XPC Connection retry - Starting retry for XPC Connection", self);
        self.xpcRetryTimeInterval = 0.5;
        mtr_weakify(self);

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t) (self.xpcRetryTimeInterval * NSEC_PER_SEC)), self.workQueue, ^{
            mtr_strongify(self);
            [self _xpcConnectionRetry];
        });
    } else {
        MTR_LOG("%@: XPC Connection retry - Not starting retry for XPC Connection, already trying", self);
    }
}

- (void)_xpcConnectionRetry
{
    MTR_LOG("%@: XPC Connection retry - timer hit", self);
    if (!self.xpcConnectedOrConnecting) {
        if (![self _setupXPCConnection]) {
#if 0 // FIXME: Not sure why this retry is not working, but I will fix this later
            MTR_LOG("%@: XPC Connection retry - Scheduling another retry", self);
            self.xpcRetryTimeInterval = self.xpcRetryTimeInterval >= 1 ? self.xpcRetryTimeInterval * 2 : 1;
            self.xpcRetryTimeInterval = MIN(60.0, self.xpcRetryTimeInterval);
            mtr_weakify(self);

            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(self.xpcRetryTimeInterval * NSEC_PER_SEC)), self.workQueue, ^{
                mtr_strongify(self);
                [self _xpcConnectionRetry];
            });
#else
            MTR_LOG("%@: XPC Connection failed retry - bailing", self);
#endif
        } else {
            MTR_LOG("%@: XPC Connection retry - connection attempt successful", self);
        }
    } else {
        MTR_LOG("%@: XPC Connection retry - Mid retry, or connected, stopping retry timer", self);
    }
}

- (BOOL)_setupXPCConnection
{
    self.xpcConnection = self.xpcParameters.xpcConnectionBlock();

    MTR_LOG("%@ Set up XPC Connection: %@", self, self.xpcConnection);
    if (self.xpcConnection) {
        mtr_weakify(self);
        self.xpcConnection.remoteObjectInterface = [self _interfaceForServerProtocol];

        self.xpcConnection.exportedInterface = [self _interfaceForClientProtocol];
        self.xpcConnection.exportedObject = self;

        self.xpcConnection.interruptionHandler = ^{
            mtr_strongify(self);
            MTR_LOG_ERROR("XPC Connection for device controller interrupted: %@", self.xpcParameters.uniqueIdentifier);
            self.xpcConnectedOrConnecting = NO;
            self.xpcConnection = nil;
            [self _startXPCConnectionRetry];
        };

        self.xpcConnection.invalidationHandler = ^{
            mtr_strongify(self);
            MTR_LOG_ERROR("XPC Connection for device controller invalidated: %@", self.xpcParameters.uniqueIdentifier);
            self.xpcConnectedOrConnecting = NO;
            self.xpcConnection = nil;
            [self _startXPCConnectionRetry];
        };

        MTR_LOG("%@ Activating new XPC connection", self);
        [self.xpcConnection activate];

        // FIXME: Trying to kick all the MTRDevices attached to this controller to re-establish connections
        //        This state needs to be stored properly and re-established at connnection time

        [self _updateRegistrationInfo];
        self.xpcConnectedOrConnecting = YES;
    } else {
        MTR_LOG_ERROR("%@ Failed to set up XPC Connection", self);
        self.xpcConnectedOrConnecting = NO;
    }

    return (self.xpcConnectedOrConnecting);
}

- (nullable instancetype)initWithParameters:(MTRDeviceControllerAbstractParameters *)parameters
                                      error:(NSError * __autoreleasing *)error
{
    if (![parameters isKindOfClass:MTRXPCDeviceControllerParameters.class]) {
        MTR_LOG_ERROR("Expected MTRXPCDeviceControllerParameters but got: %@", parameters);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    if (self = [super initForSubclasses:parameters.startSuspended]) {
        auto * xpcParameters = static_cast<MTRXPCDeviceControllerParameters *>(parameters);
        auto connectionBlock = xpcParameters.xpcConnectionBlock;
        auto * UUID = xpcParameters.uniqueIdentifier;

        MTR_LOG("Setting up XPC Controller for UUID: %@ with connection block: %p", UUID, connectionBlock);

        if (UUID == nil) {
            MTR_LOG_ERROR("MTRDeviceController_XPC initWithParameters failed, nil UUID");
            return nil;
        }
        if (connectionBlock == nil) {
            MTR_LOG_ERROR("MTRDeviceController_XPC initWithParameters failed, nil connectionBlock");
            return nil;
        }

        self.uniqueIdentifier = UUID;
        self.xpcParameters = xpcParameters;
        _workQueue = dispatch_queue_create("MTRDeviceController_XPC_queue", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
        _configurationLock = OS_UNFAIR_LOCK_INIT;

        if (![self _setupXPCConnection]) {
            return nil;
        }
    }

    return self;
}

#ifdef MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR
- (id)initWithUniqueIdentifier:(NSUUID *)UUID machServiceName:(NSString *)machServiceName options:(NSXPCConnectionOptions)options
{
    // TODO: Presumably this should end up doing some sort of
    // MTRDeviceControllerAbstractParameters thing eventually?
    if (self = [super initForSubclasses:NO]) {
        MTR_LOG("Setting up XPC Controller for UUID: %@  with machServiceName: %s options: %d", UUID, machServiceName, options);
        self.xpcConnection = [[NSXPCConnection alloc] initWithMachServiceName:machServiceName options:options];
        self.uniqueIdentifier = UUID;

        MTR_LOG("Set up XPC Connection: %@", self.xpcConnection);
        if (self.xpcConnection) {
            self.xpcConnection.remoteObjectInterface = [self _interfaceForServerProtocol];

            self.xpcConnection.exportedInterface = [self _interfaceForClientProtocol];
            self.xpcConnection.exportedObject = self;

            MTR_LOG("%s: resuming new XPC connection");
            [self.xpcConnection activate];
        } else {
            MTR_LOG_ERROR("Failed to set up XPC Connection");
            return nil;
        }
    }

    return self;
}
#endif // MTR_HAVE_MACH_SERVICE_NAME_CONSTRUCTOR

// If prefetchedClusterData is not provided, load attributes individually from controller data store
- (MTRDevice *)_setupDeviceForNodeID:(NSNumber *)nodeID prefetchedClusterData:(NSDictionary<MTRClusterPath *, MTRDeviceClusterData *> *)prefetchedClusterData
{
    MTR_LOG("%s", __PRETTY_FUNCTION__);
    os_unfair_lock_assert_owner(self.deviceMapLock);

    MTRDevice * deviceToReturn = [[MTRDevice_XPC alloc] initWithNodeID:nodeID controller:self];
    [self.nodeIDToDeviceMap setObject:deviceToReturn forKey:nodeID];
    MTR_LOG("%s: returning XPC device for node id %@", __PRETTY_FUNCTION__, nodeID);

    return deviceToReturn;
}

#pragma mark - XPC Action Overrides

// Not Supported via XPC
// - (oneway void)deviceController:(NSUUID *)controller setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload newNodeID:(NSNumber *)newNodeID withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult *)discoveredDevice payload:(MTRSetupPayload *)payload newNodeID:(NSNumber *)newNodeID withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller commissionNodeWithID:(NSNumber *)nodeID commissioningParams:(MTRCommissioningParameters *)commissioningParams withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller continueCommissioningDevice:(void *)opaqueDeviceHandle ignoreAttestationFailure:(BOOL)ignoreAttestationFailure withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller cancelCommissioningForNodeID:(NSNumber *)nodeID withReply:(void(^)(BOOL success, NSError * _Nullable error))reply;
// - (nullable MTRBaseDevice *)deviceController:(NSUUID *)controller deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error;
// - (oneway void)deviceController:(NSUUID *)controller startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate withReply:(void(^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller stopBrowseForCommissionablesWithReply:(void(^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller preWarmCommissioningSession;
// - (oneway void)deviceController:(NSUUID *)controller attestationChallengeForDeviceID:(NSNumber *)deviceID withReply:(void(^)(NSData * _Nullable))reply;

//- (oneway void)deviceController:(NSUUID *)controller addServerEndpoint:(MTRServerEndpoint *)endpoint withReply:(void(^)(BOOL success))reply;
//- (oneway void)deviceController:(NSUUID *)controller removeServerEndpoint:(MTRServerEndpoint *)endpoint;

//- (oneway void)deviceController:(NSUUID *)controller setOperationalCertificateIssuer:(nullable id<MTROperationalCertificateIssuer>)operationalCertificateIssuer queue:(nullable dispatch_queue_t)queue withReply:(void(^)(BOOL success))reply;

//- (oneway void)deviceController:(NSUUID *)controller openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration withReply:(void(^)(NSError * _Nullable error))reply;
//- (oneway void)deviceController:(NSUUID *)controller openPairingWindowWithPIN:(uint64_t)deviceID duration:(NSUInteger)duration discriminator:(NSUInteger)discriminator setupPIN:(NSUInteger)setupPIN  withReply:(void(^)(NSError * _Nullable error))reply;

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

- (oneway void)device:(NSNumber *)nodeID internalStateUpdated:(NSDictionary *)dictionary
{
    MTRDevice_XPC * device = (MTRDevice_XPC *) [self deviceForNodeID:nodeID];
    MTR_LOG("Received internalStateUpdated: %@ found device: %@", nodeID, device);

    [device device:nodeID internalStateUpdated:dictionary];
}

#pragma mark - MTRDeviceController Protocol Client

- (oneway void)controller:(NSUUID *)controller controllerConfigurationUpdated:(NSDictionary *)configuration
{
    // Reuse the same format as config dictionary, and add values for internal states
    //  @{
    //     MTRDeviceControllerRegistrationControllerContextKey: @{
    //         MTRDeviceControllerRegistrationControllerNodeIDKey: controllerNodeID
    //     }
    //     MTRDeviceControllerRegistrationNodeIDsKey: @[
    //        @{
    //            MTRDeviceControllerRegistrationNodeIDKey: nodeID,
    //            MTRDeviceControllerRegistrationDeviceInternalStateKey: deviceInternalStateDictionary
    //        }
    //     ]
    //  }

    NSDictionary * controllerContext = MTR_SAFE_CAST(configuration[MTRDeviceControllerRegistrationControllerContextKey], NSDictionary);
    if (controllerContext) {
        std::lock_guard lock(_configurationLock);

        NSNumber * controllerNodeID = MTR_SAFE_CAST(controllerContext[MTRDeviceControllerRegistrationControllerNodeIDKey], NSNumber);
        if (controllerNodeID) {
            _controllerNodeID = controllerNodeID;
        }

        NSNumber * compressedFabricID = MTR_SAFE_CAST(controllerContext[MTRDeviceControllerRegistrationControllerCompressedFabricIDKey], NSNumber);
        if (compressedFabricID) {
            _compressedFabricID = compressedFabricID;
        }
    }

    NSArray * deviceInfoList = MTR_SAFE_CAST(configuration[MTRDeviceControllerRegistrationNodeIDsKey], NSArray);

    MTR_LOG("Received controllerConfigurationUpdated: controllerNodeID %@ compressedFabricID %016lluX deviceInfoList %@", self.controllerNodeID, self.compressedFabricID.unsignedLongLongValue, deviceInfoList);

    for (NSDictionary * deviceInfo in deviceInfoList) {
        if (!MTR_SAFE_CAST(deviceInfo, NSDictionary)) {
            MTR_LOG_ERROR(" - Missing or malformed device Info");
            continue;
        }

        NSNumber * nodeID = MTR_SAFE_CAST(deviceInfo[MTRDeviceControllerRegistrationNodeIDKey], NSNumber);
        if (!nodeID) {
            MTR_LOG_ERROR(" - Missing or malformed nodeID");
            continue;
        }

        NSDictionary * deviceInternalState = MTR_SAFE_CAST(deviceInfo[MTRDeviceControllerRegistrationDeviceInternalStateKey], NSDictionary);
        if (!deviceInternalState) {
            MTR_LOG_ERROR(" - Missing or malformed deviceInternalState");
            continue;
        }

        auto * device = static_cast<MTRDevice_XPC *>([self _deviceForNodeID:nodeID createIfNeeded:NO]);
        [device device:nodeID internalStateUpdated:deviceInternalState];
    }
}

- (BOOL)isRunning
{
    // For XPC controller, always return yes
    return YES;
}

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
