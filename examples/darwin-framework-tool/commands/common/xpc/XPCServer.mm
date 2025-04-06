/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import "XPCServerProtocols.h"

#import <lib/support/logging/CHIPLogging.h>

#if TARGET_OS_MACCATALYST || TARGET_OS_IOS
#import "HomeKitConnector.h"
#endif

@interface XPCServerImpl : NSObject <MTRXPCServerProtocol_MTRDevice>
@property (nonatomic, strong) id<MTRXPCClientProtocol> clientProxy;
@property (nonatomic, strong) NSArray<MTRDeviceController *> * controllers;
@property (nonatomic, strong) dispatch_queue_t callbackQueue;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)initWithClientProxy:(id<MTRXPCClientProtocol>)proxy controllers:(NSArray<MTRDeviceController *> *)controllers;
@end

@implementation XPCServerImpl
- (instancetype)initWithClientProxy:(id<MTRXPCClientProtocol>)proxy controllers:(NSArray<MTRDeviceController *> *)controllers
{
    if ([super init]) {
        _clientProxy = proxy;
        _controllers = controllers;
        _callbackQueue = dispatch_queue_create("com.chip.xpc.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    }
    return self;
}

- (MTRDevice *)createDevice:(NSUUID *)uniqueIdentifier nodeID:(NSNumber *)nodeID
{
    for (MTRDeviceController * controller in _controllers) {
        if ([controller.uniqueIdentifier isEqual:uniqueIdentifier]) {
            return [MTRDevice deviceWithNodeID:nodeID controller:controller];
        }
    }

    ChipLogError(chipTool, "The controller '%s' could not be found.", [[uniqueIdentifier UUIDString] UTF8String]);
    return nil;
}

// TODO this is declared optional but the framework does not do any check on it, so it just crashes.
- (oneway void)deviceController:(NSUUID *)controllerUUID updateControllerConfiguration:(NSDictionary *)controllerState
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID getStateWithReply:(void (^)(MTRDeviceState state))reply
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    reply(device.state);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID getDeviceCachePrimedWithReply:(void (^)(BOOL primed))reply
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    reply(device.deviceCachePrimed);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID getEstimatedStartTimeWithReply:(void (^)(NSDate * _Nullable estimatedStartTime))reply
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    reply(device.estimatedStartTime);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID getEstimatedSubscriptionLatencyWithReply:(void (^)(NSNumber * _Nullable estimatedSubscriptionLatency))reply
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    reply(device.estimatedSubscriptionLatency);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID readAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID params:(MTRReadParams * _Nullable)params withReply:(void (^)(NSDictionary<NSString *, id> * _Nullable))reply
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    reply([device readAttributeWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID params:params]);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> *))reply
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    reply([device readAttributePaths:attributePaths]);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID writeAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID value:(id)value expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedWriteTimeout:(NSNumber * _Nullable)timeout
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    [device writeAttributeWithEndpointID:endpointID clusterID:clusterID attributeID:attributeID value:value expectedValueInterval:expectedValueInterval timedWriteTimeout:timeout];
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID invokeCommandWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID commandFields:(id)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout completion:(MTRDeviceResponseHandler)completion
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    [device invokeCommandWithEndpointID:endpointID
                              clusterID:clusterID
                              commandID:commandID
                          commandFields:commandFields
                         expectedValues:expectedValues
                  expectedValueInterval:expectedValueInterval
                     timedInvokeTimeout:timeout
                                  queue:_callbackQueue
                             completion:^(NSArray * values, NSError * _Nullable error) {
                                 completion(values, error);
                             }];
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode discriminator:(NSNumber *)discriminator duration:(NSNumber *)duration completion:(MTRDeviceOpenCommissioningWindowHandler)completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(nil, error);
}

- (oneway void)deviceController:(NSUUID *)controllerUUID nodeID:(NSNumber *)nodeID downloadLogOfType:(MTRDiagnosticLogType)type timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controllerUUID nodeID:nodeID];
    [device downloadLogOfType:type timeout:timeout queue:_callbackQueue completion:completion];
}

- (oneway void)downloadLogOfType:(MTRDiagnosticLogType)type nodeID:(NSNumber *)nodeID timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(nil, error);
}

@end

@interface XPCServer : NSObject <XPCServerExternalCertificateParametersProtocol>
@property (nonatomic, strong) NSXPCListener * listener;
@property (nonatomic, strong) NSMutableArray<MTRDeviceController *> * controllers;
@end

@implementation XPCServer
- (instancetype)init
{
    if ((self = [super init])) {
        _controllers = [NSMutableArray new];
    }

    return self;
}

- (void)start
{
    _listener = [NSXPCListener anonymousListener];
    [_listener setDelegate:self];
    [_listener resume];
}

- (void)stop
{
    [_listener suspend];
    _listener = nil;
    _controllers = nil;
}

- (MTRDeviceController *)createController:(MTRDeviceControllerExternalCertificateParameters *)params error:(NSError * __autoreleasing *)error
{
    __auto_type * local = [[MTRDeviceController alloc] initWithParameters:params error:error];
    if (nil == local) {
        return nil;
    }
    [_controllers addObject:local];

    __auto_type connectBlock = ^NSXPCConnection *
    {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:self.listener.endpoint];
    };
    __auto_type * xpcParams = [[MTRXPCDeviceControllerParameters alloc] initWithXPConnectionBlock:connectBlock uniqueIdentifier:local.uniqueIdentifier];
    return [[MTRDeviceController alloc] initWithParameters:xpcParams error:error];
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    newConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];
    newConnection.remoteObjectInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCClientProtocol)];

    auto server = [[XPCServerImpl alloc] initWithClientProxy:[newConnection remoteObjectProxy] controllers:_controllers];
    newConnection.exportedObject = server;

    newConnection.interruptionHandler = ^{
        ChipLogProgress(chipTool, "XPC connection interrupted");
    };

    newConnection.invalidationHandler = ^{
        ChipLogProgress(chipTool, "XPC connection invalidated");
    };

    [newConnection resume];
    return YES;
}
@end

@interface XPCServerWithServiceName : NSObject <XPCServerExternalCertificateParametersWithServiceNameProtocol>
@end

@implementation XPCServerWithServiceName
- (void)start
{
}

- (void)stop
{
#if TARGET_OS_MACCATALYST || TARGET_OS_IOS
    [[HomeKitConnector sharedInstance] stop];
#endif
}

- (MTRDeviceController *)createController:(NSString *)controllerID serviceName:(NSString *)serviceName error:(NSError * __autoreleasing *)error
{
    NSXPCConnection * (^connectBlock)(void) = nil;
#if TARGET_OS_OSX
    connectBlock = ^NSXPCConnection *
    {
        return [[NSXPCConnection alloc] initWithMachServiceName:serviceName options:0];
    };
#elif TARGET_OS_MACCATALYST || TARGET_OS_IOS
    connectBlock = [[HomeKitConnector sharedInstance] connectBlockFor:controllerID];
    controllerID = [[HomeKitConnector sharedInstance] homeControllerIDFor:controllerID];
#endif

    VerifyOrReturnValue(nil != connectBlock, nil, ChipLogError(chipTool, "XPCServerWithServiceName is not supported on this platform."));

    __auto_type * uniqueIdentifier = [[NSUUID alloc] initWithUUIDString:controllerID];
    __auto_type * xpcParams = [[MTRXPCDeviceControllerParameters alloc] initWithXPConnectionBlock:connectBlock uniqueIdentifier:uniqueIdentifier];
    return [[MTRDeviceController alloc] initWithParameters:xpcParams error:error];
}
@end
