//
//  MTRRemoteDeviceSampleTests.m
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
#import <Matter/MTRBaseDevice.h>
#import <Matter/Matter.h>

#import "MTRErrorTestUtils.h"
#import "MTRTestKeys.h"
#import "MTRTestStorage.h"

#import <app/util/af-enums.h>

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>

// Set the following to 1 in order to run individual test case manually.
#define MANUAL_INDIVIDUAL_TEST 0

static uint16_t kTestVendorId = 0xFFF1u;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

//
// Sample XPC Listener implementation that directly communicates with local CHIPDevice instance
//
// Note that real implementation could look almost the same as the sample if the remote device controller object
// is in a separate process in the same machine.
// If the remote device controller object is in a remote machine, the server protocol must implement
// routing the requests to the remote object in a remote machine using implementation specific transport protocol
// between the two machines.

@interface MTRXPCListenerSample<NSXPCListenerDelegate> : NSObject

@property (nonatomic, readonly, getter=listenerEndpoint) NSXPCListenerEndpoint * listenerEndpoint;

- (void)start;
- (void)stop;

@end

@interface MTRDeviceControllerServerSample<MatterDeviceControllerServerProtocol> : NSObject
@property (nonatomic, readonly, strong) NSString * identifier;
- (instancetype)initWithClientProxy:(id<MTRDeviceControllerClientProtocol>)proxy
        clusterStateCacheDictionary:(NSMutableDictionary<NSNumber *, MTRClusterStateCacheContainer *> *)cacheDictionary;
@end

@interface MTRXPCListenerSample ()

@property (nonatomic, readonly, strong) NSString * controllerId;
@property (nonatomic, readonly, strong) NSXPCInterface * serviceInterface;
@property (nonatomic, readonly, strong) NSXPCInterface * clientInterface;
@property (nonatomic, readonly, strong) NSXPCListener * xpcListener;
@property (nonatomic, readonly, strong) NSMutableDictionary<NSString *, MTRDeviceControllerServerSample *> * servers;
@property (nonatomic, readonly, strong)
    NSMutableDictionary<NSNumber *, MTRClusterStateCacheContainer *> * clusterStateCacheDictionary;

@end

@implementation MTRXPCListenerSample

- (instancetype)init
{
    if ([super init]) {
        _serviceInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRDeviceControllerServerProtocol)];
        _clientInterface = [NSXPCInterface interfaceWithProtocol:@protocol(MTRDeviceControllerClientProtocol)];
        _servers = [NSMutableDictionary dictionary];
        _clusterStateCacheDictionary = [NSMutableDictionary dictionary];
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
    __auto_type newServer = [[MTRDeviceControllerServerSample alloc] initWithClientProxy:[newConnection remoteObjectProxy]
                                                             clusterStateCacheDictionary:_clusterStateCacheDictionary];
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

@interface MTRDeviceControllerServerSample ()
@property (nonatomic, readwrite, strong) id<MTRDeviceControllerClientProtocol> clientProxy;
@property (nonatomic, readonly, strong)
    NSMutableDictionary<NSNumber *, MTRClusterStateCacheContainer *> * clusterStateCacheDictionary;
@end

// This sample does not have multiple controllers and hence controller Id shall be the same.
static NSString * const MTRDeviceControllerId = @"MTRController";

@implementation MTRDeviceControllerServerSample

- (instancetype)initWithClientProxy:(id<MTRDeviceControllerClientProtocol>)proxy
        clusterStateCacheDictionary:(NSMutableDictionary<NSNumber *, MTRClusterStateCacheContainer *> *)cacheDictionary
{
    if ([super init]) {
        _clientProxy = proxy;
        _identifier = [[NSUUID UUID] UUIDString];
        _clusterStateCacheDictionary = cacheDictionary;
    }
    return self;
}

- (void)getDeviceControllerWithFabricID:(NSNumber *)fabricID
                             completion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    // We are using a shared local device controller and hence no disctinction per fabricID.
    (void) fabricID;
    completion(MTRDeviceControllerId, nil);
}

