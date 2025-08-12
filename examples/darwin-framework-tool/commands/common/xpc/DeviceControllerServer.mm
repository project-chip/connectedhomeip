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

@interface DeviceControllerXPCServerImpl : NSObject <MTRDeviceControllerServerProtocol>
@property (nonatomic, strong) id<MTRDeviceControllerClientProtocol> clientProxy;
@property (nonatomic, strong) NSArray<MTRDeviceController *> * controllers;
@property (nonatomic, strong) dispatch_queue_t callbackQueue;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)initWithClientProxy:(id<MTRXPCClientProtocol>)proxy controllers:(NSArray<MTRDeviceController *> *)controllers;
@end

@implementation DeviceControllerXPCServerImpl
- (instancetype)initWithClientProxy:(id<MTRDeviceControllerClientProtocol>)proxy controllers:(NSArray<MTRDeviceController *> *)controllers
{
    if ([super init]) {
        _clientProxy = proxy;
        _controllers = controllers;
        _callbackQueue = dispatch_queue_create("com.chip.xpc.command", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    }
    return self;
}

- (MTRBaseDevice *)createDevice:(id)uniqueIdentifierString nodeID:(uint64_t)nodeID
{
    if (![uniqueIdentifierString isKindOfClass:[NSString class]]) {
        ChipLogError(chipTool, "The controller identifier should be a NSString.");
        return nil;
    }

    __auto_type * uniqueIdentifier = [[NSUUID alloc] initWithUUIDString:uniqueIdentifierString];
    for (MTRDeviceController * controller in _controllers) {
        if ([controller.uniqueIdentifier isEqual:uniqueIdentifier]) {
            return [MTRBaseDevice deviceWithNodeID:@(nodeID) controller:controller];
        }
    }

    ChipLogError(chipTool, "The controller '%s' could not be found.", [uniqueIdentifierString UTF8String]);
    return nil;
}

- (void)getAnyDeviceControllerWithCompletion:(MTRDeviceControllerGetterHandler)completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(nil, error);
}

- (void)readAttributeWithController:(id _Nullable)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(NSDictionary<NSString *, id> * _Nullable)params
                         completion:(MTRValuesHandler)completion
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controller nodeID:nodeId];
    [device readAttributesWithEndpointID:endpointId
                               clusterID:clusterId
                             attributeID:attributeId
                                  params:[MTRDeviceController decodeXPCReadParams:params]
                                   queue:_callbackQueue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  completion([MTRDeviceController encodeXPCResponseValues:values], error);
                              }];
}

- (void)writeAttributeWithController:(id _Nullable)controller
                              nodeId:(uint64_t)nodeId
                          endpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                          completion:(MTRValuesHandler)completion
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controller nodeID:nodeId];
    [device
        writeAttributeWithEndpointID:endpointId
                           clusterID:clusterId
                         attributeID:attributeId
                               value:value
                   timedWriteTimeout:timeoutMs
                               queue:_callbackQueue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              completion([MTRDeviceController encodeXPCResponseValues:values], error);
                          }];
}

- (void)invokeCommandWithController:(id _Nullable)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                             fields:(id)fields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                         completion:(MTRValuesHandler)completion
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controller nodeID:nodeId];
    [device
        invokeCommandWithEndpointID:endpointId
                          clusterID:clusterId
                          commandID:commandId
                      commandFields:fields
                 timedInvokeTimeout:timeoutMs
                              queue:_callbackQueue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             completion([MTRDeviceController encodeXPCResponseValues:values], error);
                         }];
}

