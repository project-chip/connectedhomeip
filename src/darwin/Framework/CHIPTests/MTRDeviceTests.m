//
//  MTRDeviceTests.m
//  MTRDeviceTests
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
#import <Matter/MTRAttributeCacheContainer.h>
#import <Matter/MTRBaseClusters.h>
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

static const uint16_t kPairingTimeoutInSeconds = 10;
static const uint16_t kCASESetupTimeoutInSeconds = 30;
static const uint16_t kTimeoutInSeconds = 3;
static const uint64_t kDeviceId = 0x12344321;
static NSString * kOnboardingPayload = @"MT:-24J0AFN00KA0648G00";
static const uint16_t kLocalPort = 5541;
static const uint16_t kTestVendorId = 0xFFF1u;

// This test suite reuses a device object to speed up the test process for CI.
// The following global variable holds the reference to the device object.
static MTRBaseDevice * mConnectedDevice;

// Singleton controller we use.
static MTRDeviceController * sController = nil;

// Keys we can use to restart the controller.
static MTRTestKeys * sTestKeys = nil;

static void WaitForCommissionee(XCTestExpectation * expectation)
{
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    [controller getBaseDevice:kDeviceId
                        queue:dispatch_get_main_queue()
                   completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                       XCTAssertEqual(error.code, 0);
                       [expectation fulfill];
                       mConnectedDevice = device;
                   }];
}

static MTRBaseDevice * GetConnectedDevice(void)
{
    XCTAssertNotNil(mConnectedDevice);
    return mConnectedDevice;
}

#ifdef DEBUG
@interface MTRBaseDevice (Test)
- (void)failSubscribers:(dispatch_queue_t)queue completion:(void (^)(void))completion;
@end
#endif

@interface MTRDeviceTestPairingDelegate : NSObject <MTRDevicePairingDelegate>
@property (nonatomic, strong) XCTestExpectation * expectation;
@end

@implementation MTRDeviceTestPairingDelegate
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

    NSError * commissionError = nil;
    [sController commissionDevice:kDeviceId commissioningParams:[[MTRCommissioningParameters alloc] init] error:&commissionError];
    XCTAssertNil(commissionError);

    // Keep waiting for onCommissioningComplete
}

- (void)onCommissioningComplete:(NSError *)error
{
    XCTAssertEqual(error.code, 0);
    [_expectation fulfill];
    _expectation = nil;
}

@end

@interface MTRDeviceTestDelegate : NSObject <MTRDeviceDelegate>
@property (nonatomic) dispatch_block_t onSubscriptionEstablished;
@end

@implementation MTRDeviceTestDelegate
- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state
{
    if (state == MTRDeviceStateReachable) {
        self.onSubscriptionEstablished();
    }
}

- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport
{
}

- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport
{
}

@end

@interface MTRDeviceTests : XCTestCase
@end

@implementation MTRDeviceTests

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

    __auto_type * factory = [MTRControllerFactory sharedInstance];
    XCTAssertNotNil(factory);

    __auto_type * storage = [[MTRTestStorage alloc] init];
    __auto_type * factoryParams = [[MTRControllerFactoryParams alloc] initWithStorage:storage];
    factoryParams.port = @(kLocalPort);

    BOOL ok = [factory startup:factoryParams];
    XCTAssertTrue(ok);

    __auto_type * testKeys = [[MTRTestKeys alloc] init];
    XCTAssertNotNil(testKeys);

    sTestKeys = testKeys;

    // Needs to match what startControllerOnExistingFabric calls elsewhere in
    // this file do.
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:testKeys
                                                                                   fabricID:@(1)
                                                                                        ipk:testKeys.ipk];
    params.vendorID = @(kTestVendorId);

    MTRDeviceController * controller = [factory startControllerOnNewFabric:params];
    XCTAssertNotNil(controller);

    sController = controller;

    MTRDeviceTestPairingDelegate * pairing = [[MTRDeviceTestPairingDelegate alloc] initWithExpectation:expectation];
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.pairing", DISPATCH_QUEUE_SERIAL);

    [controller setPairingDelegate:pairing queue:callbackQueue];

    NSError * error;
    __auto_type * payload = [MTRSetupPayload setupPayloadWithOnboardingPayload:kOnboardingPayload error:&error];
    XCTAssertNotNil(payload);
    XCTAssertNil(error);

    [controller setupCommissioningSessionWithPayload:payload newNodeID:@(kDeviceId) error:&error];
    XCTAssertNil(error);

    [self waitForExpectationsWithTimeout:kPairingTimeoutInSeconds handler:nil];

    __block XCTestExpectation * connectionExpectation = [self expectationWithDescription:@"CASE established"];
    [controller getBaseDevice:kDeviceId
                        queue:dispatch_get_main_queue()
                   completion:^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
                       XCTAssertEqual(error.code, 0);
                       [connectionExpectation fulfill];
                       connectionExpectation = nil;
                   }];
    [self waitForExpectationsWithTimeout:kCASESetupTimeoutInSeconds handler:nil];
}

