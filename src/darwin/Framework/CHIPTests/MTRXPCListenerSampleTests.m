//
//  MTRRemoteDeviceSampleTests.m
//  MTRTests
/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import "MTRTestResetCommissioneeHelper.h"
#import "MTRTestStorage.h"

#import <math.h> // For INFINITY

// system dependencies
#import <XCTest/XCTest.h>
#import <os/lock.h>

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

// serversLock controls access to _servers.
@property (nonatomic, readonly) os_unfair_lock serversLock;
@end

@implementation MTRXPCListenerSample

- (instancetype)init
{
    if ([super init]) {
        _serviceInterface = [MTRDeviceController xpcInterfaceForServerProtocol];
        _clientInterface = [MTRDeviceController xpcInterfaceForClientProtocol];
        _servers = [NSMutableDictionary dictionary];
        _clusterStateCacheDictionary = [NSMutableDictionary dictionary];
        _xpcListener = [NSXPCListener anonymousListener];
        [_xpcListener setDelegate:(id<NSXPCListenerDelegate>) self];
        _serversLock = OS_UNFAIR_LOCK_INIT;
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

    os_unfair_lock_lock(&_serversLock);
    [_servers setObject:newServer forKey:newServer.identifier];
    os_unfair_lock_unlock(&_serversLock);

    newConnection.invalidationHandler = ^{
        NSLog(@"XPC connection disconnected");
        os_unfair_lock_lock(&self->_serversLock);
        [self.servers removeObjectForKey:newServer.identifier];
        os_unfair_lock_unlock(&self->_serversLock);
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

- (void)getDeviceControllerWithFabricId:(uint64_t)fabricId
                             completion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    // We are using a shared local device controller and hence no disctinction per fabricId.
    (void) fabricId;
    completion(MTRDeviceControllerId, nil);
}

- (void)getAnyDeviceControllerWithCompletion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion
{
    completion(MTRDeviceControllerId, nil);
}

- (void)downloadLogWithController:(id)controller
                           nodeId:(NSNumber *)nodeId
                             type:(MTRDiagnosticLogType)type
                          timeout:(NSTimeInterval)timeout
                       completion:(void (^)(NSString * _Nullable url, NSError * _Nullable error))completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:nodeId controller:sharedController];
        [device downloadLogOfType:type
                          timeout:timeout
                            queue:dispatch_get_main_queue()
                       completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                           completion([url absoluteString], error);
                       }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)readAttributeWithController:(id)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                             params:(NSDictionary<NSString *, id> * _Nullable)params
                         completion:(MTRValuesHandler)completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:sharedController];
        [device
            readAttributesWithEndpointID:endpointId
                               clusterID:clusterId
                             attributeID:attributeId
                                  params:[MTRDeviceController decodeXPCReadParams:params]
                                   queue:dispatch_get_main_queue()
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  completion([MTRDeviceController encodeXPCResponseValues:values], error);
                              }];
    } else {
        NSLog(@"Failed to get shared controller");
        completion(nil, [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil]);
    }
}

