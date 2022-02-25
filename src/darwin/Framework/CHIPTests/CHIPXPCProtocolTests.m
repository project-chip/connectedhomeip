//
//  CHIPRemoteDeviceTests.m
//  CHIPTests
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
#import <CHIP/CHIP.h>

#import "CHIPErrorTestUtils.h"

#import <app/util/af-enums.h>

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

static const uint16_t kTimeoutInSeconds = 3;
// Inverted expectation timeout
static const uint16_t kNegativeTimeoutInSeconds = 1;

@interface CHIPXPCProtocolTests<NSXPCListenerDelegate, CHIPRemoteDeviceProtocol> : XCTestCase

@property (nonatomic, readwrite, strong) NSXPCListener * xpcListener;
@property (nonatomic, readwrite, strong) NSXPCInterface * serviceInterface;
@property (nonatomic, readwrite, strong) NSXPCInterface * clientInterface;
@property (readwrite, strong) NSXPCConnection * xpcConnection;
@property (nonatomic, readwrite, strong) CHIPDeviceController * remoteDeviceController;
@property (nonatomic, readwrite, strong) NSString * controllerUUID;
@property (readwrite, strong) XCTestExpectation * xpcDisconnectExpectation;

@property (readwrite, strong) void (^handleGetAnySharedRemoteControllerWithFabricId)
    (uint64_t fabricId, void (^completion)(id _Nullable controller, NSError * _Nullable error));
@property (readwrite, strong) void (^handleGetAnySharedRemoteController)
    (void (^completion)(id _Nullable controller, NSError * _Nullable error));
@property (readwrite, strong) void (^handleReadAttribute)(id controller, uint64_t nodeId, NSUInteger endpointId,
    NSUInteger clusterId, NSUInteger attributeId, void (^completion)(id _Nullable values, NSError * _Nullable error));
@property (readwrite, strong) void (^handleWriteAttribute)(id controller, uint64_t nodeId, NSUInteger endpointId,
    NSUInteger clusterId, NSUInteger attributeId, id value, void (^completion)(id _Nullable values, NSError * _Nullable error));
@property (readwrite, strong) void (^handleInvokeCommand)(id controller, uint64_t nodeId, NSUInteger endpointId,
    NSUInteger clusterId, NSUInteger commandId, id fields, void (^completion)(id _Nullable values, NSError * _Nullable error));
@property (readwrite, strong) void (^handleSubscribeAttribute)(id controller, uint64_t nodeId, NSUInteger endpointId,
    NSUInteger clusterId, NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void));

@end

@implementation CHIPXPCProtocolTests

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    XCTAssertNil(_xpcConnection);
    XCTAssertNotNil(newConnection);
    NSLog(@"XPC listener accepting connection");
    newConnection.exportedInterface = _serviceInterface;
    newConnection.remoteObjectInterface = _clientInterface;
    newConnection.exportedObject = self;
    newConnection.invalidationHandler = ^{
        NSLog(@"XPC connection disconnected");
        self.xpcConnection = nil;
        if (self.xpcDisconnectExpectation) {
            [self.xpcDisconnectExpectation fulfill];
            self.xpcDisconnectExpectation = nil;
        }
    };
    _xpcConnection = newConnection;
    [newConnection resume];
    return YES;
}

- (void)getDeviceControllerWithFabricId:(uint64_t)fabricId
                             completion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleGetAnySharedRemoteControllerWithFabricId);
        self.handleGetAnySharedRemoteControllerWithFabricId(fabricId, completion);
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
                         endpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                         completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleReadAttribute);
        self.handleReadAttribute(controller, nodeId, endpointId, clusterId, attributeId, completion);
    });
}

- (void)writeAttributeWithController:(id)controller
                              nodeId:(uint64_t)nodeId
                          endpointId:(NSUInteger)endpointId
                           clusterId:(NSUInteger)clusterId
                         attributeId:(NSUInteger)attributeId
                               value:(id)value
                          completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleWriteAttribute);
        self.handleWriteAttribute(controller, nodeId, endpointId, clusterId, attributeId, value, completion);
    });
}