- (void)shutdownStack
{
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    [controller shutdown];
    XCTAssertFalse([controller isRunning]);

    [[MTRControllerFactory sharedInstance] shutdown];
}

- (void)waitForCommissionee
{
    XCTestExpectation * expectation = [self expectationWithDescription:@"Wait for the commissioned device to be retrieved"];

    WaitForCommissionee(expectation);
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

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:nil
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

- (void)test004_InvokeTimedCommand
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke Off command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[],
    };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
                          commandFields:fields
                     timedInvokeTimeout:@10000
                                  queue:queue
                             completion:^(id _Nullable values, NSError * _Nullable error) {
                                 NSLog(@"invoke command: Off values: %@, error: %@", values, error);

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

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

static void (^globalReportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

- (void)test005_Subscribe
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        minInterval:@1
        maxInterval:@10
        params:nil
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];

    // Set up expectation for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"report received"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
        if ([result[@"data"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            globalReportHandler = nil;
        }
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : [NSArray array] };
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
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Set up expectation for 2nd report
    reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    globalReportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
        if ([result[@"data"][@"value"] boolValue] == NO) {
            [reportExpectation fulfill];
            globalReportHandler = nil;
        }
    };

    // Send command to trigger attribute change
    fields = [NSDictionary dictionaryWithObjectsAndKeys:@"Structure", @"type", [NSArray array], @"value", nil];
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
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
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                             }];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    expectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [expectation fulfill];
                                   }];
    [self waitForExpectations:@[ expectation ] timeout:kTimeoutInSeconds];
}

- (void)test006_ReadAttributeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"read failed"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device
        readAttributesWithEndpointID:@0
                           clusterID:@10000
                         attributeID:@0
                              params:nil
                               queue:queue
                          completion:^(id _Nullable values, NSError * _Nullable error) {
                              NSLog(@"read attribute: DeviceType values: %@, error: %@", values, error);

                              XCTAssertNil(values);
                              XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER);

                              [expectation fulfill];
                          }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test007_WriteAttributeFailure
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
                              XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE);

                              [expectation fulfill];
                          }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test008_InvokeCommandFailure
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
    [device
        invokeCommandWithEndpointID:@1
                          clusterID:@8
                          commandID:@40000
                      commandFields:fields
                 timedInvokeTimeout:nil
                              queue:queue
                         completion:^(id _Nullable values, NSError * _Nullable error) {
                             NSLog(@"invoke command: MoveToLevelWithOnOff values: %@, error: %@", values, error);

                             XCTAssertNil(values);
                             XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND);

                             [expectation fulfill];
                         }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test009_SubscribeFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    // Set up expectation for report
    XCTestExpectation * errorReportExpectation = [self expectationWithDescription:@"receive subscription error"];
    reportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        // Because our subscription has no existent paths, it gets an
        // InvalidAction response, which is EMBER_ZCL_STATUS_MALFORMED_COMMAND.
        XCTAssertNil(value);
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], EMBER_ZCL_STATUS_MALFORMED_COMMAND);
        [errorReportExpectation fulfill];
    };

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * cleanSubscriptionExpectation = [self expectationWithDescription:@"Previous subscriptions cleaned"];
    NSLog(@"Deregistering report handlers...");
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       NSLog(@"Report handlers deregistered");
                                       [cleanSubscriptionExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ cleanSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    __auto_type * params = [[MTRSubscribeParams alloc] init];
    params.autoResubscribe = @(NO);
    [device subscribeToAttributesWithEndpointID:@10000
        clusterID:@6
        attributeID:@0
        minInterval:@2
        maxInterval:@10
        params:params
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (reportHandler) {
                __auto_type callback = reportHandler;
                reportHandler = nil;
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

- (void)test010_ReadAllAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation =
        [self expectationWithDescription:@"read DeviceDescriptor DeviceType attribute for all endpoints"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    [device readAttributesWithEndpointID:@1
                               clusterID:@29
                             attributeID:nil
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
                                          XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
                                          XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
                                      }
                                      XCTAssertTrue([resultArray count] > 0);
                                  }

                                  [expectation fulfill];
                              }];

    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];
}