- (void)subscribeAttributeWithController:(id _Nullable)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(NSDictionary<NSString *, id> * _Nullable)params
                      establishedHandler:(dispatch_block_t)establishedHandler
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controller nodeID:nodeId];
    if (nil == device) {
        establishedHandler();
        // Send an error report so that the client knows of the failure
        [self.clientProxy handleReportWithController:controller
                                              nodeId:nodeId
                                              values:nil
                                               error:[NSError errorWithDomain:MTRErrorDomain
                                                                         code:MTRErrorCodeGeneralError
                                                                     userInfo:nil]];
        return;
    }

    auto subscriptionParams = [MTRDeviceController decodeXPCSubscribeParams:params];
    if (subscriptionParams == nil) {
        subscriptionParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
    } else {
        subscriptionParams.minInterval = minInterval;
        subscriptionParams.maxInterval = maxInterval;
    }

    [device subscribeToAttributesWithEndpointID:endpointId
                                      clusterID:clusterId
                                    attributeID:attributeId
                                         params:subscriptionParams
                                          queue:_callbackQueue
                                  reportHandler:^(
                                      NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                      [self.clientProxy
                                          handleReportWithController:controller
                                                              nodeId:nodeId
                                                              values:[MTRDeviceController encodeXPCResponseValues:values]
                                                               error:error];
                                  }
                        subscriptionEstablished:establishedHandler];
}

- (void)stopReportsWithController:(id _Nullable)controller nodeId:(uint64_t)nodeId completion:(dispatch_block_t)completion
{
    ChipLogProgress(chipTool, "XPC: %s", __func__);

    __auto_type * device = [self createDevice:controller nodeID:nodeId];
    VerifyOrReturn(nil != device, completion());

    [device deregisterReportHandlersWithQueue:_callbackQueue completion:completion];
}

- (void)subscribeWithController:(id _Nullable)controller
                         nodeId:(uint64_t)nodeId
                    minInterval:(NSNumber *)minInterval
                    maxInterval:(NSNumber *)maxInterval
                         params:(NSDictionary<NSString *, id> * _Nullable)params
                    shouldCache:(BOOL)shouldCache
                     completion:(MTRStatusCompletion)completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(error);
}

- (void)readAttributeCacheWithController:(id _Nullable)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                              completion:(MTRValuesHandler)completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(nil, error);
}

- (void)getDeviceControllerWithFabricId:(uint64_t)fabricId
                             completion:(MTRDeviceControllerGetterHandler)completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(nil, error);
}

- (void)downloadLogWithController:(id _Nullable)controller
                           nodeId:(NSNumber *)nodeId
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                       completion:(void (^)(NSString * _Nullable url, NSError * _Nullable error))completion
{
    ChipLogError(chipTool, "XPC (NotImplemented): %s", __func__);

    __auto_type * error = [NSError errorWithDomain:MTRErrorDomain
                                              code:MTRErrorCodeGeneralError
                                          userInfo:nil];
    completion(nil, error);
}

@end

@interface DeviceControllerXPCServer : NSObject <XPCServerStartupParametersProtocol>
@property (nonatomic, strong) NSXPCListener * listener;
@property (nonatomic, strong) NSMutableArray<MTRDeviceController *> * controllers;
@end

@implementation DeviceControllerXPCServer
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

- (MTRDeviceController *)createController:(MTRDeviceControllerStartupParams *)params error:(NSError * __autoreleasing *)error
{
    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    __auto_type * local = [factory createControllerOnExistingFabric:params error:error];
    if (nil == local) {
        return nil;
    }
    [_controllers addObject:local];

    __auto_type connectBlock = ^NSXPCConnection *
    {
        return [[NSXPCConnection alloc] initWithListenerEndpoint:self.listener.endpoint];
    };
    return [MTRDeviceController sharedControllerWithID:[local.uniqueIdentifier UUIDString] xpcConnectBlock:connectBlock];
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    newConnection.exportedInterface = [MTRDeviceController xpcInterfaceForServerProtocol];
    newConnection.remoteObjectInterface = [MTRDeviceController xpcInterfaceForClientProtocol];

    auto server = [[DeviceControllerXPCServerImpl alloc] initWithClientProxy:[newConnection remoteObjectProxy] controllers:_controllers];
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

@interface DeviceControllerXPCServerWithServiceName : NSObject <XPCServerStartupParametersWithServiceNameProtocol>
@end

@implementation DeviceControllerXPCServerWithServiceName
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

    VerifyOrReturnValue(nil != connectBlock, nil, ChipLogError(chipTool, "DeviceControllerXPCServerWithServiceName is not supported on this platform."));
    return [MTRDeviceController sharedControllerWithID:controllerID xpcConnectBlock:connectBlock];
}
@end