- (void)writeAttributeWithController:(id)controller
                              nodeId:(uint64_t)nodeId
                          endpointId:(NSNumber *)endpointId
                           clusterId:(NSNumber *)clusterId
                         attributeId:(NSNumber *)attributeId
                               value:(id)value
                   timedWriteTimeout:(NSNumber *)timeoutMs
                          completion:(MTRValuesHandler)completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:sharedController];
        [device
            writeAttributeWithEndpointID:endpointId
                               clusterID:clusterId
                             attributeID:attributeId
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
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSNumber *)endpointId
                          clusterId:(NSNumber *)clusterId
                          commandId:(NSNumber *)commandId
                             fields:(id)fields
                 timedInvokeTimeout:(NSNumber * _Nullable)timeoutMs
                         completion:(MTRValuesHandler)completion
{
    (void) controller;
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:sharedController];
        [device
            invokeCommandWithEndpointID:endpointId
                              clusterID:clusterId
                              commandID:commandId
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
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                             minInterval:(NSNumber *)minInterval
                             maxInterval:(NSNumber *)maxInterval
                                  params:(NSDictionary<NSString *, id> * _Nullable)params
                      establishedHandler:(dispatch_block_t)establishedHandler
{
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type * subscriptionParams = [MTRDeviceController decodeXPCSubscribeParams:params];
        if (subscriptionParams == nil) {
            subscriptionParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
        } else {
            subscriptionParams.minInterval = minInterval;
            subscriptionParams.maxInterval = maxInterval;
        }
        __auto_type device = [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:sharedController];
        [device subscribeToAttributesWithEndpointID:endpointId
                                          clusterID:clusterId
                                        attributeID:attributeId
                                             params:subscriptionParams
                                              queue:dispatch_get_main_queue()
                                      reportHandler:^(
                                          NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                          [self.clientProxy
                                              handleReportWithController:controller
                                                                  nodeId:nodeId
                                                                  values:[MTRDeviceController encodeXPCResponseValues:values]
                                                                   error:error];
                                      }
                            subscriptionEstablished:establishedHandler];
    } else {
        NSLog(@"Failed to get shared controller");
        establishedHandler();
        // Send an error report so that the client knows of the failure
        [self.clientProxy handleReportWithController:controller
                                              nodeId:nodeId
                                              values:nil
                                               error:[NSError errorWithDomain:MTRErrorDomain
                                                                         code:MTRErrorCodeGeneralError
                                                                     userInfo:nil]];
    }
}

- (void)stopReportsWithController:(id _Nullable)controller nodeId:(uint64_t)nodeId completion:(dispatch_block_t)completion
{
    __auto_type sharedController = sController;
    if (sharedController) {
        __auto_type device = [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:sharedController];
        [device deregisterReportHandlersWithQueue:dispatch_get_main_queue() completion:completion];
    } else {
        NSLog(@"Failed to get shared controller");
        completion();
    }
}