- (void)getAnyDeviceControllerWithCompletion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    completion(MTRDeviceControllerId, nil);
}

- (void)readAttributeWithController:(id)controller
                             nodeID:(NSNumber *)nodeID
                         endpointID:(NSNumber * _Nullable)endpointID
                          clusterID:(NSNumber * _Nullable)clusterID
                        attributeID:(NSNumber * _Nullable)attributeID
                             params:(NSDictionary<NSString *, id> * _Nullable)params
                         completion:(MTRValuesHandler)completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sharedController];
        [device readAttributePathWithEndpointID:endpointID
                                      clusterID:clusterID
                                    attributeID:attributeID
                                         params:[MTRDeviceController decodeXPCReadParams:params]
                                          queue:dispatch_get_main_queue()
                                     completion:^(
                                         NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                         completion([MTRDeviceController encodeXPCResponseValues:values], error);
                                     }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)writeAttributeWithController:(id)controller
                              nodeID:(NSNumber *)nodeID
                          endpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
                   timedWriteTimeout:(NSNumber *)timeoutMs
                          completion:(MTRValuesHandler)completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sharedController];
        [device
            writeAttributeWithEndpointID:endpointID
                               clusterID:clusterID
                             attributeID:attributeID
                                   value:value
                       timedWriteTimeout:timeoutMs
                                   queue:dispatch_get_main_queue()
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  completion([MTRDeviceController encodeXPCResponseValues:values], error);
                              }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)invokeCommandWithController:(id)controller
                             nodeID:(NSNumber *)nodeID
                         endpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                             fields:(id)fields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                         completion:(MTRValuesHandler)completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sharedController];
        [device
            invokeCommandWithEndpointID:endpointID
                              clusterID:clusterID
                              commandID:commandID
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:dispatch_get_main_queue()
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 completion([MTRDeviceController encodeXPCResponseValues:values], error);
                             }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)subscribeAttributeWithController:(id)controller
                                  nodeID:(NSNumber *)nodeID
                              endpointID:(NSNumber * _Nullable)endpointID
                               clusterID:(NSNumber * _Nullable)clusterID
                             attributeID:(NSNumber * _Nullable)attributeID
                                  params:(NSDictionary<NSString *, id> *)params
                      establishedHandler:(dispatch_block_t)establishedHandler
{
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sharedController];
        [device subscribeAttributePathWithEndpointID:endpointID
                                           clusterID:clusterID
                                         attributeID:attributeID
                                              params:[MTRDeviceController decodeXPCSubscribeParams:params]
                                               queue:dispatch_get_main_queue()
                                       reportHandler:^(
                                           NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                           [self.clientProxy
                                               handleReportWithController:controller
                                                                   nodeID:nodeID
                                                                   values:[MTRDeviceController encodeXPCResponseValues:values]
                                                                    error:error];
                                       }
                             subscriptionEstablished:establishedHandler];
    } else {
        NSLog(@"Failed to get shared controller");
        establishedHandler();
        // Send an error report so that the client knows of the failure
        [self.clientProxy handleReportWithController:controller
                                              nodeID:nodeID
                                              values:nil
                                               error:[NSError errorWithDomain:MTRErrorDomain
                                                                         code:MTRErrorCodeGeneralError
                                                                     userInfo:nil]];
    }
}

- (void)stopReportsWithController:(id _Nullable)controller nodeID:(NSNumber *)nodeID completion:(dispatch_block_t)completion
{
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sharedController];
        [device deregisterReportHandlersWithQueue:dispatch_get_main_queue() completion:completion];
    } else {
        NSLog(@"Failed to get shared controller");
        completion();
    }
}