- (void)test011_ReadCachedAttribute
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();
    XCTestExpectation * cleanSubscriptionExpectation = [self expectationWithDescription:@"Previous subscriptions cleaned"];
    NSLog(@"Deregistering report handlers...");
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       NSLog(@"Report handlers deregistered");
                                       [cleanSubscriptionExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ cleanSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    __auto_type attributeCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    XCTestExpectation * subscribeExpectation = [self expectationWithDescription:@"Subscription complete"];

    NSLog(@"Subscribing...");
    // reportHandler returns TRUE if it got the things it was looking for or if there's an error.
    __block BOOL (^reportHandler)(NSArray * _Nullable value, NSError * _Nullable error);
    [device subscribeWithQueue:queue
        minInterval:@(2)
        maxInterval:@(60)
        params:nil
        attributeCacheContainer:attributeCacheContainer
        attributeReportHandler:^(NSArray * value) {
            NSLog(@"Received report: %@", value);
            if (reportHandler) {
                __auto_type handler = reportHandler;
                reportHandler = nil;
                BOOL done = handler(value, nil);
                if (done == NO) {
                    // Keep waiting.
                    reportHandler = handler;
                }
            }
        }
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received report error: %@", error);
            if (reportHandler) {
                __auto_type handler = reportHandler;
                reportHandler = nil;
                handler(nil, error);
            }
        }
        subscriptionEstablished:^() {
            [subscribeExpectation fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ subscribeExpectation ] timeout:60];

    // Invoke command to set the attribute to a known state
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"Command invoked"];
    MTRBaseClusterOnOff * cluster = [[MTRBaseClusterOnOff alloc] initWithDevice:device endpointID:@(1) queue:queue];
    XCTAssertNotNil(cluster);

    NSLog(@"Invoking command...");
    [cluster onWithCompletion:^(NSError * _Nullable err) {
        NSLog(@"Invoked command with error: %@", err);
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:err], 0);
        [commandExpectation fulfill];
    }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait till reports arrive from accessory.
    NSLog(@"Waiting for reports from accessory...");
    sleep(5);

    // Read cache
    NSLog(@"Reading from cache...");
    XCTestExpectation * cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [MTRBaseClusterOnOff readAttributeOnOffWithAttributeCache:attributeCacheContainer
                                                     endpoint:@1
                                                        queue:queue
                                                   completion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
                                                       NSLog(@"Read attribute cache value: %@, error: %@", value, err);
                                                       XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:err], 0);
                                                       XCTAssertTrue([value isEqualToNumber:[NSNumber numberWithBool:YES]]);
                                                       [cacheExpectation fulfill];
                                                   }];
    [self waitForExpectations:[NSArray arrayWithObject:cacheExpectation] timeout:kTimeoutInSeconds];

    // Add another subscriber of the attribute to verify that attribute cache still works when there are other subscribers.
    NSLog(@"New subscription...");
    XCTestExpectation * newSubscriptionEstablished = [self expectationWithDescription:@"New subscription established"];
    MTRSubscribeParams * params = [[MTRSubscribeParams alloc] init];
    params.keepPreviousSubscriptions = [NSNumber numberWithBool:YES];
    [cluster subscribeAttributeOnOffWithMinInterval:[NSNumber numberWithUnsignedShort:2]
        maxInterval:[NSNumber numberWithUnsignedShort:60]
        params:params
        subscriptionEstablished:^{
            NSLog(@"New subscription was established");
            [newSubscriptionEstablished fulfill];
        }
        reportHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
            NSLog(@"New subscriber received a report: %@, error: %@", value, error);
        }];
    [self waitForExpectations:[NSArray arrayWithObject:newSubscriptionEstablished] timeout:kTimeoutInSeconds];

    __auto_type reportExpectation = [self expectationWithDescription:@"Report handler called"];
    reportHandler = ^(NSArray * _Nullable value, NSError * _Nullable error) {
        if (error != nil) {
            return YES;
        }
        NSLog(@"Report received: %@, error: %@", value, error);
        for (MTRAttributeReport * report in value) {
            if ([report.path.endpoint isEqualToNumber:@1] && [report.path.cluster isEqualToNumber:@6] &&
                [report.path.attribute isEqualToNumber:@0]) {
                NSLog(@"Report value for OnOff: %@", report.value);
                XCTAssertNotNil(report.value);
                XCTAssertTrue([report.value isKindOfClass:[NSNumber class]]);
                XCTAssertEqual([report.value boolValue], NO);
                [reportExpectation fulfill];
                return YES;
            }
        }

        return NO;
    };

    NSLog(@"Invoking another command...");
    commandExpectation = [self expectationWithDescription:@"Command invoked"];
    [cluster offWithCompletion:^(NSError * _Nullable err) {
        NSLog(@"Invoked command with error: %@", err);
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:err], 0);
        [commandExpectation fulfill];
    }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait till reports arrive from accessory.
    NSLog(@"Waiting for reports from accessory...");
    [self waitForExpectations:@[ reportExpectation ] timeout:kTimeoutInSeconds];

    NSLog(@"Disconnect accessory to test cache...");
    __auto_type idleExpectation = [self expectationWithDescription:@"Must not break out of idle"];
    idleExpectation.inverted = YES;
    [self waitForExpectations:[NSArray arrayWithObject:idleExpectation] timeout:10];

    // Read cache
    NSLog(@"Reading from cache...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [MTRBaseClusterOnOff readAttributeOnOffWithAttributeCache:attributeCacheContainer
                                                     endpoint:@1
                                                        queue:queue
                                                   completion:^(NSNumber * _Nullable value, NSError * _Nullable err) {
                                                       NSLog(@"Read attribute cache value: %@, error: %@", value, err);
                                                       XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:err], 0);
                                                       XCTAssertTrue([value isEqualToNumber:[NSNumber numberWithBool:NO]]);
                                                       [cacheExpectation fulfill];
                                                   }];
    [self waitForExpectations:[NSArray arrayWithObject:cacheExpectation] timeout:kTimeoutInSeconds];

    // Read from cache using generic path
    NSLog(@"Reading from cache using generic path...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [attributeCacheContainer
        readAttributeWithEndpointID:@1
                          clusterID:@6
                        attributeID:@0
                              queue:queue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read attribute cache value: %@, error %@", values, error);
                             XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
                             XCTAssertEqual([values count], 1);
                             MTRAttributePath * path = values[0][@"attributePath"];
                             XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                             XCTAssertEqual([path.cluster unsignedLongValue], 6);
                             XCTAssertEqual([path.attribute unsignedLongValue], 0);
                             XCTAssertNil(values[0][@"error"]);
                             XCTAssertTrue([values[0][@"data"][@"type"] isEqualToString:@"Boolean"]);
                             XCTAssertEqual([values[0][@"data"][@"value"] boolValue], NO);
                             [cacheExpectation fulfill];
                         }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard endpoint...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [attributeCacheContainer
        readAttributeWithEndpointID:nil
                          clusterID:@6
                        attributeID:@0
                              queue:queue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read attribute cache value: %@, error %@", values, error);
                             XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
                             XCTAssertTrue([values count] > 0);
                             for (NSDictionary<NSString *, id> * value in values) {
                                 MTRAttributePath * path = value[@"attributePath"];
                                 XCTAssertEqual([path.cluster unsignedLongValue], 6);
                                 XCTAssertEqual([path.attribute unsignedLongValue], 0);
                                 XCTAssertNil(value[@"error"]);
                             }
                             [cacheExpectation fulfill];
                         }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard cluster ID...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [attributeCacheContainer
        readAttributeWithEndpointID:@1
                          clusterID:nil
                        attributeID:@0
                              queue:queue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read attribute cache value: %@, error %@", values, error);
                             XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
                             XCTAssertTrue([values count] > 0);
                             for (NSDictionary<NSString *, id> * value in values) {
                                 MTRAttributePath * path = value[@"attributePath"];
                                 XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                                 XCTAssertEqual([path.attribute unsignedLongValue], 0);
                             }
                             [cacheExpectation fulfill];
                         }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard attribute ID...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [attributeCacheContainer
        readAttributeWithEndpointID:@1
                          clusterID:@6
                        attributeID:nil
                              queue:queue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read attribute cache value: %@, error %@", values, error);
                             XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
                             XCTAssertTrue([values count] > 0);
                             for (NSDictionary<NSString *, id> * value in values) {
                                 MTRAttributePath * path = value[@"attributePath"];
                                 XCTAssertEqual([path.endpoint unsignedShortValue], 1);
                                 XCTAssertEqual([path.cluster unsignedLongValue], 6);
                                 XCTAssertNil(value[@"error"]);
                             }
                             [cacheExpectation fulfill];
                         }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];

    // Read from cache with wildcard path
    NSLog(@"Reading from cache using wildcard endpoint ID and cluster ID...");
    cacheExpectation = [self expectationWithDescription:@"Attribute cache read"];
    [attributeCacheContainer
        readAttributeWithEndpointID:nil
                          clusterID:nil
                        attributeID:@0
                              queue:queue
                         completion:^(NSArray<NSDictionary<NSString *, id> *> * _Nullable values, NSError * _Nullable error) {
                             NSLog(@"Read attribute cache value: %@, error %@", values, error);
                             XCTAssertNotNil(error);
                             [cacheExpectation fulfill];
                         }];
    [self waitForExpectations:@[ cacheExpectation ] timeout:kTimeoutInSeconds];
}