- (void)subscribeWithController:(id _Nullable)controller
                         nodeId:(uint64_t)nodeId
                    minInterval:(NSNumber *)minInterval
                    maxInterval:(NSNumber *)maxInterval
                         params:(NSDictionary<NSString *, id> * _Nullable)params
                    shouldCache:(BOOL)shouldCache
                     completion:(MTRStatusCompletion)completion
{
    __auto_type sharedController = sController;
    if (sharedController) {
        MTRClusterStateCacheContainer * clusterStateCacheContainer;
        if (shouldCache) {
            clusterStateCacheContainer = [[MTRClusterStateCacheContainer alloc] init];
        }

        __auto_type device = [MTRBaseDevice deviceWithNodeID:@(nodeId) controller:sharedController];
        NSMutableArray * established = [NSMutableArray arrayWithCapacity:1];
        [established addObject:@NO];
        __auto_type * subscriptionParams = [MTRDeviceController decodeXPCSubscribeParams:params];
        if (subscriptionParams == nil) {
            subscriptionParams = [[MTRSubscribeParams alloc] initWithMinInterval:minInterval maxInterval:maxInterval];
        } else {
            subscriptionParams.minInterval = minInterval;
            subscriptionParams.maxInterval = maxInterval;
        }
        [device subscribeWithQueue:dispatch_get_main_queue()
            params:subscriptionParams
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
                NSLog(@"Attribute cache subscription succeeded for device %llu", nodeId);
                if (clusterStateCacheContainer) {
                    [self.clusterStateCacheDictionary setObject:clusterStateCacheContainer forKey:@(nodeId)];
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

- (void)readAttributeCacheWithController:(id _Nullable)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSNumber * _Nullable)endpointId
                               clusterId:(NSNumber * _Nullable)clusterId
                             attributeId:(NSNumber * _Nullable)attributeId
                              completion:(MTRValuesHandler)completion
{
    MTRClusterStateCacheContainer * clusterStateCacheContainer
        = _clusterStateCacheDictionary[[NSNumber numberWithUnsignedLongLong:nodeId]];
    if (clusterStateCacheContainer) {
        [clusterStateCacheContainer
            readAttributesWithEndpointID:endpointId
                               clusterID:clusterId
                             attributeID:attributeId
                                   queue:dispatch_get_main_queue()
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  completion([MTRDeviceController encodeXPCResponseValues:values], error);
                              }];
    } else {
        NSLog(@"Attribute cache for node ID %llu was not setup", nodeId);
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

@end

typedef void (^MTRDeviceTestDelegateDataHandler)(NSArray<NSDictionary<NSString *, id> *> *);

@interface MTRXPCDeviceTestDelegate : NSObject <MTRDeviceDelegate>
@property (nonatomic, nullable) MTRDeviceTestDelegateDataHandler onAttributeDataReceived;
@end

@implementation MTRXPCDeviceTestDelegate
- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
    if (self.onAttributeDataReceived != nil) {
        self.onAttributeDataReceived(attributeReport);
    }
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
}

- (BOOL)unitTestShouldSetUpSubscriptionForDevice:(MTRDevice *)device
{
    return NO;
}

- (BOOL)unitTestShouldSkipExpectedValuesForWrite:(MTRDevice *)device
{
    return YES;
}
@end

@interface MTRXPCListenerSampleTests : XCTestCase

@end

static BOOL sStackInitRan = NO;
static BOOL sNeedsStackShutdown = YES;

@implementation MTRXPCListenerSampleTests

+ (void)tearDown
{
    // Global teardown, runs once
    if (sNeedsStackShutdown) {
        // We don't need to worry about ResetCommissionee.  If we get here,
        // we're running only one of our test methods (using
        // -only-testing:MatterTests/MTROTAProviderTests/testMethodName), since
        // we did not run test999_TearDown.
        //        [self shutdownStack];
    }
}

- (void)setUp
{
    // Per-test setup, runs before each test.
    [super setUp];
    [self setContinueAfterFailure:NO];

    if (sStackInitRan == NO) {
        [self initStack];
        [self waitForCommissionee];
    }
}

- (void)tearDown
{
    // Per-test teardown, runs after each test.
    [super tearDown];
}

- (void)initStack
{
    sStackInitRan = YES;

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

+ (void)shutdownStack
{
    sNeedsStackShutdown = NO;

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

- (void)test000_SetUp
{
    // Nothing to do here; our setUp method handled this already.  This test
    // just exists to make the setup not look like it's happening inside other
    // tests.
}

- (void)test001_ReadAttribute
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:nil
                               clusterID:@29
                             attributeID:@0
                                  params:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                  XCTAssertNil(error);

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

                                  XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeToAttributesWithEndpointID:@1
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
        XCTAssertNil(error);
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

                                 XCTAssertNil(error);

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
    XCTestExpectation * expectation = [self expectationWithDescription:@"read failed"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:@0
                               clusterID:@10000
                             attributeID:@0
                                  params:nil
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                  XCTAssertNil(error);
                                  XCTAssertNotNil(values);

                                  {
                                      XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                      NSArray * resultArray = values;
                                      XCTAssertEqual([resultArray count], 1);
                                      NSDictionary * result = resultArray[0];

                                      MTRAttributePath * path = result[@"attributePath"];
                                      XCTAssertEqual(path.endpoint.unsignedIntegerValue, 0);
                                      XCTAssertEqual(path.cluster.unsignedIntegerValue, 10000);
                                      XCTAssertEqual(path.attribute.unsignedIntegerValue, 0);
                                      XCTAssertNotNil(result[@"error"]);
                                      XCTAssertNil(result[@"data"]);
                                      XCTAssertTrue([result[@"error"] isKindOfClass:[NSError class]]);
                                      XCTAssertTrue([MTRErrorTestUtils error:result[@"error"] isInteractionModelError:MTRInteractionErrorCodeUnsupportedCluster]);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test006_WriteAttributeFailure
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"write failed"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * writeValue = [NSDictionary
        dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithUnsignedInteger:200], @"value", nil];
    [device
        writeAttributeWithEndpointID:@1
                           clusterID:@8
                         attributeID:@10000
                               value:writeValue
                   timedWriteTimeout:nil
                               queue:queue
                          completion:^(id _Nullable values, NSError * _Nullable error) {
                              NSLog(@"write attribute: Brightness values: %@, error: %@", values, error);

                              XCTAssertNil(values);
                              XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeUnsupportedAttribute]);

                              [expectation fulfill];
                          }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test007_InvokeCommandFailure
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke command with invalid command id"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = [NSDictionary
        dictionaryWithObjectsAndKeys:@"Structure", @"type",
        [NSArray arrayWithObjects:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:0], @"contextTag",
                                                [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                                              [NSNumber numberWithUnsignedInteger:0], @"value", nil],
                                                @"data", nil],
                 [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithUnsignedInteger:1], @"contextTag",
                               [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                             [NSNumber numberWithUnsignedInteger:10], @"value", nil],
                               @"data", nil],
                 nil],
        @"value", nil];
    [device
        invokeCommandWithEndpointID:@1
                          clusterID:@8
                          commandID:@(0xff)
                      commandFields:fields
                 timedInvokeTimeout:nil
                              queue:queue
                         completion:^(id _Nullable values, NSError * _Nullable error) {
                             NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                             XCTAssertNil(values);
                             XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeUnsupportedCommand]);

                             [expectation fulfill];
                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test008_SubscribeFailure
{
    // Set up expectation for report
    XCTestExpectation * errorReportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        // Because our subscription has no existent paths, it gets an
        // InvalidAction response.
        XCTAssertNil(values);
        XCTAssertTrue([MTRErrorTestUtils error:error isInteractionModelError:MTRInteractionErrorCodeInvalidAction]);
        [errorReportExpectation fulfill];
    };

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    params.resubscribeAutomatically = NO;
    [device subscribeToAttributesWithEndpointID:@10000
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
            XCTFail("Should not have a subscriptionEstablished for an error case");
        }];

    // Wait till establishment and error report
    [self waitForExpectations:@[ errorReportExpectation ] timeout:kTimeoutInSeconds];
}