- (void)subscribeWithController:(id _Nullable)controller
                         nodeID:(NSNumber *)nodeID
                         params:(NSDictionary<NSString *, id> *)params
                    shouldCache:(BOOL)shouldCache
                     completion:(MTRStatusCompletion)completion
{
    __auto_type sharedController = sController;
    if (sharedController) {
        MTRClusterStateCacheContainer * clusterStateCacheContainer;
        if (shouldCache) {
            clusterStateCacheContainer = [[MTRClusterStateCacheContainer alloc] init];
        }

        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeID controller:sharedController];
        NSMutableArray * established = [NSMutableArray arrayWithCapacity:1];
        [established addObject:@NO];
        [device subscribeWithQueue:dispatch_get_main_queue()
            params:[MTRDeviceController decodeXPCSubscribeParams:params]
            clusterStateCacheContainer:clusterStateCacheContainer
            attributeReportHandler:^(NSArray * value) {
                NSLog(@"Received report: %@", value);
            }
            eventReportHandler:nil
            errorHandler:^(NSError * error) {
                NSLog(@"Received report error: %@", error);
                if (![established[0] boolValue]) {
                    established[0] = @YES;
                    completion(error);
                }
            }
            subscriptionEstablished:^() {
                NSLog(@"Attribute cache subscription succeeded for device %llu", [nodeID unsignedLongLongValue]);
                if (clusterStateCacheContainer) {
                    [self.clusterStateCacheDictionary setObject:clusterStateCacheContainer forKey:nodeID];
                }
                if (![established[0] boolValue]) {
                    established[0] = @YES;
                    completion(nil);
                }
            }
            resubscriptionScheduled:nil];
    } else {
        NSLog(@"Failed to get shared controller");
        completion([NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)readClusterStateCacheWithController:(id _Nullable)controller
                                     nodeID:(NSNumber *)nodeID
                                 endpointID:(NSNumber * _Nullable)endpointID
                                  clusterID:(NSNumber * _Nullable)clusterID
                                attributeID:(NSNumber * _Nullable)attributeID
                                 completion:(MTRValuesHandler)completion
{
    MTRClusterStateCacheContainer * clusterStateCacheContainer = _clusterStateCacheDictionary[nodeID];
    if (clusterStateCacheContainer) {
        [clusterStateCacheContainer
            readAttributePathWithEndpointID:endpointID
                                  clusterID:clusterID
                                attributeID:attributeID
                                      queue:dispatch_get_main_queue()
                                 completion:^(
                                     NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                     completion([MTRDeviceController encodeXPCResponseValues:values], error);
                                 }];
    } else {
        NSLog(@"Attribute cache for node ID %llu was not setup", [nodeID unsignedLongLongValue]);
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

@end

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;

// This test suite reuses a device object to speed up the test process for CI.
// The following global variable holds the reference to the device object.
static MTRBaseDevice * mConnectedDevice;
static MTRDeviceController * mDeviceController;
static MTRXPCListenerSample * mSampleListener;

static MTRBaseDevice * GetConnectedDevice(void)
{
    XCTAssertNotNil(mConnectedDevice);
    return mConnectedDevice;
}

@interface MTRRemoteDeviceSampleTestDeviceControllerDelegate : NSObject <MTRDeviceControllerDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTRRemoteDeviceSampleTestDeviceControllerDelegate
- (id)initWithExpectation:(XCTestExpectation *)expectation
{
    self = [super init];
    if (self) {
        _expectation = expectation;
    }
    return self;
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    NSError * commissionError = nil;
    [sController commissionNodeWithID:@(kDeviceId)
                  commissioningParams:[[MTRCommissioningParameters alloc] init]
                                error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for controller:commissioningComplete
}

- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError *)error
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

@interface MTRXPCListenerSampleTests : XCTestCase

@end

@implementation MTRXPCListenerSampleTests

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

    __auto_type * factory = [MTRDeviceControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRDeviceControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    NSError * error;
    BOOL ok = [factory startControllerFactory:factoryParams error:&error];
    XCTAssertTrue(ok);
    XCTAssertNil(error);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithIPK:testKeys.ipk fabricID:@(1) nocSigner:testKeys];
    params.vendorID = @(kTestVendorId);

    MTRDeviceController * controller = [factory createControllerOnNewFabric:params error:&error];
    XCTAssertNotNil(controller);
    XCTAssertNil(error);

    sController = controller;

    MTRRemoteDeviceSampleTestDeviceControllerDelegate * deviceControllerDelegate =
        [[MTRRemoteDeviceSampleTestDeviceControllerDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.device_controller_delegate", DISPATCH_QUEUE_SERIAL);

    [controller setDeviceControllerDelegate:deviceControllerDelegate queue:callbackQueue];

    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectationsWithTimeout:kPairingTimeoutInSeconds handler:nil];

    mSampleListener = [[MTRXPCListenerSample alloc] init];
    [mSampleListener start];
}

- (void)shutdownStack
{
    [mSampleListener stop];
    mSampleListener = nil;

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRDeviceControllerFactory sharedInstance] stopControllerFactory];

    mDeviceController = nil;
}

- (void)waitForCommissionee
{
    __auto_type remoteController = [MTRDeviceController
        sharedControllerWithID:MTRDeviceControllerId
               xpcConnectBlock:^NSXPCConnection * _Nonnull {
                   if (mSampleListener.listenerEndpoint) {
                       return [[NSXPCConnection alloc] initWithListenerEndpoint:mSampleListener.listenerEndpoint];
                   }
                   NSLog(@"Listener is not active");
                   return nil;
               }];
    mConnectedDevice = [MTRBaseDevice deviceWithNodeID:@(kDeviceId) controller:remoteController];
    mDeviceController = remoteController;
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

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributePathWithEndpointID:nil
                                  clusterID:@29
                                attributeID:@0
                                     params:nil
                                      queue:queue
                                 completion:^(id _Nullable values, NSError * _Nullable error) {
                                     NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                     XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                     {
                                         XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                         NSArray * resultArray = values;
                                         for (NSDictionary * result in resultArray) {
                                             MTRAttributePath * path = result[@"attributePath"];
                                             XCTAssertEqual([path.cluster unsignedIntegerValue], 29);
                                             XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
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

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointID:@1
                               clusterID:@8
                             attributeID:@17
                                   value:writeValue
                       timedWriteTimeout:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          MTRAttributePath * path = result[@"attributePath"];
                                          XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                          XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                          XCTAssertEqual([path.attribute unsignedIntegerValue], 17);
                                          XCTAssertNil(result[@"error"]);
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

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[
            @{ @"contextTag" : @0, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @0 } },
            @{ @"contextTag" : @1, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @10 } }
        ]
    };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@8
                              commandID:@4
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 4);
                                         XCTAssertNil(result[@"error"]);
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

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
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
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);

        for (NSDictionary * result in values) {
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
            XCTAssertEqual([result[@"data"][@"value"] boolValue], YES);
        }
        [expectation fulfill];
    };

    // Send command to trigger attribute change
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    XCTestExpectation * clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test005_ReadAttributeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"read failed"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributePathWithEndpointID:@0
                                  clusterID:@10000
                                attributeID:@0
                                     params:nil
                                      queue:queue
                                 completion:^(id _Nullable values, NSError * _Nullable error) {
                                     NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                     XCTAssertNil(values);
                                     // Error is copied over XPC and hence cannot use MTRErrorTestUtils utility which checks against
                                     // a local domain string object.
                                     XCTAssertTrue([error.domain isEqualToString:MTRInteractionErrorDomain]);
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

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device writeAttributeWithEndpointID:@1
                               clusterID:@8
                             attributeID:@10000
                                   value:writeValue
                       timedWriteTimeout:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertNil(values);
                                  // Error is copied over XPC and hence cannot use MTRErrorTestUtils utility which checks against a
                                  // local domain string object.
                                  XCTAssertTrue([error.domain isEqualToString:MTRInteractionErrorDomain]);
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

    MTRBaseDevice * device = GetConnectedDevice();
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
    [device invokeCommandWithEndpointID:@1 clusterID:@8 commandID:@40000 commandFields:fields queue:queue
            timedInvokeTimeout:nil
           completion:^(id _Nullable values, NSError * _Nullable error) {
               NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

               XCTAssertNil(values);
        // Error is copied over XPC and hence cannot use MTRErrorTestUtils utility which checks against a local domain string object.
        XCTAssertTrue([error.domain isEqualToString:MTRInteractionErrorDomain]);
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
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        // Because our subscription has no existent paths, it gets an
        // InvalidAction response, which is EMBER_ZCL_STATUS_MALFORMED_COMMAND.
        XCTAssertNil(values);
        // Error is copied over XPC and hence cannot use MTRErrorTestUtils utility which checks against a local domain string
        // object.
        XCTAssertTrue([error.domain isEqualToString:MTRInteractionErrorDomain]);
        XCTAssertEqual(error.code, EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        [errorReportExpectation fulfill];
    };

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    params.autoResubscribe = NO;
    [device subscribeAttributePathWithEndpointID:@10000
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
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

- (void)test009_ReadAttributeWithParams
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRReadParams * readParams = [[MTRReadParams alloc] init];
    readParams.fabricFiltered = NO;
    [device readAttributePathWithEndpointID:nil
                                  clusterID:@29
                                attributeID:@0
                                     params:readParams
                                      queue:queue
                                 completion:^(id _Nullable values, NSError * _Nullable error) {
                                     NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                     XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                     {
                                         XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                         NSArray * resultArray = values;
                                         for (NSDictionary * result in resultArray) {
                                             MTRAttributePath * path = result[@"attributePath"];
                                             XCTAssertEqual([path.cluster unsignedIntegerValue], 29);
                                             XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
                                             XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                             XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Array"]);
                                         }
                                         XCTAssertTrue([resultArray count] > 0);
                                     }

                                     [expectation fulfill];
                                 }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test010_SubscribeWithNoParams
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];

    __block void (^firstReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;
    __block void (^secondReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    // Subscribe
    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);
            XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

            if (firstReportHandler) {
                __auto_type callback = firstReportHandler;
                firstReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [subscribeExpectation fulfill];
        }];

    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    // Setup 2nd subscriber
    subscribeExpectation = [self expectationWithDescription:@"subscribe CurrentLevel attribute"];
    params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@8
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"2nd subscriber report attribute: CurrentLevel values: %@, error: %@", values, error);
            XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

            if (secondReportHandler) {
                __auto_type callback = secondReportHandler;
                secondReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"2nd subscribe attribute: CurrentLevel established");
            [subscribeExpectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    // Send command to clear attribute state
    XCTestExpectation * clearCommandExpectation = [self expectationWithDescription:@"Clearing command invoked"];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:@{ @"type" : @"Structure", @"value" : @[] }
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [clearCommandExpectation fulfill];
                             }];
    [self waitForExpectations:@[ clearCommandExpectation ] timeout:kTimeoutInSeconds];

    // Set up expectations for report
    XCTestExpectation * reportExpectation =
        [self expectationWithDescription:@"The 1st subscriber unexpectedly received OnOff attribute report"];
    reportExpectation.inverted = YES;
    firstReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([values isKindOfClass:[NSArray class]]);
            NSDictionary * result = values[0];
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
            XCTAssertEqual([result[@"data"][@"value"] boolValue], YES);
        }
        [reportExpectation fulfill];
    };

    XCTestExpectation * secondReportExpectation =
        [self expectationWithDescription:@"The 2nd subscriber received CurrentLevel attribute report"];
    secondReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([values isKindOfClass:[NSArray class]]);
            NSDictionary * result = values[0];
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"UnsignedInteger"]);
            XCTAssertNotNil(result[@"data"][@"value"]);
        }
        [secondReportExpectation fulfill];
    };

    // Send command to trigger attribute change
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:@[ reportExpectation, secondReportExpectation ] timeout:kTimeoutInSeconds];

    clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test011_SubscribeWithParams
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];

    __block void (^firstReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;
    __block void (^secondReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    // Subscribe
    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);
            XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

            if (firstReportHandler) {
                __auto_type callback = firstReportHandler;
                firstReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [subscribeExpectation fulfill];
        }];

    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    // Setup 2nd subscriber
    MTRSubscribeParams * myParams = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    myParams.keepPreviousSubscriptions = NO;
    subscribeExpectation = [self expectationWithDescription:@"subscribe CurrentLevel attribute"];
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@8
        attributeID:@0
        params:myParams
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"2nd subscriber report attribute: CurrentLevel values: %@, error: %@", values, error);
            XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

            if (secondReportHandler) {
                __auto_type callback = secondReportHandler;
                secondReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"2nd subscribe attribute: CurrentLevel established");
            [subscribeExpectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    // Send command to clear attribute state
    XCTestExpectation * clearCommandExpectation = [self expectationWithDescription:@"Clearing command invoked"];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:@{ @"type" : @"Structure", @"value" : @[] }
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [clearCommandExpectation fulfill];
                             }];
    [self waitForExpectations:@[ clearCommandExpectation ] timeout:kTimeoutInSeconds];

    // Set up expectations for report
    XCTestExpectation * reportExpectation =
        [self expectationWithDescription:@"The 1st subscriber unexpectedly received OnOff attribute report"];
    reportExpectation.inverted = YES;
    firstReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([values isKindOfClass:[NSArray class]]);
            NSDictionary * result = values[0];
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
            XCTAssertEqual([result[@"data"][@"value"] boolValue], YES);
        }
        [reportExpectation fulfill];
    };

    XCTestExpectation * secondReportExpectation =
        [self expectationWithDescription:@"The 2nd subscriber received CurrentLevel attribute report"];
    secondReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([values isKindOfClass:[NSArray class]]);
            NSDictionary * result = values[0];
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"UnsignedInteger"]);
            XCTAssertNotNil(result[@"data"][@"value"]);
        }
        [secondReportExpectation fulfill];
    };

    // Send command to trigger attribute change
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:@[ reportExpectation, secondReportExpectation ] timeout:kTimeoutInSeconds];

    clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];

    clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test012_SubscribeKeepingPreviousSubscription
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];

    __block void (^firstReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;
    __block void (^secondReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    // Subscribe
    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);
            XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

            if (firstReportHandler) {
                __auto_type callback = firstReportHandler;
                firstReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [subscribeExpectation fulfill];
        }];

    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    // Setup 2nd subscriber
    subscribeExpectation = [self expectationWithDescription:@"subscribe CurrentLevel attribute"];
    MTRSubscribeParams * myParams = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    myParams.keepPreviousSubscriptions = YES;
    [device subscribeAttributePathWithEndpointID:@1
        clusterID:@8
        attributeID:@0
        params:myParams
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"2nd subscriber report attribute: CurrentLevel values: %@, error: %@", values, error);
            XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

            if (secondReportHandler) {
                __auto_type callback = secondReportHandler;
                secondReportHandler = nil;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"2nd subscribe attribute: CurrentLevel established");
            [subscribeExpectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:@[ subscribeExpectation ] timeout:kTimeoutInSeconds];

    // Send command to clear attribute state
    XCTestExpectation * clearCommandExpectation = [self expectationWithDescription:@"Clearing command invoked"];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:@{ @"type" : @"Structure", @"value" : @[] }
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [clearCommandExpectation fulfill];
                             }];
    [self waitForExpectations:@[ clearCommandExpectation ] timeout:kTimeoutInSeconds];

    // Set up expectations for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"The 1st subscriber received OnOff attribute report"];
    firstReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([values isKindOfClass:[NSArray class]]);
            NSDictionary * result = values[0];
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
            XCTAssertEqual([result[@"data"][@"value"] boolValue], YES);
        }
        [reportExpectation fulfill];
    };

    XCTestExpectation * secondReportExpectation =
        [self expectationWithDescription:@"The 2nd subscriber received CurrentLevel attribute report"];
    secondReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

        {
            XCTAssertTrue([values isKindOfClass:[NSArray class]]);
            NSDictionary * result = values[0];
            MTRAttributePath * path = result[@"attributePath"];
            XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
            XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
            XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
            XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
            XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"UnsignedInteger"]);
            XCTAssertNotNil(result[@"data"][@"value"]);
        }
        [secondReportExpectation fulfill];
    };

    // Send command to trigger attribute change
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:@[ reportExpectation, secondReportExpectation ] timeout:kTimeoutInSeconds];

    clearExpectation = [self expectationWithDescription:@"report handlers deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [clearExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ clearExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test013_TimedWriteAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Write an initial value
    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    XCTestExpectation * expectation = [self expectationWithDescription:@"Wrote LevelControl Brightness attribute"];
    [device writeAttributeWithEndpointID:@1
                               clusterID:@8
                             attributeID:@17
                                   value:writeValue
                       timedWriteTimeout:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          MTRAttributePath * path = result[@"attributePath"];
                                          XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                          XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                          XCTAssertEqual([path.attribute unsignedIntegerValue], 17);
                                          XCTAssertNil(result[@"error"]);
                                      }
                                      XCTAssertEqual([resultArray count], 1);
                                  }

                                  [expectation fulfill];
                              }];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    // Request a timed write with a new value
    writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:100], @"value", nil];
    expectation = [self expectationWithDescription:@"Requested timed write on LevelControl Brightness attribute"];
    [device writeAttributeWithEndpointID:@1
                               clusterID:@8
                             attributeID:@17
                                   value:writeValue
                       timedWriteTimeout:@1000
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"Timed-write attribute: Brightness values: %@, error: %@", values, error);

                                  XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      for (NSDictionary * result in resultArray) {
                                          MTRAttributePath * path = result[@"attributePath"];
                                          XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                          XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                          XCTAssertEqual([path.attribute unsignedIntegerValue], 17);
                                          XCTAssertNil(result[@"error"]);
                                      }
                                      XCTAssertEqual([resultArray count], 1);
                                  }

                                  [expectation fulfill];
                              }];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

