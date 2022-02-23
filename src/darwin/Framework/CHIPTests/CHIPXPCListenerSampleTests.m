//
//  CHIPRemoteDeviceSampleTests.m
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
#import <CHIP/CHIPDevice.h>

#import "CHIPErrorTestUtils.h"

#import <app/util/af-enums.h>

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

// Set the following to 1 in order to run individual test case manually.
#define MANUAL_INDIVIDUAL_TEST 0

//
// Sample XPC Listener implementation that directly communicates with local CHIPDevice instance
//
// Note that real implementation could look almost the same as the sample if the remote device controller object
// is in a separate process in the same machine.
// If the remote device controller object is in a remote machine, the server protocol must implement
// routing the requests to the remote object in a remote machine using implementation specific transport protocol
// between the two machines.

@interface CHIPXPCListenerSample<NSXPCListenerDelegate> : NSObject

@property (nonatomic, readonly, getter=listenerEndpoint) NSXPCListenerEndpoint * listenerEndpoint;

- (void)start;
- (void)stop;

@end

@interface CHIPDeviceControllerServerSample<CHIPDeviceControllerServerProtocol> : NSObject
@property (nonatomic, readonly, strong) NSString * identifier;
- (instancetype)initWithClientProxy:(id<CHIPDeviceControllerClientProtocol>)proxy;
@end

@interface CHIPXPCListenerSample ()

@property (nonatomic, readonly, strong) NSString * controllerId;
@property (nonatomic, readonly, strong) NSXPCInterface * serviceInterface;
@property (nonatomic, readonly, strong) NSXPCInterface * clientInterface;
@property (nonatomic, readonly, strong) NSXPCListener * xpcListener;
@property (nonatomic, readonly, strong) NSMutableDictionary<NSString *, CHIPDeviceControllerServerSample *> * servers;

@end

@implementation CHIPXPCListenerSample

- (instancetype)init
{
    if ([super init]) {
        _serviceInterface = [NSXPCInterface interfaceWithProtocol:@protocol(CHIPDeviceControllerServerProtocol)];
        _clientInterface = [NSXPCInterface interfaceWithProtocol:@protocol(CHIPDeviceControllerClientProtocol)];
        _servers = [NSMutableDictionary dictionary];
        _xpcListener = [NSXPCListener anonymousListener];
        [_xpcListener setDelegate:(id<NSXPCListenerDelegate>) self];
    }
    return self;
}

- (void)start
{
    [_xpcListener resume];
}

- (void)stop
{
    [_xpcListener suspend];
}

- (NSXPCListenerEndpoint *)listenerEndpoint
{
    return _xpcListener.endpoint;
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)newConnection
{
    NSLog(@"XPC listener accepting connection");
    newConnection.exportedInterface = _serviceInterface;
    newConnection.remoteObjectInterface = _clientInterface;
    __auto_type newServer = [[CHIPDeviceControllerServerSample alloc] initWithClientProxy:[newConnection remoteObjectProxy]];
    newConnection.exportedObject = newServer;
    [_servers setObject:newServer forKey:newServer.identifier];
    newConnection.invalidationHandler = ^{
        NSLog(@"XPC connection disconnected");
        [self.servers removeObjectForKey:newServer.identifier];
    };
    [newConnection resume];
    return YES;
}

@end

@interface CHIPDeviceControllerServerSample ()
@property (nonatomic, readwrite, strong) id<CHIPDeviceControllerClientProtocol> clientProxy;
@end

// This sample does not have multiple controllers and hence controller Id shall be the same.
static NSString * const kCHIPDeviceControllerId = @"CHIPController";

@implementation CHIPDeviceControllerServerSample

- (instancetype)initWithClientProxy:(id<CHIPDeviceControllerClientProtocol>)proxy
{
    if ([super init]) {
        _clientProxy = proxy;
        _identifier = [[NSUUID UUID] UUIDString];
    }
    return self;
}

- (void)getDeviceControllerWithFabricId:(uint64_t)fabricId
                             completion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    // We are using a shared local device controller and hence no disctinction per fabricId.
    (void) fabricId;
    completion(kCHIPDeviceControllerId, nil);
}

- (void)getAnyDeviceControllerWithCompletion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    completion(kCHIPDeviceControllerId, nil);
}