- (void)test009_ReadAttributeWithParams
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRReadParams * readParams = [[MTRReadParams alloc] init];
    readParams.filterByFabric = NO;
    [device readAttributesWithEndpointID:nil
                               clusterID:@29
                             attributeID:@0
                                  params:readParams
                                   queue:queue
                              completion:^(id _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                                  XCTAssertNil(error);

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
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);
            XCTAssertNil(error);

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
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@8
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"2nd subscriber report attribute: CurrentLevel values: %@, error: %@", values, error);
            XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
        XCTAssertNil(error);

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
        XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);
            XCTAssertNil(error);

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
    myParams.replaceExistingSubscriptions = YES;
    subscribeExpectation = [self expectationWithDescription:@"subscribe CurrentLevel attribute"];
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@8
        attributeID:@0
        params:myParams
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"2nd subscriber report attribute: CurrentLevel values: %@, error: %@", values, error);
            XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
        XCTAssertNil(error);

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
        XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);
            XCTAssertNil(error);

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
    myParams.replaceExistingSubscriptions = NO;
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@8
        attributeID:@0
        params:myParams
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"2nd subscriber report attribute: CurrentLevel values: %@, error: %@", values, error);
            XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
        XCTAssertNil(error);

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
        XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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

                                  XCTAssertNil(error);

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

                                  XCTAssertNil(error);

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