#if 0 // The above attribute isn't for timed interaction. Hence, no verification till we have a capable attribute.
    // subscribe, which should get the new value at the timeout
    expectation = [self expectationWithDescription:@"Subscribed"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error);
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeAttributePathWithEndpointID:@1
            clusterID:@8
            attributeID:@17
            params:params
            queue:queue
           reportHandler:^(id _Nullable value, NSError * _Nullable error) {
               NSLog(@"report attribute: Brightness values: %@, error: %@", value, error);

               if (reportHandler) {
            __auto_type callback = reportHandler;
            callback = nil;
            callback(value, error);
        }
    }
    subscriptionEstablished:^ {
        NSLog(@"subscribe attribute: Brightness established");
        [expectation fulfill];
    }];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    // Setup report expectation
    expectation = [self expectationWithDescription:@"Report received"];
    reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 17);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"UnsignedInteger"]);
        XCTAssertEqual([result[@"data"][@"value"] unsignedIntegerValue], 100);
        [expectation fulfill];
    };
    // Wait for report
    [self waitForExpectationsWithTimeout:(kTimeoutInSeconds + 1) handler:nil];
#endif

    // Read back to see if the timed write has taken effect
    expectation = [self expectationWithDescription:@"Read LevelControl Brightness attribute after pause"];
    [device
        readAttributePathWithEndpointID:@1
                              clusterID:@8
                            attributeID:@17
                                 params:nil
                                  queue:queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"read attribute: LevelControl Brightness values: %@, error: %@", values, error);
                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
                                 for (NSDictionary<NSString *, id> * value in values) {
                                     MTRAttributePath * path = value[@"attributePath"];
                                     XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                                     XCTAssertEqual([path.cluster unsignedLongValue], 8);
                                     XCTAssertEqual([path.attribute unsignedLongValue], 17);
                                     XCTAssertTrue([value[@"data"][@"type"] isEqualToString:@"UnsignedInteger"]);
                                     XCTAssertEqual([value[@"data"][@"value"] unsignedIntegerValue], 100);
                                 }
                                 [expectation fulfill];
                             }];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test014_TimedInvokeCommand
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke MoveToLevelWithOnOff command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[
            @{ @"contextTag" : @0, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @0 } },
            @{ @"contextTag" : @1, @"data" : @ { @"type" : @"UnsignedInteger", @"value" : @10 } }
        ]
    };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@8
                              commandID:@4
                          commandFields:fields
                     timedInvokeTimeout:@1000
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 8);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 4);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }

                                 [expectation fulfill];
                             }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
    sleep(1);
}