- (void)invokeCommandWithController:(id)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                          commandId:(NSUInteger)commandId
                             fields:(id)fields
                         completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleInvokeCommand);
        self.handleInvokeCommand(controller, nodeId, endpointId, clusterId, commandId, fields, completion);
    });
}

- (void)subscribeAttributeWithController:(id)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSUInteger)endpointId
                               clusterId:(NSUInteger)clusterId
                             attributeId:(NSUInteger)attributeId
                             minInterval:(NSUInteger)minInterval
                             maxInterval:(NSUInteger)maxInterval
                      establishedHandler:(void (^)(void))establishedHandler
{
    dispatch_async(dispatch_get_main_queue(), ^{
        XCTAssertNotNil(self.handleSubscribeAttribute);
        self.handleSubscribeAttribute(
            controller, nodeId, endpointId, clusterId, attributeId, minInterval, maxInterval, establishedHandler);
    });
}

- (void)setUp
{
    _xpcListener = [NSXPCListener anonymousListener];
    [_xpcListener setDelegate:(id<NSXPCListenerDelegate>) self];
    _serviceInterface = [NSXPCInterface interfaceWithProtocol:@protocol(CHIPDeviceControllerServerProtocol)];
    _clientInterface = [NSXPCInterface interfaceWithProtocol:@protocol(CHIPDeviceControllerClientProtocol)];
    [_xpcListener resume];
    _controllerUUID = [[NSUUID UUID] UUIDString];
    _remoteDeviceController =
        [CHIPDeviceController sharedControllerWithId:_controllerUUID
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSArray * myValues =
        [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId],
                                               @"endpointId", [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId",
                                               [NSNumber numberWithUnsignedInteger:myAttributeId], @"attributeId",
                                               [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                                             [NSNumber numberWithInteger:123456], @"value", nil],
                                               @"data", nil]];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleReadAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId, NSUInteger attributeId,
        void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        [callExpectation fulfill];
        completion(myValues, nil);
    };

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Reading...");
                                  [device readAttributeWithEndpointId:myEndpointId
                                                            clusterId:myClusterId
                                                          attributeId:myAttributeId
                                                          clientQueue:dispatch_get_main_queue()
                                                           completion:^(id _Nullable value, NSError * _Nullable error) {
                                                               NSLog(@"Read value: %@", value);
                                                               XCTAssertNotNil(value);
                                                               XCTAssertNil(error);
                                                               XCTAssertTrue([myValues isEqualTo:value]);
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSError * myError = [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleReadAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId, NSUInteger attributeId,
        void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        [callExpectation fulfill];
        completion(nil, myError);
    };

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Reading...");
                                  [device readAttributeWithEndpointId:myEndpointId
                                                            clusterId:myClusterId
                                                          attributeId:myAttributeId
                                                          clientQueue:dispatch_get_main_queue()
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSDictionary * myValue =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:654321], @"value", nil];
    NSArray * myResults =
        [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId],
                                               @"endpointId", [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId",
                                               [NSNumber numberWithUnsignedInteger:myAttributeId], @"attributeId",
                                               [NSNumber numberWithInteger:0], @"status", nil]];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleWriteAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId, NSUInteger attributeId,
        id value, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertTrue([value isEqualTo:myValue]);
        [callExpectation fulfill];
        completion(myResults, nil);
    };

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Writing...");
                                  [device writeAttributeWithEndpointId:myEndpointId
                                                             clusterId:myClusterId
                                                           attributeId:myAttributeId
                                                                 value:myValue
                                                           clientQueue:dispatch_get_main_queue()
                                                            completion:^(id _Nullable value, NSError * _Nullable error) {
                                                                NSLog(@"Write response: %@", value);
                                                                XCTAssertNotNil(value);
                                                                XCTAssertNil(error);
                                                                XCTAssertTrue([myResults isEqualTo:value]);
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSDictionary * myValue =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:654321], @"value", nil];
    NSError * myError = [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleWriteAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId, NSUInteger attributeId,
        id value, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertTrue([value isEqualTo:myValue]);
        [callExpectation fulfill];
        completion(nil, myError);
    };

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Writing...");
                                  [device writeAttributeWithEndpointId:myEndpointId
                                                             clusterId:myClusterId
                                                           attributeId:myAttributeId
                                                                 value:myValue
                                                           clientQueue:dispatch_get_main_queue()
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myCommandId = 300;
    NSDictionary * myFields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type",
                                            [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"Type",
                                                                                   [NSNumber numberWithFloat:1.0], @"value", nil]],
                                            @"value", nil];
    NSArray * myResults =
        [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId],
                                               @"endpointId", [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId",
                                               [NSNumber numberWithUnsignedInteger:myCommandId], @"commandId",
                                               [NSNumber numberWithInteger:0], @"status", nil]];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleInvokeCommand = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId, NSUInteger commandId,
        id commandFields, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(commandId, myCommandId);
        XCTAssertTrue([commandFields isEqualTo:myFields]);
        [callExpectation fulfill];
        completion(myResults, nil);
    };

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Invoking command...");
                                  [device invokeCommandWithEndpointId:myEndpointId
                                                            clusterId:myClusterId
                                                            commandId:myCommandId
                                                        commandFields:myFields
                                                          clientQueue:dispatch_get_main_queue()
                                                           completion:^(id _Nullable value, NSError * _Nullable error) {
                                                               NSLog(@"Command response: %@", value);
                                                               XCTAssertNotNil(value);
                                                               XCTAssertNil(error);
                                                               XCTAssertTrue([myResults isEqualTo:value]);
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myCommandId = 300;
    NSDictionary * myFields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type",
                                            [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"Type",
                                                                                   [NSNumber numberWithFloat:1.0], @"value", nil]],
                                            @"value", nil];
    NSError * myError = [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * responseExpectation = [self expectationWithDescription:@"XPC response received"];

    __auto_type uuid = self.controllerUUID;
    _handleInvokeCommand = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId, NSUInteger commandId,
        id commandFields, void (^completion)(id _Nullable values, NSError * _Nullable error)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(commandId, myCommandId);
        XCTAssertTrue([commandFields isEqualTo:myFields]);
        [callExpectation fulfill];
        completion(nil, myError);
    };

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Invoking command...");
                                  [device invokeCommandWithEndpointId:myEndpointId
                                                            clusterId:myClusterId
                                                            commandId:myCommandId
                                                        commandFields:myFields
                                                          clientQueue:dispatch_get_main_queue()
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
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testBadlyFormattedReport
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    // Incorrect report value. Report must be a single NSDictionary
    __block id myReport =
        [NSArray arrayWithObject:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId],
                                               @"endpointId", [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId",
                                               [NSNumber numberWithUnsignedInteger:myAttributeId], @"attributeId",
                                               [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                                             [NSNumber numberWithInteger:123456], @"value", nil],
                                               @"data", nil]];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject badly formatted report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report, which isn't expected.
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"Report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    _xpcDisconnectExpectation.inverted = NO;
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedEndpointId
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId + 1], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report which isn't expected
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedClusterId
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId + 1], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report not to come
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedAttributeId
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId + 1],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report not to come
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testReportWithUnrelatedNode
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Unexpected report sent"];
    reportExpectation.inverted = YES;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId + 1 value:myReport error:nil];

    // Wait for report not to come
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeMultiEndpoints
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, 0xffff);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:0xffff
                                      clusterId:myClusterId
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeMultiClusters
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, 0xffffffff);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:0xffffffff
                                      attributeId:myAttributeId
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kNegativeTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testSubscribeMultiAttributes
{
    uint64_t myNodeId = 9876543210;
    NSUInteger myEndpointId = 100;
    NSUInteger myClusterId = 200;
    NSUInteger myAttributeId = 300;
    NSUInteger myMinInterval = 5;
    NSUInteger myMaxInterval = 60;
    __block NSDictionary * myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:123456], @"value", nil],
        @"data", nil];
    XCTestExpectation * callExpectation = [self expectationWithDescription:@"XPC call received"];
    XCTestExpectation * establishExpectation = [self expectationWithDescription:@"Established called"];
    __block XCTestExpectation * reportExpectation = [self expectationWithDescription:@"Report sent"];

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, 0xffffffff);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  XCTAssertNotNil(device);
                                  XCTAssertNil(error);
                                  NSLog(@"Device acquired. Subscribing...");
                                  [device subscribeAttributeWithEndpointId:myEndpointId
                                      clusterId:myClusterId
                                      attributeId:0xffffffff
                                      minInterval:myMinInterval
                                      maxInterval:myMaxInterval
                                      clientQueue:dispatch_get_main_queue()
                                      reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                                          NSLog(@"Report value: %@", value);
                                          XCTAssertNotNil(value);
                                          XCTAssertNil(error);
                                          XCTAssertTrue([myReport isEqualTo:value]);
                                          [reportExpectation fulfill];
                                      }
                                      subscriptionEstablished:^{
                                          [establishExpectation fulfill];
                                      }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];

    // Inject report
    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Inject another report
    reportExpectation = [self expectationWithDescription:@"2nd report sent"];
    myReport = [NSDictionary
        dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:myEndpointId], @"endpointId",
        [NSNumber numberWithUnsignedInteger:myClusterId], @"clusterId", [NSNumber numberWithUnsignedInteger:myAttributeId],
        @"attributeId",
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:771234], @"value", nil],
        @"data", nil];
    [clientObject handleReportWithController:uuid nodeId:myNodeId value:myReport error:nil];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Deregister report handler
    [_remoteDeviceController getConnectedDevice:myNodeId
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                       }];
                              }];

    // Wait for disconnection
    [self waitForExpectations:[NSArray arrayWithObject:_xpcDisconnectExpectation] timeout:kTimeoutInSeconds];
    XCTAssertNil(_xpcConnection);
}