#if 0
    // The above attribute isn't for timed interaction. Hence, no verification till we have a capable attribute.
    // subscribe, which should get the new value at the timeout
    expectation = [self expectationWithDescription:@"Subscribed"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error);
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(2) maxInterval:@(10)];
    [device subscribeToAttributesWithEndpointID:@1
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
        XCTAssertNil(error);
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
    [device readAttributesWithEndpointID:@1
                               clusterID:@8
                             attributeID:@17
                                  params:nil
                                   queue:queue
                              completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                                  NSLog(@"read attribute: LevelControl Brightness values: %@, error: %@", values, error);
                                  XCTAssertNil(error);
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

                                 XCTAssertNil(error);

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

- (void)test015_MTRDeviceInteraction
{
    __auto_type * device = [MTRDevice deviceWithNodeID:@(kDeviceId) controller:mDeviceController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    __auto_type * delegate = [[MTRXPCDeviceTestDelegate alloc] init];
    [device setDelegate:delegate queue:queue];

    __auto_type * endpoint = @(1);

    __auto_type * onOffCluster = [[MTRClusterOnOff alloc] initWithDevice:device endpointID:endpoint queue:queue];

    // The previous tests have left us in a not-so-great state where the device
    // is (1) on and (2) in the middle of a level move.  Reset to a known state
    // where the device is off, the level is midway, so it's not doing either on
    // or off due to the level move, and there is no level move going on.
    XCTestExpectation * initialOffExpectation = [self expectationWithDescription:@"Turning off to reset to base state"];
    [onOffCluster offWithParams:nil expectedValues:nil expectedValueInterval:nil completion:^(NSError * error) {
        XCTAssertNil(error);
        [initialOffExpectation fulfill];
    }];
    [self waitForExpectations:@[ initialOffExpectation ] timeout:kTimeoutInSeconds];

    __auto_type * levelCluster = [[MTRClusterLevelControl alloc] initWithDevice:device endpointID:endpoint queue:queue];
    XCTestExpectation * initialLevelExpectation = [self expectationWithDescription:@"Setting midpoint level"];
    __auto_type * params = [[MTRLevelControlClusterMoveToLevelParams alloc] init];
    params.level = @(128);
    params.transitionTime = @(0);
    params.optionsMask = @(MTRLevelControlOptionsExecuteIfOff);
    params.optionsOverride = @(MTRLevelControlOptionsExecuteIfOff);
    [levelCluster moveToLevelWithParams:params expectedValues:nil expectedValueInterval:nil completion:^(NSError * error) {
        XCTAssertNil(error);
        [initialLevelExpectation fulfill];
    }];
    [self waitForExpectations:@[ initialLevelExpectation ] timeout:kTimeoutInSeconds];

    // Since we have no subscription, sync reads don't really work right.  After doing
    // them, if we get a value that does not match expectation we need to wait for our
    // delegate to be notified about the attribute.
    __auto_type waitForValue = ^(NSNumber * attributeID, NSDictionary<NSString *, id> * (^readBlock)(void), NSNumber * value) {
        XCTestExpectation * expectation = [self expectationWithDescription:[NSString stringWithFormat:@"Waiting for attribute %@=%@", attributeID, value]];
        __auto_type * path = [MTRAttributePath attributePathWithEndpointID:endpoint clusterID:@(MTRClusterIDTypeOnOffID) attributeID:attributeID];

        __block __auto_type checkValue = ^(NSDictionary<NSString *, id> * responseValue) {
            if (![path isEqual:responseValue[MTRAttributePathKey]]) {
                // Not our attribute.
                return NO;
            }

            NSError * error;
            __auto_type * report = [[MTRAttributeReport alloc] initWithResponseValue:responseValue error:&error];
            XCTAssertNil(error);
            XCTAssertNotNil(report);
            XCTAssertNil(report.error);
            XCTAssertNotNil(report.value);

            if ([report.value isEqual:value]) {
                delegate.onAttributeDataReceived = nil;
                [expectation fulfill];
                return YES;
            }

            // Keep waiting.
            return NO;
        };

        delegate.onAttributeDataReceived = ^(NSArray<NSDictionary<NSString *, id> *> * responseValues) {
            for (NSDictionary<NSString *, id> * responseValue in responseValues) {
                if (checkValue(responseValue)) {
                    return;
                }
            }
        };

        __auto_type * attrValue = readBlock();
        if (attrValue != nil) {
            __auto_type * responseValue = @{
                MTRAttributePathKey : path,
                MTRDataKey : attrValue,
            };

            checkValue(responseValue);
        }

        [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
    };

    // Wait until the OnOff value is read.  But issue reads for multiple
    // attributes, so that we test what happens if multiple reads are issued in
    // a row.  The attribute we care about should be last, so it gets batched
    // with the others, and we do more than 2 reads so that even if the first
    // one is dispatched immediately the others batch.  Make sure none of the
    // other reads involve attributes we care about later in this test.
    waitForValue(
        @(MTRAttributeIDTypeClusterOnOffAttributeOnOffID), ^{
            [onOffCluster readAttributeOffWaitTimeWithParams:nil];
            [onOffCluster readAttributeGlobalSceneControlWithParams:nil];
            [onOffCluster readAttributeStartUpOnOffWithParams:nil];
            return [onOffCluster readAttributeOnOffWithParams:nil];
        }, @(NO));

    waitForValue(
        @(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID), ^{
            return [onOffCluster readAttributeOnTimeWithParams:nil];
        }, @(0));

    // Test that writes work.
    [onOffCluster writeAttributeOnTimeWithValue:@{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(100),
    }
                          expectedValueInterval:@(0)];

    // Wait until the new value is read.
    waitForValue(
        @(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID), ^{
            return [onOffCluster readAttributeOnTimeWithParams:nil];
        }, @(100));

    [onOffCluster writeAttributeOnTimeWithValue:@{
        MTRTypeKey : MTRUnsignedIntegerValueType,
        MTRValueKey : @(0),
    }
                          expectedValueInterval:@(0)];

    // Now wait until the new value is read.
    waitForValue(
        @(MTRAttributeIDTypeClusterOnOffAttributeOnTimeID), ^{
            return [onOffCluster readAttributeOnTimeWithParams:nil];
        }, @(0));

    // Test that invokes work.
    XCTestExpectation * onExpectation = [self expectationWithDescription:@"Turning on"];
    [onOffCluster onWithParams:nil expectedValues:nil expectedValueInterval:nil completion:^(NSError * error) {
        XCTAssertNil(error);
        [onExpectation fulfill];
    }];
    [self waitForExpectations:@[ onExpectation ] timeout:kTimeoutInSeconds];

    waitForValue(
        @(MTRAttributeIDTypeClusterOnOffAttributeOnOffID), ^{
            return [onOffCluster readAttributeOnOffWithParams:nil];
        }, @(YES));

    XCTestExpectation * offExpectation = [self expectationWithDescription:@"Turning off"];
    [onOffCluster offWithParams:nil expectedValues:nil expectedValueInterval:nil completion:^(NSError * error) {
        XCTAssertNil(error);
        [offExpectation fulfill];
    }];
    [self waitForExpectations:@[ offExpectation ] timeout:kTimeoutInSeconds];

    waitForValue(
        @(MTRAttributeIDTypeClusterOnOffAttributeOnOffID), ^{
            return [onOffCluster readAttributeOnOffWithParams:nil];
        }, @(NO));
}

- (void)test016_DownloadLog
{
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"Download EndUserSupport log"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device downloadLogOfType:MTRDiagnosticLogTypeEndUserSupport
                      timeout:10
                        queue:queue
                   completion:^(NSURL * _Nullable url, NSError * _Nullable error) {
                       NSLog(@"downloadLogOfType: url: %@, error: %@", url, error);
                       XCTAssertNil(error);

                       NSError * readError;
                       NSString * fileContent = [NSString stringWithContentsOfURL:url encoding:NSUTF8StringEncoding error:&readError];
                       XCTAssertNil(readError);
                       XCTAssertEqualObjects(fileContent, @"This is a simple log\n");
                       [expectation fulfill];
                   }];
    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test900_SubscribeClusterStateCache
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe attributes by cache"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRClusterStateCacheContainer * clusterStateCacheContainer = [[MTRClusterStateCacheContainer alloc] init];
    NSLog(@"Setting up attribute cache subscription...");
    __auto_type * params = [[MTRSubscribeParams alloc] initWithMinInterval:@(0) maxInterval:@(60)];
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

                                 XCTAssertNil(error);

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

                                 XCTAssertNil(error);

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
        readAttributesWithEndpointID:@1
                           clusterID:@6
                         attributeID:@0
                               queue:queue
                          completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                              NSLog(@"Cached attribute read: %@, error: %@", values, error);
                              XCTAssertNil(error);
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

- (void)test999_TearDown
{
    ResetCommissionee(
        [MTRBaseDevice deviceWithNodeID:@(kDeviceId) controller:sController], dispatch_get_main_queue(), self, kTimeoutInSeconds);
    [[self class] shutdownStack];
}

@end
