/**
 *    Copyright (c) 2026 Project CHIP Authors
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

#import <Matter/Matter.h>
#import <XCTest/XCTest.h>

static const NSUInteger kControllerCount = 50;
static const NSTimeInterval kTimeoutInSeconds = 5;

@interface MTRDeviceControllerXPCTestServer : NSObject <NSXPCListenerDelegate, MTRXPCServerProtocol_MTRDeviceController>
@property (nonatomic, readonly) NSXPCListener * listener;
@property (nonatomic, readonly) NSUInteger acceptedCount;
@property (nonatomic, readonly) NSUInteger invalidatedCount;
@property (atomic, strong, nullable) XCTestExpectation * configurationExpectation;
@property (atomic, strong, nullable) XCTestExpectation * invalidationExpectation;
@end

@implementation MTRDeviceControllerXPCTestServer {
    NSUInteger _acceptedCount;
    NSUInteger _invalidatedCount;
}

- (instancetype)init
{
    if (self = [super init]) {
        _listener = [NSXPCListener anonymousListener];
        _listener.delegate = self;
        [_listener resume];
    }
    return self;
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    @synchronized(self) {
        _acceptedCount++;
    }
    newConnection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRXPCServerProtocol)];
    newConnection.exportedObject = self;
    newConnection.invalidationHandler = ^{
        @synchronized(self) {
            self->_invalidatedCount++;
        }
        [self.invalidationExpectation fulfill];
    };
    [newConnection resume];
    return YES;
}

- (NSUInteger)acceptedCount
{
    @synchronized(self) {
        return _acceptedCount;
    }
}

- (NSUInteger)invalidatedCount
{
    @synchronized(self) {
        return _invalidatedCount;
    }
}

- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState
{
    [self.configurationExpectation fulfill];
}

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> *))reply
{
    reply(@[]);
}

@end

@interface MTRDeviceControllerXPCTests : XCTestCase
@property (nonatomic, strong) MTRDeviceControllerXPCTestServer * server;
@property (atomic, weak, nullable) NSXPCConnection * lastClientConnection;
@end

@implementation MTRDeviceControllerXPCTests

- (void)setUp
{
    [super setUp];
    self.server = [[MTRDeviceControllerXPCTestServer alloc] init];
}

- (void)tearDown
{
    [self.server.listener invalidate];
    self.server = nil;
    [super tearDown];
}

- (MTRDeviceController *)_startConnectedController
{
    NSXPCListenerEndpoint * endpoint = self.server.listener.endpoint;
    __weak MTRDeviceControllerXPCTests * weakSelf = self;
    __auto_type * parameters = [[MTRXPCDeviceControllerParameters alloc] initWithXPCConnectionBlock:^NSXPCConnection * {
        NSXPCConnection * connection = [[NSXPCConnection alloc] initWithListenerEndpoint:endpoint];
        weakSelf.lastClientConnection = connection;
        return connection;
    }
                                                                                   uniqueIdentifier:[NSUUID UUID]];

    self.server.configurationExpectation = [self expectationWithDescription:@"Controller registered with the server"];
    NSError * error;
    MTRDeviceController * controller = [[MTRDeviceController alloc] initWithParameters:parameters error:&error];
    XCTAssertNotNil(controller);
    XCTAssertNil(error);
    [self waitForExpectations:@[ self.server.configurationExpectation ] timeout:kTimeoutInSeconds];
    self.server.configurationExpectation = nil;
    return controller;
}

- (void)testShutdownReleasesControllers
{
    // Each controller closes its connection on shutdown and is then deallocated.
    NSHashTable<MTRDeviceController *> * controllers = [NSHashTable weakObjectsHashTable];
    XCTestExpectation * invalidated = [self expectationWithDescription:@"Every controller connection invalidated"];
    invalidated.expectedFulfillmentCount = kControllerCount;
    self.server.invalidationExpectation = invalidated;

    for (NSUInteger i = 0; i < kControllerCount; i++) {
        @autoreleasepool {
            MTRDeviceController * controller = [self _startConnectedController];
            [controllers addObject:controller];
            [controller shutdown];
        }
    }

    [self waitForExpectations:@[ invalidated ] timeout:kTimeoutInSeconds];
    self.server.invalidationExpectation = nil;
    XCTAssertEqual(self.server.acceptedCount, kControllerCount);
    XCTAssertEqual(self.server.invalidatedCount, kControllerCount);

    NSPredicate * allReleased = [NSPredicate predicateWithBlock:^BOOL(id _Nullable object, NSDictionary * _Nullable bindings) {
        return controllers.allObjects.count == 0;
    }];
    [self waitForExpectations:@[ [[XCTNSPredicateExpectation alloc] initWithPredicate:allReleased object:nil] ] timeout:kTimeoutInSeconds];
    XCTAssertEqual(controllers.allObjects.count, 0);
}

- (void)testRepeatedShutdownDoesNotReconnect
{
    MTRDeviceController * controller = [self _startConnectedController];
    XCTAssertTrue(controller.running);

    XCTestExpectation * invalidated = [self expectationWithDescription:@"Controller connection invalidated"];
    self.server.invalidationExpectation = invalidated;
    [controller shutdown];
    [controller shutdown];
    [self waitForExpectations:@[ invalidated ] timeout:kTimeoutInSeconds];
    self.server.invalidationExpectation = nil;
    XCTAssertFalse(controller.running);

    // Longer than the controller's reconnect delay after an invalidated connection.
    XCTestExpectation * noReconnect = [self expectationWithDescription:@"No reconnect after shutdown"];
    noReconnect.inverted = YES;
    self.server.configurationExpectation = noReconnect;
    [self waitForExpectations:@[ noReconnect ] timeout:2];
    XCTAssertEqual(self.server.acceptedCount, 1);
    XCTAssertEqual(self.server.invalidatedCount, 1);
}

- (void)testShutdownInvalidatesDevices
{
    // Shutdown invalidates the controller's devices, which cancels a pending attribute wait.
    MTRDeviceController * controller = [self _startConnectedController];
    MTRDevice * device = [MTRDevice deviceWithNodeID:@(1) controller:controller];
    MTRAttributePath * path = [MTRAttributePath attributePathWithEndpointID:@(0)
                                                                  clusterID:@(MTRClusterIDTypeBasicInformationID)
                                                                attributeID:@(MTRAttributeIDTypeClusterBasicInformationAttributeVendorIDID)];
    NSDictionary<NSString *, id> * value = @{ MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @(1) };

    XCTestExpectation * waitCancelled = [self expectationWithDescription:@"Attribute wait cancelled"];
    MTRAttributeValueWaiter * waiter = [device waitForAttributeValues:@{ path : value } timeout:60 queue:dispatch_get_main_queue() completion:^(NSError * _Nullable error) {
        XCTAssertEqual(error.domain, MTRErrorDomain);
        XCTAssertEqual(error.code, MTRErrorCodeCancelled);
        [waitCancelled fulfill];
    }];
    XCTAssertNotNil(waiter);

    [controller shutdown];
    [self waitForExpectations:@[ waitCancelled ] timeout:kTimeoutInSeconds];
    XCTAssertNotNil(device);
    XCTAssertNotNil(waiter);
}

- (void)testShutdownStopsTrackingNewDevices
{
    MTRDeviceController * controller = [self _startConnectedController];
    [controller shutdown];

    MTRDevice * first = [MTRDevice deviceWithNodeID:@(1) controller:controller];
    MTRDevice * second = [MTRDevice deviceWithNodeID:@(1) controller:controller];
    XCTAssertNotNil(first);
    XCTAssertNotNil(second);
    XCTAssertNotEqual(first, second);
}

- (void)testInterruptedConnectionIsInvalidated
{
    XCTestExpectation * invalidated = [self expectationWithDescription:@"Interrupted and replacement connections invalidated"];
    invalidated.expectedFulfillmentCount = 2;
    self.server.invalidationExpectation = invalidated;

    __weak MTRDeviceController * weakController;
    @autoreleasepool {
        MTRDeviceController * controller = [self _startConnectedController];
        weakController = controller;

        XCTestExpectation * reconnected = [self expectationWithDescription:@"Controller reconnected"];
        self.server.configurationExpectation = reconnected;
        NSXPCConnection * interrupted = self.lastClientConnection;
        XCTAssertNotNil(interrupted);
        interrupted.interruptionHandler();
        [self waitForExpectations:@[ reconnected ] timeout:kTimeoutInSeconds];
        self.server.configurationExpectation = nil;
        XCTAssertNotEqual(self.lastClientConnection, interrupted);

        [controller shutdown];
    }

    [self waitForExpectations:@[ invalidated ] timeout:kTimeoutInSeconds];
    self.server.invalidationExpectation = nil;
    XCTAssertEqual(self.server.acceptedCount, 2);
    XCTAssertEqual(self.server.invalidatedCount, 2);

    NSPredicate * released = [NSPredicate predicateWithBlock:^BOOL(id _Nullable object, NSDictionary * _Nullable bindings) {
        return weakController == nil;
    }];
    [self waitForExpectations:@[ [[XCTNSPredicateExpectation alloc] initWithPredicate:released object:nil] ] timeout:kTimeoutInSeconds];
    XCTAssertNil(weakController);
}

@end