- (void)readAttributeWithController:(id)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                         completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    (void) controller;
    __auto_type sharedController = [CHIPDeviceController sharedController];
    if (sharedController) {
        [sharedController getConnectedDevice:nodeId
                                       queue:dispatch_get_main_queue()
                           completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                               if (error) {
                                   NSLog(@"Failed to get connected device");
                                   completion(nil, error);
                               } else {
                                   [device readAttributeWithEndpointId:endpointId
                                                             clusterId:clusterId
                                                           attributeId:attributeId
                                                           clientQueue:dispatch_get_main_queue()
                                                            completion:completion];
                               }
                           }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)writeAttributeWithController:(id)controller
                              nodeId:(uint64_t)nodeId
                          endpointId:(NSUInteger)endpointId
                           clusterId:(NSUInteger)clusterId
                         attributeId:(NSUInteger)attributeId
                               value:(id)value
                          completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    (void) controller;
    __auto_type sharedController = [CHIPDeviceController sharedController];
    if (sharedController) {
        [sharedController getConnectedDevice:nodeId
                                       queue:dispatch_get_main_queue()
                           completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                               if (error) {
                                   NSLog(@"Failed to get connected device");
                                   completion(nil, error);
                               } else {
                                   [device writeAttributeWithEndpointId:endpointId
                                                              clusterId:clusterId
                                                            attributeId:attributeId
                                                                  value:value
                                                            clientQueue:dispatch_get_main_queue()
                                                             completion:completion];
                               }
                           }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)invokeCommandWithController:(id)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                          commandId:(NSUInteger)commandId
                             fields:(id)fields
                         completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion
{
    (void) controller;
    __auto_type sharedController = [CHIPDeviceController sharedController];
    if (sharedController) {
        [sharedController getConnectedDevice:nodeId
                                       queue:dispatch_get_main_queue()
                           completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                               if (error) {
                                   NSLog(@"Failed to get connected device");
                                   completion(nil, error);
                               } else {
                                   [device invokeCommandWithEndpointId:endpointId
                                                             clusterId:clusterId
                                                             commandId:commandId
                                                         commandFields:fields
                                                           clientQueue:dispatch_get_main_queue()
                                                            completion:completion];
                               }
                           }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:CHIPErrorDomain code:CHIPErrorCodeGeneralError userInfo:nil]);
    }
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
    __auto_type sharedController = [CHIPDeviceController sharedController];
    if (sharedController) {
        [sharedController getConnectedDevice:nodeId
                                       queue:dispatch_get_main_queue()
                           completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                               if (error) {
                                   NSLog(@"Failed to get connected device");
                                   establishedHandler();
                                   // Send an error report so that the client knows of the failure
                                   [self.clientProxy handleReportWithController:controller
                                                                         nodeId:nodeId
                                                                          value:nil
                                                                          error:[NSError errorWithDomain:CHIPErrorDomain
                                                                                                    code:CHIPErrorCodeGeneralError
                                                                                                userInfo:nil]];
                               } else {
                                   [device subscribeAttributeWithEndpointId:endpointId
                                                                  clusterId:clusterId
                                                                attributeId:attributeId
                                                                minInterval:minInterval
                                                                maxInterval:maxInterval
                                                                clientQueue:dispatch_get_main_queue()
                                                              reportHandler:^(NSDictionary<NSString *, id> * _Nullable value,
                                                                  NSError * _Nullable error) {
                                                                  [self.clientProxy handleReportWithController:controller
                                                                                                        nodeId:nodeId
                                                                                                         value:value
                                                                                                         error:error];
                                                              }
                                                    subscriptionEstablished:establishedHandler];
                               }
                           }];
    } else {
        NSLog(@"Failed to get shared controller");
        establishedHandler();
        // Send an error report so that the client knows of the failure
        [self.clientProxy handleReportWithController:controller
                                              nodeId:nodeId
                                               value:nil
                                               error:[NSError errorWithDomain:CHIPErrorDomain
                                                                         code:CHIPErrorCodeGeneralError
                                                                     userInfo:nil]];
    }
}

@end

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kCASESetupTimeoutInSeconds = 30;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId = 0x12344321;
static const uint16_t kDiscriminator = 3840;
static const uint32_t kSetupPINCode = 20202021;
static const uint16_t kRemotePort = 5540;
static const uint16_t kLocalPort = 5541;
static NSString * kAddress = @"::1";