#ifdef DEBUG
// Test an error to subscription
- (void)test012_SubscriptionError
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();
    XCTestExpectation * deregisterExpectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [deregisterExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ deregisterExpectation ] timeout:kTimeoutInSeconds];

    // Subscribe
    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0
        minInterval:@1
        maxInterval:@10
        params:nil
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"report attribute: OnOff values: %@, error: %@", values, error);

            if (globalReportHandler) {
                __auto_type callback = globalReportHandler;
                callback(values, error);
            }
        }
        subscriptionEstablished:^{
            NSLog(@"subscribe attribute: OnOff established");
            [expectation fulfill];
        }];

    // Wait till establishment
    [self waitForExpectations:[NSArray arrayWithObject:expectation] timeout:kTimeoutInSeconds];

    // Set up expectation for report
    XCTestExpectation * reportExpectation = [self expectationWithDescription:@"report received"];
    globalReportHandler = ^(id _Nullable value, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([value isKindOfClass:[NSArray class]]);
        NSDictionary * result = value[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertEqual([path.attribute unsignedIntegerValue], 0);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        XCTAssertTrue([result[@"data"][@"type"] isEqualToString:@"Boolean"]);
        if ([result[@"data"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            globalReportHandler = nil;
        }
    };

    // Send commands to trigger attribute change
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : [NSArray array] };
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
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Trigger reader failure
    XCTestExpectation * failureExpectation = [self expectationWithDescription:@"failed on purpose"];
    [device failSubscribers:queue
                 completion:^{
                     [failureExpectation fulfill];
                 }];
    [self waitForExpectations:@[ failureExpectation ] timeout:kTimeoutInSeconds];

    deregisterExpectation = [self expectationWithDescription:@"Report handler deregistered"];
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       [deregisterExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ deregisterExpectation ] timeout:kTimeoutInSeconds];
}
#endif