- (void)test900_SubscribeClusterStateCache
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe attributes by cache"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRClusterStateCacheContainer * clusterStateCacheContainer = [[MTRClusterStateCacheContainer alloc] init];
    NSLog(@"Setting up attribute cache subscription...");
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(1) maxInterval:@(60)];
    [device subscribeWithQueue:queue
        params:params
        clusterStateCacheContainer:clusterStateCacheContainer
        attributeReportHandler:^(NSArray * value) {
            NSLog(@"Report for attribute cache: %@", value);
        }
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received error for attribute cache: %@", error);
        }
        subscriptionEstablished:^() {
            NSLog(@"Attribute cache subscribed attributes");
            [expectation fulfill];
        }
        resubscriptionScheduled:nil];
    // Wait for subscription establishment. This can take very long to collect initial reports.
    NSLog(@"Waiting for initial report...");
    [self waitForExpectations:@[ expectation ] timeout:120];

    // Send command to reset attribute state
    NSLog(@"Invoking clearing command...");
    expectation = [self expectationWithDescription:@"Clearing command invoked"];
    NSDictionary * fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoked command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [expectation fulfill];
                             }];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];

    // Send command to trigger attribute change
    NSLog(@"Invoking command to trigger report...");
    expectation = [self expectationWithDescription:@"Command invoked"];
    fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@1
                          commandFields:fields
                     timedInvokeTimeout:nil
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoked command: On values: %@, error: %@", values, error);

                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);

                                 {
                                     XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                     NSArray * resultArray = values;
                                     for (NSDictionary * result in resultArray) {
                                         MTRCommandPath * path = result[@"commandPath"];
                                         XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                         XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                         XCTAssertEqual([path.command unsignedIntegerValue], 1);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [expectation fulfill];
                             }];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];

    // Read attribute cache
    sleep(1);
    NSLog(@"Reading from attribute cache...");
    expectation = [self expectationWithDescription:@"Cache read"];
    [clusterStateCacheContainer
        readAttributePathWithEndpointID:@1
                              clusterID:@6
                            attributeID:@0
                                  queue:queue
                             completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"Cached attribute read: %@, error: %@", values, error);
                                 XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
                                 XCTAssertEqual([values count], 1);
                                 for (NSDictionary<NSString *, id> * value in values) {
                                     XCTAssertTrue([value isKindOfClass:[NSDictionary class]]);
                                     NSDictionary * result = value;
                                     MTRAttributePath * path = result[@"attributePath"];
                                     XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                     XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
                                     XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
                                     XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                     XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
                                     XCTAssertEqual([result[@"data"][@"value"] boolValue], YES);
                                 }
                                 [expectation fulfill];
                             }];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test999_TearDown
{
    [self shutdownStack];
}
#endif

@end