- (void)testMutiSubscriptions
{
    uint64_t nodeIds[] = { 9876543210, 9876543211 };
    NSUInteger endpointIds[] = { 100, 150 };
    NSUInteger clusterIds[] = { 200, 250 };
    NSUInteger attributeIds[] = { 300, 350 };
    NSUInteger minIntervals[] = { 5, 7 };
    NSUInteger maxIntervals[] = { 60, 68 };
    __block uint64_t myNodeId = nodeIds[0];
    __block NSUInteger myEndpointId = endpointIds[0];
    __block NSUInteger myClusterId = clusterIds[0];
    __block NSUInteger myAttributeId = attributeIds[0];
    __block NSUInteger myMinInterval = minIntervals[0];
    __block NSUInteger myMaxInterval = maxIntervals[0];
    __block NSArray<NSDictionary *> * myReports;
    __block XCTestExpectation * callExpectation;
    __block XCTestExpectation * establishExpectation;
    __block NSArray<XCTestExpectation *> * reportExpectations;

    __auto_type uuid = self.controllerUUID;
    _handleSubscribeAttribute = ^(id controller, uint64_t nodeId, NSUInteger endpointId, NSUInteger clusterId,
        NSUInteger attributeId, NSUInteger minInterval, NSUInteger maxInterval, void (^establishedHandler)(void)) {
        XCTAssertTrue([controller isEqualToString:uuid]);
        XCTAssertEqual(nodeId, myNodeId);
        XCTAssertEqual(endpointId, myEndpointId);
        XCTAssertEqual(clusterId, myClusterId);
        XCTAssertEqual(attributeId, myAttributeId);
        XCTAssertEqual(minInterval, myMinInterval);
        XCTAssertEqual(maxInterval, myMaxInterval);
        [callExpectation fulfill];
        establishedHandler();
    };

    _xpcDisconnectExpectation = [self expectationWithDescription:@"XPC Disconnected"];

    // Multi-subscriptions
    for (int i = 0; i < 2; i++) {
        myNodeId = nodeIds[i];
        myEndpointId = endpointIds[i];
        myClusterId = clusterIds[i];
        myAttributeId = attributeIds[i];
        myMinInterval = minIntervals[i];
        myMaxInterval = maxIntervals[i];
        callExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"XPC call (%d) received", i]];
        establishExpectation = [self expectationWithDescription:[NSString stringWithFormat:@"Established (%d) called", i]];
        [_remoteDeviceController
            getConnectedDevice:myNodeId
                         queue:dispatch_get_main_queue()
             completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                 XCTAssertNotNil(device);
                 XCTAssertNil(error);
                 NSLog(@"Device acquired. Subscribing...");
                 [device subscribeAttributeWithEndpointId:myEndpointId
                     clusterId:myClusterId
                     attributeId:myAttributeId
                     minInterval:myMinInterval
                     maxInterval:myMaxInterval
                     clientQueue:dispatch_get_main_queue()
                     reportHandler:^(NSDictionary<NSString *, id> * _Nullable value, NSError * _Nullable error) {
                         NSLog(@"Subscriber [%d] report value: %@", i, value);
                         XCTAssertNotNil(value);
                         XCTAssertNil(error);
                         XCTAssertTrue([myReports[i] isEqualTo:value]);
                         [reportExpectations[i] fulfill];
                     }
                     subscriptionEstablished:^{
                         [establishExpectation fulfill];
                     }];
             }];

        [self waitForExpectations:[NSArray arrayWithObjects:callExpectation, establishExpectation, nil] timeout:kTimeoutInSeconds];
    }

    id<CHIPDeviceControllerClientProtocol> clientObject = _xpcConnection.remoteObjectProxy;

    // Inject reports
    for (int count = 0; count < 2; count++) {
        reportExpectations = [NSArray
            arrayWithObjects:[self expectationWithDescription:[NSString
                                                                  stringWithFormat:@"Report(%d) for first subscriber sent", count]],
            [self expectationWithDescription:[NSString stringWithFormat:@"Report(%d) for second subscriber sent", count]], nil];
        myReports = [NSArray
            arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:endpointIds[0]],
                                           @"endpointId", [NSNumber numberWithUnsignedInteger:clusterIds[0]], @"clusterId",
                                           [NSNumber numberWithUnsignedInteger:attributeIds[0]], @"attributeId",
                                           [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                                         [NSNumber numberWithInteger:123456 + count * 100], @"value", nil],
                                           @"data", nil],
            [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:endpointIds[1]], @"endpointId",
                          [NSNumber numberWithUnsignedInteger:clusterIds[1]], @"clusterId",
                          [NSNumber numberWithUnsignedInteger:attributeIds[1]], @"attributeId",
                          [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                        [NSNumber numberWithInteger:123457 + count * 100], @"value", nil],
                          @"data", nil],
            nil];
        for (int i = 0; i < 2; i++) {
            NSUInteger nodeId = nodeIds[i];
            dispatch_async(dispatch_get_main_queue(), ^{
                [clientObject handleReportWithController:uuid nodeId:nodeId value:myReports[i] error:nil];
            });
        }
        [self waitForExpectations:reportExpectations timeout:kTimeoutInSeconds];
    }

    // Deregister report handler for first subscriber
    __auto_type deregisterExpectation = [self expectationWithDescription:@"First subscriber deregistered"];
    [_remoteDeviceController getConnectedDevice:nodeIds[0]
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                           [deregisterExpectation fulfill];
                                                                       }];
                              }];

    [self waitForExpectations:[NSArray arrayWithObject:deregisterExpectation] timeout:kTimeoutInSeconds];

    // Inject reports
    for (int count = 0; count < 1; count++) {
        reportExpectations = [NSArray
            arrayWithObjects:[self expectationWithDescription:[NSString
                                                                  stringWithFormat:@"Report(%d) for first subscriber sent", count]],
            [self expectationWithDescription:[NSString stringWithFormat:@"Report(%d) for second subscriber sent", count]], nil];
        reportExpectations[0].inverted = YES;
        myReports = [NSArray
            arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:endpointIds[0]],
                                           @"endpointId", [NSNumber numberWithUnsignedInteger:clusterIds[0]], @"clusterId",
                                           [NSNumber numberWithUnsignedInteger:attributeIds[0]], @"attributeId",
                                           [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                                         [NSNumber numberWithInteger:223456 + count * 100], @"value", nil],
                                           @"data", nil],
            [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:endpointIds[1]], @"endpointId",
                          [NSNumber numberWithUnsignedInteger:clusterIds[1]], @"clusterId",
                          [NSNumber numberWithUnsignedInteger:attributeIds[1]], @"attributeId",
                          [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                        [NSNumber numberWithInteger:223457 + count * 100], @"value", nil],
                          @"data", nil],
            nil];
        for (int i = 0; i < 2; i++) {
            NSUInteger nodeId = nodeIds[i];
            dispatch_async(dispatch_get_main_queue(), ^{
                [clientObject handleReportWithController:uuid nodeId:nodeId value:myReports[i] error:nil];
            });
        }
        [self waitForExpectations:reportExpectations timeout:kTimeoutInSeconds];
    }

    // Deregister report handler for second subscriber
    __auto_type secondDeregisterExpectation = [self expectationWithDescription:@"Second subscriber deregistered"];
    [_remoteDeviceController getConnectedDevice:nodeIds[1]
                                          queue:dispatch_get_main_queue()
                              completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                  NSLog(@"Device acquired. Deregistering...");
                                  [device deregisterReportHandlersWithClientQueue:dispatch_get_main_queue()
                                                                       completion:^{
                                                                           NSLog(@"Deregistered");
                                                                           [secondDeregisterExpectation fulfill];
                                                                       }];
                              }];

    // Wait for deregistration and disconnection
    [self waitForExpectations:[NSArray arrayWithObjects:secondDeregisterExpectation, _xpcDisconnectExpectation, nil]
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
        myReports = [NSArray
            arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:endpointIds[0]],
                                           @"endpointId", [NSNumber numberWithUnsignedInteger:clusterIds[0]], @"clusterId",
                                           [NSNumber numberWithUnsignedInteger:attributeIds[0]], @"attributeId",
                                           [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                                         [NSNumber numberWithInteger:223456 + count * 100], @"value", nil],
                                           @"data", nil],
            [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:endpointIds[1]], @"endpointId",
                          [NSNumber numberWithUnsignedInteger:clusterIds[1]], @"clusterId",
                          [NSNumber numberWithUnsignedInteger:attributeIds[1]], @"attributeId",
                          [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type",
                                        [NSNumber numberWithInteger:223457 + count * 100], @"value", nil],
                          @"data", nil],
            nil];
        for (int i = 0; i < 2; i++) {
            NSUInteger nodeId = nodeIds[i];
            dispatch_async(dispatch_get_main_queue(), ^{
                [clientObject handleReportWithController:uuid nodeId:nodeId value:myReports[i] error:nil];
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
        [CHIPDeviceController sharedControllerWithId:nil
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
    [unspecifiedRemoteDeviceController getConnectedDevice:myNodeId
                                                    queue:dispatch_get_main_queue()
                                        completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                                            XCTAssertNotNil(device);
                                            XCTAssertNil(error);
                                            [deviceAcquired fulfill];
                                        }];

    [self waitForExpectations:[NSArray arrayWithObjects:anySharedRemoteControllerCallExpectation, deviceAcquired, nil]
                      timeout:kTimeoutInSeconds];
}

@end