- (void)test013_ReuseChipClusterObject
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);

    __block MTRBaseDevice * device;
    __block XCTestExpectation * connectionExpectation = [self expectationWithDescription:@"CASE established"];
    [controller getBaseDevice:kDeviceId
                        queue:dispatch_get_main_queue()
                   completion:^(MTRBaseDevice * _Nullable retrievedDevice, NSError * _Nullable error) {
                       XCTAssertEqual(error.code, 0);
                       [connectionExpectation fulfill];
                       connectionExpectation = nil;
                       device = retrievedDevice;
                   }];
    [self waitForExpectationsWithTimeout:kCASESetupTimeoutInSeconds handler:nil];

    XCTestExpectation * expectation = [self expectationWithDescription:@"ReuseMTRClusterObjectFirstCall"];

    dispatch_queue_t queue = dispatch_get_main_queue();
    MTRBaseClusterTestCluster * cluster = [[MTRBaseClusterTestCluster alloc] initWithDevice:device endpointID:@(1) queue:queue];
    XCTAssertNotNil(cluster);

    [cluster testWithCompletion:^(NSError * err) {
        NSLog(@"ReuseMTRClusterObject test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];

    expectation = [self expectationWithDescription:@"ReuseMTRClusterObjectSecondCall"];

    // Reuse the MTRCluster Object for multiple times.

    [cluster testWithCompletion:^(NSError * err) {
        NSLog(@"ReuseMTRClusterObject test Error: %@", err);
        XCTAssertEqual(err.code, 0);
        [expectation fulfill];
    }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test014_InvokeCommandWithDifferentIdResponse
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    XCTestExpectation * expectation = [self expectationWithDescription:@"invoke Off command"];

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    NSDictionary * fields = @{
        @"type" : @"Structure",
        @"value" : @[],
    };
    // KeySetReadAllIndices in the Group Key Management has id 4 and a data response with id 5
    [device
        invokeCommandWithEndpointID:@0
                          clusterID:@(0x003F)
                          commandID:@4
                      commandFields:fields
                 timedInvokeTimeout:nil
                              queue:queue
                         completion:^(id _Nullable values, NSError * _Nullable error) {
                             NSLog(@"invoke command: KeySetReadAllIndices values: %@, error: %@", values, error);

                             XCTAssertNil(error);

                             {
                                 XCTAssertTrue([values isKindOfClass:[NSArray class]]);
                                 NSArray * resultArray = values;
                                 for (NSDictionary * result in resultArray) {
                                     MTRCommandPath * path = result[MTRCommandPathKey];
                                     XCTAssertEqual([path.endpoint unsignedIntegerValue], 0);
                                     XCTAssertEqual([path.cluster unsignedIntegerValue], 0x003F);
                                     XCTAssertEqual([path.command unsignedIntegerValue], 5);
                                     // We expect a KeySetReadAllIndicesResponse struct,
                                     // which has context tag 0 pointing to a list with one
                                     // item: 0 (the IPK's keyset id).
                                     NSDictionary * expectedResult = @{
                                         MTRTypeKey : MTRStructureValueType,
                                         MTRValueKey : @[ @{
                                             MTRContextTagKey : @0,
                                             MTRDataKey : @ {
                                                 MTRTypeKey : MTRArrayValueType,
                                                 MTRValueKey : @[ @{
                                                     MTRDataKey : @ { MTRTypeKey : MTRUnsignedIntegerValueType, MTRValueKey : @0 }
                                                 } ]
                                             }
                                         } ],
                                     };
                                     XCTAssertEqualObjects(result[MTRDataKey], expectedResult);
                                     XCTAssertNil(result[MTRErrorKey]);
                                 }
                                 XCTAssertEqual([resultArray count], 1);
                             }

                             [expectation fulfill];
                         }];

    [self waitForExpectationsWithTimeout:kTimeoutInSeconds handler:nil];
}

- (void)test015_FailedSubscribeWithQueueAcrossShutdown
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    XCTestExpectation * firstSubscribeExpectation = [self expectationWithDescription:@"First subscription complete"];
    XCTestExpectation * errorExpectation = [self expectationWithDescription:@"First subscription errored out"];

    // Create first subscription.  It needs to be using subscribeWithQueue and
    // must have a clusterStateCacheContainer to exercise the onDone case.
    NSLog(@"Subscribing...");
    __auto_type clusterStateCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    __auto_type * params = [[MTRSubscribeParams alloc] init];
    params.autoResubscribe = @(NO);
    [device subscribeWithQueue:queue
        minInterval:1
        maxInterval:2
        params:params
        cacheContainer:clusterStateCacheContainer
        attributeReportHandler:nil
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received report error: %@", error);

            // Restart the controller here, to exercise our various event queue bits.
            [controller shutdown];

            // Wait a bit before restart, to allow whatever async things are going on after this is called to try to happen.
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), queue, ^{
                __auto_type * factory = [MTRControllerFactory sharedInstance];
                XCTAssertNotNil(factory);

                // Needs to match what initStack does.
                __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:sTestKeys
                                                                                               fabricId:1
                                                                                                    ipk:sTestKeys.ipk];
                __auto_type * newController = [factory startControllerOnExistingFabric:params];
                XCTAssertNotNil(newController);

                sController = newController;

                WaitForCommissionee(errorExpectation);
            });
        }
        subscriptionEstablished:^() {
            [firstSubscribeExpectation fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ firstSubscribeExpectation ] timeout:60];

    // Create second subscription which will cancel the first subscription.  We
    // can use a non-existent path here to cut down on the work that gets done.
    [device subscribeAttributeWithEndpointId:@10000
                                   clusterId:@6
                                 attributeId:@0
                                 minInterval:@(1)
                                 maxInterval:@(2)
                                      params:params
                                 clientQueue:queue
                               reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                               }
                     subscriptionEstablished:^() {
                     }];
    [self waitForExpectations:@[ errorExpectation ] timeout:60];
}