// This test suite reuses a device object to speed up the test process for CI.
// The following global variable holds the reference to the device object.
static CHIPDevice * mConnectedDevice;
static CHIPXPCListenerSample * mSampleListener;

static CHIPDevice * GetConnectedDevice(void)
{
    XCTAssertNotNil(mConnectedDevice);
    return mConnectedDevice;
}

@interface CHIPRemoteDeviceSampleTestPairingDelegate : NSObject <CHIPDevicePairingDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation CHIPRemoteDeviceSampleTestPairingDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)onPairingComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

- (void)onCommissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

- (void)onAddressUpdated:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}
@end

@interface CHIPXPCListenerSampleTests : XCTestCase

@end

@implementation CHIPXPCListenerSampleTests

- (void)setUp
{
    [super setUp];
    [self setContinueAfterFailure:NO];
}

- (void)tearDown
{
#if MANUAL_INDIVIDUAL_TEST
    [self shutdownStack];
#endif
    [super tearDown];
}

- (void)initStack
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Pairing Complete"];

    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    CHIPRemoteDeviceSampleTestPairingDelegate * pairing =
        [[CHIPRemoteDeviceSampleTestPairingDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    [controller setListenPort:kLocalPort];
    [controller setPairingDelegate:pairing queue:callbackQueue];

    BOOL started = [controller startup:nil vendorId:0 nocSigner:nil];
    XCTAssertTrue(started);

    NSError * error;
    [controller pairDevice:kDeviceId
                   address:kAddress
                      port:kRemotePort
             discriminator:kDiscriminator
              setupPINCode:kSetupPINCode
                     error:&error];
    XCTAssertEqual(error.code, 0);

    [self waitForExpectationsWithTimeout:kPairingTimeoutInSeconds handler:nil];

    __block XCTestExpectation * connectionExpectation = [self expectationWithDescription:@"CASE established"];
    [controller getConnectedDevice:kDeviceId
                             queue:dispatch_get_main_queue()
                 completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                     XCTAssertEqual(error.code, 0);
                     [connectionExpectation fulfill];
                     connectionExpectation = nil;
                 }];
    [self waitForExpectationsWithTimeout:kCASESetupTimeoutInSeconds handler:nil];

    mSampleListener = [[CHIPXPCListenerSample alloc] init];
    [mSampleListener start];
}

- (void)shutdownStack
{
    [mSampleListener stop];
    mSampleListener = nil;

    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    XCTAssertNotNil(controller);

    BOOL stopped = [controller shutdown];
    XCTAssertTrue(stopped);
}

- (void)waitForCommissionee
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Wait for the commissioned device to be retrieved"];

    dispatch_queue_t queue = dispatch_get_main_queue();
    __auto_type remoteController = [CHIPDeviceController
        sharedControllerWithId:kCHIPDeviceControllerId
               xpcConnectBlock:^NSXPCConnection * _Nonnull {
                   if (mSampleListener.listenerEndpoint) {
                       return [[NSXPCConnection alloc] initWithListenerEndpoint:mSampleListener.listenerEndpoint];
                   }
                   NSLog(@"Listener is not active");
                   return nil;
               }];
    [remoteController getConnectedDevice:kDeviceId
                                   queue:queue
                       completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                           mConnectedDevice = device;
                           [expectation fulfill];
                       }];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test000_SetUp
{
    [self initStack];
    [self waitForCommissionee];
}
#endif

- (void)test001_ReadAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributeWithEndpointId:0xffff
                              clusterId:29
                            attributeId:0
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 29);
                                         XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 0);
                                         XCTAssertTrue([result[@"endpointId"] isKindOfClass:[NSNumber class]]);
                                         XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                         XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Array"]);
                                     }
                                     XCTAssertTrue([resultArray count] > 0);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test002_WriteAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"write LevelControl Brightness attribute"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointId:1
                               clusterId:8
                             attributeId:17
                                   value:writeValue
                             clientQueue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                          XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 8);
                                          XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 17);
                                          XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                      }
                                      XCTAssertEqual([resultArray count], 1);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test003_InvokeCommand
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = [NSDictionary
        dictionaryWithObjectsAndKeys:@"Structure", @"type",
        [NSArray arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:0], @"tag",
                                                [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                                              [NSNumber numberWithUnsignedInteger:0], @"value", nil],
                                                @"value", nil],
                 [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:1], @"tag",
                               [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                             [NSNumber numberWithUnsignedInteger:10], @"value", nil],
                               @"value", nil],
                 nil],
        @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:8
                              commandId:4
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 8);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 4);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

