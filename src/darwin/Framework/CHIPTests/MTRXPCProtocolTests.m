//
//  MTRRemoteDeviceTests.m
//  MTRTests
/*
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

// module headers
#import <Matter/Matter.h>

#import "MTRErrorTestUtils.h"

#import <app/util/af-enums.h>

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

static const uint16_t kTimeoutInSeconds = 3;
// Inverted expectation timeout
static const uint16_t kNegativeTimeoutInSeconds = 1;

@interface MTRAttributePath (Test)
- (BOOL)isEqual:(id)object;
@end

@implementation MTRAttributePath (Test)
- (BOOL)isEqual:(id)object
{
    if ([object isKindOfClass:[MTRAttributePath class]]) {
        MTRAttributePath * other = object;
        return [self.endpoint isEqualToNumber:other.endpoint] && [self.cluster isEqualToNumber:other.cluster] &&
            [self.attribute isEqualToNumber:other.attribute];
    }
    return NO;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MTRAttributePath(%@,%@,%@)", self.endpoint, self.cluster, self.attribute];
}
@end

@interface MTRCommandPath (Test)
- (BOOL)isEqual:(id)object;
@end

@implementation MTRCommandPath (Test)
- (BOOL)isEqual:(id)object
{
    if ([object isKindOfClass:[MTRCommandPath class]]) {
        MTRCommandPath * other = object;
        return [self.endpoint isEqualToNumber:other.endpoint] && [self.cluster isEqualToNumber:other.cluster] &&
            [self.command isEqualToNumber:other.command];
    }
    return NO;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MTRCommandPath(%@,%@,%@)", self.endpoint, self.cluster, self.command];
}
@end

@interface MTRAttributeCacheContainer (Test)
// Obsolete method is moved to this test suite to keep tests compatible
- (void)subscribeWithDeviceController:(MTRDeviceController *)deviceController
                             deviceID:(NSNumber *)deviceID
                               params:(MTRSubscribeParams * _Nullable)params
                                queue:(dispatch_queue_t)queue
                           completion:(void (^)(NSError * _Nullable error))completion;
@end

@implementation MTRAttributeCacheContainer (Test)
- (void)subscribeWithDeviceController:(MTRDeviceController *)deviceController
                             deviceID:(NSNumber *)deviceID
                               params:(MTRSubscribeParams * _Nullable)params
                                queue:queue
                           completion:(void (^)(NSError * _Nullable error))completion
{
    __auto_type workQueue = dispatch_get_main_queue();
    __auto_type completionHandler = ^(NSError * _Nullable error) {
        dispatch_async(queue, ^{
            completion(error);
        });
    };
    [deviceController
        getBaseDevice:deviceID.unsignedLongLongValue
                queue:workQueue
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               if (error) {
                   NSLog(@"Error: Failed to get connected device (%llu) for attribute cache: %@", deviceID.unsignedLongLongValue,
                       error);
                   completionHandler(error);
                   return;
               }
               __auto_type established = [NSMutableArray arrayWithCapacity:1];
               [established addObject:@NO];
               [device subscribeWithQueue:queue
                   minInterval:@(1)
                   maxInterval:@(43200)
                   params:params
                   attributeCacheContainer:self
                   attributeReportHandler:^(NSArray * value) {
                       NSLog(@"Report received for attribute cache: %@", value);
                   }
                   eventReportHandler:nil
                   errorHandler:^(NSError * error) {
                       NSLog(@"Report error received for attribute cache: %@", error);
                       if (![established[0] boolValue]) {
                           established[0] = @YES;
                           completionHandler(error);
                       }
                   }
                   subscriptionEstablished:^() {
                       NSLog(@"Attribute cache subscription succeeded for device %llu", deviceID.unsignedLongLongValue);
                       if (![established[0] boolValue]) {
                           established[0] = @YES;
                           completionHandler(nil);
                       }
                   }
                   resubscriptionScheduled:nil];
           }];
}
@end

@interface MTRXPCProtocolTests<NSXPCListenerDelegate, CHIPRemoteDeviceProtocol> : XCTestCase

@property (nonatomic, readwrite, strong) NSXPCListener * xpcListener;
@property (nonatomic, readwrite, strong) NSXPCInterface * serviceInterface;
@property (nonatomic, readwrite, strong) NSXPCInterface * clientInterface;
@property (readwrite, strong) NSXPCConnection * xpcConnection;
@property (nonatomic, readwrite, strong) MTRDeviceController * remoteDeviceController;
@property (nonatomic, readwrite, strong) NSString * controllerUUID;
@property (readwrite, strong) XCTestExpectation * xpcDisconnectExpectation;

@property (readwrite, strong) void (^handleGetAnySharedRemoteControllerWithFabricId)
    (NSNumber * fabricId, void (^completion)(id _Nullable controller, NSError * _Nullable error));
@property (readwrite, strong) void (^handleGetAnySharedRemoteController)
    (void (^completion)(id _Nullable controller, NSError * _Nullable error));
@property (readwrite, strong) void (^handleReadAttribute)(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId,
    NSNumber * _Nullable clusterId, NSNumber * _Nullable attributeId, MTRReadParams * _Nullable params,
    void (^completion)(id _Nullable values, NSError * _Nullable error));
@property (readwrite, strong) void (^handleWriteAttribute)
    (id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId, id value,
        NSNumber * _Nullable timedWriteTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error));
@property (readwrite, strong) void (^handleInvokeCommand)
    (id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, id fields,
        NSNumber * _Nullable timedInvokeTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error));
@property (readwrite, strong) void (^handleSubscribeAttribute)(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId,
    NSNumber * _Nullable clusterId, NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval,
    MTRSubscribeParams * _Nullable params, void (^establishedHandler)(void));
@property (readwrite, strong) void (^handleStopReports)(id controller, NSNumber * nodeId, void (^completion)(void));
@property (readwrite, strong) void (^handleSubscribeAll)(id controller, NSNumber * nodeId, NSNumber * minInterval,
    NSNumber * maxInterval, MTRSubscribeParams * _Nullable params, BOOL shouldCache, void (^completion)(NSError * _Nullable error));
@property (readwrite, strong) void (^handleReadAttributeCache)
    (id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, void (^completion)(id _Nullable values, NSError * _Nullable error));

@end

@implementation MTRXPCProtocolTests

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    XCTAssertNil(_xpcConnection);
    XCTAssertNotNil(newConnection);
    NSLog(@"XPC listener accepting connection");
    newConnection.exportedInterface = _serviceInterface;
    newConnection.remoteObjectInterface = _clientInterface;
    newConnection.exportedObject = self;
    newConnection.invalidationHandler = ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            NSLog(@"XPC connection disconnected");
            self.xpcConnection = nil;
            if (self.xpcDisconnectExpectation) {
                [self.xpcDisconnectExpectation fulfill];
                self.xpcDisconnectExpectation = nil;
            }
        });
    };
    dispatch_async(dispatch_get_main_queue(), ^{
        self.xpcConnection = newConnection;
        [newConnection resume];
    });
    return YES;
}

- (void)getDeviceControllerWithFabricId:(uint64_t)fabricId
                             completion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleGetAnySharedRemoteControllerWithFabricId);
        self.handleGetAnySharedRemoteControllerWithFabricId(@(fabricId), completion);
    });
}

- (void)getAnyDeviceControllerWithCompletion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleGetAnySharedRemoteController);
        self.handleGetAnySharedRemoteController(completion);
    });
}

- (void)readAttributeWithController:(id)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(NSDictionary<NSString *, id> * _Nullable)params
                         completion:(MTRValuesHandler)completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleReadAttribute);
        self.handleReadAttribute(controller, @(nodeId), endpointId, clusterId, attributeId,
            [MTRDeviceController decodeXPCReadParams:params], completion);
    });
}

- (void)writeAttributeWithController:(id)controller
                              nodeId:(uint64_t)nodeId
                          endpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber * _Nullable)timeoutMs
                          completion:(MTRValuesHandler)completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleWriteAttribute);
        self.handleWriteAttribute(controller, @(nodeId), endpointId, clusterId, attributeId, value, timeoutMs, completion);
    });
}

- (void)invokeCommandWithController:(id)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                             fields:(id)fields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                         completion:(MTRValuesHandler)completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleInvokeCommand);
        self.handleInvokeCommand(controller, @(nodeId), endpointId, clusterId, commandId, fields, timeoutMs, completion);
    });
}

- (void)subscribeAttributeWithController:(id)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(NSDictionary<NSString *, id> * _Nullable)params
                      establishedHandler:(dispatch_block_t)establishedHandler
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleSubscribeAttribute);
        self.handleSubscribeAttribute(controller, @(nodeId), endpointId, clusterId, attributeId, minInterval, maxInterval,
            [MTRDeviceController decodeXPCSubscribeParams:params], establishedHandler);
    });
}

- (void)stopReportsWithController:(id)controller nodeId:(uint64_t)nodeId completion:(dispatch_block_t)completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleStopReports);
        self.handleStopReports(controller, @(nodeId), completion);
    });
}

- (void)subscribeWithController:(id _Nullable)controller
                         nodeId:(uint64_t)nodeId
                    minInterval:(NSNumber *)minInterval
                    maxInterval:(NSNumber *)maxInterval
                         params:(NSDictionary<NSString *, id> * _Nullable)params
                    shouldCache:(BOOL)shouldCache
                     completion:(MTRStatusCompletion)completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleSubscribeAll);
        self.handleSubscribeAll(controller, @(nodeId), minInterval, maxInterval,
            [MTRDeviceController decodeXPCSubscribeParams:params], shouldCache, completion);
    });
}

- (void)readAttributeCacheWithController:(id _Nullable)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                              completion:(MTRValuesHandler)completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleReadAttributeCache);
        self.handleReadAttributeCache(controller, @(nodeId), endpointId, clusterId, attributeId, completion);
    });
}

- (void)setUp
{
    [self setContinueAfterFailure:NO];

    _xpcListener = [NSXPCListener anonymousListener];
    [_xpcListener setDelegate:(id<NSXPCListenerDelegate>) self];
    _serviceInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRDeviceControllerServerProtocol)];
    _clientInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRDeviceControllerClientProtocol)];
    [_xpcListener resume];
    _controllerUUID = [[NSUUID UUID] UUIDString];
    _remoteDeviceController =
        [MTRDeviceController sharedControllerWithID:_controllerUUID
                                    xpcConnectBlock:^NSXPCConnection * {
                                        return [[NSXPCConnection alloc] initWithListenerEndpoint:self.xpcListener.endpoint];
                                    }];
}

- (void)tearDown
{
    _remoteDeviceController = nil;
    [_xpcListener suspend];
    _xpcListener = nil;
    _xpcDisconnectExpectation = nil;
}

- (void)testReadAttributeSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSArray * myValues = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];

    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleReadAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, MTRReadParams * _Nullable params,
        void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        completion([MTRDeviceController encodeXPCResponseValues:myValues], nil);
    };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Reading...");
                                    [device readAttributesWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                  params:nil
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Read value: %@", value);
                                                                  XCTAssertNotNil(value);
                                                                  XCTAssertNil(error);
                                                                  XCTAssertTrue([myValues isEqual:value]);
                                                                  [responseExpectation fulfill];
                                                                  self.xpcDisconnectExpectation =
                                                                      [self expectationWithDescription:@"XPC Disconnected"];
                                                              }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When read is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReadAttributeWithParamsSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSArray * myValues = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    MTRReadParams * myParams = [[MTRReadParams alloc] init];
    myParams.fabricFiltered = @NO;

    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleReadAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, MTRReadParams * _Nullable params,
        void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertNotNil(params);
        XCTAssertEqual([params.fabricFiltered boolValue], [myParams.fabricFiltered boolValue]);
        [callExpectation fulfill];
        completion([MTRDeviceController encodeXPCResponseValues:myValues], nil);
    };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Reading...");
                                    [device readAttributesWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                  params:myParams
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Read value: %@", value);
                                                                  XCTAssertNotNil(value);
                                                                  XCTAssertNil(error);
                                                                  XCTAssertTrue([myValues isEqual:value]);
                                                                  [responseExpectation fulfill];
                                                                  self.xpcDisconnectExpectation =
                                                                      [self expectationWithDescription:@"XPC Disconnected"];
                                                              }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When read is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReadAttributeFailure
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSError * myError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleReadAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, MTRReadParams * _Nullable params,
        void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        completion(nil, myError);
    };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Reading...");
                                    [device readAttributesWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                  params:nil
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Read value: %@", value);
                                                                  XCTAssertNil(value);
                                                                  XCTAssertNotNil(error);
                                                                  [responseExpectation fulfill];
                                                                  self.xpcDisconnectExpectation =
                                                                      [self expectationWithDescription:@"XPC Disconnected"];
                                                              }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When read is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testWriteAttributeSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSDictionary * myValue =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:654321], @"value", nil];
    NSArray * myResults = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId]
    } ];

    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleWriteAttribute = ^(id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId,
        id value, NSNumber * _Nullable timedWriteTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertTrue([value isEqual:myValue]);
        XCTAssertNil(timedWriteTimeout);
        [callExpectation fulfill];
        completion([MTRDeviceController encodeXPCResponseValues:myResults], nil);
    };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Writing...");
                                    [device writeAttributeWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                   value:myValue
                                                       timedWriteTimeout:nil
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Write response: %@", value);
                                                                  XCTAssertNotNil(value);
                                                                  XCTAssertNil(error);
                                                                  XCTAssertTrue([myResults isEqual:value]);
                                                                  [responseExpectation fulfill];
                                                                  self.xpcDisconnectExpectation =
                                                                      [self expectationWithDescription:@"XPC Disconnected"];
                                                              }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When write is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testTimedWriteAttributeSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSDictionary * myValue =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:654321], @"value", nil];
    NSNumber * myTimedWriteTimeout = @1234;
    NSArray * myResults = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId]
    } ];

    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleWriteAttribute = ^(id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId,
        id value, NSNumber * _Nullable timedWriteTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertTrue([value isEqual:myValue]);
        XCTAssertNotNil(timedWriteTimeout);
        XCTAssertEqual([timedWriteTimeout unsignedShortValue], [myTimedWriteTimeout unsignedShortValue]);
        [callExpectation fulfill];
        completion([MTRDeviceController encodeXPCResponseValues:myResults], nil);
    };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Writing...");
                                    [device writeAttributeWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                   value:myValue
                                                       timedWriteTimeout:myTimedWriteTimeout
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Write response: %@", value);
                                                                  XCTAssertNotNil(value);
                                                                  XCTAssertNil(error);
                                                                  XCTAssertTrue([myResults isEqual:value]);
                                                                  [responseExpectation fulfill];
                                                                  self.xpcDisconnectExpectation =
                                                                      [self expectationWithDescription:@"XPC Disconnected"];
                                                              }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When write is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testWriteAttributeFailure
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSDictionary * myValue =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:654321], @"value", nil];
    NSError * myError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleWriteAttribute = ^(id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * attributeId,
        id value, NSNumber * _Nullable timedWriteTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertTrue([value isEqual:myValue]);
        XCTAssertNil(timedWriteTimeout);
        [callExpectation fulfill];
        completion(nil, myError);
    };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Writing...");
                                    [device writeAttributeWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                   value:myValue
                                                       timedWriteTimeout:nil
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Write response: %@", value);
                                                                  XCTAssertNil(value);
                                                                  XCTAssertNotNil(error);
                                                                  [responseExpectation fulfill];
                                                                  self.xpcDisconnectExpectation =
                                                                      [self expectationWithDescription:@"XPC Disconnected"];
                                                              }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When write is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testInvokeCommandSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myCommandId = @300;
    NSDictionary * myFields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type",
                                            [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"Type",
                                                                                   [NSNumber numberWithFloat:1.0], @"value", nil]],
                                            @"value", nil];
    NSArray * myResults = @[
        @{ @"commandPath" : [MTRCommandPath commandPathWithEndpointID:myEndpointId clusterID:myClusterId commandID:myCommandId] }
    ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleInvokeCommand
        = ^(id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, id commandFields,
            NSNumber * _Nullable timedInvokeTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
              XCTAssertTrue([controller isEqualToString:uuid]);
              XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
              XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
              XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
              XCTAssertEqual([commandId unsignedLongValue], [myCommandId unsignedLongValue]);
              XCTAssertTrue([commandFields isEqual:myFields]);
              XCTAssertNil(timedInvokeTimeout);
              [callExpectation fulfill];
              completion([MTRDeviceController encodeXPCResponseValues:myResults], nil);
          };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Invoking command...");
                                    [device invokeCommandWithEndpointID:myEndpointId
                                                              clusterID:myClusterId
                                                              commandID:myCommandId
                                                          commandFields:myFields
                                                     timedInvokeTimeout:nil
                                                                  queue:dispatch_get_main_queue()
                                                             completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                 NSLog(@"Command response: %@", value);
                                                                 XCTAssertNotNil(value);
                                                                 XCTAssertNil(error);
                                                                 XCTAssertTrue([myResults isEqual:value]);
                                                                 [responseExpectation fulfill];
                                                                 self.xpcDisconnectExpectation =
                                                                     [self expectationWithDescription:@"XPC Disconnected"];
                                                             }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When command is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testTimedInvokeCommandSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myCommandId = @300;
    NSNumber * myTimedInvokeTimeout = @5678;
    NSDictionary * myFields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type",
                                            [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"Type",
                                                                                   [NSNumber numberWithFloat:1.0], @"value", nil]],
                                            @"value", nil];
    NSArray * myResults = @[
        @{ @"commandPath" : [MTRCommandPath commandPathWithEndpointID:myEndpointId clusterID:myClusterId commandID:myCommandId] }
    ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleInvokeCommand
        = ^(id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, id commandFields,
            NSNumber * _Nullable timedInvokeTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
              XCTAssertTrue([controller isEqualToString:uuid]);
              XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
              XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
              XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
              XCTAssertEqual([commandId unsignedLongValue], [myCommandId unsignedLongValue]);
              XCTAssertTrue([commandFields isEqual:myFields]);
              XCTAssertNotNil(timedInvokeTimeout);
              XCTAssertEqual([timedInvokeTimeout unsignedShortValue], [myTimedInvokeTimeout unsignedShortValue]);
              [callExpectation fulfill];
              completion([MTRDeviceController encodeXPCResponseValues:myResults], nil);
          };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Invoking command...");
                                    [device invokeCommandWithEndpointID:myEndpointId
                                                              clusterID:myClusterId
                                                              commandID:myCommandId
                                                          commandFields:myFields
                                                     timedInvokeTimeout:myTimedInvokeTimeout
                                                                  queue:dispatch_get_main_queue()
                                                             completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                 NSLog(@"Command response: %@", value);
                                                                 XCTAssertNotNil(value);
                                                                 XCTAssertNil(error);
                                                                 XCTAssertTrue([myResults isEqual:value]);
                                                                 [responseExpectation fulfill];
                                                                 self.xpcDisconnectExpectation =
                                                                     [self expectationWithDescription:@"XPC Disconnected"];
                                                             }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When command is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testInvokeCommandFailure
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myCommandId = @300;
    NSDictionary * myFields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type",
                                            [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"Type",
                                                                                   [NSNumber numberWithFloat:1.0], @"value", nil]],
                                            @"value", nil];
    NSError * myError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleInvokeCommand
        = ^(id controller, NSNumber * nodeId, NSNumber * endpointId, NSNumber * clusterId, NSNumber * commandId, id commandFields,
            NSNumber * _Nullable timedInvokeTimeout, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
              XCTAssertTrue([controller isEqualToString:uuid]);
              XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
              XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
              XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
              XCTAssertEqual([commandId unsignedLongValue], [myCommandId unsignedLongValue]);
              XCTAssertTrue([commandFields isEqual:myFields]);
              XCTAssertNil(timedInvokeTimeout);
              [callExpectation fulfill];
              completion(nil, myError);
          };

    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Invoking command...");
                                    [device invokeCommandWithEndpointID:myEndpointId
                                                              clusterID:myClusterId
                                                              commandID:myCommandId
                                                          commandFields:myFields
                                                     timedInvokeTimeout:nil
                                                                  queue:dispatch_get_main_queue()
                                                             completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                 NSLog(@"Command response: %@", value);
                                                                 XCTAssertNil(value);
                                                                 XCTAssertNotNil(error);
                                                                 [responseExpectation fulfill];
                                                                 self.xpcDisconnectExpectation =
                                                                     [self expectationWithDescription:@"XPC Disconnected"];
                                                             }];
                                }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, responseExpectation, nil] timeout:kTimeoutInSeconds];

    // When command is done, connection should have been released
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeAttributeSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeAttributeWithParamsSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    MTRSubscribeParams * myParams = [[MTRSubscribeParams alloc] init];
    myParams.fabricFiltered = @NO;
    myParams.keepPreviousSubscriptions = @NO;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNotNil(params);
        XCTAssertEqual([params.fabricFiltered boolValue], [myParams.fabricFiltered boolValue]);
        XCTAssertEqual([params.keepPreviousSubscriptions boolValue], [myParams.keepPreviousSubscriptions boolValue]);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:myParams
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testBadlyFormattedReport
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    // Incorrect serialized report value. Report should have ben a single NSDictionary
    __block id myReport = @{
        @"attributePath" : @[ myEndpointId, myClusterId, myAttributeId ],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    };
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject badly formatted report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId values:myReport error:nil];

    // Wait for report, which isn't expected.
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"Report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    _xpcDisconnectExpectation.inverted = NO;
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedEndpointId
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:@([myEndpointId unsignedShortValue] + 1)
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report which isn't expected
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedClusterId
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:@([myClusterId unsignedLongValue] + 1)
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report not to come
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedAttributeId
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:@([myAttributeId unsignedLongValue] + 1)],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report not to come
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedNode
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId + 1
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report not to come
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeMultiEndpoints
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertNil(endpointId);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:nil
                   clusterID:myClusterId
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeMultiClusters
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertNil(clusterId);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:nil
                   attributeID:myAttributeId
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeMultiAttributes
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSNumber * myMinInterval = @5;
    NSNumber * myMaxInterval = @60;
    __block NSArray * myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertNil(attributeId);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController
        getBaseDevice:myNodeId
                queue:dispatch_get_main_queue()
           completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
               XCTAssertNotNil(device);
               XCTAssertNil(error);
               NSLog(@"Device acquired. Subscribing...");
               [device subscribeToAttributesWithEndpointID:myEndpointId
                   clusterID:myClusterId
                   attributeID:nil
                   minInterval:myMinInterval
                   maxInterval:myMaxInterval
                   params:nil
                   queue:dispatch_get_main_queue()
                   reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                       NSLog(@"Report value: %@", values);
                       XCTAssertNotNil(values);
                       XCTAssertNil(error);
                       XCTAssertTrue([myReport isEqual:values]);
                       [reportExpectation fulfill];
                   }
                   subscriptionEstablished:^{
                       [establishExpectation fulfill];
                   }];
           }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @771234 }
    } ];
    [clientObject handleReportWithController:uuid
                                      nodeId:myNodeId
                                      values:[MTRDeviceController encodeXPCResponseValues:myReport]
                                       error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler
    [_remoteDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                   }];
                                }];

    // Wait for disconnection
    [self waitForExpectations:@[ _xpcDisconnectExpectation, stopExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testMutiSubscriptions
{
    uint64_t nodeIds[] = { 9876543210, 9876543211 };
    NSNumber * endpointIds[] = { @100, @150 };
    NSNumber * clusterIds[] = { @200, @250 };
    NSNumber * attributeIds[] = { @300, @350 };
    NSNumber * minIntervals[] = { @5, @7 };
    NSNumber * maxIntervals[] = { @60, @68 };
    __block uint64_t myNodeId = nodeIds[0];
    __block NSNumber * myEndpointId = endpointIds[0];
    __block NSNumber * myClusterId = clusterIds[0];
    __block NSNumber * myAttributeId = attributeIds[0];
    __block NSNumber * myMinInterval = minIntervals[0];
    __block NSNumber * myMaxInterval = maxIntervals[0];
    __block NSArray<NSArray *> * myReports;
    __block XCTestExpectation * callExpectation;
    __block XCTestExpectation * establishExpectation;
    __block NSArray<XCTestExpectation *> * reportExpectations;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, NSNumber * minInterval, NSNumber * maxInterval, MTRSubscribeParams * _Nullable params,
        void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        XCTAssertEqual([minInterval unsignedShortValue], [myMinInterval unsignedShortValue]);
        XCTAssertEqual([maxInterval unsignedShortValue], [myMaxInterval unsignedShortValue]);
        XCTAssertNil(params);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    // Multi-subscriptions
    for (unsigned int i = 0; i < 2; i++) {
        myNodeId = nodeIds[i];
        myEndpointId = endpointIds[i];
        myClusterId = clusterIds[i];
        myAttributeId = attributeIds[i];
        myMinInterval = minIntervals[i];
        myMaxInterval = maxIntervals[i];
        callExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"XPC call (%u) received", i]];
        establishExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"Established (%u) called", i]];
        [_remoteDeviceController
            getBaseDevice:myNodeId
                    queue:dispatch_get_main_queue()
               completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                   XCTAssertNotNil(device);
                   XCTAssertNil(error);
                   NSLog(@"Device acquired. Subscribing...");
                   [device subscribeToAttributesWithEndpointID:myEndpointId
                       clusterID:myClusterId
                       attributeID:myAttributeId
                       minInterval:myMinInterval
                       maxInterval:myMaxInterval
                       params:nil
                       queue:dispatch_get_main_queue()
                       reportHandler:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                           NSLog(@"Subscriber [%d] report value: %@", i, values);
                           XCTAssertNotNil(values);
                           XCTAssertNil(error);
                           XCTAssertTrue([myReports[i] isEqual:values]);
                           [reportExpectations[i] fulfill];
                       }
                       subscriptionEstablished:^{
                           [establishExpectation fulfill];
                       }];
               }];

        [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];
    }

    id<MTRDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;

    // Inject reports
    for (int count = 0; count < 2; count++) {
        reportExpectations = [NSArray
            arrayWithObjects:[self expectationWithDescription:[NSString
                                                                  stringWithFormat:@"Report(%d) for first subscriber sent", count]],
            [self expectationWithDescription:[NSString stringWithFormat:@"Report(%d) for second subscriber sent", count]], nil];
        myReports = @[
            @[ @{
                @"attributePath" : [MTRAttributePath attributePathWithEndpointID:endpointIds[0]
                                                                       clusterID:clusterIds[0]
                                                                     attributeID:attributeIds[0]],
                @"data" : @ { @"type" : @"SignedInteger", @"value" : [NSNumber numberWithInteger:123456 + count * 100] }
            } ],
            @[ @{
                @"attributePath" : [MTRAttributePath attributePathWithEndpointID:endpointIds[1]
                                                                       clusterID:clusterIds[1]
                                                                     attributeID:attributeIds[1]],
                @"data" : @ { @"type" : @"SignedInteger", @"value" : [NSNumber numberWithInteger:123457 + count * 100] }
            } ]
        ];
        for (unsigned int i = 0; i < 2; i++) {
            NSUInteger nodeId = nodeIds[i];
            dispatch_async(dispatch_get_main_queue(), ^{
                [clientObject handleReportWithController:uuid
                                                  nodeId:nodeId
                                                  values:[MTRDeviceController encodeXPCResponseValues:myReports[i]]
                                                   error:nil];
            });
        }
        [self waitForExpectations:reportExpectations timeout:kTimeoutInSeconds];
    }

    // Setup stop report handler
    XCTestExpectation * stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    __auto_type nodeToStop = nodeIds[0];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], nodeToStop);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler for first subscriber
    __auto_type deregisterExpectation = [self expectationWithDescription:@"First subscriber deregistered"];
    [_remoteDeviceController getBaseDevice:nodeToStop
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                       [deregisterExpectation fulfill];
                                                                   }];
                                }];

    [self waitForExpectations:@[ stopExpectation, deregisterExpectation ] timeout:kTimeoutInSeconds];

    // Inject reports
    for (int count = 0; count < 1; count++) {
        reportExpectations = [NSArray
            arrayWithObjects:[self expectationWithDescription:[NSString
                                                                  stringWithFormat:@"Report(%d) for first subscriber sent", count]],
            [self expectationWithDescription:[NSString stringWithFormat:@"Report(%d) for second subscriber sent", count]], nil];
        reportExpectations[0].inverted = YES;
        myReports = @[
            @[ @{
                @"attributePath" : [MTRAttributePath attributePathWithEndpointID:endpointIds[0]
                                                                       clusterID:clusterIds[0]
                                                                     attributeID:attributeIds[0]],
                @"data" : @ { @"type" : @"SignedInteger", @"value" : [NSNumber numberWithInteger:223456 + count * 100] }
            } ],
            @[ @{
                @"attributePath" : [MTRAttributePath attributePathWithEndpointID:endpointIds[1]
                                                                       clusterID:clusterIds[1]
                                                                     attributeID:attributeIds[1]],
                @"data" : @ { @"type" : @"SignedInteger", @"value" : [NSNumber numberWithInteger:223457 + count * 100] }
            } ]
        ];
        for (unsigned int i = 0; i < 2; i++) {
            NSUInteger nodeId = nodeIds[i];
            dispatch_async(dispatch_get_main_queue(), ^{
                [clientObject handleReportWithController:uuid
                                                  nodeId:nodeId
                                                  values:[MTRDeviceController encodeXPCResponseValues:myReports[i]]
                                                   error:nil];
            });
        }
        [self waitForExpectations:reportExpectations timeout:kTimeoutInSeconds];
    }

    // Setup stop report handler
    stopExpectation = [self expectationWithDescription:@"Reports stopped"];
    nodeToStop = nodeIds[1];
    _handleStopReports = ^(id _Nullable controller, NSNumber * nodeId, void (^completion)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], nodeToStop);
        completion();
        [stopExpectation fulfill];
    };

    // Deregister report handler for second subscriber
    __auto_type secondDeregisterExpectation = [self expectationWithDescription:@"Second subscriber deregistered"];
    [_remoteDeviceController getBaseDevice:nodeToStop
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    NSLog(@"Device acquired. Deregistering...");
                                    [device deregisterReportHandlersWithQueue:dispatch_get_main_queue()
                                                                   completion:^{
                                                                       NSLog(@"Deregistered");
                                                                       [secondDeregisterExpectation fulfill];
                                                                   }];
                                }];

    // Wait for deregistration and disconnection
    [self waitForExpectations:@[ secondDeregisterExpectation, _xpcDisconnectExpectation, stopExpectation ]
                      timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);

    // Inject reports
    for (int count = 0; count < 1; count++) {
        reportExpectations = [NSArray
            arrayWithObjects:[self expectationWithDescription:[NSString
                                                                  stringWithFormat:@"Report(%d) for first subscriber sent", count]],
            [self expectationWithDescription:[NSString stringWithFormat:@"Report(%d) for second subscriber sent", count]], nil];
        reportExpectations[0].inverted = YES;
        reportExpectations[1].inverted = YES;
        myReports = @[
            @[ @{
                @"attributePath" : [MTRAttributePath attributePathWithEndpointID:endpointIds[0]
                                                                       clusterID:clusterIds[0]
                                                                     attributeID:attributeIds[0]],
                @"data" : @ { @"type" : @"SignedInteger", @"value" : [NSNumber numberWithInteger:223456 + count * 100] }
            } ],
            @[ @{
                @"attributePath" : [MTRAttributePath attributePathWithEndpointID:endpointIds[1]
                                                                       clusterID:clusterIds[1]
                                                                     attributeID:attributeIds[1]],
                @"data" : @ { @"type" : @"SignedInteger", @"value" : [NSNumber numberWithInteger:223457 + count * 100] }
            } ]
        ];
        for (unsigned int i = 0; i < 2; i++) {
            NSUInteger nodeId = nodeIds[i];
            dispatch_async(dispatch_get_main_queue(), ^{
                [clientObject handleReportWithController:uuid
                                                  nodeId:nodeId
                                                  values:[MTRDeviceController encodeXPCResponseValues:myReports[i]]
                                                   error:nil];
            });
        }
        [self waitForExpectations:reportExpectations timeout:kNegativeTimeoutInSeconds];
    }
}

- (void)testAnySharedRemoteController
{
    NSString * myUUID = [[NSUUID UUID] UUIDString];
    uint64_t myNodeId = 9876543210;

    __auto_type unspecifiedRemoteDeviceController =
        [MTRDeviceController sharedControllerWithID:nil
                                    xpcConnectBlock:^NSXPCConnection * {
                                        return [[NSXPCConnection alloc] initWithListenerEndpoint:self.xpcListener.endpoint];
                                    }];

    __auto_type anySharedRemoteControllerCallExpectation =
        [self expectationWithDescription:@"getAnySharedRemoteController was called"];
    _handleGetAnySharedRemoteController = ^(void (^completion)(id _Nullable controller, NSError * _Nullable error)) {
        completion(myUUID, nil);
        [anySharedRemoteControllerCallExpectation fulfill];
    };

    __auto_type deviceAcquired = [self expectationWithDescription:@"Connected device was acquired"];
    [unspecifiedRemoteDeviceController getBaseDevice:myNodeId
                                               queue:dispatch_get_main_queue()
                                          completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                              XCTAssertNotNil(device);
                                              XCTAssertNil(error);
                                              [deviceAcquired fulfill];
                                          }];

    [self waitForExpectations:[NSArray arrayWithObjects:anySharedRemoteControllerCallExpectation, deviceAcquired, nil]
                      timeout:kTimeoutInSeconds];
}

- (void)testSubscribeAttributeCacheSuccess
{
    uint64_t myNodeId = 9876543210;
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    __auto_type attributeCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    _handleSubscribeAll = ^(id controller, NSNumber * nodeId, NSNumber * minInterval, NSNumber * maxInterval,
        MTRSubscribeParams * _Nullable params, BOOL shouldCache, void (^completion)(NSError * _Nullable error)) {
        NSLog(@"Subscribe called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertNil(params);
        [callExpectation fulfill];
        completion(nil);
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];
    [attributeCacheContainer subscribeWithDeviceController:_remoteDeviceController
                                                  deviceID:@(myNodeId)
                                                    params:nil
                                                     queue:dispatch_get_main_queue()
                                                completion:^(NSError * _Nullable error) {
                                                    NSLog(@"Subscribe completion called with error: %@", error);
                                                    XCTAssertNil(error);
                                                    [responseExpectation fulfill];
                                                }];

    [self waitForExpectations:@[ callExpectation, responseExpectation, self.xpcDisconnectExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeAttributeCacheWithParamsSuccess
{
    uint64_t myNodeId = 9876543210;
    MTRSubscribeParams * myParams = [[MTRSubscribeParams alloc] init];
    myParams.fabricFiltered = @YES;
    myParams.keepPreviousSubscriptions = @YES;
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    __auto_type attributeCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    _handleSubscribeAll = ^(id controller, NSNumber * nodeId, NSNumber * minInterval, NSNumber * maxInterval,
        MTRSubscribeParams * _Nullable params, BOOL shouldCache, void (^completion)(NSError * _Nullable error)) {
        NSLog(@"Subscribe attribute cache called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertNotNil(params);
        XCTAssertEqual([params.fabricFiltered boolValue], [myParams.fabricFiltered boolValue]);
        XCTAssertEqual([params.keepPreviousSubscriptions boolValue], [myParams.keepPreviousSubscriptions boolValue]);
        [callExpectation fulfill];
        completion(nil);
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];
    [attributeCacheContainer subscribeWithDeviceController:_remoteDeviceController
                                                  deviceID:@(myNodeId)
                                                    params:myParams
                                                     queue:dispatch_get_main_queue()
                                                completion:^(NSError * _Nullable error) {
                                                    NSLog(@"Subscribe completion called with error: %@", error);
                                                    XCTAssertNil(error);
                                                    [responseExpectation fulfill];
                                                }];

    [self waitForExpectations:@[ callExpectation, responseExpectation, self.xpcDisconnectExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeAttributeCacheFailure
{
    uint64_t myNodeId = 9876543210;
    NSError * myError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    __auto_type attributeCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    _handleSubscribeAll = ^(id controller, NSNumber * nodeId, NSNumber * minInterval, NSNumber * maxInterval,
        MTRSubscribeParams * _Nullable params, BOOL shouldCache, void (^completion)(NSError * _Nullable error)) {
        NSLog(@"Subscribe attribute cache called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertNil(params);
        [callExpectation fulfill];
        completion(myError);
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];
    [attributeCacheContainer subscribeWithDeviceController:_remoteDeviceController
                                                  deviceID:@(myNodeId)
                                                    params:nil
                                                     queue:dispatch_get_main_queue()
                                                completion:^(NSError * _Nullable error) {
                                                    NSLog(@"Subscribe completion called with error: %@", error);
                                                    XCTAssertNotNil(error);
                                                    [responseExpectation fulfill];
                                                }];

    [self waitForExpectations:@[ callExpectation, responseExpectation, _xpcDisconnectExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReadAttributeCacheSuccess
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSArray * myValues = @[ @{
        @"attributePath" : [MTRAttributePath attributePathWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId],
        @"data" : @ { @"type" : @"SignedInteger", @"value" : @123456 }
    } ];

    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"Cache subscription complete"];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    __auto_type attributeCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    _handleSubscribeAll = ^(id controller, NSNumber * nodeId, NSNumber * minInterval, NSNumber * maxInterval,
        MTRSubscribeParams * _Nullable params, BOOL shouldCache, void (^completion)(NSError * _Nullable error)) {
        NSLog(@"Subscribe attribute cache called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertNil(params);
        completion(nil);
    };

    _handleReadAttributeCache = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        NSLog(@"Read attribute cache called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        [callExpectation fulfill];
        completion([MTRDeviceController encodeXPCResponseValues:myValues], nil);
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [attributeCacheContainer subscribeWithDeviceController:_remoteDeviceController
                                                  deviceID:@(myNodeId)
                                                    params:nil
                                                     queue:dispatch_get_main_queue()
                                                completion:^(NSError * _Nullable error) {
                                                    NSLog(@"Subscribe completion called with error: %@", error);
                                                    XCTAssertNil(error);
                                                    [subscribeExpectation fulfill];
                                                }];
    [self waitForExpectations:@[ subscribeExpectation, _xpcDisconnectExpectation ] timeout:kTimeoutInSeconds];

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];
    [attributeCacheContainer
        readAttributeWithEndpointID:myEndpointId
                          clusterID:myClusterId
                        attributeID:myAttributeId
                              queue:dispatch_get_main_queue()
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read cached value: %@", values);
                             XCTAssertNotNil(values);
                             XCTAssertNil(error);
                             XCTAssertTrue([myValues isEqual:values]);
                             [responseExpectation fulfill];
                         }];
    [self waitForExpectations:@[ callExpectation, responseExpectation, _xpcDisconnectExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReadAttributeCacheFailure
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    NSError * myError = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"Cache subscription complete"];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    __auto_type attributeCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    _handleSubscribeAll = ^(id controller, NSNumber * nodeId, NSNumber * minInterval, NSNumber * maxInterval,
        MTRSubscribeParams * _Nullable params, BOOL shouldCache, void (^completion)(NSError * _Nullable error)) {
        NSLog(@"Subscribe attribute cache called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertNil(params);
        completion(nil);
    };

    _handleReadAttributeCache = ^(id controller, NSNumber * nodeId, NSNumber * _Nullable endpointId, NSNumber * _Nullable clusterId,
        NSNumber * _Nullable attributeId, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        NSLog(@"Read attribute cache called");
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual([nodeId unsignedLongLongValue], myNodeId);
        XCTAssertEqual([endpointId unsignedShortValue], [myEndpointId unsignedShortValue]);
        XCTAssertEqual([clusterId unsignedLongValue], [myClusterId unsignedLongValue]);
        XCTAssertEqual([attributeId unsignedLongValue], [myAttributeId unsignedLongValue]);
        [callExpectation fulfill];
        completion(nil, myError);
    };

    [attributeCacheContainer subscribeWithDeviceController:_remoteDeviceController
                                                  deviceID:@(myNodeId)
                                                    params:nil
                                                     queue:dispatch_get_main_queue()
                                                completion:^(NSError * _Nullable error) {
                                                    NSLog(@"Subscribe completion called with error: %@", error);
                                                    XCTAssertNil(error);
                                                    [subscribeExpectation fulfill];
                                                }];
    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];
    [attributeCacheContainer
        readAttributeWithEndpointID:myEndpointId
                          clusterID:myClusterId
                        attributeID:myAttributeId
                              queue:dispatch_get_main_queue()
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read cached value: %@", values);
                             XCTAssertNil(values);
                             XCTAssertNotNil(error);
                             [responseExpectation fulfill];
                         }];
    [self waitForExpectations:@[ callExpectation, responseExpectation, _xpcDisconnectExpectation ] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testXPCConnectionFailure
{
    uint64_t myNodeId = 9876543210;
    NSNumber * myEndpointId = @100;
    NSNumber * myClusterId = @200;
    NSNumber * myAttributeId = @300;
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"Read response received"];

    // Test with a device controller which wouldn't connect to XPC listener successfully
    __auto_type failingDeviceController = [MTRDeviceController sharedControllerWithID:_controllerUUID
                                                                      xpcConnectBlock:^NSXPCConnection * {
                                                                          return nil;
                                                                      }];

    [failingDeviceController getBaseDevice:myNodeId
                                     queue:dispatch_get_main_queue()
                                completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                                    XCTAssertNotNil(device);
                                    XCTAssertNil(error);
                                    NSLog(@"Device acquired. Reading...");
                                    [device readAttributesWithEndpointID:myEndpointId
                                                               clusterID:myClusterId
                                                             attributeID:myAttributeId
                                                                  params:nil
                                                                   queue:dispatch_get_main_queue()
                                                              completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                  NSLog(@"Read value: %@", value);
                                                                  XCTAssertNil(value);
                                                                  XCTAssertNotNil(error);
                                                                  [responseExpectation fulfill];
                                                              }];
                                }];

    [self waitForExpectations:@[ responseExpectation ] timeout:kTimeoutInSeconds];
}

@end