- (void)test016_FailedSubscribeWithCacheReadDuringFailure
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif

    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    MTRDeviceController * controller = sController;
    XCTAssertNotNil(controller);
    XCTestExpectation * firstSubscribeExpectation = [self expectationWithDescription:@"First subscription complete"];
    XCTestExpectation * errorExpectation = [self expectationWithDescription:@"First subscription errored out"];

    // Create first subscription.  It needs to be using subscribeWithQueue and
    // must have a clusterStateCacheContainer to exercise the onDone case.
    NSLog(@"Subscribing...");
    __auto_type clusterStateCacheContainer = [[MTRAttributeCacheContainer alloc] init];
    __auto_type * params = [[MTRSubscribeParams alloc] init];
    params.autoResubscribe = @(NO);
    [device subscribeWithQueue:queue
        minInterval:1
        maxInterval:2
        params:params
        cacheContainer:clusterStateCacheContainer
        attributeReportHandler:nil
        eventReportHandler:nil
        errorHandler:^(NSError * error) {
            NSLog(@"Received report error: %@", error);

            [MTRBaseClusterOnOff readAttributeOnOffWithAttributeCache:clusterStateCacheContainer
                                                             endpoint:@1
                                                                queue:queue
                                                    completionHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                                                        [errorExpectation fulfill];
                                                    }];
        }
        subscriptionEstablished:^() {
            [firstSubscribeExpectation fulfill];
        }
        resubscriptionScheduled:nil];
    [self waitForExpectations:@[ firstSubscribeExpectation ] timeout:60];

    // Create second subscription which will cancel the first subscription.  We
    // can use a non-existent path here to cut down on the work that gets done.
    [device subscribeAttributeWithEndpointId:@10000
                                   clusterId:@6
                                 attributeId:@0
                                 minInterval:@(1)
                                 maxInterval:@(2)
                                      params:params
                                 clientQueue:queue
                               reportHandler:^(id _Nullable values, NSError * _Nullable error) {
                               }
                     subscriptionEstablished:^() {
                     }];
    [self waitForExpectations:@[ errorExpectation ] timeout:60];
}

- (void)test017_TestMTRDeviceBasics
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif

    __auto_type * device = [MTRDevice deviceWithNodeID:kDeviceId deviceController:sController];
    dispatch_queue_t queue = dispatch_get_main_queue();

    XCTestExpectation * subscriptionExpectation = [self expectationWithDescription:@"Subscription has been set up"];

    __auto_type * delegate = [[MTRDeviceTestDelegate alloc] init];
    delegate.onSubscriptionEstablished = ^() {
        [subscriptionExpectation fulfill];
    };

    [device setDelegate:delegate queue:queue];

    [self waitForExpectations:@[ subscriptionExpectation ] timeout:60];
}