static void (^globalReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

- (void)test004_Subscribe
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device subscribeAttributeWithEndpointId:1
        clusterId:6
        attributeId:0
        minInterval:2
        maxInterval:10
        clientQueue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                globalReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    // Set up expectation for report
    expectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    globalReportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([value isKindOfClass:[NSDictionary class]]);
            NSDictionary * result = value;
            XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
            XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
            XCTAssertEqual([result[@"attributeId"] unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"value"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"value"][@"type"] isEqualToString:@"Boolean"]);
            XCTAssertEqual([result[@"value"][@"value"] boolValue], YES);
        }
        [expectation fulfill];
    };

    // Send command to trigger attribute change
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointId:1
                              clusterId:6
                              commandId:1
                          commandFields:fields
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([CHIPErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         XCTAssertEqual([result[@"endpointId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"clusterId"] unsignedIntegerValue], 6);
                                         XCTAssertEqual([result[@"commandId"] unsignedIntegerValue], 1);
                                         XCTAssertEqual([result[@"status"] unsignedIntegerValue], 0);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test005_ReadAttributeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"read failed"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributeWithEndpointId:0
                              clusterId:10000
                            attributeId:0
                            clientQueue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                 XCTAssertNil(values);
                                 // Error is copied over XPC and hence cannot use CHIPErrorTestUtils utility which checks against a
                                 // local domain string object.
                                 XCTAssertTrue([error.domain isEqualToString:MatterInteractionErrorDomain]);
                                 XCTAssertEqual(error.code, EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER);

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test006_WriteAttributeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"write failed"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointId:1
                               clusterId:8
                             attributeId:10000
                                   value:writeValue
                             clientQueue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertNil(values);
                                  // Error is copied over XPC and hence cannot use CHIPErrorTestUtils utility which checks against a
                                  // local domain string object.
                                  XCTAssertTrue([error.domain isEqualToString:MatterInteractionErrorDomain]);
                                  XCTAssertEqual(error.code, EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE);

                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

#if 0 // Re-enable test if the crash bug in CHIP stack is fixed to handle bad command Id
- (void)test007_InvokeCommandFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary *fields = [NSDictionary dictionaryWithObjectsAndKeys:
                            @"Structure", @"type",
                            [NSArray arrayWithObjects:
                             [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInteger:0], @"tag",
                              [NSDictionary dictionaryWithObjectsAndKeys:
                               @"UnsignedInteger", @"type",
                               [NSNumber numberWithUnsignedInteger:0], @"value", nil], @"value", nil],
                             [NSDictionary dictionaryWithObjectsAndKeys:
                              [NSNumber numberWithUnsignedInteger:1], @"tag",
                              [NSDictionary dictionaryWithObjectsAndKeys:
                               @"UnsignedInteger", @"type",
                               [NSNumber numberWithUnsignedInteger:10], @"value", nil], @"value", nil],
                             nil], @"value", nil];
    [device invokeCommandWithEndpointId:1 clusterId:8 commandId:40000 commandFields:fields clientQueue:queue completion:^(id _Nullable values, NSError * _Nullable error) {
        NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

        XCTAssertNil(values);
        // Error is copied over XPC and hence cannot use CHIPErrorTestUtils utility which checks against a local domain string object.
        XCTAssertTrue([error.domain isEqualToString:MatterInteractionErrorDomain]);
        XCTAssertEqual(error.code, EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);

        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}
#endif

- (void)test008_SubscribeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];

    // Set up expectation for report
    XCTestExpectation * errorReportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    globalReportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertNil(value);
        // Error is copied over XPC and hence cannot use CHIPErrorTestUtils utility which checks against a local domain string
        // object.
        XCTAssertTrue([error.domain isEqualToString:MatterInteractionErrorDomain]);
        XCTAssertEqual(error.code, EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT);
        [errorReportExpectation fulfill];
    };

    CHIPDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device subscribeAttributeWithEndpointId:10000
        clusterId:6
        attributeId:0
        minInterval:2
        maxInterval:10
        clientQueue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                globalReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment and error report
    [self waitForExpectations:[NSArray arrayWithObjects:expectation, errorReportExpectation, nil] timeout:kTimeoutInSeconds];
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test999_TearDown
{
    [self shutdownStack];
}
#endif

@end