- (void)test900_SubscribeAllAttributes
{
#if MANUAL_INDIVIDUAL_TEST
    [self initStack];
    [self waitForCommissionee];
#endif
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();
    XCTestExpectation * cleanSubscriptionExpectation = [self expectationWithDescription:@"Previous subscriptions cleaned"];
    NSLog(@"Deregistering report handlers...");
    [device deregisterReportHandlersWithQueue:queue
                                   completion:^{
                                       NSLog(@"Report handlers deregistered");
                                       [cleanSubscriptionExpectation fulfill];
                                   }];
    [self waitForExpectations:@[ cleanSubscriptionExpectation ] timeout:kTimeoutInSeconds];

    XCTestExpectation * expectation = [self expectationWithDescription:@"subscribe OnOff attribute"];
    __block void (^reportHandler)(id _Nullable values, NSError * _Nullable error) = nil;

    [device subscribeToAttributesWithEndpointID:@1
        clusterID:@6
        attributeID:@0xffffffff
        minInterval:@2
        maxInterval:@10
        params:nil
        queue:queue
        reportHandler:^(id _Nullable values, NSError * _Nullable error) {
            NSLog(@"Subscribe all - report attribute values: %@, error: %@, report handler: %d", values, error,
                (reportHandler != nil));

            if (reportHandler) {
                __auto_type callback = reportHandler;
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
    __auto_type reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);

        if ([path.attribute unsignedIntegerValue] == 0 && [result[@"data"][@"value"] boolValue] == YES) {
            [reportExpectation fulfill];
            reportHandler = nil;
        }
    };

    // Send commands to set attribute state to a known state
    XCTestExpectation * commandExpectation = [self expectationWithDescription:@"command responded"];
    NSDictionary * fields = @{ @"type" : @"Structure", @"value" : @[] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
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
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Send commands to trigger attribute change
    commandExpectation = [self expectationWithDescription:@"command responded"];
    fields = @{ @"type" : @"Structure", @"value" : @[] };
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
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];

    // Set up expectation for 2nd report
    reportExpectation = [self expectationWithDescription:@"receive OnOff attribute report"];
    reportHandler = ^(id _Nullable values, NSError * _Nullable error) {
        XCTAssertEqual([MTRErrorTestUtils errorToZCLErrorCode:error], 0);
        XCTAssertTrue([values isKindOfClass:[NSArray class]]);
        NSDictionary * result = values[0];
        MTRAttributePath * path = result[@"attributePath"];
        XCTAssertEqual([path.endpoint unsignedIntegerValue], 1);
        XCTAssertEqual([path.cluster unsignedIntegerValue], 6);
        XCTAssertTrue([result[@"data"] isKindOfClass:[NSDictionary class]]);
        if ([path.attribute unsignedIntegerValue] == 0 && [result[@"data"][@"value"] boolValue] == NO) {
            [reportExpectation fulfill];
            reportHandler = nil;
        }
    };

    // Send command to trigger attribute change
    commandExpectation = [self expectationWithDescription:@"command responded"];
    fields = @{ @"type" : @"Structure", @"value" : @[] };
    [device invokeCommandWithEndpointID:@1
                              clusterID:@6
                              commandID:@0
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
                                         XCTAssertEqual([path.command unsignedIntegerValue], 0);
                                         XCTAssertNil(result[@"error"]);
                                     }
                                     XCTAssertEqual([resultArray count], 1);
                                 }
                                 [commandExpectation fulfill];
                             }];
    [self waitForExpectations:[NSArray arrayWithObject:commandExpectation] timeout:kTimeoutInSeconds];

    // Wait for report
    [self waitForExpectations:[NSArray arrayWithObject:reportExpectation] timeout:kTimeoutInSeconds];
}

#if !MANUAL_INDIVIDUAL_TEST
- (void)test999_TearDown
{
    // Put the device back in the state we found it: open commissioning window, no fabrics commissioned.
    MTRBaseDevice * device = GetConnectedDevice();
    dispatch_queue_t queue = dispatch_get_main_queue();

    // Get our current fabric index, for later deletion.
    XCTestExpectation * readFabricIndexExpectation = [self expectationWithDescription:@"Fabric index read"];

    __block NSNumber * fabricIndex;
    __auto_type * opCredsCluster = [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device endpointID:@(0) queue:queue];
    [opCredsCluster
        readAttributeCurrentFabricIndexWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable readError) {
            XCTAssertNil(readError);
            XCTAssertNotNil(value);
            fabricIndex = value;
            [readFabricIndexExpectation fulfill];
        }];

    [self waitForExpectations:@[ readFabricIndexExpectation ] timeout:kTimeoutInSeconds];

    // Open a commissioning window.
    XCTestExpectation * openCommissioningWindowExpectation = [self expectationWithDescription:@"Commissioning window opened"];

    __auto_type * adminCommissioningCluster = [[MTRBaseClusterAdministratorCommissioning alloc] initWithDevice:device
                                                                                                    endpointID:@(0)
                                                                                                         queue:queue];
    __auto_type * openWindowParams = [[MTRAdministratorCommissioningClusterOpenBasicCommissioningWindowParams alloc] init];
    openWindowParams.commissioningTimeout = @(900);
    openWindowParams.timedInvokeTimeoutMs = @(50000);
    [adminCommissioningCluster openBasicCommissioningWindowWithParams:openWindowParams
                                                    completionHandler:^(NSError * _Nullable error) {
                                                        XCTAssertNil(error);
                                                        [openCommissioningWindowExpectation fulfill];
                                                    }];

    [self waitForExpectations:@[ openCommissioningWindowExpectation ] timeout:kTimeoutInSeconds];

    // Remove our fabric from the device.
    XCTestExpectation * removeFabricExpectation = [self expectationWithDescription:@"Fabric removed"];

    __auto_type * removeParams = [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
    removeParams.fabricIndex = fabricIndex;

    [opCredsCluster removeFabricWithParams:removeParams
                         completionHandler:^(
                             MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable removeError) {
                             XCTAssertNil(removeError);
                             XCTAssertNotNil(data);
                             XCTAssertEqualObjects(data.statusCode, @(0));
                             [removeFabricExpectation fulfill];
                         }];

    [self waitForExpectations:@[ removeFabricExpectation ] timeout:kTimeoutInSeconds];

    [self shutdownStack];
}
#endif

@end

@interface MTRBaseDevice (Test)
// Test function for whitebox testing
+ (id)CHIPEncodeAndDecodeNSObject:(id)object;
@end

@interface MTRDeviceEncoderTests : XCTestCase
@end

@implementation MTRDeviceEncoderTests

- (void)testSignedInteger
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:-713], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testSignedInteger64Bits
{
    NSDictionary * input = [NSDictionary
        dictionaryWithObjectsAndKeys:@"SignedInteger", @"type", [NSNumber numberWithInteger:-0x7000111122223333ll], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testUnsignedInteger
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type", [NSNumber numberWithInteger:1025], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testUnsignedInteger64Bits
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"UnsignedInteger", @"type",
                                         [NSNumber numberWithUnsignedLongLong:0xCCCCDDDDEEEEFFFFull], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testBoolean
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Boolean", @"type", [NSNumber numberWithBool:YES], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testUTF8String
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"UTF8String", @"type", @"Hello World", @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testOctetString
{
    const uint8_t data[] = { 0x00, 0xF2, 0x63 };
    NSDictionary * input = [NSDictionary
        dictionaryWithObjectsAndKeys:@"OctetString", @"type", [NSData dataWithBytes:data length:sizeof(data)], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testFloat
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Float", @"type", [NSNumber numberWithFloat:0.1245f], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output[@"type"] isEqualToString:@"Float"]);
    XCTAssertTrue(([output[@"value"] floatValue] - [input[@"value"] floatValue]) < 0.0001);
}

- (void)testDouble
{
    NSDictionary * input =
        [NSDictionary dictionaryWithObjectsAndKeys:@"Double", @"type", [NSNumber numberWithDouble:0.1245], @"value", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output[@"type"] isEqualToString:@"Double"]);
    XCTAssertTrue(([output[@"value"] doubleValue] - [input[@"value"] doubleValue]) < 0.0001);
}

- (void)testNull
{
    NSDictionary * input = [NSDictionary dictionaryWithObjectsAndKeys:@"Null", @"type", nil];
    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:input];
    NSLog(@"Conversion input: %@\nOutput: %@", input, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:input]);
}

- (void)testStructure
{
    NSArray * inputFields = @[
        @{
            @"contextTag" : @1,
            @"data" : @ { @"type" : @"Boolean", @"value" : @NO },
        },
        @{
            @"contextTag" : @2,
            @"data" : @ { @"type" : @"SignedInteger", @"value" : @5 },
        }
    ];
    NSDictionary * inputValue = @{ @"type" : @"Structure", @"value" : inputFields };

    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);

    XCTAssertTrue([output isEqual:inputValue]);
}

- (void)testArray
{
    NSArray * inputFields = @[
        @{ @"data" : @ { @"type" : @"Boolean", @"value" : @NO } }, @{ @"data" : @ { @"type" : @"SignedInteger", @"value" : @5 } }
    ];
    NSDictionary * inputValue = @{ @"type" : @"Array", @"value" : inputFields };

    id output = [MTRBaseDevice CHIPEncodeAndDecodeNSObject:inputValue];
    NSLog(@"Conversion input: %@\nOutput: %@", inputValue, output);
    XCTAssertNotNil(output);
    XCTAssertTrue([output isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([output isEqual:inputValue]);
}

@end
